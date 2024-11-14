/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "userinterfaceform.h"
#include "ui_userinterfacesettings.h"

#include <QDebug>
#include <QFileDialog>
#include <QFont>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QScreen>
#include <QStyleFactory>
#include <QTime>
#include <QVector>

#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/style.h"
#include "src/widget/tool/recursivesignalblocker.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"

/**
 * @class UserInterfaceForm
 *
 * This form contains all settings which change the UI appearance or behaviour.
 * It also contains the smiley configuration.
 */

/**
 * @brief Constructor of UserInterfaceForm.
 * @param myParent Setting widget which will contain this form as tab.
 *
 * Restores all controls from the settings.
 */
UserInterfaceForm::UserInterfaceForm(SmileyPack& smileyPack_, Settings& settings_,
    Style& style_, SettingsWidget* myParent)
    : GenericForm(QPixmap(":/img/settings/general.png"), style_)
    , smileyPack{smileyPack_}
    , settings{settings_}
    , style{style_}
{
    parent = myParent;

    bodyUI = new Ui::UserInterfaceSettings;
    bodyUI->setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

    const QFont chatBaseFont = settings.getChatMessageFont();
    bodyUI->txtChatFontSize->setValue(QFontInfo(chatBaseFont).pixelSize());
    bodyUI->txtChatFont->setCurrentFont(chatBaseFont);
    int index = static_cast<int>(settings.getStylePreference());
    bodyUI->textStyleComboBox->setCurrentIndex(index);
    bodyUI->useNameColors->setChecked(settings.getEnableGroupChatsColor());

    bodyUI->notify->setChecked(settings.getNotify());
    // Note: UI is boolean inversed from settings to maintain setting file backwards compatibility
    bodyUI->groupOnlyNotfiyWhenMentioned->setChecked(!settings.getGroupAlwaysNotify());
    bodyUI->groupOnlyNotfiyWhenMentioned->setEnabled(settings.getNotify());
    bodyUI->notifySound->setChecked(settings.getNotifySound());
    bodyUI->notifyHide->setChecked(settings.getNotifyHide());
    bodyUI->notifySound->setEnabled(settings.getNotify());
    bodyUI->busySound->setChecked(settings.getBusySound());
    bodyUI->busySound->setEnabled(settings.getNotifySound() && settings.getNotify());
#if DESKTOP_NOTIFICATIONS
    bodyUI->desktopNotify->setChecked(settings.getDesktopNotify());
    bodyUI->desktopNotify->setEnabled(settings.getNotify());
#else
    bodyUI->desktopNotify->hide();
#endif

    bodyUI->showWindow->setChecked(settings.getShowWindow());

    bodyUI->cbGroupchatPosition->setChecked(settings.getGroupchatPosition());
    bodyUI->cbCompactLayout->setChecked(settings.getCompactLayout());
    bodyUI->cbSeparateWindow->setChecked(settings.getSeparateWindow());
    bodyUI->cbDontGroupWindows->setChecked(settings.getDontGroupWindows());
    bodyUI->cbDontGroupWindows->setEnabled(settings.getSeparateWindow());
    bodyUI->cbShowIdenticons->setChecked(settings.getShowIdenticons());

    bodyUI->useEmoticons->setChecked(settings.getUseEmoticons());
    for (auto entry : SmileyPack::listSmileyPacks())
        bodyUI->smileyPackBrowser->addItem(entry.first, entry.second);

    smileLabels = {bodyUI->smile1, bodyUI->smile2, bodyUI->smile3, bodyUI->smile4, bodyUI->smile5};

    int currentPack = bodyUI->smileyPackBrowser->findData(settings.getSmileyPack());
    bodyUI->smileyPackBrowser->setCurrentIndex(currentPack);
    reloadSmileys();
    bodyUI->smileyPackBrowser->setEnabled(bodyUI->useEmoticons->isChecked());

    bodyUI->styleBrowser->addItem(tr("None"));
    bodyUI->styleBrowser->addItems(QStyleFactory::keys());

    QString textStyle;
    if (QStyleFactory::keys().contains(settings.getStyle()))
        textStyle = settings.getStyle();
    else
        textStyle = tr("None");

    bodyUI->styleBrowser->setCurrentText(textStyle);

    for (QString color : Style::getThemeColorNames())
        bodyUI->themeColorCBox->addItem(color);

    bodyUI->themeColorCBox->setCurrentIndex(settings.getThemeColor());
    bodyUI->emoticonSize->setValue(settings.getEmojiFontPointSize());

    QLocale ql;
    QStringList timeFormats;
    timeFormats << ql.timeFormat(QLocale::ShortFormat) << ql.timeFormat(QLocale::LongFormat)
                << "hh:mm AP"
                << "hh:mm:ss AP"
                << "hh:mm:ss";
    timeFormats.removeDuplicates();
    bodyUI->timestamp->addItems(timeFormats);

    QRegularExpression re(QString("^[^\\n]{0,%0}$").arg(MAX_FORMAT_LENGTH));
    QRegularExpressionValidator* validator = new QRegularExpressionValidator(re, this);
    QString timeFormat = settings.getTimestampFormat();

    if (!re.match(timeFormat).hasMatch())
        timeFormat = timeFormats[0];

    bodyUI->timestamp->setCurrentText(timeFormat);
    bodyUI->timestamp->setValidator(validator);
    on_timestamp_editTextChanged(timeFormat);

    QStringList dateFormats;
    dateFormats << QStringLiteral("yyyy-MM-dd") // ISO 8601
                                                // format strings from system locale
                << ql.dateFormat(QLocale::LongFormat) << ql.dateFormat(QLocale::ShortFormat)
                << ql.dateFormat(QLocale::NarrowFormat) << "dd-MM-yyyy"
                << "d-MM-yyyy"
                << "dddd dd-MM-yyyy"
                << "dddd d-MM";

    dateFormats.removeDuplicates();
    bodyUI->dateFormats->addItems(dateFormats);

    QString dateFormat = settings.getDateFormat();
    if (!re.match(dateFormat).hasMatch())
        dateFormat = dateFormats[0];

    bodyUI->dateFormats->setCurrentText(dateFormat);
    bodyUI->dateFormats->setValidator(validator);
    on_dateFormats_editTextChanged(dateFormat);

    eventsInit();
    Translator::registerHandler(std::bind(&UserInterfaceForm::retranslateUi, this), this);
}

UserInterfaceForm::~UserInterfaceForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

void UserInterfaceForm::on_styleBrowser_currentIndexChanged(QString textStyle)
{
    if (bodyUI->styleBrowser->currentIndex() == 0)
        settings.setStyle("None");
    else
        settings.setStyle(textStyle);

    setStyle(QStyleFactory::create(textStyle));
    parent->setBodyHeadStyle(textStyle);
}

void UserInterfaceForm::on_emoticonSize_editingFinished()
{
    settings.setEmojiFontPointSize(bodyUI->emoticonSize->value());
}

void UserInterfaceForm::on_timestamp_editTextChanged(const QString& format)
{
    QString timeExample = QTime::currentTime().toString(format);
    bodyUI->timeExample->setText(timeExample);

    settings.setTimestampFormat(format);
    QString locale = settings.getTranslation();
    Translator::translate(locale);
}

void UserInterfaceForm::on_dateFormats_editTextChanged(const QString& format)
{
    QString dateExample = QDate::currentDate().toString(format);
    bodyUI->dateExample->setText(dateExample);

    settings.setDateFormat(format);
    QString locale = settings.getTranslation();
    Translator::translate(locale);
}

void UserInterfaceForm::on_useEmoticons_stateChanged()
{
    settings.setUseEmoticons(bodyUI->useEmoticons->isChecked());
    bodyUI->smileyPackBrowser->setEnabled(bodyUI->useEmoticons->isChecked());
}

void UserInterfaceForm::on_textStyleComboBox_currentTextChanged()
{
    Settings::StyleType styleType =
        static_cast<Settings::StyleType>(bodyUI->textStyleComboBox->currentIndex());
    settings.setStylePreference(styleType);
}

void UserInterfaceForm::on_smileyPackBrowser_currentIndexChanged(int index)
{
    QString filename = bodyUI->smileyPackBrowser->itemData(index).toString();
    settings.setSmileyPack(filename);
    reloadSmileys();
}

/**
 * @brief Reload smileys and size information.
 */
void UserInterfaceForm::reloadSmileys()
{
    QList<QStringList> emoticons = smileyPack.getEmoticons();

    // sometimes there are no emoticons available, don't crash in this case
    if (emoticons.isEmpty()) {
        qDebug() << "reloadSmilies: No emoticons found";
        return;
    }

    QStringList smileys;
    for (int i = 0; i < emoticons.size(); ++i)
        smileys.push_front(emoticons.at(i).first());

    emoticonsIcons.clear();
    const QSize size(18, 18);
    for (int i = 0; i < smileLabels.size(); ++i) {
        std::shared_ptr<QIcon> icon = smileyPack.getAsIcon(smileys[i]);
        emoticonsIcons.append(icon);
        smileLabels[i]->setPixmap(icon->pixmap(size));
        smileLabels[i]->setToolTip(smileys[i]);
    }

    // set maximum size of emoji
    auto geometry = QGuiApplication::primaryScreen()->geometry();
    // 8 is the count of row and column in emoji's in widget
    const int sideSize = 8;
    int maxSide = qMin(
        geometry.height() / sideSize, geometry.width() / sideSize);
    QSize maxSize(maxSide, maxSide);

    QSize actualSize = emoticonsIcons.first()->actualSize(maxSize);
    bodyUI->emoticonSize->setMaximum(actualSize.width());
}

void UserInterfaceForm::on_notify_stateChanged()
{
    const bool notify = bodyUI->notify->isChecked();
    settings.setNotify(notify);
    bodyUI->groupOnlyNotfiyWhenMentioned->setEnabled(notify);
    bodyUI->notifySound->setEnabled(notify);
    bodyUI->busySound->setEnabled(notify && bodyUI->notifySound->isChecked());
    bodyUI->desktopNotify->setEnabled(notify);
}

void UserInterfaceForm::on_notifySound_stateChanged()
{
    const bool notify = bodyUI->notifySound->isChecked();
    settings.setNotifySound(notify);
    bodyUI->busySound->setEnabled(notify);
}

void UserInterfaceForm::on_desktopNotify_stateChanged()
{
    const bool notify = bodyUI->desktopNotify->isChecked();
    settings.setDesktopNotify(notify);
}

void UserInterfaceForm::on_busySound_stateChanged()
{
    settings.setBusySound(bodyUI->busySound->isChecked());
}

void UserInterfaceForm::on_showWindow_stateChanged()
{
    settings.setShowWindow(bodyUI->showWindow->isChecked());
}

void UserInterfaceForm::on_groupOnlyNotfiyWhenMentioned_stateChanged()
{
    // Note: UI is boolean inversed from settings to maintain setting file backwards compatibility
    settings.setGroupAlwaysNotify(!bodyUI->groupOnlyNotfiyWhenMentioned->isChecked());
}

void UserInterfaceForm::on_cbCompactLayout_stateChanged()
{
    settings.setCompactLayout(bodyUI->cbCompactLayout->isChecked());
}

void UserInterfaceForm::on_cbSeparateWindow_stateChanged()
{
    bool checked = bodyUI->cbSeparateWindow->isChecked();
    bodyUI->cbDontGroupWindows->setEnabled(checked);
    settings.setSeparateWindow(checked);
}

void UserInterfaceForm::on_cbDontGroupWindows_stateChanged()
{
    settings.setDontGroupWindows(bodyUI->cbDontGroupWindows->isChecked());
}

void UserInterfaceForm::on_cbGroupchatPosition_stateChanged()
{
    settings.setGroupchatPosition(bodyUI->cbGroupchatPosition->isChecked());
}

void UserInterfaceForm::on_cbShowIdenticons_stateChanged()
{
    settings.setShowIdenticons(bodyUI->cbShowIdenticons->isChecked());
}

void UserInterfaceForm::on_themeColorCBox_currentIndexChanged(int index)
{
    settings.setThemeColor(index);
    style.setThemeColor(settings, index);
    style.applyTheme();
}

/**
 * @brief Retranslate all elements in the form.
 */
void UserInterfaceForm::retranslateUi()
{
    // Block signals during translation to prevent settings change
    RecursiveSignalBlocker signalBlocker{this};

    bodyUI->retranslateUi(this);

    // Restore text style index once translation is complete
    bodyUI->textStyleComboBox->setCurrentIndex(
        static_cast<int>(settings.getStylePreference()));

    QStringList colorThemes(Style::getThemeColorNames());
    for (int i = 0; i < colorThemes.size(); ++i) {
        bodyUI->themeColorCBox->setItemText(i, colorThemes[i]);
    }

    bodyUI->styleBrowser->setItemText(0, tr("None"));
}

void UserInterfaceForm::on_txtChatFont_currentFontChanged(const QFont& f)
{
    QFont tmpFont = f;
    const int px = bodyUI->txtChatFontSize->value();

    if (QFontInfo(tmpFont).pixelSize() != px)
        tmpFont.setPixelSize(px);

    settings.setChatMessageFont(tmpFont);
}

void UserInterfaceForm::on_txtChatFontSize_valueChanged(int px)
{
    Settings& s = settings;
    QFont tmpFont = s.getChatMessageFont();
    const int fontSize = QFontInfo(tmpFont).pixelSize();

    if (px != fontSize) {
        tmpFont.setPixelSize(px);
        s.setChatMessageFont(tmpFont);
    }
}

void UserInterfaceForm::on_useNameColors_stateChanged(int value)
{
    settings.setEnableGroupChatsColor(value);
}

void UserInterfaceForm::on_notifyHide_stateChanged(int value)
{
    settings.setNotifyHide(value);
}
