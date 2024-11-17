/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "aboutform.h"
#include "ui_aboutsettings.h"

#include "src/net/updatecheck.h"
#include "src/version.h"
#include "src/widget/form/settings/textcompose.h"
#include "src/widget/style.h"
#include "src/widget/tool/recursivesignalblocker.h"
#include "src/widget/translator.h"

#include <tox/tox.h>

#include <QDebug>
#include <QDesktopServices>
#include <QPushButton>
#include <QTimer>
#include <utility>

// index of UI in the QStackedWidget
enum class updateIndex
{
    available = 0,
    upToDate = 1,
    failed = 2
};

/**
 * @class AboutForm
 *
 * This form contains information about qTox and libraries versions, external
 * links and licence text. Shows progress during an update.
 */

/**
 * @brief Constructor of AboutForm.
 */
AboutForm::AboutForm(UpdateCheck& updateCheck_, QString contactInfo_, Style& style_)
    : GenericForm(QPixmap(":/img/settings/general.png"), style_)
    , bodyUI(new Ui::AboutSettings)
    , progressTimer(new QTimer(this))
    , updateCheck(updateCheck_)
    , contactInfo{std::move(contactInfo_)}
    , style{style_}
{
    bodyUI->setupUi(this);

    bodyUI->updateStack->setVisible(updateCheck.canCheck());
    bodyUI->unstableVersion->setVisible(false);
    connect(&updateCheck, &UpdateCheck::versionIsUnstable, this, &AboutForm::onUnstableVersion);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

    replaceVersions();

    if (VersionInfo::gitVersion().indexOf(" ") > -1)
        bodyUI->gitVersion->setOpenExternalLinks(false);

    eventsInit();
    Translator::registerHandler([this] { retranslateUi(); }, this);
}

/**
 * @brief Update versions and links.
 *
 * Update commit hash if built with git, show author and known issues info
 * It also updates qTox, toxcore and Qt versions.
 */
void AboutForm::replaceVersions()
{
    const QString TOXCORE_VERSION =
        QStringLiteral("%1.%2.%3").arg(tox_version_major()).arg(tox_version_minor()).arg(tox_version_patch());

    bodyUI->youAreUsing->setText(tr("You are using qTox version %1.").arg(VersionInfo::gitDescribe()));

    connect(&updateCheck, &UpdateCheck::updateAvailable, this, &AboutForm::onUpdateAvailable);
    connect(&updateCheck, &UpdateCheck::upToDate, this, &AboutForm::onUpToDate);
    connect(&updateCheck, &UpdateCheck::updateCheckFailed, this, &AboutForm::onUpdateCheckFailed);

    if (!updateCheck.canCheck()) {
        qDebug() << "AboutForm not showing updates, qTox built without UPDATE_CHECK";
    }

    const QString commitLink = "https://github.com/TokTok/qTox/commit/" + VersionInfo::gitVersion();
    bodyUI->gitVersion->setText(
        tr("Commit hash: %1").arg(createLink(commitLink, VersionInfo::gitVersion())));

    bodyUI->toxCoreVersion->setText(tr("toxcore version: %1").arg(TOXCORE_VERSION));
    bodyUI->qtVersion->setText(tr("Qt version: %1").arg(QT_VERSION_STR));

    bodyUI->knownIssues->setText(
        tr("A list of all known issues may be found at our %1 at Github."
           " If you discover a bug or security vulnerability within"
           " qTox, please report it according to the guidelines in our"
           " %2 wiki article.",

           "`%1` is replaced by translation of `bug tracker`"
           "\n`%2` is replaced by translation of `Writing Useful Bug Reports`")
            .arg(createLink("https://github.com/TokTok/qTox/issues",
                            tr("bug-tracker", "Replaces `%1` in the `A list of all known…`")))
            .arg(createLink("https://github.com/TokTok/qTox/wiki/Writing-Useful-Bug-Reports",
                            tr("Writing Useful Bug Reports",
                               "Replaces `%2` in the `A list of all known…`"))));

    using TextCompose::urlEncode;
    bodyUI->clickToReport->setText(createLink( //
        QStringLiteral(                        //
            "https://github.com/TokTok/qTox/issues/new"
            "?labels=bug"
            "&template=bug.yaml"
            "&contact=%1"
            "&title=%2"
            "&qtox_version=%3"
            "&commit_hash=%4"
            "&toxcore_version=%5"
            "&qt_version=%6"
            "&os_detail=%7")
            .arg(urlEncode("tox:" + contactInfo), urlEncode("Bug: "),
                 urlEncode(VersionInfo::gitDescribe()), urlEncode(VersionInfo::gitVersion()),
                 urlEncode(TOXCORE_VERSION), urlEncode(QT_VERSION_STR),
                 urlEncode(QSysInfo::prettyProductName())),
        QStringLiteral("<b>%1</b>").arg(tr("Click here to report a bug."))));

    const QString authorInfo =
        QStringLiteral("<p>%1</p><p>%2</p><p>%3</p>")
            .arg(tr("Original author: %1").arg(createLink("https://github.com/tux3", "tux3")),
                 tr("See a full list of %1 at Github",
                    "`%1` is replaced with translation of word `contributors`")
                     .arg(createLink("https://github.com/TokTok/qTox/graphs/contributors",
                                     tr("contributors", "Replaces `%1` in `See a full list of…`"))),
                 tr("This version of qTox is being maintained by the TokTok team "
                    "following the archiving of the original qTox project."));

    bodyUI->authorInfo->setText(authorInfo);
}

void AboutForm::onUpdateAvailable(QString latestVersion, QUrl link)
{
    std::ignore = latestVersion;
    QObject::disconnect(linkConnection);
    linkConnection = connect(bodyUI->updateAvailableButton, &QPushButton::clicked, this,
                             [link]() { QDesktopServices::openUrl(link); });
    bodyUI->updateStack->setCurrentIndex(static_cast<int>(updateIndex::available));
}

void AboutForm::onUpToDate()
{
    bodyUI->updateStack->setCurrentIndex(static_cast<int>(updateIndex::upToDate));
}

void AboutForm::onUpdateCheckFailed()
{
    bodyUI->updateStack->setCurrentIndex(static_cast<int>(updateIndex::failed));
}

void AboutForm::reloadTheme()
{
    replaceVersions();
}

void AboutForm::onUnstableVersion()
{
    bodyUI->updateStack->hide();
    bodyUI->unstableVersion->setVisible(true);
}

/**
 * @brief Creates hyperlink with specific style.
 * @param path The URL of the page the link goes to.
 * @param text Text, which will be clickable.
 * @return Hyperlink to paste.
 */
inline QString AboutForm::createLink(QString path, QString text) const
{
    return TextCompose::createLink(style, path, text);
}

AboutForm::~AboutForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

/**
 * @brief Retranslate all elements in the form.
 */
void AboutForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
    replaceVersions();
}
