/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "settings.h"

#include "src/core/core.h"
#include "src/nexus.h"
#include "src/persistence/globalsettingsupgrader.h"
#include "src/persistence/personalsettingsupgrader.h"
#include "src/persistence/profile.h"
#include "src/persistence/profilelocker.h"
#include "src/persistence/settingsserializer.h"

#ifdef QTOX_PLATFORM_EXT
#include "src/platform/autorun.h"
#endif
#include "src/widget/style.h"
#include "src/widget/tool/imessageboxmanager.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QErrorMessage>
#include <QFile>
#include <QFont>
#include <QHostInfo>
#include <QList>
#include <QMutexLocker>
#include <QNetworkProxy>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QStyleHints>
#include <QThread>
#include <QtCore/QCommandLineParser>

namespace {
template <typename T, typename F>
void inGroup(T& ps, const QString& group, F&& f)
{
    ps.beginGroup(group);
    f();
    ps.endGroup();
}

template <typename T, typename F>
void inArray(T& ps, const QString& array, int size, F&& f)
{
    ps.beginWriteArray(array, size);
    f();
    ps.endArray();
}

template <typename T, typename F>
void inArray(T& ps, const QString& array, F&& f)
{
    const int size = ps.beginReadArray(array);
    f(size);
    ps.endArray();
}
} // namespace

/**
 * @var QHash<QString, QByteArray> Settings::widgetSettings
 * @brief Assume all widgets have unique names
 * @warning Don't use it to save every single thing you want to save, use it
 * for some general purpose widgets, such as MainWindows or Splitters,
 * which have widget->saveX() and widget->loadX() methods.
 */

const QString Settings::globalSettingsFile = "qtox.ini";
QRecursiveMutex Settings::bigLock;
QThread* Settings::settingsThread{nullptr};
static constexpr int GLOBAL_SETTINGS_VERSION = 1;
static constexpr int PERSONAL_SETTINGS_VERSION = 1;

Settings::Settings(IMessageBoxManager& messageBoxManager_)
    : loaded(false)
    , useCustomDhtList{false}
    , currentProfileId(0)
    , messageBoxManager{messageBoxManager_}
{
    settingsThread = new QThread();
    settingsThread->setObjectName("qTox Settings");
    settingsThread->start(QThread::LowPriority);
    qRegisterMetaType<const ToxEncrypt*>("const ToxEncrypt*");
    moveToThread(settingsThread);
    loadGlobal();
}

Settings::~Settings()
{
    sync();
    settingsThread->exit(0);
    settingsThread->wait();
    delete settingsThread;
}

void Settings::loadGlobal()
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    if (loaded)
        return;

    createSettingsDir();

    const QDir dir(paths.getSettingsDirPath());
    QString filePath = dir.filePath(globalSettingsFile);

    // If no settings file exist -- use the default one
    bool defaultSettings = false;
    if (!QFile(filePath).exists()) {
        qDebug() << "No settings file found, using defaults";
        filePath = ":/conf/" + globalSettingsFile;
        defaultSettings = true;
    }

    qDebug() << "Loading settings from" << filePath;

    QSettings s(filePath, QSettings::IniFormat);

    inGroup(s, "Version", [this, &s, defaultSettings] {
        const auto defaultVersion = defaultSettings ? GLOBAL_SETTINGS_VERSION : 0;
        globalSettingsVersion = s.value("settingsVersion", defaultVersion).toInt();
    });

    auto upgradeSuccess =
        GlobalSettingsUpgrader::doUpgrade(*this, globalSettingsVersion, GLOBAL_SETTINGS_VERSION);
    if (!upgradeSuccess) {
        messageBoxManager.showError(
            tr("Failed to load global settings"),
            tr("Unable to upgrade settings from version %1 to version %2. Cannot start qTox.")
                .arg(globalSettingsVersion)
                .arg(GLOBAL_SETTINGS_VERSION));
        std::terminate();
    }
    globalSettingsVersion = GLOBAL_SETTINGS_VERSION;

    inGroup(s, "Login", [this, &s] { //
        autoLogin = s.value("autoLogin", false).toBool();
    });

    inGroup(s, "General", [this, &s] {
        translation = s.value("translation", "en").toString();
        showSystemTray = s.value("showSystemTray", true).toBool();
        autostartInTray = s.value("autostartInTray", false).toBool();
        closeToTray = s.value("closeToTray", true).toBool();
        if (currentProfile.isEmpty()) {
            currentProfile = s.value("currentProfile", "").toString();
            currentProfileId = makeProfileId(currentProfile);
        }
        autoAwayTime = s.value("autoAwayTime", 10).toInt();
        checkUpdates = s.value("checkUpdates", true).toBool();
        // note: notifySound and busySound UI elements are now under UI settings
        // page, but kept under General in settings file to be backwards compatible
        notifySound = s.value("notifySound", true).toBool();
        notifyHide = s.value("notifyHide", false).toBool();
        busySound = s.value("busySound", false).toBool();
        autoSaveEnabled = s.value("autoSaveEnabled", false).toBool();
        globalAutoAcceptDir = s.value("globalAutoAcceptDir",
                                      QStandardPaths::locate(QStandardPaths::HomeLocation, QString(),
                                                             QStandardPaths::LocateDirectory))
                                  .toString();
        autoAcceptMaxSize =
            static_cast<size_t>(s.value("autoAcceptMaxSize", 20 << 20 /*20 MB*/).toLongLong());
        stylePreference = static_cast<StyleType>(s.value("stylePreference", 1).toInt());
    });

    inGroup(s, "Advanced", [this, &s] {
        paths.setPortable(s.value("makeToxPortable", false).toBool());
        enableIPv6 = s.value("enableIPv6", true).toBool();
        forceTCP = s.value("forceTCP", false).toBool();
        enableLanDiscovery = s.value("enableLanDiscovery", true).toBool();
        enableDebug = s.value("enableDebug", false).toBool();
    });

    inGroup(s, "Widgets", [this, &s] {
        const QList<QString> objectNames = s.childKeys();
        for (const QString& name : objectNames)
            widgetSettings[name] = s.value(name).toByteArray();
    });

    inGroup(s, "GUI", [this, &s] {
        showWindow = s.value("showWindow", true).toBool();
        notify = s.value("notify", true).toBool();
        desktopNotify = s.value("desktopNotify", true).toBool();
        notifySystemBackend = s.value("notifySystemBackend", true).toBool();
        conferenceAlwaysNotify = s.value("conferenceAlwaysNotify", true).toBool();
        conferencePosition = s.value("conferencePosition", true).toBool();
        separateWindow = s.value("separateWindow", false).toBool();
        dontGroupWindows = s.value("dontGroupWindows", false).toBool();
        showIdenticons = s.value("showIdenticons", true).toBool();

        const QString DEFAULT_SMILEYS = ":/smileys/EmojiOne/emoticons.xml";
        smileyPack = s.value("smileyPack", DEFAULT_SMILEYS).toString();
        if (!QFile::exists(smileyPack)) {
            smileyPack = DEFAULT_SMILEYS;
        }

        const Style::MainTheme systemTheme =
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
            QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark
#else
            QGuiApplication::palette().color(QPalette::Window).value() < 128
#endif
                ? Style::MainTheme::Dark
                : Style::MainTheme::Light;
        qDebug() << "System theme:" << systemTheme;

        emojiFontPointSize = s.value("emojiFontPointSize", 24).toInt();
        firstColumnHandlePos = s.value("firstColumnHandlePos", 50).toInt();
        secondColumnHandlePosFromRight = s.value("secondColumnHandlePosFromRight", 50).toInt();
        timestampFormat = s.value("timestampFormat", "hh:mm:ss").toString();
        dateFormat = s.value("dateFormat", "yyyy-MM-dd").toString();
        minimizeOnClose = s.value("minimizeOnClose", false).toBool();
        minimizeToTray = s.value("minimizeToTray", false).toBool();
        lightTrayIcon = s.value("lightTrayIcon", systemTheme == Style::MainTheme::Dark).toBool();
        useEmoticons = s.value("useEmoticons", true).toBool();
        statusChangeNotificationEnabled = s.value("statusChangeNotificationEnabled", false).toBool();
        showConferenceJoinLeaveMessages = s.value("showConferenceJoinLeaveMessages", false).toBool();
        spellCheckingEnabled = s.value("spellCheckingEnabled", true).toBool();
        themeColor = s.value("themeColor", Style::defaultThemeColor(systemTheme)).toInt();
        style = s.value("style", "").toString();
        if (style == "") // Default to Fusion if available, otherwise no style
        {
            if (QStyleFactory::keys().contains("Fusion"))
                style = "Fusion";
            else
                style = "None";
        }
        nameColors = s.value("nameColors", false).toBool();
        imagePreview = s.value("imagePreview", true).toBool();
        chatMaxWindowSize = s.value("chatMaxWindowSize", 100).toInt();
        chatWindowChunkSize = s.value("chatWindowChunkSize", 50).toInt();
    });

    inGroup(s, "Chat", [this, &s] {
        chatMessageFont = s.value("chatMessageFont", Style::getFont(Style::Font::Big)).value<QFont>();
    });

    inGroup(s, "State", [this, &s] {
        windowGeometry = s.value("windowGeometry", QByteArray()).toByteArray();
        windowState = s.value("windowState", QByteArray()).toByteArray();
        splitterState = s.value("splitterState", QByteArray()).toByteArray();
        dialogGeometry = s.value("dialogGeometry", QByteArray()).toByteArray();
        dialogSplitterState = s.value("dialogSplitterState", QByteArray()).toByteArray();
        dialogSettingsGeometry = s.value("dialogSettingsGeometry", QByteArray()).toByteArray();
    });

    inGroup(s, "Audio", [this, &s] {
        inDev = s.value("inDev", "").toString();
        audioInDevEnabled = s.value("audioInDevEnabled", true).toBool();
        outDev = s.value("outDev", "").toString();
        audioOutDevEnabled = s.value("audioOutDevEnabled", true).toBool();
        audioInGainDecibel = s.value("inGain", 0).toReal();
        audioThreshold = s.value("audioThreshold", 0).toReal();
        outVolume = s.value("outVolume", 100).toInt();
        enableTestSound = s.value("enableTestSound", true).toBool();
        audioBitrate = s.value("audioBitrate", 64).toInt();
    });

    inGroup(s, "Video", [this, &s] {
        videoDev = s.value("videoDev", "").toString();
        camVideoRes = s.value("camVideoRes", QRect()).toRect();
        screenRegion = s.value("screenRegion", QRect()).toRect();
        screenGrabbed = s.value("screenGrabbed", false).toBool();
        camVideoFPS = static_cast<quint16>(s.value("camVideoFPS", 0).toUInt());
    });

    loaded = true;
}

void Settings::updateProfileData(Profile& profile, const QCommandLineParser* parser, bool newProfile)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    setCurrentProfile(profile.getName());
    saveGlobal();
    loadPersonal(profile, newProfile);
    if (parser != nullptr) {
        applyCommandLineOptions(*parser);
    }
}

/**
 * Verifies that commandline proxy settings are at least reasonable. Does not verify provided IP
 * or hostname addresses are valid. Code duplication with Settings::applyCommandLineOptions, which
 * also verifies arguments, should be removed in a future refactor.
 * @param parser QCommandLineParser instance
 */
bool Settings::verifyProxySettings(const QCommandLineParser& parser)
{
    const QString ipv6SettingString = parser.value("I").toLower();
    const QString lanSettingString = parser.value("L").toLower();
    const QString udpSettingString = parser.value("U").toLower();
    const QString proxySettingString = parser.value("proxy").toLower();
    const QStringList proxySettingStrings = proxySettingString.split(":");

    const QString SOCKS5 = QStringLiteral("socks5");
    const QString HTTP = QStringLiteral("http");
    const QString NONE = QStringLiteral("none");
    const QString ON = QStringLiteral("on");
    const QString OFF = QStringLiteral("off");

    // Check for incompatible settings
    bool activeProxyType = false;

    if (parser.isSet("P")) {
        activeProxyType = proxySettingStrings[0] == SOCKS5 || proxySettingStrings[0] == HTTP;
    }

    if (parser.isSet("I")) {
        if (!(ipv6SettingString == ON || ipv6SettingString == OFF)) {
            qCritical() << "Unable to parse IPv6 setting (-I was" << ipv6SettingString
                        << "but should be on/off).";
            return false;
        }
    }

    if (parser.isSet("U")) {
        if (!(udpSettingString == ON || udpSettingString == OFF)) {
            qCritical() << "Unable to parse UDP setting (-U was" << udpSettingString
                        << "but should be on/off).";
            return false;
        }
    }

    if (parser.isSet("L")) {
        if (!(lanSettingString == ON || lanSettingString == OFF)) {
            qCritical() << "Unable to parse LAN setting (-L was" << lanSettingString
                        << "but should be on/off).";
            return false;
        }
    }
    if (activeProxyType && udpSettingString == ON) {
        qCritical() << "Cannot set UDP on with proxy.";
        return false;
    }

    if (activeProxyType && lanSettingString == ON) {
        qCritical() << "Cannot use LAN discovery with proxy.";
        return false;
    }

    if (lanSettingString == ON && udpSettingString == OFF) {
        qCritical() << "Incompatible UDP/LAN settings: LAN discovery requires UDP.";
        return false;
    }

    if (parser.isSet("P")) {
        if (proxySettingStrings[0] == NONE) {
            // slightly lazy check here, accepting 'NONE[:.*]' is fine since no other
            // arguments will be investigated when proxy settings are applied.
            return true;
        }
        // Since the first argument isn't 'none', verify format of remaining arguments
        if (proxySettingStrings.size() != 3) {
            qCritical() << "Invalid number of proxy arguments (should be 3).";
            return false;
        }

        if (!(proxySettingStrings[0] == SOCKS5 || proxySettingStrings[0] == HTTP)) {
            qCritical() << "Unable to parse proxy type (was" << proxySettingStrings[0]
                        << "but should be SOCKS5/HTTP).";
            return false;
        }

        const QHostInfo hostInfo = QHostInfo::fromName(proxySettingStrings[1]);
        if (hostInfo.addresses().isEmpty()) {
            qCritical() << "Invalid proxy address (DNS resolution failed):" << proxySettingStrings[1];
            return false;
        }

        bool ok;
        const int portNumber = proxySettingStrings[2].toInt(&ok);
        if (!ok) {
            qCritical() << "Invalid port number" << proxySettingStrings[2] << "(must be an integer).";
        }
        if (portNumber < 1 || portNumber > 65535) {
            qCritical() << "Invalid port number range: was" << portNumber << "but should be 1-65535.";
        }
    }
    return true;
}

/**
 * Applies command line options on top of loaded settings. Fails without changes if attempting to
 * apply contradicting settings.
 * @param parser QCommandLineParser instance
 * @return Success indicator (success = true)
 */
bool Settings::applyCommandLineOptions(const QCommandLineParser& parser)
{
    if (!verifyProxySettings(parser)) {
        return false;
    }

    const QString ipv6Setting = parser.value("I").toUpper();
    const QString lanSetting = parser.value("L").toUpper();
    const QString udpSetting = parser.value("U").toUpper();
    const QString proxySettingString = parser.value("proxy").toUpper();
    QStringList proxySettings = proxySettingString.split(":");

    const QString SOCKS5 = QStringLiteral("SOCKS5");
    const QString HTTP = QStringLiteral("HTTP");
    const QString NONE = QStringLiteral("NONE");
    const QString ON = QStringLiteral("ON");
    const QString OFF = QStringLiteral("OFF");

    if (parser.isSet("I")) {
        enableIPv6 = ipv6Setting == ON;
        qDebug() << "Setting IPv6 to" << ipv6Setting;
    }

    if (parser.isSet("P")) {
        qDebug() << "Setting proxy type to" << proxySettings[0];

        quint16 portNumber = 0;
        QString address = "";

        if (proxySettings[0] == NONE) {
            proxyType = ICoreSettings::ProxyType::ptNone;
        } else {
            if (proxySettings[0] == SOCKS5) {
                proxyType = ICoreSettings::ProxyType::ptSOCKS5;
            } else if (proxySettings[0] == HTTP) {
                proxyType = ICoreSettings::ProxyType::ptHTTP;
            } else {
                qCritical() << "Failed to set valid proxy type";
                assert(false); // verifyProxySettings should've made this impossible
            }

            forceTCP = true;
            enableLanDiscovery = false;

            address = proxySettings[1];
            portNumber = static_cast<quint16>(proxySettings[2].toInt());
        }


        proxyAddr = address;
        qDebug() << "Setting proxy address to" << address;
        proxyPort = portNumber;
        qDebug() << "Setting port number to" << portNumber;
    }

    if (parser.isSet("U")) {
        const bool shouldForceTCP = udpSetting == OFF;
        if (!shouldForceTCP && proxyType != ICoreSettings::ProxyType::ptNone) {
            qDebug() << "Cannot use UDP with proxy; disable proxy explicitly with '-P none'.";
        } else {
            forceTCP = shouldForceTCP;
            qDebug() << "Setting UDP" << udpSetting;
        }

        // LANSetting == ON is caught by verifyProxySettings, the OFF check removes needless debug
        if (shouldForceTCP && !(lanSetting == OFF) && enableLanDiscovery) {
            qDebug() << "Cannot perform LAN discovery without UDP; disabling LAN discovery.";
            enableLanDiscovery = false;
        }
    }

    if (parser.isSet("L")) {
        const bool shouldEnableLAN = lanSetting == ON;

        if (shouldEnableLAN && proxyType != ICoreSettings::ProxyType::ptNone) {
            qDebug()
                << "Cannot use LAN discovery with proxy; disable proxy explicitly with '-P none'.";
        } else if (shouldEnableLAN && forceTCP) {
            qDebug() << "Cannot use LAN discovery without UDP; enable UDP explicitly with '-U on'.";
        } else {
            enableLanDiscovery = shouldEnableLAN;
            qDebug() << "Setting LAN Discovery" << lanSetting;
        }
    }
    return true;
}

void Settings::loadPersonal(const Profile& profile, bool newProfile)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    loadedProfile = &profile;
    const QDir dir(paths.getSettingsDirPath());
    QString filePath = dir.filePath(globalSettingsFile);

    // load from a profile specific friend data list if possible
    const QString tmp = dir.filePath(profile.getName() + ".ini");
    if (QFile(tmp).exists()) { // otherwise, filePath remains the global file
        filePath = tmp;
    }

    qDebug() << "Loading personal settings from" << filePath;

    SettingsSerializer ps(filePath, profile.getPasskey());
    ps.load();
    friendLst.clear();

    inGroup(ps, "Version", [this, &ps, newProfile] {
        const auto defaultVersion = newProfile ? PERSONAL_SETTINGS_VERSION : 0;
        personalSettingsVersion = ps.value("settingsVersion", defaultVersion).toInt();
    });

    auto upgradeSuccess =
        PersonalSettingsUpgrader::doUpgrade(ps, personalSettingsVersion, PERSONAL_SETTINGS_VERSION);
    if (!upgradeSuccess) {
        messageBoxManager.showError(
            tr("Failed to load personal settings"),
            tr("Unable to upgrade settings from version %1 to version %2. Cannot start qTox.")
                .arg(personalSettingsVersion)
                .arg(PERSONAL_SETTINGS_VERSION));
        std::terminate();
    }
    personalSettingsVersion = PERSONAL_SETTINGS_VERSION;

    inGroup(ps, "Privacy", [this, &ps] {
        typingNotification = ps.value("typingNotification", true).toBool();
        enableLogging = ps.value("enableLogging", true).toBool();
        blockList = ps.value("blackList").toString().split('\n');
    });

    inGroup(ps, "Friends", [this, &ps] {
        inArray(ps, "Friend", [this, &ps](int size) {
            friendLst.reserve(size);
            for (int i = 0; i < size; i++) {
                ps.setArrayIndex(i);
                friendProp fp{ps.value("addr").toString()};
                fp.alias = ps.value("alias").toString();
                fp.note = ps.value("note").toString();
                fp.autoAcceptDir = ps.value("autoAcceptDir").toString();

                if (fp.autoAcceptDir == "")
                    fp.autoAcceptDir = ps.value("autoAccept").toString();

                fp.autoAcceptCall =
                    Settings::AutoAcceptCallFlags(QFlag(ps.value("autoAcceptCall", 0).toInt()));
                fp.autoConferenceInvite = ps.value("autoConferenceInvite").toBool();
                fp.circleID = ps.value("circle", -1).toInt();

                if (getEnableLogging())
                    fp.activity = ps.value("activity", QDateTime()).toDateTime();
                friendLst.insert(ToxPk(fp.addr).getByteArray(), fp);
            }
        });
    });

    inGroup(ps, "Requests", [this, &ps] {
        inArray(ps, "Request", [this, &ps](int size) {
            friendRequests.clear();
            friendRequests.reserve(size);
            for (int i = 0; i < size; i++) {
                ps.setArrayIndex(i);
                Request request;
                request.address = ps.value("addr").toString();
                request.message = ps.value("message").toString();
                request.read = ps.value("read").toBool();
                friendRequests.push_back(request);
            }
        });
    });

    inGroup(ps, "GUI", [this, &ps] {
        compactLayout = ps.value("compactLayout", true).toBool();
        sortingMode = static_cast<FriendListSortingMode>(
            ps.value("friendSortingMethod", static_cast<int>(FriendListSortingMode::Name)).toInt());
    });

    inGroup(ps, "Proxy", [this, &ps] {
        proxyType = static_cast<ProxyType>(ps.value("proxyType", 0 /* ProxyType::None */).toInt());
        proxyType = fixInvalidProxyType(proxyType);
        proxyAddr = ps.value("proxyAddr", proxyAddr).toString();
        proxyPort = static_cast<quint16>(ps.value("proxyPort", proxyPort).toUInt());
    });

    inGroup(ps, "Circles", [this, &ps] {
        inArray(ps, "Circle", [this, &ps](int size) {
            circleLst.clear();
            circleLst.reserve(size);
            for (int i = 0; i < size; i++) {
                ps.setArrayIndex(i);
                circleProp cp;
                cp.name = ps.value("name").toString();
                cp.expanded = ps.value("expanded", true).toBool();
                circleLst.push_back(cp);
            }
        });
    });
}

void Settings::resetToDefault()
{
    // To stop saving
    loaded = false;

    // Remove file with profile settings
    const QDir dir(paths.getSettingsDirPath());
    const QString localPath = dir.filePath(loadedProfile->getName() + ".ini");
    QFile local(localPath);
    if (local.exists())
        local.remove();
}

/**
 * @brief Asynchronous, saves the global settings.
 */
void Settings::saveGlobal()
{
    if (QThread::currentThread() != settingsThread)
        return (void)QMetaObject::invokeMethod(this, "saveGlobal");

    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    if (!loaded)
        return;

    const QString path = paths.getSettingsDirPath() + globalSettingsFile;
    qDebug() << "Saving global settings at" << path;

    QSettings s(path, QSettings::IniFormat);

    s.clear();

    inGroup(s, "Login", [this, &s] { //
        s.setValue("autoLogin", autoLogin);
    });

    inGroup(s, "General", [this, &s] {
        s.setValue("translation", translation);
        s.setValue("showSystemTray", showSystemTray);
        s.setValue("autostartInTray", autostartInTray);
        s.setValue("closeToTray", closeToTray);
        s.setValue("currentProfile", currentProfile);
        s.setValue("autoAwayTime", autoAwayTime);
        s.setValue("checkUpdates", checkUpdates);
        s.setValue("notifySound", notifySound);
        s.setValue("notifyHide", notifyHide);
        s.setValue("busySound", busySound);
        s.setValue("autoSaveEnabled", autoSaveEnabled);
        s.setValue("autoAcceptMaxSize", static_cast<qlonglong>(autoAcceptMaxSize));
        s.setValue("globalAutoAcceptDir", globalAutoAcceptDir);
        s.setValue("stylePreference", static_cast<int>(stylePreference));
    });

    inGroup(s, "Advanced", [this, &s] {
        s.setValue("makeToxPortable", paths.isPortable());
        s.setValue("enableIPv6", enableIPv6);
        s.setValue("forceTCP", forceTCP);
        s.setValue("enableLanDiscovery", enableLanDiscovery);
        s.setValue("dbSyncType", static_cast<int>(dbSyncType));
        s.setValue("enableDebug", enableDebug);
    });

    inGroup(s, "Widgets", [this, &s] {
        const QList<QString> widgetNames = widgetSettings.keys();
        for (const QString& name : widgetNames)
            s.setValue(name, widgetSettings.value(name));
    });

    inGroup(s, "GUI", [this, &s] {
        s.setValue("showWindow", showWindow);
        s.setValue("notify", notify);
        s.setValue("desktopNotify", desktopNotify);
        s.setValue("notifySystemBackend", notifySystemBackend);
        s.setValue("conferenceAlwaysNotify", conferenceAlwaysNotify);
        s.setValue("separateWindow", separateWindow);
        s.setValue("dontGroupWindows", dontGroupWindows);
        s.setValue("conferencePosition", conferencePosition);
        s.setValue("showIdenticons", showIdenticons);

        s.setValue("smileyPack", smileyPack);
        s.setValue("emojiFontPointSize", emojiFontPointSize);
        s.setValue("firstColumnHandlePos", firstColumnHandlePos);
        s.setValue("secondColumnHandlePosFromRight", secondColumnHandlePosFromRight);
        s.setValue("timestampFormat", timestampFormat);
        s.setValue("dateFormat", dateFormat);
        s.setValue("minimizeOnClose", minimizeOnClose);
        s.setValue("minimizeToTray", minimizeToTray);
        s.setValue("lightTrayIcon", lightTrayIcon);
        s.setValue("useEmoticons", useEmoticons);
        s.setValue("themeColor", themeColor);
        s.setValue("style", style);
        s.setValue("nameColors", nameColors);
        s.setValue("imagePreview", imagePreview);
        s.setValue("chatMaxWindowSize", chatMaxWindowSize);
        s.setValue("chatWindowChunkSize", chatWindowChunkSize);

        s.setValue("statusChangeNotificationEnabled", statusChangeNotificationEnabled);
        s.setValue("showConferenceJoinLeaveMessages", showConferenceJoinLeaveMessages);
        s.setValue("spellCheckingEnabled", spellCheckingEnabled);
    });

    inGroup(s, "Chat", [this, &s] { //
        s.setValue("chatMessageFont", chatMessageFont);
    });

    inGroup(s, "State", [this, &s] {
        s.setValue("windowGeometry", windowGeometry);
        s.setValue("windowState", windowState);
        s.setValue("splitterState", splitterState);
        s.setValue("dialogGeometry", dialogGeometry);
        s.setValue("dialogSplitterState", dialogSplitterState);
        s.setValue("dialogSettingsGeometry", dialogSettingsGeometry);
    });

    inGroup(s, "Audio", [this, &s] {
        s.setValue("inDev", inDev);
        s.setValue("audioInDevEnabled", audioInDevEnabled);
        s.setValue("outDev", outDev);
        s.setValue("audioOutDevEnabled", audioOutDevEnabled);
        s.setValue("inGain", audioInGainDecibel);
        s.setValue("audioThreshold", audioThreshold);
        s.setValue("outVolume", outVolume);
        s.setValue("enableTestSound", enableTestSound);
        s.setValue("audioBitrate", audioBitrate);
    });

    inGroup(s, "Video", [this, &s] {
        s.setValue("videoDev", videoDev);
        s.setValue("camVideoRes", camVideoRes);
        s.setValue("camVideoFPS", camVideoFPS);
        s.setValue("screenRegion", screenRegion);
        s.setValue("screenGrabbed", screenGrabbed);
    });

    inGroup(s, "Version", [this, &s] { //
        s.setValue("settingsVersion", globalSettingsVersion);
    });
}

/**
 * @brief Asynchronous, saves the profile.
 */
void Settings::savePersonal()
{
    if (loadedProfile == nullptr) {
        qDebug() << "Could not save personal settings because there is no active profile";
        return;
    }
    QMetaObject::invokeMethod(this, "savePersonal", Q_ARG(QString, loadedProfile->getName()),
                              Q_ARG(const ToxEncrypt*, loadedProfile->getPasskey()));
}

void Settings::savePersonal(QString profileName, const ToxEncrypt* passkey)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    if (!loaded)
        return;

    const QString path = paths.getSettingsDirPath() + profileName + ".ini";

    qDebug() << "Saving personal settings at" << path;

    SettingsSerializer ps(path, passkey);
    inGroup(ps, "Friends", [this, &ps] {
        inArray(ps, "Friend", friendLst.size(), [this, &ps] {
            int index = 0;
            for (auto& frnd : friendLst) {
                ps.setArrayIndex(index);
                ps.setValue("addr", frnd.addr);
                ps.setValue("alias", frnd.alias);
                ps.setValue("note", frnd.note);
                ps.setValue("autoAcceptDir", frnd.autoAcceptDir);
                ps.setValue("autoAcceptCall", static_cast<int>(frnd.autoAcceptCall));
                ps.setValue("autoConferenceInvite", frnd.autoConferenceInvite);
                ps.setValue("circle", frnd.circleID);

                if (getEnableLogging())
                    ps.setValue("activity", frnd.activity);

                ++index;
            }
        });
    });

    inGroup(ps, "Requests", [this, &ps] {
        inArray(ps, "Request", friendRequests.size(), [this, &ps] {
            int index = 0;
            for (auto& request : friendRequests) {
                ps.setArrayIndex(index);
                ps.setValue("addr", request.address);
                ps.setValue("message", request.message);
                ps.setValue("read", request.read);

                ++index;
            }
        });
    });

    inGroup(ps, "GUI", [this, &ps] {
        ps.setValue("compactLayout", compactLayout);
        ps.setValue("friendSortingMethod", static_cast<int>(sortingMode));
    });

    inGroup(ps, "Proxy", [this, &ps] {
        ps.setValue("proxyType", static_cast<int>(proxyType));
        ps.setValue("proxyAddr", proxyAddr);
        ps.setValue("proxyPort", proxyPort);
    });

    inGroup(ps, "Circles", [this, &ps] {
        inArray(ps, "Circle", circleLst.size(), [this, &ps] {
            int index = 0;
            for (auto& circle : circleLst) {
                ps.setArrayIndex(index);
                ps.setValue("name", circle.name);
                ps.setValue("expanded", circle.expanded);
                ++index;
            }
        });
    });

    inGroup(ps, "Privacy", [this, &ps] {
        ps.setValue("typingNotification", typingNotification);
        ps.setValue("enableLogging", enableLogging);
        ps.setValue("blackList", blockList.join('\n'));
    });

    inGroup(ps, "Version", [this, &ps] { //
        ps.setValue("settingsVersion", personalSettingsVersion);
    });
    ps.save();
}

uint32_t Settings::makeProfileId(const QString& profile)
{
    const QByteArray data = QCryptographicHash::hash(profile.toUtf8(), QCryptographicHash::Md5);
    const auto* dwords = reinterpret_cast<const uint32_t*>(data.constData());
    return dwords[0] ^ dwords[1] ^ dwords[2] ^ dwords[3];
}

Paths& Settings::getPaths()
{
    return paths;
}

bool Settings::getEnableTestSound() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return enableTestSound;
}

void Settings::setEnableTestSound(bool newValue)
{
    if (setVal(enableTestSound, newValue)) {
        emit enableTestSoundChanged(newValue);
    }
}

bool Settings::getEnableIPv6() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return enableIPv6;
}

void Settings::setEnableIPv6(bool enabled)
{
    if (setVal(enableIPv6, enabled)) {
        emit enableIPv6Changed(enabled);
    }
}

bool Settings::getMakeToxPortable() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return paths.isPortable();
}

void Settings::setMakeToxPortable(bool newValue)
{
    bool changed = false;
    {
        const QMutexLocker<QRecursiveMutex> locker{&bigLock};
        const auto oldSettingsPath = paths.getSettingsDirPath() + globalSettingsFile;
        changed = paths.setPortable(newValue);
        if (changed) {
            QFile(oldSettingsPath).remove();
            saveGlobal();
            emit makeToxPortableChanged(newValue);
        }
    }
}

bool Settings::getAutorun() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

#ifdef QTOX_PLATFORM_EXT
    return Platform::getAutorun(*this);
#else
    return false;
#endif
}

void Settings::setAutorun(bool newValue)
{
#ifdef QTOX_PLATFORM_EXT
    const bool autorun = Platform::getAutorun(*this);

    if (newValue != autorun) {
        Platform::setAutorun(*this, newValue);
        emit autorunChanged(autorun);
    }
#else
    std::ignore = newValue;
#endif
}

bool Settings::getAutostartInTray() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return autostartInTray;
}

QString Settings::getStyle() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return style;
}

void Settings::setStyle(const QString& newStyle)
{
    if (setVal(style, newStyle)) {
        emit styleChanged(style);
    }
}

bool Settings::getShowSystemTray() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return showSystemTray;
}

void Settings::setShowSystemTray(bool newValue)
{
    if (setVal(showSystemTray, newValue)) {
        emit showSystemTrayChanged(newValue);
    }
}

void Settings::setUseEmoticons(bool newValue)
{
    if (setVal(useEmoticons, newValue)) {
        emit useEmoticonsChanged(newValue);
    }
}

bool Settings::getUseEmoticons() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return useEmoticons;
}

void Settings::setAutoSaveEnabled(bool newValue)
{
    if (setVal(autoSaveEnabled, newValue)) {
        emit autoSaveEnabledChanged(newValue);
    }
}

bool Settings::getAutoSaveEnabled() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return autoSaveEnabled;
}

void Settings::setEnableDebug(bool newValue)
{
    if (setVal(enableDebug, newValue)) {
        emit enableDebugChanged(newValue);
    }
}

bool Settings::getEnableDebug() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return enableDebug;
}

void Settings::setAutostartInTray(bool newValue)
{
    if (setVal(autostartInTray, newValue)) {
        emit autostartInTrayChanged(newValue);
    }
}

bool Settings::getCloseToTray() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return closeToTray;
}

void Settings::setCloseToTray(bool newValue)
{
    if (setVal(closeToTray, newValue)) {
        emit closeToTrayChanged(newValue);
    }
}

bool Settings::getMinimizeToTray() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return minimizeToTray;
}

void Settings::setMinimizeToTray(bool newValue)
{
    if (setVal(minimizeToTray, newValue)) {
        emit minimizeToTrayChanged(newValue);
    }
}

bool Settings::getLightTrayIcon() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return lightTrayIcon;
}

void Settings::setLightTrayIcon(bool newValue)
{
    if (setVal(lightTrayIcon, newValue)) {
        emit lightTrayIconChanged(newValue);
    }
}

bool Settings::getStatusChangeNotificationEnabled() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return statusChangeNotificationEnabled;
}

void Settings::setStatusChangeNotificationEnabled(bool newValue)
{
    if (setVal(statusChangeNotificationEnabled, newValue)) {
        emit statusChangeNotificationEnabledChanged(newValue);
    }
}

bool Settings::getShowConferenceJoinLeaveMessages() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return showConferenceJoinLeaveMessages;
}

void Settings::setShowConferenceJoinLeaveMessages(bool newValue)
{
    if (setVal(showConferenceJoinLeaveMessages, newValue)) {
        emit showConferenceJoinLeaveMessagesChanged(newValue);
    }
}

bool Settings::getSpellCheckingEnabled() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return spellCheckingEnabled;
}

void Settings::setSpellCheckingEnabled(bool newValue)
{
    if (setVal(spellCheckingEnabled, newValue)) {
        emit spellCheckingEnabledChanged(newValue);
    }
}

bool Settings::getNotifySound() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return notifySound;
}

void Settings::setNotifySound(bool newValue)
{
    if (setVal(notifySound, newValue)) {
        emit notifySoundChanged(newValue);
    }
}

bool Settings::getNotifyHide() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return notifyHide;
}

void Settings::setNotifyHide(bool newValue)
{
    if (setVal(notifyHide, newValue)) {
        emit notifyHideChanged(newValue);
    }
}

bool Settings::getBusySound() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return busySound;
}

void Settings::setBusySound(bool newValue)
{
    if (setVal(busySound, newValue)) {
        emit busySoundChanged(newValue);
    }
}

bool Settings::getConferenceAlwaysNotify() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return conferenceAlwaysNotify;
}

void Settings::setConferenceAlwaysNotify(bool newValue)
{
    if (setVal(conferenceAlwaysNotify, newValue)) {
        emit conferenceAlwaysNotifyChanged(newValue);
    }
}

QString Settings::getTranslation() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return translation;
}

void Settings::setTranslation(const QString& newValue)
{
    if (setVal(translation, newValue)) {
        emit translationChanged(newValue);
    }
}

bool Settings::getForceTCP() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return forceTCP;
}

void Settings::setForceTCP(bool enabled)
{
    if (setVal(forceTCP, enabled)) {
        emit forceTCPChanged(enabled);
    }
}

bool Settings::getEnableLanDiscovery() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return enableLanDiscovery;
}

void Settings::setEnableLanDiscovery(bool enabled)
{
    if (setVal(enableLanDiscovery, enabled)) {
        emit enableLanDiscoveryChanged(enabled);
    }
}

QNetworkProxy Settings::getProxy() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    QNetworkProxy proxy;
    switch (Settings::getProxyType()) {
    case ProxyType::ptNone:
        proxy.setType(QNetworkProxy::NoProxy);
        break;
    case ProxyType::ptSOCKS5:
        proxy.setType(QNetworkProxy::Socks5Proxy);
        break;
    case ProxyType::ptHTTP:
        proxy.setType(QNetworkProxy::HttpProxy);
        break;
    default:
        proxy.setType(QNetworkProxy::NoProxy);
        qWarning() << "Invalid proxy type, setting to NoProxy";
        break;
    }

    proxy.setHostName(Settings::getProxyAddr());
    proxy.setPort(Settings::getProxyPort());
    return proxy;
}

Settings::ProxyType Settings::getProxyType() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return proxyType;
}

void Settings::setProxyType(ProxyType newValue)
{
    if (setVal(proxyType, newValue)) {
        emit proxyTypeChanged(newValue);
    }
}

QString Settings::getProxyAddr() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return proxyAddr;
}

void Settings::setProxyAddr(const QString& address)
{
    if (setVal(proxyAddr, address)) {
        emit proxyAddressChanged(address);
    }
}

quint16 Settings::getProxyPort() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return proxyPort;
}

void Settings::setProxyPort(quint16 port)
{
    if (setVal(proxyPort, port)) {
        emit proxyPortChanged(port);
    }
}

QString Settings::getCurrentProfile() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return currentProfile;
}

uint32_t Settings::getCurrentProfileId() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return currentProfileId;
}

void Settings::setCurrentProfile(const QString& profile)
{
    bool updated = false;
    uint32_t newProfileId = 0;
    {
        const QMutexLocker<QRecursiveMutex> locker{&bigLock};

        if (profile != currentProfile) {
            currentProfile = profile;
            currentProfileId = makeProfileId(currentProfile);
            newProfileId = currentProfileId;
            updated = true;
        }
    }
    if (updated) {
        emit currentProfileIdChanged(newProfileId);
    }
}

bool Settings::getEnableLogging() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return enableLogging;
}

void Settings::setEnableLogging(bool newValue)
{
    if (setVal(enableLogging, newValue)) {
        emit enableLoggingChanged(newValue);
    }
}

int Settings::getAutoAwayTime() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return autoAwayTime;
}

/**
 * @brief Sets how long the user may stay idle, before online status is set to "away".
 * @param[in] newValue  the user idle duration in minutes
 * @note Values < 0 default to 10 minutes.
 */
void Settings::setAutoAwayTime(int newValue)
{
    if (newValue < 0) {
        newValue = 10;
    }

    if (setVal(autoAwayTime, newValue)) {
        emit autoAwayTimeChanged(newValue);
    }
}

QString Settings::getAutoAcceptDir(const ToxPk& id) const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    auto it = friendLst.find(id.getByteArray());
    if (it != friendLst.end())
        return it->autoAcceptDir;

    return {};
}

void Settings::setAutoAcceptDir(const ToxPk& id, const QString& dir)
{
    bool updated = false;
    {
        const QMutexLocker<QRecursiveMutex> locker{&bigLock};

        auto& frnd = getOrInsertFriendPropRef(id);

        if (frnd.autoAcceptDir != dir) {
            frnd.autoAcceptDir = dir;
            updated = true;
        }
    }
    if (updated) {
        emit autoAcceptDirChanged(id, dir);
    }
}

Settings::AutoAcceptCallFlags Settings::getAutoAcceptCall(const ToxPk& id) const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    auto it = friendLst.find(id.getByteArray());
    if (it != friendLst.end())
        return it->autoAcceptCall;

    return {};
}

void Settings::setAutoAcceptCall(const ToxPk& id, AutoAcceptCallFlags accept)
{
    bool updated = false;
    {
        const QMutexLocker<QRecursiveMutex> locker{&bigLock};

        auto& frnd = getOrInsertFriendPropRef(id);

        if (frnd.autoAcceptCall != accept) {
            frnd.autoAcceptCall = accept;
            updated = true;
        }
    }
    if (updated) {
        emit autoAcceptCallChanged(id, accept);
    }
}

bool Settings::getAutoConferenceInvite(const ToxPk& id) const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    auto it = friendLst.find(id.getByteArray());
    if (it != friendLst.end()) {
        return it->autoConferenceInvite;
    }

    return false;
}

void Settings::setAutoConferenceInvite(const ToxPk& id, bool accept)
{
    bool updated = false;
    {
        const QMutexLocker<QRecursiveMutex> locker{&bigLock};

        auto& frnd = getOrInsertFriendPropRef(id);

        if (frnd.autoConferenceInvite != accept) {
            frnd.autoConferenceInvite = accept;
            updated = true;
        }
    }

    if (updated) {
        emit autoConferenceInviteChanged(id, accept);
    }
}

QString Settings::getContactNote(const ToxPk& id) const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    auto it = friendLst.find(id.getByteArray());
    if (it != friendLst.end())
        return it->note;

    return {};
}

void Settings::setContactNote(const ToxPk& id, const QString& note)
{
    bool updated = false;
    {
        const QMutexLocker<QRecursiveMutex> locker{&bigLock};

        auto& frnd = getOrInsertFriendPropRef(id);

        if (frnd.note != note) {
            frnd.note = note;
            updated = true;
        }
    }
    if (updated) {
        emit contactNoteChanged(id, note);
    }
}

QString Settings::getGlobalAutoAcceptDir() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return globalAutoAcceptDir;
}

void Settings::setGlobalAutoAcceptDir(const QString& newValue)
{
    if (setVal(globalAutoAcceptDir, newValue)) {
        emit globalAutoAcceptDirChanged(newValue);
    }
}

size_t Settings::getMaxAutoAcceptSize() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return autoAcceptMaxSize;
}

void Settings::setMaxAutoAcceptSize(size_t size)
{
    if (setVal(autoAcceptMaxSize, size)) {
        emit autoAcceptMaxSizeChanged(size);
    }
}

const QFont& Settings::getChatMessageFont() const
{
    const QMutexLocker<QRecursiveMutex> locker(&bigLock);
    return chatMessageFont;
}

void Settings::setChatMessageFont(const QFont& font)
{
    if (setVal(chatMessageFont, font)) {
        emit chatMessageFontChanged(font);
    }
}

void Settings::setWidgetData(const QString& uniqueName, const QByteArray& data)
{
    bool updated = false;
    {
        const QMutexLocker<QRecursiveMutex> locker{&bigLock};

        if (!widgetSettings.contains(uniqueName) || widgetSettings[uniqueName] != data) {
            widgetSettings[uniqueName] = data;
            updated = true;
        }
    }
    if (updated) {
        emit widgetDataChanged(uniqueName);
    }
}

QByteArray Settings::getWidgetData(const QString& uniqueName) const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return widgetSettings.value(uniqueName);
}

QString Settings::getSmileyPack() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return smileyPack;
}

void Settings::setSmileyPack(const QString& value)
{
    if (setVal(smileyPack, value)) {
        emit smileyPackChanged(value);
    }
}

int Settings::getEmojiFontPointSize() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return emojiFontPointSize;
}

void Settings::setEmojiFontPointSize(int value)
{
    if (setVal(emojiFontPointSize, value)) {
        emit emojiFontPointSizeChanged(value);
    }
}

const QString& Settings::getTimestampFormat() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return timestampFormat;
}

void Settings::setTimestampFormat(const QString& format)
{
    if (setVal(timestampFormat, format)) {
        emit timestampFormatChanged(format);
    }
}

const QString& Settings::getDateFormat() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return dateFormat;
}

void Settings::setDateFormat(const QString& format)
{
    if (setVal(dateFormat, format)) {
        emit dateFormatChanged(format);
    }
}

Settings::StyleType Settings::getStylePreference() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return stylePreference;
}

void Settings::setStylePreference(StyleType newValue)
{
    if (setVal(stylePreference, newValue)) {
        emit stylePreferenceChanged(newValue);
    }
}

QByteArray Settings::getWindowGeometry() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return windowGeometry;
}

void Settings::setWindowGeometry(const QByteArray& value)
{
    if (setVal(windowGeometry, value)) {
        emit windowGeometryChanged(value);
    }
}

QByteArray Settings::getWindowState() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return windowState;
}

void Settings::setWindowState(const QByteArray& value)
{
    if (setVal(windowState, value)) {
        emit windowStateChanged(value);
    }
}

bool Settings::getCheckUpdates() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return checkUpdates;
}

void Settings::setCheckUpdates(bool newValue)
{
    if (setVal(checkUpdates, newValue)) {
        emit checkUpdatesChanged(newValue);
    }
}

bool Settings::getNotify() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return notify;
}

void Settings::setNotify(bool newValue)
{
    if (setVal(notify, newValue)) {
        emit notifyChanged(newValue);
    }
}

bool Settings::getShowWindow() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return showWindow;
}

void Settings::setShowWindow(bool newValue)
{
    if (setVal(showWindow, newValue)) {
        emit showWindowChanged(newValue);
    }
}

bool Settings::getDesktopNotify() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return desktopNotify;
}

void Settings::setDesktopNotify(bool newValue)
{
    if (setVal(desktopNotify, newValue)) {
        emit desktopNotifyChanged(newValue);
    }
}

bool Settings::getNotifySystemBackend() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return notifySystemBackend;
}

void Settings::setNotifySystemBackend(bool newValue)
{
    if (setVal(notifySystemBackend, newValue)) {
        emit notifySystemBackendChanged(newValue);
    }
}

QByteArray Settings::getSplitterState() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return splitterState;
}

void Settings::setSplitterState(const QByteArray& value)
{
    if (setVal(splitterState, value)) {
        emit splitterStateChanged(value);
    }
}

QByteArray Settings::getDialogGeometry() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return dialogGeometry;
}

void Settings::setDialogGeometry(const QByteArray& value)
{
    if (setVal(dialogGeometry, value)) {
        emit dialogGeometryChanged(value);
    }
}

QByteArray Settings::getDialogSplitterState() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return dialogSplitterState;
}

void Settings::setDialogSplitterState(const QByteArray& value)
{
    if (setVal(dialogSplitterState, value)) {
        emit dialogSplitterStateChanged(value);
    }
}

QByteArray Settings::getDialogSettingsGeometry() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return dialogSettingsGeometry;
}

void Settings::setDialogSettingsGeometry(const QByteArray& value)
{
    if (setVal(dialogSettingsGeometry, value)) {
        emit dialogSettingsGeometryChanged(value);
    }
}

bool Settings::getMinimizeOnClose() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return minimizeOnClose;
}

void Settings::setMinimizeOnClose(bool newValue)
{
    if (setVal(minimizeOnClose, newValue)) {
        emit minimizeOnCloseChanged(newValue);
    }
}

bool Settings::getTypingNotification() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return typingNotification;
}

void Settings::setTypingNotification(bool enabled)
{
    if (setVal(typingNotification, enabled)) {
        emit typingNotificationChanged(enabled);
    }
}

QStringList Settings::getBlockList() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return blockList;
}

void Settings::setBlockList(const QStringList& blist)
{
    if (setVal(blockList, blist)) {
        emit blockListChanged(blist);
    }
}

QString Settings::getInDev() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return inDev;
}

void Settings::setInDev(const QString& deviceSpecifier)
{
    if (setVal(inDev, deviceSpecifier)) {
        emit inDevChanged(deviceSpecifier);
    }
}

bool Settings::getAudioInDevEnabled() const
{
    const QMutexLocker<QRecursiveMutex> locker(&bigLock);
    return audioInDevEnabled;
}

void Settings::setAudioInDevEnabled(bool enabled)
{
    if (setVal(audioInDevEnabled, enabled)) {
        emit audioInDevEnabledChanged(enabled);
    }
}

qreal Settings::getAudioInGainDecibel() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return audioInGainDecibel;
}

void Settings::setAudioInGainDecibel(qreal dB)
{
    if (setVal(audioInGainDecibel, dB)) {
        emit audioInGainDecibelChanged(audioInGainDecibel);
    }
}

qreal Settings::getAudioThreshold() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return audioThreshold;
}

void Settings::setAudioThreshold(qreal percent)
{
    if (setVal(audioThreshold, percent)) {
        emit audioThresholdChanged(percent);
    }
}

QString Settings::getVideoDev() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return videoDev;
}

void Settings::setVideoDev(const QString& deviceSpecifier)
{
    if (setVal(videoDev, deviceSpecifier)) {
        emit videoDevChanged(deviceSpecifier);
    }
}

QString Settings::getOutDev() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return outDev;
}

void Settings::setOutDev(const QString& deviceSpecifier)
{
    if (setVal(outDev, deviceSpecifier)) {
        emit outDevChanged(deviceSpecifier);
    }
}

bool Settings::getAudioOutDevEnabled() const
{
    const QMutexLocker<QRecursiveMutex> locker(&bigLock);
    return audioOutDevEnabled;
}

void Settings::setAudioOutDevEnabled(bool enabled)
{
    if (setVal(audioOutDevEnabled, enabled)) {
        emit audioOutDevEnabledChanged(enabled);
    }
}

int Settings::getOutVolume() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return outVolume;
}

void Settings::setOutVolume(int volume)
{
    if (setVal(outVolume, volume)) {
        emit outVolumeChanged(volume);
    }
}

int Settings::getAudioBitrate() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return audioBitrate;
}

void Settings::setAudioBitrate(int bitrate)
{
    if (setVal(audioBitrate, bitrate)) {
        emit audioBitrateChanged(bitrate);
    }
}

QRect Settings::getScreenRegion() const
{
    const QMutexLocker<QRecursiveMutex> locker(&bigLock);
    return screenRegion;
}

void Settings::setScreenRegion(const QRect& value)
{
    if (setVal(screenRegion, value)) {
        emit screenRegionChanged(value);
    }
}

bool Settings::getScreenGrabbed() const
{
    const QMutexLocker<QRecursiveMutex> locker(&bigLock);
    return screenGrabbed;
}

void Settings::setScreenGrabbed(bool value)
{
    if (setVal(screenGrabbed, value)) {
        emit screenGrabbedChanged(value);
    }
}

QRect Settings::getCamVideoRes() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return camVideoRes;
}

void Settings::setCamVideoRes(QRect newValue)
{
    if (setVal(camVideoRes, newValue)) {
        emit camVideoResChanged(newValue);
    }
}

float Settings::getCamVideoFPS() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return camVideoFPS;
}

void Settings::setCamVideoFPS(float newValue)
{
    if (setVal(camVideoFPS, newValue)) {
        emit camVideoFPSChanged(newValue);
    }
}

void Settings::updateFriendAddress(const QString& newAddr)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    auto key = ToxPk(newAddr);
    auto& frnd = getOrInsertFriendPropRef(key);
    frnd.addr = newAddr;
}

QString Settings::getFriendAlias(const ToxPk& id) const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    auto it = friendLst.find(id.getByteArray());
    if (it != friendLst.end())
        return it->alias;

    return {};
}

void Settings::setFriendAlias(const ToxPk& id, const QString& alias)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    auto& frnd = getOrInsertFriendPropRef(id);
    frnd.alias = alias;
}

int Settings::getFriendCircleID(const ToxPk& id) const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    auto it = friendLst.find(id.getByteArray());
    if (it != friendLst.end())
        return it->circleID;

    return -1;
}

void Settings::setFriendCircleID(const ToxPk& id, int circleID)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    auto& frnd = getOrInsertFriendPropRef(id);
    frnd.circleID = circleID;
}

QDateTime Settings::getFriendActivity(const ToxPk& id) const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    auto it = friendLst.find(id.getByteArray());
    if (it != friendLst.end())
        return it->activity;

    return {};
}

void Settings::setFriendActivity(const ToxPk& id, const QDateTime& activity)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    auto& frnd = getOrInsertFriendPropRef(id);
    frnd.activity = activity;
}

void Settings::saveFriendSettings(const ToxPk& id)
{
    std::ignore = id;
    savePersonal();
}

void Settings::removeFriendSettings(const ToxPk& id)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    friendLst.remove(id.getByteArray());
}

bool Settings::getCompactLayout() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return compactLayout;
}

void Settings::setCompactLayout(bool value)
{
    if (setVal(compactLayout, value)) {
        emit compactLayoutChanged(value);
    }
}

Settings::FriendListSortingMode Settings::getFriendSortingMode() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return sortingMode;
}

void Settings::setFriendSortingMode(FriendListSortingMode mode)
{
    if (setVal(sortingMode, mode)) {
        emit sortingModeChanged(mode);
    }
}

bool Settings::getSeparateWindow() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return separateWindow;
}

void Settings::setSeparateWindow(bool value)
{
    if (setVal(separateWindow, value)) {
        emit separateWindowChanged(value);
    }
}

bool Settings::getDontGroupWindows() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return dontGroupWindows;
}

void Settings::setDontGroupWindows(bool value)
{
    if (setVal(dontGroupWindows, value)) {
        emit dontGroupWindowsChanged(value);
    }
}

bool Settings::getConferencePosition() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return conferencePosition;
}

void Settings::setConferencePosition(bool value)
{
    if (setVal(conferencePosition, value)) {
        emit conferencePositionChanged(value);
    }
}

bool Settings::getShowIdenticons() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return showIdenticons;
}

void Settings::setShowIdenticons(bool value)
{
    if (setVal(showIdenticons, value)) {
        emit showIdenticonsChanged(value);
    }
}

int Settings::getCircleCount() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return circleLst.size();
}

QString Settings::getCircleName(int id) const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return circleLst[id].name;
}

void Settings::setCircleName(int id, const QString& name)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    circleLst[id].name = name;
    savePersonal();
}

int Settings::addCircle(const QString& name)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    circleProp cp;
    cp.expanded = false;

    if (name.isEmpty())
        cp.name = tr("Circle #%1").arg(circleLst.count() + 1);
    else
        cp.name = name;

    circleLst.append(cp);
    savePersonal();
    return circleLst.count() - 1;
}

bool Settings::getCircleExpanded(int id) const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return circleLst[id].expanded;
}

void Settings::setCircleExpanded(int id, bool expanded)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    circleLst[id].expanded = expanded;
}

bool Settings::addFriendRequest(const QString& friendAddress, const QString& message)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    for (auto queued : friendRequests) {
        if (queued.address == friendAddress) {
            queued.message = message;
            queued.read = false;
            return false;
        }
    }

    Request request;
    request.address = friendAddress;
    request.message = message;
    request.read = false;

    friendRequests.push_back(request);
    return true;
}

unsigned int Settings::getUnreadFriendRequests() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    unsigned int unreadFriendRequests = 0;
    for (auto request : friendRequests)
        if (!request.read)
            ++unreadFriendRequests;

    return unreadFriendRequests;
}

Settings::Request Settings::getFriendRequest(int index) const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return friendRequests.at(index);
}

int Settings::getFriendRequestSize() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return friendRequests.size();
}

void Settings::clearUnreadFriendRequests()
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    for (auto& request : friendRequests)
        request.read = true;
}

void Settings::removeFriendRequest(int index)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    friendRequests.removeAt(index);
}

void Settings::readFriendRequest(int index)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    friendRequests[index].read = true;
}

int Settings::removeCircle(int id)
{
    // Replace index with last one and remove last one instead.
    // This gives you contiguous ids all the time.
    circleLst[id] = circleLst.last();
    circleLst.pop_back();
    savePersonal();
    return circleLst.count();
}

int Settings::getThemeColor() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return themeColor;
}

void Settings::setThemeColor(int value)
{
    if (setVal(themeColor, value)) {
        emit themeColorChanged(value);
    }
}

bool Settings::getAutoLogin() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return autoLogin;
}

void Settings::setAutoLogin(bool state)
{
    if (setVal(autoLogin, state)) {
        emit autoLoginChanged(state);
    }
}

void Settings::setEnableConferencesColor(bool state)
{
    if (setVal(nameColors, state)) {
        emit nameColorsChanged(state);
    }
}

bool Settings::getEnableConferencesColor() const
{
    return nameColors;
}

bool Settings::getImagePreview() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return imagePreview;
}

void Settings::setImagePreview(bool newValue)
{
    if (setVal(imagePreview, newValue)) {
        emit imagePreviewChanged(newValue);
    }
}

int Settings::getChatMaxWindowSize() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return chatMaxWindowSize;
}

void Settings::setChatMaxWindowSize(int value)
{
    if (setVal(chatMaxWindowSize, value)) {
        emit chatMaxWindowSizeChanged(value);
    }
}

int Settings::getChatWindowChunkSize() const
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    return chatWindowChunkSize;
}

void Settings::setChatWindowChunkSize(int value)
{
    if (setVal(chatWindowChunkSize, value)) {
        emit chatWindowChunkSizeChanged(value);
    }
}

/**
 * @brief Write a default personal .ini settings file for a profile.
 * @param basename Filename without extension to save settings.
 *
 * @note If basename is "profile", settings will be saved in profile.ini
 */
void Settings::createPersonal(const Paths& paths, const QString& basename)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    const QString path = paths.getSettingsDirPath() + QDir::separator() + basename + ".ini";
    qDebug() << "Creating new profile settings in" << path;

    QSettings ps(path, QSettings::IniFormat);
    inGroup(ps, "Friends", [&ps] { //
        inArray(ps, "Friend", 0, [] {});
    });

    inGroup(ps, "Privacy", [] {});
}

/**
 * @brief Creates a path to the settings dir, if it doesn't already exist
 */
void Settings::createSettingsDir()
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};

    const QString dir = paths.getSettingsDirPath();
    const QDir directory(dir);
    if (!directory.exists() && !directory.mkpath(directory.absolutePath()))
        qCritical() << "Error while creating directory" << dir;
}

/**
 * @brief Waits for all asynchronous operations to complete
 */
void Settings::sync()
{
    if (QThread::currentThread() != settingsThread) {
        QMetaObject::invokeMethod(this, "sync", Qt::BlockingQueuedConnection);
        return;
    }

    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    qApp->processEvents();
}

Settings::friendProp& Settings::getOrInsertFriendPropRef(const ToxPk& id)
{
    // No mutex lock, this is a private fn that should only be called by other
    // public functions that already locked the mutex
    auto it = friendLst.find(id.getByteArray());
    if (it != friendLst.end()) {
        return *it;
    }

    return *friendLst.insert(id.getByteArray(), friendProp{id.toString()});
}

ICoreSettings::ProxyType Settings::fixInvalidProxyType(ICoreSettings::ProxyType proxyType)
{
    // Repair uninitialized enum that was saved to settings due to bug (https://github.com/qTox/qTox/issues/5311)
    switch (proxyType) {
    case ICoreSettings::ProxyType::ptNone:
    case ICoreSettings::ProxyType::ptSOCKS5:
    case ICoreSettings::ProxyType::ptHTTP:
        return proxyType;
    default:
        qWarning() << "Repairing invalid ProxyType, UDP will be enabled";
        return ICoreSettings::ProxyType::ptNone;
    }
}

template <typename T>
bool Settings::setVal(T& savedVal, T newVal)
{
    const QMutexLocker<QRecursiveMutex> locker{&bigLock};
    if (savedVal != newVal) {
        savedVal = newVal;
        return true;
    }
    return false;
}
