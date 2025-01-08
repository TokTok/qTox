/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */


#include "nexus.h"
#include "audio/audio.h"
#include "persistence/settings.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/ipc.h"
#include "src/model/conferenceinvite.h"
#include "src/model/status.h"
#include "src/persistence/profile.h"
#include "src/widget/style.h"
#include "src/widget/widget.h"
#include "video/camerasource.h"
#include "widget/loginscreen.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QScreen>
#include <QStandardPaths>
#include <QThread>

#include <vpx/vpx_image.h>

#include <cassert>

#ifdef Q_OS_MAC
#include <QActionGroup>
#include <QMenuBar>
#include <QSignalMapper>
#include <QWindow>
#endif

/**
 * @class Nexus
 *
 * This class is in charge of connecting various systems together
 * and forwarding signals appropriately to the right objects,
 * it is in charge of starting the GUI and the Core.
 */

Q_DECLARE_OPAQUE_POINTER(ToxAV*)

Nexus::Nexus(Settings& settings_, IMessageBoxManager& messageBoxManager_,
             CameraSource& cameraSource_, IPC& ipc_, ImageLoader& imageLoader_, QObject* parent)
    : QObject(parent)
    , profile{nullptr}
    , settings{settings_}
    , widget{nullptr}
    , cameraSource{cameraSource_}
    , style{new Style()}
    , messageBoxManager{messageBoxManager_}
    , ipc{ipc_}
    , imageLoader{imageLoader_}
{
    QObject::connect(this, &Nexus::saveGlobal, &settings, &Settings::saveGlobal);
}

Nexus::~Nexus()
{
    widget = nullptr;
    if (profile != nullptr) {
        profile->save();
        profile = nullptr;
    }
    emit saveGlobal();
#ifdef Q_OS_MAC
    delete globalMenuBar;
#endif
}

/**
 * @brief Sets up invariants and calls showLogin
 *
 * Hides the login screen and shows the GUI for the given profile.
 * Will delete the current GUI, if it exists.
 */
void Nexus::start()
{
    qDebug() << "Starting up";

    // Setup the environment
    qRegisterMetaType<Status::Status>("Status::Status");
    qRegisterMetaType<vpx_image>("vpx_image");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<const int16_t*>("const int16_t*");
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<QPixmap>("QPixmap");
    qRegisterMetaType<Profile*>("Profile*");
    qRegisterMetaType<ToxAV*>("ToxAV*");
    qRegisterMetaType<ToxFile>("ToxFile");
    qRegisterMetaType<ToxFile::FileDirection>("ToxFile::FileDirection");
    qRegisterMetaType<std::shared_ptr<VideoFrame>>("std::shared_ptr<VideoFrame>");
    qRegisterMetaType<ToxPk>("ToxPk");
    qRegisterMetaType<ToxId>("ToxId");
    qRegisterMetaType<ToxPk>("ConferenceId");
    qRegisterMetaType<ToxPk>("ChatId");
    qRegisterMetaType<ConferenceInvite>("ConferenceInvite");
    qRegisterMetaType<ReceiptNum>("ReceiptNum");
    qRegisterMetaType<RowId>("RowId");
    qRegisterMetaType<uint64_t>("uint64_t");

    qApp->setQuitOnLastWindowClosed(false);

#ifdef Q_OS_MAC
    // TODO: still needed?
    globalMenuBar = new QMenuBar();
    dockMenu = new QMenu(globalMenuBar);

    viewMenu = globalMenuBar->addMenu(QString());

    windowMenu = globalMenuBar->addMenu(QString());
    globalMenuBar->addAction(windowMenu->menuAction());

    fullScreenAction = viewMenu->addAction(QString());
    fullScreenAction->setShortcut(QKeySequence::FullScreen);
    connect(fullScreenAction, &QAction::triggered, this, &Nexus::toggleFullScreen);

    minimizeAction = windowMenu->addAction(QString());
    minimizeAction->setShortcut(Qt::CTRL | Qt::Key_M);
    connect(minimizeAction, &QAction::triggered, this, [this]() {
        minimizeAction->setEnabled(false);
        QApplication::focusWindow()->showMinimized();
    });

    windowMenu->addSeparator();
    frontAction = windowMenu->addAction(QString());
    connect(frontAction, &QAction::triggered, this, &Nexus::bringAllToFront);

    QAction* quitAction = new QAction(globalMenuBar);
    quitAction->setMenuRole(QAction::QuitRole);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    retranslateUi();
#endif
    showMainGUI();
}

/**
 * @brief Hides the main GUI, delete the profile, and shows the login screen
 */
int Nexus::showLogin(const QString& profileName)
{
    // Delete the current GUI.
    widget = nullptr;

    // Delete at the end of the function, but get ready to set a new one.
    // We need to keep it alive for a while longer because it may be used on shutdown.
    auto oldProfile = std::move(profile);
    // Finish saving the current profile if needed.
    if (oldProfile != nullptr) {
        oldProfile->save();
    }

    LoginScreen loginScreen{settings.getPaths(), *style, settings.getThemeColor(), profileName};
    connectLoginScreen(loginScreen);

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    // TODO(kriby): Move core out of profile
    // This is awkward because the core is in the profile
    // The connection order ensures profile will be ready for bootstrap for now
    connect(this, &Nexus::currentProfileChanged, this, &Nexus::bootstrapWithProfile);
    int returnval = loginScreen.exec();
    if (returnval == QDialog::Rejected) {
        // Kriby: This will terminate the main application loop, necessary until we refactor
        // away the split startup/return to login behavior.
        qApp->quit();
    }
    disconnect(this, &Nexus::currentProfileChanged, this, &Nexus::bootstrapWithProfile);

    return returnval;
}

void Nexus::bootstrapWithProfile(Profile* p)
{
    // kriby: This is a hack until a proper controller is written

    // We take ownership of the profile here.
    profile = std::unique_ptr<Profile>(p);

    if (profile) {
        audioControl = std::unique_ptr<IAudioControl>(Audio::makeAudio(settings));
        assert(audioControl != nullptr);
        profile->getCore().getAv()->setAudio(*audioControl);
        start();
    }
}

void Nexus::connectLoginScreen(const LoginScreen& loginScreen)
{
    // TODO(kriby): Move connect sequences to a controller class object instead

    // Nexus -> LoginScreen
    QObject::connect(this, &Nexus::profileLoaded, &loginScreen, &LoginScreen::onProfileLoaded);
    QObject::connect(this, &Nexus::profileLoadFailed, &loginScreen, &LoginScreen::onProfileLoadFailed);
    // LoginScreen -> Nexus
    QObject::connect(&loginScreen, &LoginScreen::createNewProfile, this, &Nexus::onCreateNewProfile);
    QObject::connect(&loginScreen, &LoginScreen::loadProfile, this, &Nexus::onLoadProfile);
    // LoginScreen -> Settings
    QObject::connect(&loginScreen, &LoginScreen::autoLoginChanged, &settings, &Settings::setAutoLogin);
    QObject::connect(&loginScreen, &LoginScreen::autoLoginChanged, &settings, &Settings::saveGlobal);
    // Settings -> LoginScreen
    QObject::connect(&settings, &Settings::autoLoginChanged, &loginScreen,
                     &LoginScreen::onAutoLoginChanged);
}

void Nexus::showMainGUI()
{
    // TODO(kriby): Rewrite as view-model connect sequence only, add to a controller class object
    assert(profile);

    // Create GUI
    widget = std::make_unique<Widget>(*profile, *audioControl, cameraSource, settings, *style, ipc,
                                      imageLoader, *this);

    // Start GUI
    widget->init();

    // Zetok protection
    // There are small instants on startup during which no
    // profile is loaded but the GUI could still receive events,
    // e.g. between two modal windows. Disable the GUI to prevent that.
    widget->setEnabled(false);

    // Connections
    connect(profile.get(), &Profile::selfAvatarChanged, widget.get(), &Widget::onSelfAvatarLoaded);

    connect(profile.get(), &Profile::coreChanged, widget.get(), &Widget::onCoreChanged);

    connect(profile.get(), &Profile::failedToStart, widget.get(), &Widget::onFailedToStartCore,
            Qt::BlockingQueuedConnection);

    connect(profile.get(), &Profile::badProxy, widget.get(), &Widget::onBadProxyCore,
            Qt::BlockingQueuedConnection);

    profile->startCore();

    widget->setEnabled(true);
}

/**
 * @brief Get current user profile.
 * @return nullptr if not started, profile otherwise.
 */
Profile* Nexus::getProfile()
{
    return profile.get();
}

/**
 * @brief Creates a new profile and replaces the current one.
 * @param name New username
 * @param pass New password
 */
void Nexus::onCreateNewProfile(const QString& name, const QString& pass)
{
    setProfile(Profile::createProfile(name, pass, settings, parser, cameraSource, messageBoxManager));
    parser = nullptr; // only apply cmdline proxy settings once
}

/**
 * Loads an existing profile and replaces the current one.
 */
void Nexus::onLoadProfile(const QString& name, const QString& pass)
{
    setProfile(Profile::loadProfile(name, pass, settings, parser, cameraSource, messageBoxManager));
    parser = nullptr; // only apply cmdline proxy settings once
}
/**
 * Changes the loaded profile and notifies listeners.
 * @param p
 */
void Nexus::setProfile(Profile* p)
{
    if (!p) {
        emit profileLoadFailed();
        // Warnings are issued during respective createNew/load calls
        return;
    }
    emit profileLoaded();


    emit currentProfileChanged(p);
}

void Nexus::setParser(QCommandLineParser* parser_)
{
    parser = parser_;
}

void Nexus::registerIpcHandlers()
{
    widget->registerIpcHandlers();
}

bool Nexus::handleToxSave(const QString& path)
{
    assert(widget);
    return widget->handleToxSave(path);
}

#ifdef Q_OS_MAC
void Nexus::retranslateUi()
{
    viewMenu->menuAction()->setText(tr("View", "macOS Menu bar"));
    windowMenu->menuAction()->setText(tr("Window", "macOS Menu bar"));
    minimizeAction->setText(tr("Minimize", "macOS Menu bar"));
    frontAction->setText((tr("Bring All to Front", "macOS Menu bar")));
}

void Nexus::onWindowStateChanged(Qt::WindowStates state)
{
    minimizeAction->setEnabled(QApplication::activeWindow() != nullptr);

    if (QApplication::activeWindow() != nullptr && sender() == QApplication::activeWindow()) {
        if (state & Qt::WindowFullScreen)
            minimizeAction->setEnabled(false);

        if (state & Qt::WindowFullScreen)
            fullScreenAction->setText(tr("Exit Full Screen"));
        else
            fullScreenAction->setText(tr("Enter Full Screen"));

        updateWindows();
    }

    updateWindowsStates();
}

void Nexus::updateWindows()
{
    updateWindowsArg(nullptr);
}

void Nexus::updateWindowsArg(QWindow* closedWindow)
{
    QWindowList windowList = QApplication::topLevelWindows();
    delete windowActions;
    windowActions = new QActionGroup(this);

    windowMenu->addSeparator();

    QAction* dockLast;
    if (!dockMenu->actions().isEmpty())
        dockLast = dockMenu->actions().first();
    else
        dockLast = nullptr;

    QWindow* activeWindow;

    if (QApplication::activeWindow())
        activeWindow = QApplication::activeWindow()->windowHandle();
    else
        activeWindow = nullptr;

    for (int i = 0; i < windowList.size(); ++i) {
        if (closedWindow == windowList[i])
            continue;

        QAction* action = windowActions->addAction(windowList[i]->title());
        action->setCheckable(true);
        action->setChecked(windowList[i] == activeWindow);
        connect(action, &QAction::triggered, this,
                [this, window = windowList[i]] { onOpenWindow(window); });
        windowMenu->addAction(action);
        dockMenu->insertAction(dockLast, action);
    }

    if (dockLast && !dockLast->isSeparator())
        dockMenu->insertSeparator(dockLast);
}

void Nexus::updateWindowsClosed()
{
    updateWindowsArg(static_cast<QWidget*>(sender())->windowHandle());
}

void Nexus::updateWindowsStates()
{
    bool exists = false;
    QWindowList windowList = QApplication::topLevelWindows();

    for (QWindow* window : windowList) {
        if (!(window->windowState() & Qt::WindowMinimized)) {
            exists = true;
            break;
        }
    }

    frontAction->setEnabled(exists);
}

void Nexus::onOpenWindow(QObject* object)
{
    QWindow* window = static_cast<QWindow*>(object);

    if (window->windowState() & QWindow::Minimized)
        window->showNormal();

    window->raise();
    window->requestActivate();
}

void Nexus::toggleFullScreen()
{
    QWidget* window = QApplication::activeWindow();

    if (window->isFullScreen())
        window->showNormal();
    else
        window->showFullScreen();
}

void Nexus::bringAllToFront()
{
    QWindowList windowList = QApplication::topLevelWindows();
    QWindow* focused = QApplication::focusWindow();

    for (QWindow* window : windowList)
        window->raise();

    focused->raise();
}
#endif

void Nexus::screenshot()
{
    const QString filename = QString::fromUtf8(qgetenv("QTOX_SCREENSHOT"));
    if (filename.isEmpty()) {
        // No-op unless QTOX_SCREENSHOT is set.
        return;
    }

    if (widget == nullptr) {
        qWarning() << "No widget to take screenshot of";
        return;
    }

    // Paint the widget onto a pixmap. This makes sure the screenshot still
    // works even if we don't have screen capture permissions.
    QPixmap pixmap(widget->size());
    widget->render(&pixmap);

    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (path.isEmpty()) {
        path = QDir::currentPath();
    }
    path += "/" + filename;
    if (pixmap.save(path)) {
        qDebug() << "Saved screenshot to" << path;
    } else {
        qWarning() << "Failed to save screenshot to" << path;
    }
}
