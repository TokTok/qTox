/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QHBoxLayout>
#include <QPushButton>
#include <QStyleFactory>

#include <array>
#include <memory>

class Camera;
class Core;
class GenericForm;
class GeneralForm;
class IAudioControl;
class PrivacyForm;
class AVForm;
class QLabel;
class QTabWidget;
class ContentLayout;
class UpdateCheck;
class Widget;
class SmileyPack;
class CameraSource;
class Settings;
class Style;
class IMessageBoxManager;
class Profile;

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    SettingsWidget(UpdateCheck& updateCheck, IAudioControl& audio, Core* core, SmileyPack& smileyPack,
                   CameraSource& cameraSource, Settings& settings, Style& style,
                   IMessageBoxManager& messageBoxManager, Profile& profile, Widget* parent = nullptr);
    ~SettingsWidget() override;

    bool isShown() const;
    void show(ContentLayout* contentLayout);
    void setBodyHeadStyle(QString style);

    void showAbout();

public slots:
    void onUpdateAvailable();

private slots:
    void onTabChanged(int index);

private:
    void retranslateUi();

private:
    std::unique_ptr<QVBoxLayout> bodyLayout;
    std::unique_ptr<QTabWidget> settingsWidgets;
    std::array<std::unique_ptr<GenericForm>, 6> cfgForms;
    int currentIndex;
};
