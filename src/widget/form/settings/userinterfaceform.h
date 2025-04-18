/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "genericsettings.h"

#include "src/widget/form/settingswidget.h"

#include <memory>

namespace Ui {
class UserInterfaceSettings;
}
class Settings;
class SmileyPack;
class Style;

class UserInterfaceForm : public GenericForm
{
    Q_OBJECT
public:
    UserInterfaceForm(SmileyPack& smileyPack, Settings& settings, Style& style,
                      SettingsWidget* myParent);
    ~UserInterfaceForm() override;
    QString getFormName() final
    {
        return tr("User Interface");
    }

private slots:
    void on_smileyPackBrowser_currentIndexChanged(int index);
    void on_emoticonSize_editingFinished();
    void on_styleBrowser_currentTextChanged(QString textStyle);
    void on_timestamp_editTextChanged(const QString& format);
    void on_dateFormats_editTextChanged(const QString& format);
    void on_textStyleComboBox_currentTextChanged();
    void on_useEmoticons_stateChanged();
    void on_notify_stateChanged();
    void on_desktopNotify_stateChanged();
    void on_notifySystemBackend_stateChanged();
    void on_notifySound_stateChanged();
    void on_notifyHide_stateChanged(int value);
    void on_busySound_stateChanged();
    void on_showWindow_stateChanged();
    void on_conferenceOnlyNotifyWhenMentioned_stateChanged();
    void on_cbCompactLayout_stateChanged();
    void on_cbSeparateWindow_stateChanged();
    void on_cbDontGroupWindows_stateChanged();
    void on_cbConferencePosition_stateChanged();
    void on_themeColorCBox_currentIndexChanged(int index);
    void on_cbShowIdenticons_stateChanged();
    void on_txtChatFont_currentFontChanged(const QFont& f);
    void on_txtChatFontSize_valueChanged(int px);
    void on_useNameColors_stateChanged(int value);
    void on_cbImagePreview_stateChanged();
    void on_cbHidePostNullSuffix_stateChanged();
    void on_chatLogMaxTxt_valueChanged(int value);
    void on_chatLogChunkTxt_valueChanged(int value);

private:
    void retranslateUi();
    void reloadSmileys();

private:
    QList<QLabel*> smileLabels;
    QList<std::shared_ptr<QIcon>> emoticonsIcons;
    SettingsWidget* parent;
    Ui::UserInterfaceSettings* bodyUI;
    const int MAX_FORMAT_LENGTH = 128;
    SmileyPack& smileyPack;
    Settings& settings;
    Style& style;
};
