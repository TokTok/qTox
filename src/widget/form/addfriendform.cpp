/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#include "addfriendform.h"

#include "src/core/core.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/widget/contentlayout.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/tool/imessageboxmanager.h"
#include "src/widget/translator.h"

#include <QApplication>
#include <QClipboard>
#include <QErrorMessage>
#include <QFileDialog>
#include <QFont>
#include <QMessageBox>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSignalMapper>
#include <QTabWidget>
#include <QWindow>

namespace {
QString getToxId(const QString& id)
{
    const QString toxUriPrefix{"tox:"};
    QString strippedId = id.trimmed();
    if (strippedId.startsWith(toxUriPrefix)) {
        strippedId.remove(0, toxUriPrefix.length());
    }
    return strippedId;
}

bool checkIsValidId(const QString& id)
{
    return ToxId::isToxId(id);
}

bool checkIsValidPublicKey(const QString& id)
{
    // benevolently allow length of Public Key up to length of Tox ID for convenience
    // as long as there is enough to match ToxPk::ToxPkRegEx
    return id.length() <= ToxId::numHexChars
           && id.left(ToxPk::numHexChars).contains(ToxPk::ToxPkRegEx);
}
} // namespace

/**
 * @var QString AddFriendForm::lastUsername
 * @brief Cached username so we can retranslate the invite message
 */

AddFriendForm::AddFriendForm(ToxId ownId_, Settings& settings_, Style& style_,
                             IMessageBoxManager& messageBoxManager_, Core& core_)
    : ownId{std::move(ownId_)}
    , settings{settings_}
    , style{style_}
    , messageBoxManager{messageBoxManager_}
    , core{core_}
{
    tabWidget = new QTabWidget();
    main = new QWidget(tabWidget);
    head = new QWidget();
    QFont bold;
    bold.setBold(true);
    headLabel.setFont(bold);
    toxIdLabel.setTextFormat(Qt::RichText);

    main->setLayout(&layout);
    layout.addWidget(&toxIdLabel);
    layout.addWidget(&toxId);
    layout.addWidget(&messageLabel);
    layout.addWidget(&message);
    layout.addWidget(&sendButton);
    layout.addWidget(&addWithoutRequestButton);
    tabWidget->addTab(main, QString());

    importContacts = new QWidget(tabWidget);
    importContacts->setLayout(&importContactsLayout);
    importFileLine.addWidget(&importFileLabel);
    importFileLine.addStretch();
    importFileLine.addWidget(&importFileButton);
    importContactsLayout.addLayout(&importFileLine);
    importContactsLayout.addWidget(&importMessageLabel);
    importContactsLayout.addWidget(&importMessage);
    importContactsLayout.addWidget(&importSendButton);
    tabWidget->addTab(importContacts, QString());

    auto* scrollArea = new QScrollArea(tabWidget);
    auto* requestWidget = new QWidget(tabWidget);
    scrollArea->setWidget(requestWidget);
    scrollArea->setWidgetResizable(true);
    requestsLayout = new QVBoxLayout(requestWidget);
    requestsLayout->addStretch(1);
    tabWidget->addTab(scrollArea, QString());

    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);

    connect(&toxId, &QLineEdit::returnPressed, this, &AddFriendForm::onSendTriggered);
    connect(&toxId, &QLineEdit::textChanged, this, &AddFriendForm::onIdChanged);
    connect(tabWidget, &QTabWidget::currentChanged, this, &AddFriendForm::onCurrentChanged);
    connect(&sendButton, &QPushButton::clicked, this, &AddFriendForm::onSendTriggered);
    connect(&addWithoutRequestButton, &QPushButton::clicked, this,
            &AddFriendForm::onAddWithoutRequestTriggered);
    connect(&importSendButton, &QPushButton::clicked, this, &AddFriendForm::onImportSendClicked);
    connect(&importFileButton, &QPushButton::clicked, this, &AddFriendForm::onImportOpenClicked);
    connect(&core, &Core::usernameSet, this, &AddFriendForm::onUsernameSet);

    // accessibility stuff
    toxIdLabel.setAccessibleDescription(tr("Tox ID, 76 hexadecimal characters"));
    toxId.setAccessibleDescription(tr("Type in Tox ID of your friend"));
    messageLabel.setAccessibleDescription(tr("Friend request message"));
    message.setAccessibleDescription(tr(
        "Type message to send with the friend request or leave empty to send a default message"));
    message.setTabChangesFocus(true);

    retranslateUi();
    Translator::registerHandler([this] { retranslateUi(); }, this);

    const int size = settings.getFriendRequestSize();
    for (int i = 0; i < size; ++i) {
        const Settings::Request request = settings.getFriendRequest(i);
        addFriendRequestWidget(request.address, request.message);
    }
}

AddFriendForm::~AddFriendForm()
{
    Translator::unregister(this);
    head->deleteLater();
    tabWidget->deleteLater();
}

bool AddFriendForm::isShown() const
{
    if (head->isVisible()) {
        head->window()->windowHandle()->alert(0);
        return true;
    }

    return false;
}

void AddFriendForm::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(tabWidget);
    contentLayout->mainHead->layout()->addWidget(head);
    tabWidget->show();
    head->show();
    setIdFromClipboard();
    toxId.setFocus();

    // Fix #3421
    // Needed to update tab after opening window
    const int index = tabWidget->currentIndex();
    onCurrentChanged(index);
}

QString AddFriendForm::getMessage() const
{
    const QString msg = message.toPlainText();
    return !msg.isEmpty() ? msg : message.placeholderText();
}

QString AddFriendForm::getImportMessage() const
{
    const QString msg = importMessage.toPlainText();
    return msg.isEmpty() ? importMessage.placeholderText() : msg;
}

void AddFriendForm::setMode(Mode mode)
{
    tabWidget->setCurrentIndex(mode);
}

bool AddFriendForm::addFriendRequest(const QString& friendAddress, const QString& message_)
{
    if (settings.addFriendRequest(friendAddress, message_)) {
        addFriendRequestWidget(friendAddress, message_);
        if (isShown()) {
            onCurrentChanged(tabWidget->currentIndex());
        }

        return true;
    }
    return false;
}

void AddFriendForm::onUsernameSet(const QString& username)
{
    lastUsername = username;
    retranslateUi();
}

void AddFriendForm::addFriend(const QString& idText)
{
    const ToxId friendId(idText);

    if (!friendId.isValid()) {
        messageBoxManager.showWarning(tr("Couldn't add friend"),
                                      tr("%1 Tox ID is invalid", "Tox address error").arg(idText));
        return;
    }

    deleteFriendRequest(friendId);
    if (friendId == ownId) {
        messageBoxManager.showWarning(tr("Couldn't add friend"),
                                      //: When trying to add your own Tox ID as friend
                                      tr("You can't add yourself as a friend!"));
    } else {
        emit friendRequested(friendId, getMessage());
    }
}

void AddFriendForm::addFriendWithoutRequest(const QString& idText)
{
    if (!checkIsValidPublicKey(idText)) {
        messageBoxManager.showWarning(tr("Couldn't add friend"),
                                      tr("%1 Tox ID is invalid", "Tox address error").arg(idText));
        return;
    }

    const ToxPk friendPk(idText.left(ToxPk::numHexChars));

    // take care of possibly pending requests that may be for the same friend
    const bool requestCancelled = removeFriendRequest(friendPk);
    if (friendPk == ownId.getPublicKey()) {
        messageBoxManager.showWarning(tr("Couldn't add friend"),
                                      //: When trying to add your own Tox ID as friend
                                      tr("You can't add yourself as a friend!"));
    } else {
        emit friendAddedWithoutRequest(friendPk);
    }

    if (requestCancelled) {
        emit friendRequestsUpdate();
    }
}

void AddFriendForm::onSendTriggered()
{
    const QString id = getToxId(toxId.text());
    addFriend(id);

    toxId.clear();
    message.clear();
}

void AddFriendForm::onAddWithoutRequestTriggered()
{
    // accepting both Tox ID as well as Tox Public Key
    const QString pk = getToxId(toxId.text());
    addFriendWithoutRequest(pk);

    toxId.clear();
    // do not clear the message since we did not used use it
}

void AddFriendForm::onImportSendClicked()
{
    for (const QString& id : contactsToImport) {
        addFriend(id);
    }

    contactsToImport.clear();
    importMessage.clear();
    retranslateUi(); // Update the importFileLabel
}

void AddFriendForm::onImportOpenClicked()
{
    const QString path = QFileDialog::getOpenFileName(Q_NULLPTR, tr("Open contact list"));
    if (path.isEmpty()) {
        return;
    }

    QFile contactFile(path);
    if (!contactFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        messageBoxManager.showWarning(tr("Couldn't open file"),
                                      //: Error message when trying to open a contact list file to import
                                      tr("Couldn't open the contact file"));
        return;
    }

    contactsToImport = QString::fromUtf8(contactFile.readAll()).split('\n');
    qDebug() << "Import list:";
    for (auto it = contactsToImport.begin(); it != contactsToImport.end();) {
        const QString id = getToxId(*it);
        if (checkIsValidId(id)) {
            *it = id;
            qDebug() << *it;
            ++it;
        } else {
            if (!id.isEmpty()) {
                qDebug() << "Invalid ID:" << *it;
            }
            it = contactsToImport.erase(it);
        }
    }

    if (contactsToImport.isEmpty()) {
        messageBoxManager.showWarning(tr("Invalid file"),
                                      tr("We couldn't find any contacts to import in this file!"));
    }

    retranslateUi(); // Update the importFileLabel to show how many contacts we have
}

void AddFriendForm::onIdChanged(const QString& id)
{
    const QString strippedId = getToxId(id);

    const bool isValidId = checkIsValidId(strippedId);
    const bool isValidPublicKey = checkIsValidPublicKey(strippedId);
    const bool isValidOrEmpty = strippedId.isEmpty() || isValidId || isValidPublicKey;
    const bool isFullyValid = isValidId && isValidPublicKey;

    //: Tox ID of the person you're sending a friend request to
    const QString toxIdText(tr("Tox ID"));
    //: Tox ID format description
    const QString toxIdComment(tr("76 hexadecimal characters"));

    //: Tox Public Key of the person you're sending a friend request to
    const QString publicKeyText(tr("Tox Public Key"));
    //: Tox Public Key format description
    const QString publicKeyComment(tr("64 hexadecimal characters"));

    // take care of highlights based on different validity
    const QString idComment = isValidId
        ? QStringLiteral("(%1)").arg(toxIdComment)
        : QStringLiteral("<font color='red'>(%1)</font>").arg(toxIdComment);
    const QString publicKeyCommentColored = isValidPublicKey
        ? QStringLiteral("(%1)").arg(publicKeyComment)
        : QStringLiteral("<font color='red'>(%1)</font>").arg(publicKeyComment);

    const QString labelText =
        QStringLiteral("%1 %2 or %3 %4").arg(toxIdText, idComment, publicKeyText, publicKeyCommentColored);
    toxIdLabel.setText(labelText);
    toxId.setStyleSheet(isValidOrEmpty ? QStringLiteral("")
                                       : style.getStylesheet("addFriendForm/toxId.qss", settings));
    const QString toolTip =
        (strippedId.isEmpty() ? QStringLiteral("")
                              : isFullyValid ? QStringLiteral("")
                                             : isValidPublicKey ? tr("Valid Tox Public Key format only")
                                                                : tr("Invalid Tox ID or Tox Public Key format")); 
    toxId.setToolTip(toolTip);

    sendButton.setEnabled(isValidId);
    addWithoutRequestButton.setEnabled(isValidPublicKey);
}

void AddFriendForm::setIdFromClipboard()
{
    const QClipboard* clipboard = QApplication::clipboard();
    const QString trimmedId = clipboard->text().trimmed();
    const QString strippedId = getToxId(trimmedId);
    const bool isSelf = ToxId::isToxId(strippedId) && ToxId(strippedId) != ownId;
    if (!strippedId.isEmpty() && ToxId::isToxId(strippedId) && isSelf) {
        toxId.setText(trimmedId);
    }
}

void AddFriendForm::deleteFriendRequest(const ToxId& toxId_)
{
    const int size = settings.getFriendRequestSize();
    for (int i = 0; i < size; ++i) {
        const Settings::Request request = settings.getFriendRequest(i);
        if (toxId_.getPublicKey() == ToxPk(request.address)) {
            settings.removeFriendRequest(i);
            return;
        }
    }
}

bool AddFriendForm::removeFriendRequest(const ToxPk& friendPk)
{
    const QString address = friendPk.toString();

    // clean up visible request widget if there is one that match
    const int widgetCount = requestsLayout->count();
    for (int i = 0; i < widgetCount; ++i) {
        auto* friendWidget = qobject_cast<QWidget*>(requestsLayout->itemAt(i)->widget());
        if (friendWidget && friendWidget->property("toxId").toString() == address) {
            removeFriendRequestWidget(friendWidget);
            break;
        }
    }

    // clean up persistent request record if there is one that match
    const int size = settings.getFriendRequestSize();
    for (int i = 0; i < size; ++i) {
        if (ToxPk(settings.getFriendRequest(i).address) == friendPk) {
            settings.removeFriendRequest(i);
            settings.savePersonal();
            return true;
        }
    }

    return false;
}

void AddFriendForm::onFriendRequestAccepted()
{
    auto* acceptButton = static_cast<QPushButton*>(sender());
    QWidget* friendWidget = acceptButton->parentWidget();
    const int index = requestsLayout->indexOf(friendWidget);
    removeFriendRequestWidget(friendWidget);
    const int indexFromEnd = requestsLayout->count() - index - 1;
    const Settings::Request request = settings.getFriendRequest(indexFromEnd);
    emit friendRequestAccepted(ToxPk{request.address});
    settings.removeFriendRequest(indexFromEnd);
    settings.savePersonal();
}

void AddFriendForm::onFriendRequestRejected()
{
    auto* rejectButton = static_cast<QPushButton*>(sender());
    QWidget* friendWidget = rejectButton->parentWidget();
    const int index = requestsLayout->indexOf(friendWidget);
    removeFriendRequestWidget(friendWidget);
    const int indexFromEnd = requestsLayout->count() - index - 1;
    settings.removeFriendRequest(indexFromEnd);
    settings.savePersonal();
}

void AddFriendForm::onCurrentChanged(int index)
{
    if (index == FriendRequest && settings.getUnreadFriendRequests() != 0) {
        settings.clearUnreadFriendRequests();
        settings.savePersonal();
        emit friendRequestsSeen();
    }
}

void AddFriendForm::retranslateUi()
{
    headLabel.setText(tr("Add Friends"));
    //: The message you send in friend requests
    static const QString messageLabelText = tr("Message");
    messageLabel.setText(messageLabelText);
    importMessageLabel.setText(messageLabelText);
    //: Button to choose a file with a list of contacts to import
    importFileButton.setText(tr("Open"));
    importSendButton.setText(tr("Send friend requests"));
    sendButton.setText(tr("Send friend request"));
    addWithoutRequestButton.setText(tr("Add friend without request"));
    //: Default message in friend requests if the field is left blank. Write something appropriate!
    message.setPlaceholderText(tr("%1 here! Tox me maybe?").arg(lastUsername));
    importMessage.setPlaceholderText(message.placeholderText());

    importFileLabel.setText(
        contactsToImport.isEmpty()
            ? tr("Import a list of contacts, one Tox ID per line")
            //: Shows the number of contacts we're about to import from a file (at least one)
            : tr("Ready to import %n contact(s), click send to confirm", "", contactsToImport.size()));

    onIdChanged(toxId.text());

    tabWidget->setTabText(AddFriend, tr("Add friend"));
    tabWidget->setTabText(ImportContacts, tr("Import contacts"));
    tabWidget->setTabText(FriendRequest, tr("Friend requests"));

    for (QPushButton* acceptButton : acceptButtons) {
        retranslateAcceptButton(acceptButton);
    }

    for (QPushButton* rejectButton : rejectButtons) {
        retranslateRejectButton(rejectButton);
    }
}

void AddFriendForm::addFriendRequestWidget(const QString& friendAddress_, const QString& message_)
{
    auto* friendWidget = new QWidget(tabWidget);
    friendWidget->setProperty("toxId", friendAddress_);
    auto* friendLayout = new QHBoxLayout(friendWidget);
    auto* horLayout = new QVBoxLayout();
    horLayout->setContentsMargins(0, 0, 0, 0);
    friendLayout->addLayout(horLayout);

    auto* friendLabel = new CroppingLabel(friendWidget);
    friendLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    friendLabel->setText("<b>" + friendAddress_ + "</b>");
    horLayout->addWidget(friendLabel);

    auto* messageLabel_ = new QLabel(message_);
    // allow to select text, but treat links as plaintext to prevent phishing
    messageLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    messageLabel_->setTextFormat(Qt::PlainText);
    messageLabel_->setWordWrap(true);
    horLayout->addWidget(messageLabel_, 1);

    auto* acceptButton = new QPushButton(friendWidget);
    acceptButtons.append(acceptButton);
    connect(acceptButton, &QPushButton::released, this, &AddFriendForm::onFriendRequestAccepted);
    friendLayout->addWidget(acceptButton);
    retranslateAcceptButton(acceptButton);

    auto* rejectButton = new QPushButton(friendWidget);
    rejectButtons.append(rejectButton);
    connect(rejectButton, &QPushButton::released, this, &AddFriendForm::onFriendRequestRejected);
    friendLayout->addWidget(rejectButton);
    retranslateRejectButton(rejectButton);

    requestsLayout->insertWidget(0, friendWidget);
}

void AddFriendForm::removeFriendRequestWidget(QWidget* friendWidget)
{
    const int index = requestsLayout->indexOf(friendWidget);
    requestsLayout->removeWidget(friendWidget);
    acceptButtons.removeAt(index);
    rejectButtons.removeAt(index);
    friendWidget->deleteLater();
}

void AddFriendForm::retranslateAcceptButton(QPushButton* acceptButton)
{
    acceptButton->setText(tr("Accept"));
}

void AddFriendForm::retranslateRejectButton(QPushButton* rejectButton)
{
    rejectButton->setText(tr("Reject"));
}
