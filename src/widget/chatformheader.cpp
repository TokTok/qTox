/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "chatformheader.h"

#include "src/model/status.h"

#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/style.h"
#include "src/widget/tool/callconfirmwidget.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/translator.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QTextDocument>
#include <QToolButton>
#include <memory>

namespace {
const QSize AVATAR_SIZE{40, 40};
const short HEAD_LAYOUT_SPACING = 5;
const short MIC_BUTTONS_LAYOUT_SPACING = 4;
const short BUTTONS_LAYOUT_HOR_SPACING = 4;

const QString STYLE_PATH = QStringLiteral("chatForm/buttons.qss");

const QString STATE_NAME[] = {
    QString{},                //
    QStringLiteral("green"),  //
    QStringLiteral("red"),    //
    QStringLiteral("yellow"), //
    QStringLiteral("yellow"), //
};

const QString CALL_TOOL_TIP[] = {
    ChatFormHeader::tr("Can't start audio call"), //
    ChatFormHeader::tr("Start audio call"),       //
    ChatFormHeader::tr("End audio call"),         //
    ChatFormHeader::tr("Cancel audio call"),      //
    ChatFormHeader::tr("Accept audio call"),      //
};

const QString VIDEO_TOOL_TIP[] = {
    ChatFormHeader::tr("Can't start video call"), //
    ChatFormHeader::tr("Start video call"),       //
    ChatFormHeader::tr("End video call"),         //
    ChatFormHeader::tr("Cancel video call"),      //
    ChatFormHeader::tr("Accept video call"),      //
};

const QString VOL_TOOL_TIP[] = {
    ChatFormHeader::tr("Sound can be disabled only during a call"), //
    ChatFormHeader::tr("Mute call"),                                //
    ChatFormHeader::tr("Unmute call"),                              //
};

const QString MIC_TOOL_TIP[] = {
    ChatFormHeader::tr("Microphone can be muted only during a call"), //
    ChatFormHeader::tr("Mute microphone"),                            //
    ChatFormHeader::tr("Unmute microphone"),                          //
};

template <class T, class Fun>
QPushButton* createButton(const QString& name, T* self, Fun onClickSlot, Settings& settings, Style& style)
{
    QPushButton* btn = new QPushButton();
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setObjectName(name);
    btn->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
    QObject::connect(btn, &QPushButton::clicked, self, onClickSlot);
    return btn;
}

template <class State>
void setStateToolTip(QAbstractButton* btn, State state, const QString toolTip[])
{
    const int index = static_cast<int>(state);
    btn->setToolTip(toolTip[index]);
}

template <class State>
void setStateName(QAbstractButton* btn, State state)
{
    const int index = static_cast<int>(state);
    btn->setProperty("state", STATE_NAME[index]);
    btn->setEnabled(index != 0);
}

} // namespace

ChatFormHeader::ChatFormHeader(Settings& settings_, Style& style_, QWidget* parent)
    : QWidget(parent)
    , mode{Mode::AV}
    , callState{CallButtonState::Disabled}
    , videoState{CallButtonState::Disabled}
    , volState{ToolButtonState::Disabled}
    , micState{ToolButtonState::Disabled}
    , settings{settings_}
    , style{style_}
{
    QHBoxLayout* headLayout = new QHBoxLayout();
    avatar = new MaskablePixmapWidget(this, AVATAR_SIZE, ":/img/avatar_mask.svg");
    avatar->setObjectName("avatar");

    nameLine = new QHBoxLayout();
    nameLine->setSpacing(3);

    nameLabel = new CroppingLabel();
    nameLabel->setObjectName("nameLabel");
    nameLabel->setMinimumHeight(Style::getFont(Style::Font::Medium).pixelSize());
    nameLabel->setEditable(true);
    nameLabel->setTextFormat(Qt::PlainText);
    connect(nameLabel, &CroppingLabel::editFinished, this, &ChatFormHeader::nameChanged);

    nameLine->addWidget(nameLabel);

    headTextLayout = new QVBoxLayout();
    headTextLayout->addStretch();
    headTextLayout->addLayout(nameLine);
    headTextLayout->addStretch();

    micButton = createButton("micButton", this, &ChatFormHeader::micMuteToggle, settings, style);
    volButton = createButton("volButton", this, &ChatFormHeader::volMuteToggle, settings, style);
    callButton = createButton("callButton", this, &ChatFormHeader::callTriggered, settings, style);
    videoButton =
        createButton("videoButton", this, &ChatFormHeader::videoCallTriggered, settings, style);

    QVBoxLayout* micButtonsLayout = new QVBoxLayout();
    micButtonsLayout->setSpacing(MIC_BUTTONS_LAYOUT_SPACING);
    micButtonsLayout->addWidget(micButton, Qt::AlignTop | Qt::AlignRight);
    micButtonsLayout->addWidget(volButton, Qt::AlignTop | Qt::AlignRight);

    QGridLayout* buttonsLayout = new QGridLayout();
    buttonsLayout->addLayout(micButtonsLayout, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignRight);
    buttonsLayout->addWidget(callButton, 0, 1, 2, 1, Qt::AlignTop);
    buttonsLayout->addWidget(videoButton, 0, 2, 2, 1, Qt::AlignTop);
    buttonsLayout->setVerticalSpacing(0);
    buttonsLayout->setHorizontalSpacing(BUTTONS_LAYOUT_HOR_SPACING);

    headLayout->addWidget(avatar);
    headLayout->addSpacing(HEAD_LAYOUT_SPACING);
    headLayout->addLayout(headTextLayout);
    headLayout->addLayout(buttonsLayout);

    setLayout(headLayout);

    updateButtonsView();
    Translator::registerHandler([this] { retranslateUi(); }, this);

    connect(&style, &Style::themeReload, this, &ChatFormHeader::reloadTheme);
}

ChatFormHeader::~ChatFormHeader() = default;

void ChatFormHeader::setName(const QString& newName)
{
    nameLabel->setText(newName);
    // for overlength names
    nameLabel->setToolTip(Qt::convertFromPlainText(newName, Qt::WhiteSpaceNormal));
}

void ChatFormHeader::setMode(ChatFormHeader::Mode mode_)
{
    mode = mode_;
    if (mode == Mode::None) {
        callButton->hide();
        videoButton->hide();
        volButton->hide();
        micButton->hide();
    }
}

void ChatFormHeader::retranslateUi()
{
    setStateToolTip(callButton, callState, CALL_TOOL_TIP);
    setStateToolTip(videoButton, videoState, VIDEO_TOOL_TIP);
    setStateToolTip(micButton, micState, MIC_TOOL_TIP);
    setStateToolTip(volButton, volState, VOL_TOOL_TIP);
}

void ChatFormHeader::updateButtonsView()
{
    setStateName(callButton, callState);
    setStateName(videoButton, videoState);
    setStateName(micButton, micState);
    setStateName(volButton, volState);
    retranslateUi();
    Style::repolish(this);
}

void ChatFormHeader::showOutgoingCall(bool video)
{
    CallButtonState& state = video ? videoState : callState;
    state = CallButtonState::Outgoing;
    updateButtonsView();
}

void ChatFormHeader::createCallConfirm(bool video)
{
    QWidget* btn = video ? videoButton : callButton;
    callConfirm = std::make_unique<CallConfirmWidget>(settings, style, btn);
    connect(callConfirm.get(), &CallConfirmWidget::accepted, this, &ChatFormHeader::callAccepted);
    connect(callConfirm.get(), &CallConfirmWidget::rejected, this, &ChatFormHeader::callRejected);
}

void ChatFormHeader::showCallConfirm()
{
    if (callConfirm && !callConfirm->isVisible())
        callConfirm->show();
}

void ChatFormHeader::removeCallConfirm()
{
    callConfirm.reset(nullptr);
}

void ChatFormHeader::updateCallButtons(bool online, bool audio, bool video)
{
    const bool audioAvailable = online && ((mode & Mode::Audio) != 0);
    const bool videoAvailable = online && ((mode & Mode::Video) != 0);
    if (!audioAvailable) {
        callState = CallButtonState::Disabled;
    } else if (video) {
        callState = CallButtonState::Disabled;
    } else if (audio) {
        callState = CallButtonState::InCall;
    } else {
        callState = CallButtonState::Available;
    }

    if (!videoAvailable) {
        videoState = CallButtonState::Disabled;
    } else if (video) {
        videoState = CallButtonState::InCall;
    } else if (audio) {
        videoState = CallButtonState::Disabled;
    } else {
        videoState = CallButtonState::Available;
    }

    updateButtonsView();
}

void ChatFormHeader::updateMuteMicButton(bool active, bool inputMuted)
{
    micButton->setEnabled(active);
    if (active) {
        micState = inputMuted ? ToolButtonState::On : ToolButtonState::Off;
    } else {
        micState = ToolButtonState::Disabled;
    }

    updateButtonsView();
}

void ChatFormHeader::updateMuteVolButton(bool active, bool outputMuted)
{
    volButton->setEnabled(active);
    if (active) {
        volState = outputMuted ? ToolButtonState::On : ToolButtonState::Off;
    } else {
        volState = ToolButtonState::Disabled;
    }

    updateButtonsView();
}

void ChatFormHeader::setAvatar(const QPixmap& img)
{
    avatar->setPixmap(img);
}

QSize ChatFormHeader::getAvatarSize() const
{
    return QSize{avatar->width(), avatar->height()};
}

void ChatFormHeader::reloadTheme()
{
    setStyleSheet(style.getStylesheet("chatArea/chatHead.qss", settings));
    callButton->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
    videoButton->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
    volButton->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
    micButton->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
}

void ChatFormHeader::addWidget(QWidget* widget, int stretch, Qt::Alignment alignment)
{
    headTextLayout->addWidget(widget, stretch, alignment);
}

void ChatFormHeader::addLayout(QLayout* layout)
{
    headTextLayout->addLayout(layout);
}

void ChatFormHeader::addStretch()
{
    headTextLayout->addStretch();
}
