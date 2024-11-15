/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/toxid.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSet>
#include <QTextEdit>
#include <QVBoxLayout>

class QTabWidget;

class ContentLayout;
class Settings;
class Style;
class IMessageBoxManager;
class Core;

class AddFriendForm : public QObject
{
    Q_OBJECT
public:
    enum Mode
    {
        AddFriend = 0,
        ImportContacts = 1,
        FriendRequest = 2
    };

    AddFriendForm(ToxId ownId_, Settings& settings, Style& style,
        IMessageBoxManager& messageBoxManager, Core& core);
    AddFriendForm(const AddFriendForm&) = delete;
    AddFriendForm& operator=(const AddFriendForm&) = delete;
    ~AddFriendForm();

    bool isShown() const;
    void show(ContentLayout* contentLayout);
    void setMode(Mode mode);

    bool addFriendRequest(const QString& friendAddress, const QString& message_);

signals:
    void friendRequested(const ToxId& friendAddress, const QString& message);
    void friendRequestAccepted(const ToxPk& friendAddress);
    void friendRequestsSeen();

public slots:
    void onUsernameSet(const QString& userName);

private slots:
    void onSendTriggered();
    void onIdChanged(const QString& id);
    void onImportSendClicked();
    void onImportOpenClicked();
    void onFriendRequestAccepted();
    void onFriendRequestRejected();
    void onCurrentChanged(int index);

private:
    void addFriend(const QString& idText);
    void retranslateUi();
    void addFriendRequestWidget(const QString& friendAddress_, const QString& message_);
    void removeFriendRequestWidget(QWidget* friendWidget);
    void retranslateAcceptButton(QPushButton* acceptButton);
    void retranslateRejectButton(QPushButton* rejectButton);
    void deleteFriendRequest(const ToxId& toxId_);
    void setIdFromClipboard();
    QString getMessage() const;
    QString getImportMessage() const;

private:
    QLabel headLabel;
    QLabel toxIdLabel;
    QLabel messageLabel;
    QLabel importFileLabel;
    QLabel importMessageLabel;

    QPushButton sendButton;
    QPushButton importFileButton;
    QPushButton importSendButton;
    QLineEdit toxId;
    QTextEdit message;
    QTextEdit importMessage;
    QVBoxLayout layout;
    QVBoxLayout headLayout;
    QVBoxLayout importContactsLayout;
    QHBoxLayout importFileLine;
    QWidget* head;
    QWidget* main;
    QWidget* importContacts;
    QString lastUsername;
    QTabWidget* tabWidget;
    QVBoxLayout* requestsLayout;
    QList<QPushButton*> acceptButtons;
    QList<QPushButton*> rejectButtons;
    QList<QString> contactsToImport;

    ToxId ownId;
    Settings& settings;
    Style& style;
    IMessageBoxManager& messageBoxManager;
    Core& core;
};
