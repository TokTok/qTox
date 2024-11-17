/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "generalform.h"
#include "ui_generalsettings.h"

#include <QFileDialog>
#include <QFontDatabase>
#include <cmath>

#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/widget/form/settings/textcompose.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/style.h"
#include "src/widget/tool/recursivesignalblocker.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"

namespace {
const QStringList locales = {
    "ar",      // Arabic
    "be",      // Belarusian
    "ber",     // Berber
    "bg",      // Bulgarian
    "bn",      // Bengali
    "cs",      // Czech
    "da",      // Danish
    "de",      // German
    "el",      // Greek
    "en",      // English
    "eo",      // Esperanto
    "es",      // Spanish
    "et",      // Estonian
    "fa",      // Persian
    "fi",      // Finnish
    "fr",      // French
    "gl",      // Galician
    "he",      // Hebrew
    "hr",      // Croatian
    "hu",      // Hungarian
    "is",      // Icelandic
    "it",      // Italian
    "ja",      // Japanese
    "jbo",     // Lojban
    "kn",      // Kannada
    "ko",      // Korean
    "li",      // Limburgish
    "lt",      // Lithuanian
    "lv",      // Latvian
    "mk",      // Macedonian
    "nb_NO",   // Norwegian Bokmål
    "nl",      // Dutch
    "nl_BE",   // Flemish
    "pl",      // Polish
    "pr",      // Pirate
    "pt",      // Portuguese
    "pt_BR",   // Brazilian Portuguese
    "ro",      // Romanian
    "ru",      // Russian
    "si",      // Sinhala
    "sk",      // Slovak
    "sl",      // Slovenian
    "sq",      // Albanian
    "sr",      // Serbian
    "sr_Latn", // Serbian (Latin)
    "sv",      // Swedish
    "sw",      // Swahili
    "ta",      // Tamil
    "tr",      // Turkish
    "ug",      // Uyghur
    "uk",      // Ukrainian
    "ur",      // Urdu
    "vi",      // Vietnamese
    "zh_CN",   // Simplified Chinese
    "zh_TW",   // Traditional Chinese
};

QFontDatabase::WritingSystem writingSystem(QLocale::Script script)
{
    switch (script) {
    case QLocale::Script::AnyScript:
        return QFontDatabase::WritingSystem::Any;
    case QLocale::Script::ArabicScript:
        return QFontDatabase::WritingSystem::Arabic;
    case QLocale::Script::ArmenianScript:
        return QFontDatabase::WritingSystem::Armenian;
    case QLocale::Script::BengaliScript:
        return QFontDatabase::WritingSystem::Bengali;
    case QLocale::Script::CanadianAboriginalScript:
        return QFontDatabase::WritingSystem::Devanagari;
    case QLocale::Script::CaucasianAlbanianScript:
        return QFontDatabase::WritingSystem::Armenian;
    case QLocale::Script::CyrillicScript:
        return QFontDatabase::WritingSystem::Cyrillic;
    case QLocale::Script::DevanagariScript:
        return QFontDatabase::WritingSystem::Devanagari;
    case QLocale::Script::GeorgianScript:
        return QFontDatabase::WritingSystem::Georgian;
    case QLocale::Script::GreekScript:
        return QFontDatabase::WritingSystem::Greek;
    case QLocale::Script::GujaratiScript:
        return QFontDatabase::WritingSystem::Gujarati;
    case QLocale::Script::GurmukhiScript:
        return QFontDatabase::WritingSystem::Gurmukhi;
    case QLocale::Script::HebrewScript:
        return QFontDatabase::WritingSystem::Hebrew;
    case QLocale::Script::JapaneseScript:
        return QFontDatabase::WritingSystem::Japanese;
    case QLocale::Script::KannadaScript:
        return QFontDatabase::WritingSystem::Kannada;
    case QLocale::Script::KhmerScript:
        return QFontDatabase::WritingSystem::Khmer;
    case QLocale::Script::KoreanScript:
        return QFontDatabase::WritingSystem::Korean;
    case QLocale::Script::LaoScript:
        return QFontDatabase::WritingSystem::Lao;
    case QLocale::Script::LatinScript:
        return QFontDatabase::WritingSystem::Latin;
    case QLocale::Script::MalayalamScript:
        return QFontDatabase::WritingSystem::Malayalam;
    case QLocale::Script::MyanmarScript:
        return QFontDatabase::WritingSystem::Myanmar;
    case QLocale::Script::OriyaScript:
        return QFontDatabase::WritingSystem::Oriya;
    case QLocale::Script::SinhalaScript:
        return QFontDatabase::WritingSystem::Sinhala;
    case QLocale::Script::TamilScript:
        return QFontDatabase::WritingSystem::Tamil;
    case QLocale::Script::TeluguScript:
        return QFontDatabase::WritingSystem::Telugu;
    case QLocale::Script::ThaiScript:
        return QFontDatabase::WritingSystem::Thai;
    case QLocale::Script::TibetanScript:
        return QFontDatabase::WritingSystem::Tibetan;
    case QLocale::Script::SimplifiedChineseScript:
        return QFontDatabase::WritingSystem::SimplifiedChinese;
    case QLocale::Script::TraditionalChineseScript:
        return QFontDatabase::WritingSystem::TraditionalChinese;

    default:
        qWarning() << "Unknown script" << script;
        return QFontDatabase::WritingSystem::Any;
    }
}

} // namespace

const QStringList& GeneralForm::getLocales()
{
    return locales;
}

/**
 * @class GeneralForm
 *
 * This form contains all settings that are not suited to other forms
 */
GeneralForm::GeneralForm(Settings& settings_, Style& style_)
    : GenericForm(QPixmap(":/img/settings/general.png"), style_)
    , bodyUI{new Ui::GeneralSettings}
    , settings{settings_}
    , style{style_}
{
    bodyUI->setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

#ifndef UPDATE_CHECK_ENABLED
    bodyUI->checkUpdates->setVisible(false);
#endif

#ifndef SPELL_CHECKING
    bodyUI->cbSpellChecking->setVisible(false);
#endif

    bodyUI->checkUpdates->setChecked(settings.getCheckUpdates());

    for (int i = 0; i < locales.size(); ++i) {
        QString langName;

        if (locales[i].startsWith(QLatin1String("jbo"))) {
            langName = QLatin1String("Lojban");
        } else if (locales[i].startsWith(QLatin1String("li"))) {
            langName = QLatin1String("Limburgs");
        } else if (locales[i].startsWith(QLatin1String("pr"))) {
            langName = QLatin1String("Pirate");
        } else {
            const QLocale locale{locales[i]};
            if (!QFontDatabase::families(writingSystem(locale.script())).isEmpty())
                langName = locale.nativeLanguageName();
            else
                langName = tr("%1 (no fonts)").arg(QLocale::languageToString(locale.language()));
            if (langName.isEmpty()) {
                langName = QLocale::languageToString(locale.language());
            }
        }

        bodyUI->transComboBox->insertItem(i, langName);
    }

    bodyUI->transComboBox->setCurrentIndex(locales.indexOf(settings.getTranslation()));

    bodyUI->cbAutorun->setChecked(settings.getAutorun());

    bodyUI->cbSpellChecking->setChecked(settings.getSpellCheckingEnabled());
    bodyUI->lightTrayIcon->setChecked(settings.getLightTrayIcon());
    const bool showSystemTray = settings.getShowSystemTray();

    bodyUI->showSystemTray->setChecked(showSystemTray);
    bodyUI->startInTray->setChecked(settings.getAutostartInTray());
    bodyUI->startInTray->setEnabled(showSystemTray);
    bodyUI->minimizeToTray->setChecked(settings.getMinimizeToTray());
    bodyUI->minimizeToTray->setEnabled(showSystemTray);
    bodyUI->closeToTray->setChecked(settings.getCloseToTray());
    bodyUI->closeToTray->setEnabled(showSystemTray);

    bodyUI->statusChanges->setChecked(settings.getStatusChangeNotificationEnabled());
    bodyUI->conferenceJoinLeaveMessages->setChecked(settings.getShowConferenceJoinLeaveMessages());

    bodyUI->autoAwaySpinBox->setValue(settings.getAutoAwayTime());
    bodyUI->autoSaveFilesDir->setText(settings.getGlobalAutoAcceptDir());
    bodyUI->maxAutoAcceptSizeMB->setValue(static_cast<double>(settings.getMaxAutoAcceptSize())
                                          / 1024 / 1024);
    bodyUI->autoacceptFiles->setChecked(settings.getAutoSaveEnabled());

#ifndef QTOX_PLATFORM_EXT
    bodyUI->autoAwayLabel->setEnabled(false); // these don't seem to change the appearance of the widgets,
    bodyUI->autoAwaySpinBox->setEnabled(false); // though they are unusable
#endif

    eventsInit();
    Translator::registerHandler([this] { retranslateUi(); }, this);
}

GeneralForm::~GeneralForm()
{
    Translator::unregister(this);
}

void GeneralForm::on_transComboBox_currentIndexChanged(int index)
{
    const QString& locale = locales[index];
    settings.setTranslation(locale);
    Translator::translate(locale);
}

void GeneralForm::on_cbAutorun_stateChanged()
{
    settings.setAutorun(bodyUI->cbAutorun->isChecked());
}

void GeneralForm::on_cbSpellChecking_stateChanged()
{
    settings.setSpellCheckingEnabled(bodyUI->cbSpellChecking->isChecked());
}

void GeneralForm::on_showSystemTray_stateChanged()
{
    settings.setShowSystemTray(bodyUI->showSystemTray->isChecked());
    settings.saveGlobal();
}

void GeneralForm::on_startInTray_stateChanged()
{
    settings.setAutostartInTray(bodyUI->startInTray->isChecked());
}

void GeneralForm::on_closeToTray_stateChanged()
{
    settings.setCloseToTray(bodyUI->closeToTray->isChecked());
}

void GeneralForm::on_lightTrayIcon_stateChanged()
{
    settings.setLightTrayIcon(bodyUI->lightTrayIcon->isChecked());
    emit updateIcons();
}

void GeneralForm::on_minimizeToTray_stateChanged()
{
    settings.setMinimizeToTray(bodyUI->minimizeToTray->isChecked());
}

void GeneralForm::on_statusChanges_stateChanged()
{
    settings.setStatusChangeNotificationEnabled(bodyUI->statusChanges->isChecked());
}

void GeneralForm::on_conferenceJoinLeaveMessages_stateChanged()
{
    settings.setShowConferenceJoinLeaveMessages(bodyUI->conferenceJoinLeaveMessages->isChecked());
}

void GeneralForm::on_autoAwaySpinBox_editingFinished()
{
    const int minutes = bodyUI->autoAwaySpinBox->value();
    settings.setAutoAwayTime(minutes);
}

void GeneralForm::on_autoacceptFiles_stateChanged()
{
    settings.setAutoSaveEnabled(bodyUI->autoacceptFiles->isChecked());
}

void GeneralForm::on_autoSaveFilesDir_clicked()
{
    const QString previousDir = settings.getGlobalAutoAcceptDir();
    QString directory =
        QFileDialog::getExistingDirectory(Q_NULLPTR,
                                          tr("Choose an auto accept directory", "popup title"),
                                          QDir::homePath());
    if (directory.isEmpty()) // cancel was pressed
        directory = previousDir;

    settings.setGlobalAutoAcceptDir(directory);
    bodyUI->autoSaveFilesDir->setText(directory);
}

void GeneralForm::on_maxAutoAcceptSizeMB_editingFinished()
{
    auto newMaxSizeMB = bodyUI->maxAutoAcceptSizeMB->value();
    auto newMaxSizeB = std::lround(newMaxSizeMB * 1024 * 1024);

    settings.setMaxAutoAcceptSize(newMaxSizeB);
}

void GeneralForm::on_checkUpdates_stateChanged()
{
    settings.setCheckUpdates(bodyUI->checkUpdates->isChecked());
}

/**
 * @brief Retranslate all elements in the form.
 */
void GeneralForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
    bodyUI->transWeblate->setText(
        TextCompose::createLink(style, QStringLiteral("https://hosted.weblate.org/projects/qtox/qtox/"),
                                bodyUI->transWeblate->text()));
}
