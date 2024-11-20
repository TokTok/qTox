/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QWidget>

#include <memory>

class MaskablePixmapWidget;
class QVBoxLayout;
class QHBoxLayout;
class CroppingLabel;
class QPushButton;
class QToolButton;
class CallConfirmWidget;
class QLabel;
class Settings;
class Style;

class ChatFormHeader : public QWidget
{
    Q_OBJECT
public:
    enum class CallButtonState
    {
        Disabled = 0,  // Grey
        Avaliable = 1, // Green
        InCall = 2,    // Red
        Outgoing = 3,  // Yellow
        Incoming = 4,  // Yellow
    };
    enum class ToolButtonState
    {
        Disabled = 0, // Grey
        Off = 1,      // Green
        On = 2,       // Red
    };
    enum Mode
    {
        None = 0,
        Audio = 1,
        Video = 2,
        AV = Audio | Video
    };

    ChatFormHeader(Settings& settings, Style& style, QWidget* parent = nullptr);
    ~ChatFormHeader();

    void setName(const QString& newName);
    void setMode(Mode mode_);

    void showOutgoingCall(bool video);
    void createCallConfirm(bool video);
    void showCallConfirm();
    void removeCallConfirm();

    void updateCallButtons(bool online, bool audio, bool video = false);
    void updateMuteMicButton(bool active, bool inputMuted);
    void updateMuteVolButton(bool active, bool outputMuted);

    void setAvatar(const QPixmap& img);
    QSize getAvatarSize() const;

    // TODO: Remove
    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
    void addLayout(QLayout* layout);
    void addStretch();

public slots:
    void reloadTheme();

signals:
    void callTriggered();
    void videoCallTriggered();
    void micMuteToggle();
    void volMuteToggle();

    void nameChanged(const QString& name);

    void callAccepted();
    void callRejected();

private slots:
    void retranslateUi();
    void updateButtonsView();

private:
    Mode mode;
    MaskablePixmapWidget* avatar;
    QVBoxLayout* headTextLayout;
    QHBoxLayout* nameLine;
    CroppingLabel* nameLabel;

    QPushButton* callButton;
    QPushButton* videoButton;
    QPushButton* volButton;
    QPushButton* micButton;

    CallButtonState callState;
    CallButtonState videoState;
    ToolButtonState volState;
    ToolButtonState micState;

    std::unique_ptr<CallConfirmWidget> callConfirm;
    Settings& settings;
    Style& style;
};
