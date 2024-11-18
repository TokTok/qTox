/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "appmanager.h"

#include "src/ipc.h"
#include "src/net/toxuri.h"
#include "src/net/updatecheck.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/persistence/toxsave.h"
#include "src/platform/posixsignalnotifier.h"
#include "src/version.h"
#include "src/video/camerasource.h"
#include "src/widget/tool/messageboxmanager.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFontDatabase>
#include <QMessageBox>
#include <QObject>
#include <memory>

namespace {
// logMessageHandler and associated data must be static due to qInstallMessageHandler's
// inability to register a void* to get back to a class
#ifdef LOG_TO_FILE
QAtomicPointer<FILE> logFileFile = nullptr;
QList<QByteArray>* logBuffer = new QList<QByteArray>(); // Store log messages until log file opened
QMutex* logBufferMutex = new QMutex();
#endif

constexpr std::string_view sourceRootPath()
{
    // We're not using QT_MESSAGELOG_FILE here, because that can be 0, NULL, or
    // nullptr in release builds.
    constexpr std::string_view path = __FILE__;
    constexpr size_t srcRootPos = [=]() {
        size_t pos = path.rfind("/src/");
        if (pos == std::string_view::npos) {
            pos = path.rfind("\\src\\");
        }
        return pos;
    }();
    // If this fails, we might not be getting a long enough path from __FILE__.
    // In that case, we'll need to use a different method to find the source root,
    // or only show the filename without the path.
    static_assert(srcRootPos != std::string_view::npos);
    return path.substr(0, srcRootPos);
}

// Clean up the file path to avoid leaking the user's username or build directory structure.
QString canonicalLogFilePath(const char* filename)
{
    QString file = QString::fromUtf8(filename);
    constexpr std::string_view srcPath = sourceRootPath();
    if (file.startsWith(QString::fromUtf8(srcPath.data(), srcPath.size()))) {
        file = file.mid(srcPath.length() + 1);
    }

    // The file path is outside of the project directory, but if it's in the user's $HOME, we should
    // still strip the path to prevent leaking the user's username.
    const auto home = QDir::homePath();
    if (file.startsWith(home)) {
        file = QStringLiteral("~") + file.mid(home.length());
    }

    return file;
}

// Replace the user's home with "~" to prevent leaking the user's username in log messages.
QString canonicalLogMessage(QString msg)
{
    // Replace the user's home with "~" to prevent leaking the user's username in log messages.
    return msg.replace(QDir::homePath(), QStringLiteral("~"));
}

void logMessageHandler(QtMsgType type, const QMessageLogContext& ctxt, const QString& msg)
{
    // Silence qWarning spam due to bug in QTextBrowser (trying to open a file for base64 images)
    if (QString::fromUtf8(ctxt.function)
            == QStringLiteral("virtual bool QFSFileEngine::open(QIODevice::OpenMode)")
        && msg == QStringLiteral("QFSFileEngine::open: No file name specified")) {
        return;
    }
    if (msg.startsWith("Unable to find any suggestion for")) {
        // Prevent sonnet's complaints from leaking user chat messages to logs
        return;
    }

    if (msg
        == QStringLiteral(
            "attempted to send message with network family 10 (probably IPv6) on IPv4 socket")) {
        // non-stop c-toxcore spam for IPv4 users: https://github.com/TokTok/c-toxcore/issues/1432
        return;
    }

    const QString file = canonicalLogFilePath(ctxt.file);
    const QString category =
        (ctxt.category != nullptr) ? QString::fromUtf8(ctxt.category) : QStringLiteral("default");
    if ((type == QtDebugMsg && category == QStringLiteral("tox.core")
         && (file == QStringLiteral("rtp.c") || file == QStringLiteral("video.c")))
        || (file == QStringLiteral("bwcontroller.c") && msg.contains("update"))) {
        // Don't log verbose toxav messages.
        return;
    }

    // Time should be in UTC to save user privacy on log sharing
    const QTime time = QDateTime::currentDateTime().toUTC().time();
    QString logPrefix =
        QStringLiteral("[%1 UTC] (%2) %3:%4 : ")
            .arg(time.toString("HH:mm:ss.zzz"), category, file, QString::number(ctxt.line));
    switch (type) {
    case QtDebugMsg:
        logPrefix += "Debug";
        break;
    case QtInfoMsg:
        logPrefix += "Info";
        break;
    case QtWarningMsg:
        logPrefix += "Warning";
        break;
    case QtCriticalMsg:
        logPrefix += "Critical";
        break;
    case QtFatalMsg:
        logPrefix += "Fatal";
        break;
    default:
        break;
    }

    QString logMsg;
    for (const auto& line : msg.split('\n')) {
        logMsg += logPrefix + ": " + canonicalLogMessage(line) + "\n";
    }

    const QByteArray LogMsgBytes = logMsg.toUtf8();
    fwrite(LogMsgBytes.constData(), 1, LogMsgBytes.size(), stderr);

#ifdef LOG_TO_FILE
    FILE* logFilePtr = logFileFile.loadRelaxed(); // atomically load the file pointer
    if (logFilePtr == nullptr) {
        logBufferMutex->lock();
        if (logBuffer != nullptr)
            logBuffer->append(LogMsgBytes);

        logBufferMutex->unlock();
    } else {
        logBufferMutex->lock();
        if (logBuffer != nullptr) {
            // empty logBuffer to file
            for (const QByteArray& bufferedMsg : *logBuffer) {
                fwrite(bufferedMsg.constData(), 1, bufferedMsg.size(), logFilePtr);
            }

            delete logBuffer; // no longer needed
            logBuffer = nullptr;
        }
        logBufferMutex->unlock();

        fwrite(LogMsgBytes.constData(), 1, LogMsgBytes.size(), logFilePtr);
        fflush(logFilePtr);
    }
#endif
}

bool toxURIEventHandler(const QByteArray& eventData, void* userData)
{
    ToxURIDialog* uriDialog = static_cast<ToxURIDialog*>(userData);
    if (!eventData.startsWith("tox:")) {
        return false;
    }

    if (uriDialog == nullptr) {
        return false;
    }

    uriDialog->handleToxURI(QString::fromUtf8(eventData));
    return true;
}
} // namespace

AppManager::AppManager(int& argc, char** argv)
    : qapp((preConstructionInitialization(), new QApplication(argc, argv)))
    , messageBoxManager(new MessageBoxManager(nullptr))
    , settings(new Settings(*messageBoxManager))
    , ipc(new IPC(settings->getCurrentProfileId()))
{
}

void AppManager::preConstructionInitialization()
{
    qInstallMessageHandler(logMessageHandler);
}

int AppManager::startGui(QCommandLineParser& parser)
{
    if (ipc->isAttached()) {
        connect(settings.get(), &Settings::currentProfileIdChanged, ipc.get(), &IPC::setProfileId);
    } else {
        qWarning() << "Can't init IPC, maybe we're in a jail? Continuing with reduced multi-client "
                      "functionality.";
    }

#ifdef LOG_TO_FILE
    const QString logFileDir = settings->getPaths().getAppCacheDirPath();
    QDir(logFileDir).mkpath(".");

    const QString logfile = logFileDir + "qtox.log";
    FILE* mainLogFilePtr = fopen(logfile.toLocal8Bit().constData(), "a");

    // Trim log file if over 1MB
    if (QFileInfo(logfile).size() > 1000000) {
        qDebug() << "Log file over 1MB, rotating...";

        // close old logfile (need for windows)
        if (mainLogFilePtr != nullptr)
            fclose(mainLogFilePtr);

        QDir dir(logFileDir);

        // Check if log.1 already exists, and if so, delete it
        if (dir.remove(logFileDir + "qtox.log.1"))
            qDebug() << "Removed old log successfully";
        else
            qWarning() << "Unable to remove old log file";

        if (!dir.rename(logFileDir + "qtox.log", logFileDir + "qtox.log.1"))
            qCritical() << "Unable to move logs";

        // open a new logfile
        mainLogFilePtr = fopen(logfile.toLocal8Bit().constData(), "a");
    }

    if (mainLogFilePtr == nullptr) {
        qCritical() << "Couldn't open logfile" << logfile;
    } else {
        qDebug() << "Logging to" << logfile;
    }

    logFileFile.storeRelaxed(mainLogFilePtr); // atomically set the logFile
#endif

    // Windows platform plugins DLL hell fix
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    qapp->addLibraryPath("platforms");

    qDebug() << "Commit:" << VersionInfo::gitVersion();
    qDebug() << "Process ID:" << QCoreApplication::applicationPid();

    QString profileName;
    bool autoLogin = settings->getAutoLogin();

    uint32_t ipcDest = 0;
    bool doIpc = ipc->isAttached();
    QString eventType;
    QString firstParam;
    if (parser.isSet("p")) {
        profileName = parser.value("p");
        if (!Profile::exists(profileName, settings->getPaths())) {
            qWarning() << "-p profile" << profileName + ".tox" << "doesn't exist, opening login screen";
            doIpc = false;
            autoLogin = false;
        } else {
            ipcDest = Settings::makeProfileId(profileName);
            autoLogin = true;
        }
    } else if (parser.isSet("l")) {
        doIpc = false;
        autoLogin = false;
    } else {
        profileName = settings->getCurrentProfile();
    }

    if (parser.positionalArguments().empty()) {
        eventType = "activate";
    } else {
        firstParam = parser.positionalArguments()[0];
        // Tox URIs. If there's already another qTox instance running, we ask it to handle the URI
        // and we exit
        // Otherwise we start a new qTox instance and process it ourselves
        if (firstParam.startsWith("tox:")) {
            eventType = "uri";
        } else if (firstParam.endsWith(".tox")) {
            eventType = ToxSave::eventHandlerKey;
        } else {
            qCritical() << "Invalid argument";
            return EXIT_FAILURE;
        }
    }

    if (doIpc && !ipc->isCurrentOwner()) {
        const time_t event = ipc->postEvent(eventType, firstParam.toUtf8(), ipcDest);
        // If someone else processed it, we're done here, no need to actually start qTox
        if (ipc->waitUntilAccepted(event, 2)) {
            if (eventType == "activate") {
                qDebug()
                    << "Another qTox instance is already running. If you want to start a second "
                       "instance, please open login screen (qtox -l) or start with a profile (qtox "
                       "-p <profile name>).";
            } else {
                qDebug() << "Event" << eventType << "was handled by other client.";
            }
            return EXIT_SUCCESS;
        }
    }

    if (!Settings::verifyProxySettings(parser)) {
        return -1;
    }

    // TODO(kriby): Consider moving application initializing variables into a globalSettings object
    //  note: Because Settings is shouldering global settings as well as model specific ones it
    //  cannot be integrated into a central model object yet
    cameraSource = std::make_unique<CameraSource>(*settings);
    nexus = std::make_unique<Nexus>(*settings, *messageBoxManager, *cameraSource, *ipc);
    // Autologin
    // TODO (kriby): Shift responsibility of linking views to model objects from nexus
    // Further: generate view instances separately (loginScreen, mainGUI, audio)
    Profile* profile = nullptr;
    if (autoLogin && Profile::exists(profileName, settings->getPaths())
        && !Profile::isEncrypted(profileName, settings->getPaths())) {
        profile = Profile::loadProfile(profileName, QString(), *settings, &parser, *cameraSource,
                                       *messageBoxManager);
        if (profile == nullptr) {
            QMessageBox::information(nullptr, tr("Error"), tr("Failed to load profile automatically."));
        }
    }
    if (profile != nullptr) {
        nexus->bootstrapWithProfile(profile);
    } else {
        nexus->setParser(&parser);
        const int returnval = nexus->showLogin(profileName);
        if (returnval == QDialog::Rejected) {
            return -1;
        }
        profile = nexus->getProfile();
    }

    uriDialog = std::make_unique<ToxURIDialog>(nullptr, profile->getCore(), *messageBoxManager);

    if (ipc->isAttached()) {
        // Start to accept Inter-process communication
        ipc->registerEventHandler("uri", &toxURIEventHandler, uriDialog.get());
        nexus->registerIpcHandlers();
    }

    // Event was not handled by already running instance therefore we handle it ourselves
    if (eventType == "uri") {
        uriDialog->handleToxURI(firstParam);
    } else if (eventType == ToxSave::eventHandlerKey) {
        nexus->handleToxSave(firstParam);
    }

    connect(qapp.get(), &QApplication::aboutToQuit, this, &AppManager::cleanup);

    return 0;
}

int AppManager::run()
{
    // Terminating signals are connected directly to quit() without any filtering.
    connect(&PosixSignalNotifier::globalInstance(), &PosixSignalNotifier::terminatingSignal,
            qapp.get(), &QApplication::quit);
    PosixSignalNotifier::watchCommonTerminatingSignals();

    // User signal 1 is used to screenshot the application.
    connect(&PosixSignalNotifier::globalInstance(), &PosixSignalNotifier::usrSignal, qapp.get(),
            [this](PosixSignalNotifier::UserSignal signal) {
                if (signal == PosixSignalNotifier::UserSignal::Screenshot) {
                    nexus->screenshot();
                }
            });
    PosixSignalNotifier::watchUsrSignals();

    qapp->setApplicationName("qTox");
    qapp->setDesktopFileName("io.github.qtox.qTox");
    qapp->setApplicationVersion(QStringLiteral("%1, git commit %2 (%3)")
                                    .arg(VersionInfo::gitDescribe())
                                    .arg(VersionInfo::gitVersion())
                                    .arg(UpdateCheck::isCurrentVersionStable()
                                             ? QStringLiteral("stable")
                                             : QStringLiteral("unstable")));

    // Install Unicode 6.1 supporting font
    // Keep this as close to the beginning of `main()` as possible, otherwise
    // on systems that have poor support for Unicode qTox will look bad.
    if (QFontDatabase::addApplicationFont("://font/DejaVuSans.ttf") == -1) {
        qWarning() << "Couldn't load font";
    }

    const QString locale = settings->getTranslation();
    // We need to init the resources in the translations_library explicitly.
    // See https://doc.qt.io/qt-5/resources.html#using-resources-in-a-library
    Q_INIT_RESOURCE(translations);
    Translator::translate(locale);

    // Process arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("qTox, version: " + VersionInfo::gitVersion());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("uri", tr("Tox URI to parse"));
    parser.addOptions({
        {{"p", "profile"}, tr("Starts new instance and loads specified profile."), tr("profile")},
        {{"D", "portable"}, tr("Starts in portable mode; loads profile from this directory."), "dir"},
        {{"l", "login"}, tr("Starts new instance and opens the login screen.")},
        {{"I", "ipv6"}, tr("Sets IPv6 <on>/<off>. Default is ON."), "on/off"},
        {{"U", "udp"}, tr("Sets UDP <on>/<off>. Default is ON."), "on/off"},
        {{"L", "lan"}, tr("Sets LAN discovery <on>/<off>. UDP off overrides. Default is ON."), "on/off"},
        {{"P", "proxy"}, tr("Sets proxy settings. Default is NONE."), "(SOCKS5/HTTP/NONE):(ADDRESS):(PORT)"},
#ifdef UPDATE_CHECK_ENABLED
        {{"u", "update-check"}, tr("Checks whether this program is running the latest qTox version.")},
#endif // UPDATE_CHECK_ENABLED
    });
    parser.process(*qapp);

    if (parser.isSet("portable")) {
        // We don't go through settings here, because we're not making qTox
        // portable (which moves files around). Instead, we start up in
        // portable mode as a one-off.
        settings->getPaths().setPortable(true);
        settings->getPaths().setPortablePath(parser.value("portable"));
    }

    // If update-check is requested, do it and exit.
    if (parser.isSet("update-check")) {
        UpdateCheck* updateCheck = new UpdateCheck(*settings, qapp.get());
        updateCheck->checkForUpdate();
        connect(updateCheck, &UpdateCheck::updateCheckFailed, qapp.get(), &QApplication::quit);
        connect(updateCheck, &UpdateCheck::complete, this,
                [this](QString currentVersion, QString latestVersion, const QUrl& link) {
                    const QString message =
                        QStringLiteral("Current version: %1\nLatest version: %2\n%3\n")
                            .arg(currentVersion, latestVersion, link.toString());
                    // Output to stdout.
                    QTextStream(stdout) << message;
                    qapp->quit();
                });
    } else {
        const int result = startGui(parser);
        if (result != 0) {
            return result;
        }
    }

    return qapp->exec();
}

AppManager::~AppManager() = default;

void AppManager::cleanup()
{
    // force save early even though destruction saves, because Windows OS will
    // close qTox before cleanup() is finished if logging out or shutting down,
    // once the top level window has exited, which occurs in ~Widget within
    // ~nexus-> Re-ordering Nexus destruction is not trivial.
    if (settings) {
        settings->saveGlobal();
        settings->savePersonal();
        settings->sync();
    }

    qDebug() << "Cleanup success";

#ifdef LOG_TO_FILE
    FILE* f = logFileFile.loadRelaxed();
    if (f != nullptr) {
        fclose(f);
        logFileFile.storeRelaxed(nullptr); // atomically disable logging to file
    }
#endif
}
