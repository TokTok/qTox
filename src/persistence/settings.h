/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "audio/iaudiosettings.h"
#include "src/core/icoresettings.h"
#include "src/core/idebugsettings.h"
#include "src/core/toxencrypt.h"
#include "src/persistence/iconferencesettings.h"
#include "src/persistence/ifriendsettings.h"
#include "src/persistence/inotificationsettings.h"
#include "src/persistence/ismileysettings.h"
#include "src/persistence/paths.h"
#include "src/video/ivideosettings.h"

#include <QDateTime>
#include <QFlags>
#include <QFont>
#include <QHash>
#include <QMutex>
#include <QNetworkProxy>
#include <QObject>
#include <QPixmap>

#include <utility>

class Profile;
class QCommandLineParser;
class IMessageBoxManager;

namespace Db {
enum class syncType;
}

class Settings : public QObject,
                 public ICoreSettings,
                 public IDebugSettings,
                 public IFriendSettings,
                 public IConferenceSettings,
                 public IAudioSettings,
                 public IVideoSettings,
                 public INotificationSettings,
                 public ISmileySettings
{
    Q_OBJECT

    Q_ENUMS(StyleType)

    // general
    Q_PROPERTY(bool compactLayout READ getCompactLayout WRITE setCompactLayout NOTIFY compactLayoutChanged FINAL)
    Q_PROPERTY(bool autorun READ getAutorun WRITE setAutorun NOTIFY autorunChanged FINAL)

    // GUI
    Q_PROPERTY(bool separateWindow READ getSeparateWindow WRITE setSeparateWindow NOTIFY
                   separateWindowChanged FINAL)
    Q_PROPERTY(QString smileyPack READ getSmileyPack WRITE setSmileyPack NOTIFY smileyPackChanged FINAL)
    Q_PROPERTY(int emojiFontPointSize READ getEmojiFontPointSize WRITE setEmojiFontPointSize NOTIFY
                   emojiFontPointSizeChanged FINAL)
    Q_PROPERTY(bool minimizeOnClose READ getMinimizeOnClose WRITE setMinimizeOnClose NOTIFY
                   minimizeOnCloseChanged FINAL)
    Q_PROPERTY(QByteArray windowGeometry READ getWindowGeometry WRITE setWindowGeometry NOTIFY
                   windowGeometryChanged FINAL)
    Q_PROPERTY(QByteArray windowState READ getWindowState WRITE setWindowState NOTIFY windowStateChanged FINAL)
    Q_PROPERTY(QByteArray splitterState READ getSplitterState WRITE setSplitterState NOTIFY
                   splitterStateChanged FINAL)
    Q_PROPERTY(QByteArray dialogGeometry READ getDialogGeometry WRITE setDialogGeometry NOTIFY
                   dialogGeometryChanged FINAL)
    Q_PROPERTY(QByteArray dialogSplitterState READ getDialogSplitterState WRITE
                   setDialogSplitterState NOTIFY dialogSplitterStateChanged FINAL)
    Q_PROPERTY(QByteArray dialogSettingsGeometry READ getDialogSettingsGeometry WRITE
                   setDialogSettingsGeometry NOTIFY dialogSettingsGeometryChanged FINAL)
    Q_PROPERTY(QString style READ getStyle WRITE setStyle NOTIFY styleChanged FINAL)
    Q_PROPERTY(bool showSystemTray READ getShowSystemTray WRITE setShowSystemTray NOTIFY
                   showSystemTrayChanged FINAL)
    Q_PROPERTY(bool showIdenticons READ getShowIdenticons WRITE setShowIdenticons NOTIFY
                   showIdenticonsChanged FINAL)

    // ChatView
    Q_PROPERTY(bool conferencePosition READ getConferencePosition WRITE setConferencePosition NOTIFY
                   conferencePositionChanged FINAL)
    Q_PROPERTY(QFont chatMessageFont READ getChatMessageFont WRITE setChatMessageFont NOTIFY
                   chatMessageFontChanged FINAL)
    Q_PROPERTY(StyleType stylePreference READ getStylePreference WRITE setStylePreference NOTIFY
                   stylePreferenceChanged FINAL)
    Q_PROPERTY(QString timestampFormat READ getTimestampFormat WRITE setTimestampFormat NOTIFY
                   timestampFormatChanged FINAL)
    Q_PROPERTY(QString dateFormat READ getDateFormat WRITE setDateFormat NOTIFY dateFormatChanged FINAL)
    Q_PROPERTY(bool statusChangeNotificationEnabled READ getStatusChangeNotificationEnabled WRITE
                   setStatusChangeNotificationEnabled NOTIFY statusChangeNotificationEnabledChanged FINAL)
    Q_PROPERTY(bool spellCheckingEnabled READ getSpellCheckingEnabled WRITE setSpellCheckingEnabled
                   NOTIFY spellCheckingEnabledChanged FINAL)

    // Privacy
    Q_PROPERTY(bool typingNotification READ getTypingNotification WRITE setTypingNotification NOTIFY
                   typingNotificationChanged FINAL)
    Q_PROPERTY(QStringList blockList READ getBlockList WRITE setBlockList NOTIFY blockListChanged FINAL)

    // Audio
    Q_PROPERTY(QString inDev READ getInDev WRITE setInDev NOTIFY inDevChanged FINAL)
    Q_PROPERTY(bool audioInDevEnabled READ getAudioInDevEnabled WRITE setAudioInDevEnabled NOTIFY
                   audioInDevEnabledChanged FINAL)
    Q_PROPERTY(qreal audioInGainDecibel READ getAudioInGainDecibel WRITE setAudioInGainDecibel
                   NOTIFY audioInGainDecibelChanged FINAL)
    Q_PROPERTY(qreal audioThreshold READ getAudioThreshold WRITE setAudioThreshold NOTIFY
                   audioThresholdChanged FINAL)
    Q_PROPERTY(QString outDev READ getOutDev WRITE setOutDev NOTIFY outDevChanged FINAL)
    Q_PROPERTY(bool audioOutDevEnabled READ getAudioOutDevEnabled WRITE setAudioOutDevEnabled NOTIFY
                   audioOutDevEnabledChanged FINAL)
    Q_PROPERTY(int outVolume READ getOutVolume WRITE setOutVolume NOTIFY outVolumeChanged FINAL)
    Q_PROPERTY(int audioBitrate READ getAudioBitrate WRITE setAudioBitrate NOTIFY audioBitrateChanged FINAL)

    // Video
    Q_PROPERTY(QString videoDev READ getVideoDev WRITE setVideoDev NOTIFY videoDevChanged FINAL)
    Q_PROPERTY(QRect camVideoRes READ getCamVideoRes WRITE setCamVideoRes NOTIFY camVideoResChanged FINAL)
    Q_PROPERTY(QRect screenRegion READ getScreenRegion WRITE setScreenRegion NOTIFY screenRegionChanged FINAL)
    Q_PROPERTY(bool screenGrabbed READ getScreenGrabbed WRITE setScreenGrabbed NOTIFY screenGrabbedChanged FINAL)
    Q_PROPERTY(float camVideoFPS READ getCamVideoFPS WRITE setCamVideoFPS NOTIFY camVideoFPSChanged FINAL)

public:
    enum class StyleType
    {
        NONE = 0,
        WITH_CHARS = 1,
        WITHOUT_CHARS = 2
    };

    enum class FriendListSortingMode
    {
        Name,
        Activity,
    };

public:
    explicit Settings(IMessageBoxManager& messageBoxManager);
    ~Settings() override;
    Settings(Settings& settings) = delete;
    Settings& operator=(const Settings&) = delete;

    Paths& getPaths();
    void createSettingsDir();
    static void createPersonal(const Paths& paths, const QString& basename);

    void savePersonal();

    void loadGlobal();
    void loadPersonal(const Profile& profile, bool newProfile);

    void resetToDefault();

    struct Request
    {
        QString address;
        QString message;
        bool read;
    };

public slots:
    void saveGlobal();
    void sync();
    void setAutoLogin(bool state);
    void updateProfileData(Profile& profile, const QCommandLineParser* parser, bool newProfile);

signals:
    // General
    void autorunChanged(bool enabled);
    void autoSaveEnabledChanged(bool enabled);
    void autostartInTrayChanged(bool enabled);
    void closeToTrayChanged(bool enabled);
    void lightTrayIconChanged(bool enabled);
    void minimizeToTrayChanged(bool enabled);
    void notifyChanged(bool enabled);
    void desktopNotifyChanged(bool enabled);
    void notifySystemBackendChanged(bool enabled);
    void showWindowChanged(bool enabled);
    void makeToxPortableChanged(bool enabled);
    void busySoundChanged(bool enabled);
    void notifySoundChanged(bool enabled);
    void notifyHideChanged(bool enabled);
    void conferenceAlwaysNotifyChanged(bool enabled);
    void translationChanged(const QString& translation);
    void currentProfileIdChanged(quint32 id);
    void enableLoggingChanged(bool enabled);
    void autoAwayTimeChanged(int minutes);
    void globalAutoAcceptDirChanged(const QString& path);
    void autoAcceptMaxSizeChanged(size_t size);
    void checkUpdatesChanged(bool enabled);
    void widgetDataChanged(const QString& key);

    // GUI
    void autoLoginChanged(bool enabled);
    void separateWindowChanged(bool enabled);
    void showSystemTrayChanged(bool enabled);
    bool minimizeOnCloseChanged(bool enabled);
    void windowGeometryChanged(const QByteArray& rect);
    void windowStateChanged(const QByteArray& state);
    void splitterStateChanged(const QByteArray& state);
    void dialogGeometryChanged(const QByteArray& rect);
    void dialogSplitterStateChanged(const QByteArray& state);
    void dialogSettingsGeometryChanged(const QByteArray& rect);
    void styleChanged(const QString& style);
    void themeColorChanged(int color);
    void compactLayoutChanged(bool enabled);
    void sortingModeChanged(FriendListSortingMode mode);
    void showIdenticonsChanged(bool enabled);
    void nameColorsChanged(bool enabled);
    void imagePreviewChanged(bool enabled);
    void chatMaxWindowSizeChanged(int max);
    void chatWindowChunkSizeChanged(int chunk);

    // ChatView
    void useEmoticonsChanged(bool enabled);
    void emojiFontPointSizeChanged(int size);
    void dontGroupWindowsChanged(bool enabled);
    void conferencePositionChanged(bool enabled);
    void chatMessageFontChanged(const QFont& font);
    void stylePreferenceChanged(StyleType type);
    void timestampFormatChanged(const QString& format);
    void dateFormatChanged(const QString& format);
    void statusChangeNotificationEnabledChanged(bool enabled);
    void spellCheckingEnabledChanged(bool enabled);
    void hidePostNullSuffixChanged(bool show);

    // Privacy
    void typingNotificationChanged(bool enabled);
    void dbSyncTypeChanged(Db::syncType type);

public:
    bool applyCommandLineOptions(const QCommandLineParser& parser);
    static bool verifyProxySettings(const QCommandLineParser& parser);

    bool getMakeToxPortable() const;
    void setMakeToxPortable(bool newValue);

    bool getAutorun() const;
    void setAutorun(bool newValue);

    bool getAutostartInTray() const;
    void setAutostartInTray(bool newValue);

    bool getCloseToTray() const;
    void setCloseToTray(bool newValue);

    bool getMinimizeToTray() const;
    void setMinimizeToTray(bool newValue);

    bool getLightTrayIcon() const;
    void setLightTrayIcon(bool newValue);

    QString getStyle() const;
    void setStyle(const QString& newStyle);

    bool getShowSystemTray() const;
    void setShowSystemTray(bool newValue);

    bool getUseEmoticons() const;
    void setUseEmoticons(bool newValue);

    QString getCurrentProfile() const;
    uint32_t getCurrentProfileId() const;
    void setCurrentProfile(const QString& profile);

    QString getTranslation() const;
    void setTranslation(const QString& newValue);

    void setAutoSaveEnabled(bool newValue);
    bool getAutoSaveEnabled() const;

    SIGNAL_IMPL(Settings, enableDebugChanged, bool enabled)

    void setEnableDebug(bool newValue) override;
    bool getEnableDebug() const override;

    // ICoreSettings
    bool getEnableIPv6() const override;
    void setEnableIPv6(bool enabled) override;

    bool getForceTCP() const override;
    void setForceTCP(bool enabled) override;

    bool getEnableLanDiscovery() const override;
    void setEnableLanDiscovery(bool enabled) override;

    QString getProxyAddr() const override;
    void setProxyAddr(const QString& address) override;

    ICoreSettings::ProxyType getProxyType() const override;
    void setProxyType(ICoreSettings::ProxyType newValue) override;

    quint16 getProxyPort() const override;
    void setProxyPort(quint16 port) override;

    QNetworkProxy getProxy() const override;

    SIGNAL_IMPL(Settings, enableIPv6Changed, bool enabled)
    SIGNAL_IMPL(Settings, forceTCPChanged, bool enabled)
    SIGNAL_IMPL(Settings, enableLanDiscoveryChanged, bool enabled)
    SIGNAL_IMPL(Settings, proxyTypeChanged, ICoreSettings::ProxyType type)
    SIGNAL_IMPL(Settings, proxyAddressChanged, const QString& address)
    SIGNAL_IMPL(Settings, proxyPortChanged, quint16 port)

    bool getEnableLogging() const;
    void setEnableLogging(bool newValue);

    Db::syncType getDbSyncType() const;
    void setDbSyncType(Db::syncType newValue);

    int getAutoAwayTime() const;
    void setAutoAwayTime(int newValue);

    bool getCheckUpdates() const;
    void setCheckUpdates(bool newValue);

    bool getNotify() const override;
    void setNotify(bool newValue) override;

    bool getShowWindow() const override;
    void setShowWindow(bool newValue) override;

    bool getDesktopNotify() const override;
    void setDesktopNotify(bool newValue) override;

    bool getNotifySystemBackend() const override;
    void setNotifySystemBackend(bool newValue) override;

    bool getNotifySound() const override;
    void setNotifySound(bool newValue) override;

    bool getNotifyHide() const override;
    void setNotifyHide(bool newValue) override;

    bool getBusySound() const override;
    void setBusySound(bool newValue) override;

    bool getConferenceAlwaysNotify() const override;
    void setConferenceAlwaysNotify(bool newValue) override;

    QString getInDev() const override;
    void setInDev(const QString& deviceSpecifier) override;

    bool getAudioInDevEnabled() const override;
    void setAudioInDevEnabled(bool enabled) override;

    QString getOutDev() const override;
    void setOutDev(const QString& deviceSpecifier) override;

    bool getAudioOutDevEnabled() const override;
    void setAudioOutDevEnabled(bool enabled) override;

    qreal getAudioInGainDecibel() const override;
    void setAudioInGainDecibel(qreal dB) override;

    qreal getAudioThreshold() const override;
    void setAudioThreshold(qreal percent) override;

    int getOutVolume() const override;
    int getOutVolumeMin() const override
    {
        return 0;
    }
    int getOutVolumeMax() const override
    {
        return 100;
    }
    void setOutVolume(int volume) override;

    int getAudioBitrate() const override;
    void setAudioBitrate(int bitrate) override;

    bool getEnableTestSound() const override;
    void setEnableTestSound(bool newValue) override;

    SIGNAL_IMPL(Settings, inDevChanged, const QString& device)
    SIGNAL_IMPL(Settings, audioInDevEnabledChanged, bool enabled)

    SIGNAL_IMPL(Settings, outDevChanged, const QString& device)
    SIGNAL_IMPL(Settings, audioOutDevEnabledChanged, bool enabled)

    SIGNAL_IMPL(Settings, audioInGainDecibelChanged, qreal dB)
    SIGNAL_IMPL(Settings, audioThresholdChanged, qreal percent)
    SIGNAL_IMPL(Settings, outVolumeChanged, int volume)
    SIGNAL_IMPL(Settings, audioBitrateChanged, int bitrate)
    SIGNAL_IMPL(Settings, enableTestSoundChanged, bool newValue)

    QString getVideoDev() const override;
    void setVideoDev(const QString& deviceSpecifier) override;

    QRect getScreenRegion() const override;
    void setScreenRegion(const QRect& value) override;

    bool getScreenGrabbed() const override;
    void setScreenGrabbed(bool value) override;

    QRect getCamVideoRes() const override;
    void setCamVideoRes(QRect newValue) override;

    float getCamVideoFPS() const override;
    void setCamVideoFPS(float newValue) override;

    SIGNAL_IMPL(Settings, videoDevChanged, const QString& device)
    SIGNAL_IMPL(Settings, screenRegionChanged, const QRect& region)
    SIGNAL_IMPL(Settings, screenGrabbedChanged, bool enabled)
    SIGNAL_IMPL(Settings, camVideoResChanged, const QRect& region)
    SIGNAL_IMPL(Settings, camVideoFPSChanged, unsigned short fps)

    bool isAnimationEnabled() const;
    void setAnimationEnabled(bool newValue);

    QString getSmileyPack() const override;
    void setSmileyPack(const QString& value);
    SIGNAL_IMPL(Settings, smileyPackChanged, const QString& name)

    int getThemeColor() const;
    void setThemeColor(int value);

    StyleType getStylePreference() const;
    void setStylePreference(StyleType newValue);

    int getEmojiFontPointSize() const;
    void setEmojiFontPointSize(int value);

    QString getContactNote(const ToxPk& id) const override;
    void setContactNote(const ToxPk& id, const QString& note) override;

    QString getAutoAcceptDir(const ToxPk& id) const override;
    void setAutoAcceptDir(const ToxPk& id, const QString& dir) override;

    AutoAcceptCallFlags getAutoAcceptCall(const ToxPk& id) const override;
    void setAutoAcceptCall(const ToxPk& id, AutoAcceptCallFlags accept) override;

    QString getGlobalAutoAcceptDir() const;
    void setGlobalAutoAcceptDir(const QString& newValue);

    size_t getMaxAutoAcceptSize() const;
    void setMaxAutoAcceptSize(size_t size);

    bool getAutoConferenceInvite(const ToxPk& id) const override;
    void setAutoConferenceInvite(const ToxPk& id, bool accept) override;

    // ChatView
    const QFont& getChatMessageFont() const;
    void setChatMessageFont(const QFont& font);

    const QString& getTimestampFormat() const;
    void setTimestampFormat(const QString& format);

    const QString& getDateFormat() const;
    void setDateFormat(const QString& format);

    bool getMinimizeOnClose() const;
    void setMinimizeOnClose(bool newValue);

    bool getStatusChangeNotificationEnabled() const;
    void setStatusChangeNotificationEnabled(bool newValue);

    bool getSpellCheckingEnabled() const;
    void setSpellCheckingEnabled(bool newValue);

    bool getHidePostNullSuffix() const override;
    void setHidePostNullSuffix(bool hide) override;

    // Privacy
    bool getTypingNotification() const;
    void setTypingNotification(bool enabled);

    QStringList getBlockList() const override;
    void setBlockList(const QStringList& blist) override;
    SIGNAL_IMPL(Settings, blockListChanged, const QStringList& blist)

    bool getShowConferenceJoinLeaveMessages() const override;
    void setShowConferenceJoinLeaveMessages(bool newValue) override;
    SIGNAL_IMPL(Settings, showConferenceJoinLeaveMessagesChanged, bool show)

    // State
    QByteArray getWindowGeometry() const;
    void setWindowGeometry(const QByteArray& value);

    QByteArray getWindowState() const;
    void setWindowState(const QByteArray& value);

    QByteArray getSplitterState() const;
    void setSplitterState(const QByteArray& value);

    QByteArray getDialogGeometry() const;
    void setDialogGeometry(const QByteArray& value);

    QByteArray getDialogSplitterState() const;
    void setDialogSplitterState(const QByteArray& value);

    QByteArray getDialogSettingsGeometry() const;
    void setDialogSettingsGeometry(const QByteArray& value);

    void updateFriendAddress(const QString& newAddr);

    QString getFriendAlias(const ToxPk& id) const override;
    void setFriendAlias(const ToxPk& id, const QString& alias) override;

    int getFriendCircleID(const ToxPk& id) const override;
    void setFriendCircleID(const ToxPk& id, int circleID) override;

    QDateTime getFriendActivity(const ToxPk& id) const override;
    void setFriendActivity(const ToxPk& id, const QDateTime& activity) override;

    void saveFriendSettings(const ToxPk& id) override;
    void removeFriendSettings(const ToxPk& id) override;

    SIGNAL_IMPL(Settings, autoAcceptCallChanged, const ToxPk& id,
                IFriendSettings::AutoAcceptCallFlags accept)
    SIGNAL_IMPL(Settings, autoConferenceInviteChanged, const ToxPk& id, bool accept)
    SIGNAL_IMPL(Settings, autoAcceptDirChanged, const ToxPk& id, const QString& dir)
    SIGNAL_IMPL(Settings, contactNoteChanged, const ToxPk& id, const QString& note)

    bool getCompactLayout() const;
    void setCompactLayout(bool value);

    FriendListSortingMode getFriendSortingMode() const;
    void setFriendSortingMode(FriendListSortingMode mode);

    bool getSeparateWindow() const;
    void setSeparateWindow(bool value);

    bool getDontGroupWindows() const;
    void setDontGroupWindows(bool value);

    bool getConferencePosition() const;
    void setConferencePosition(bool value);

    bool getShowIdenticons() const;
    void setShowIdenticons(bool value);

    bool getImagePreview() const;
    void setImagePreview(bool newValue);

    // Maximum number of rendered messages at any given time
    int getChatMaxWindowSize() const;
    void setChatMaxWindowSize(int value);

    // Amount of messages to purge when removing messages
    int getChatWindowChunkSize() const;
    void setChatWindowChunkSize(int value);

    bool getAutoLogin() const;
    void setEnableConferencesColor(bool state);
    bool getEnableConferencesColor() const;

    int getCircleCount() const;
    int addCircle(const QString& name = QString());
    int removeCircle(int id);
    QString getCircleName(int id) const;
    void setCircleName(int id, const QString& name);
    bool getCircleExpanded(int id) const;
    void setCircleExpanded(int id, bool expanded);

    bool addFriendRequest(const QString& friendAddress, const QString& message);
    unsigned int getUnreadFriendRequests() const;
    Request getFriendRequest(int index) const;
    int getFriendRequestSize() const;
    void clearUnreadFriendRequests();
    void removeFriendRequest(int index);
    void readFriendRequest(int index);

    QByteArray getWidgetData(const QString& uniqueName) const;
    void setWidgetData(const QString& uniqueName, const QByteArray& data);

    // Wrappers around getWidgetData() and setWidgetData()
    // Assume widget has a unique objectName set
    template <class T>
    void restoreGeometryState(T* widget) const
    {
        widget->restoreGeometry(getWidgetData(widget->objectName() + "Geometry"));
        widget->restoreState(getWidgetData(widget->objectName() + "State"));
    }
    template <class T>
    void saveGeometryState(const T* widget)
    {
        setWidgetData(widget->objectName() + "Geometry", widget->saveGeometry());
        setWidgetData(widget->objectName() + "State", widget->saveState());
    }

    static uint32_t makeProfileId(const QString& profile);

private:
    struct FriendProp;
    FriendProp& getOrInsertFriendPropRef(const ToxPk& id);
    static ICoreSettings::ProxyType fixInvalidProxyType(ICoreSettings::ProxyType proxyType);

    template <typename T>
    bool setVal(T& savedVal, T newVal);

private slots:
    void savePersonal(QString profileName, const ToxEncrypt* passkey);

private:
    bool loaded;

    bool useCustomDhtList;
    int dhtServerId;
    bool dontShowDhtDialog;

    bool autoLogin;
    bool compactLayout;
    FriendListSortingMode sortingMode;
    bool conferencePosition;
    bool separateWindow;
    bool dontGroupWindows;
    bool showIdenticons;
    bool enableIPv6;
    QString translation;
    bool autostartInTray;
    bool closeToTray;
    bool minimizeToTray;
    bool lightTrayIcon;
    bool useEmoticons;
    bool checkUpdates;
    bool notify;
    bool desktopNotify;
    bool notifySystemBackend;
    bool showWindow;
    bool notifySound;
    bool notifyHide;
    bool busySound;
    bool conferenceAlwaysNotify;
    bool nameColors;
    bool imagePreview;
    int chatMaxWindowSize;
    int chatWindowChunkSize;

    bool forceTCP;
    bool enableLanDiscovery;

    ICoreSettings::ProxyType proxyType;
    QString proxyAddr;
    quint16 proxyPort;

    QString currentProfile;
    uint32_t currentProfileId;

    bool enableLogging;

    int autoAwayTime;

    bool enableDebug;

    QHash<QString, QByteArray> widgetSettings;
    QHash<QString, QString> autoAccept;
    bool autoSaveEnabled;
    QString globalAutoAcceptDir;
    size_t autoAcceptMaxSize;

    QList<Request> friendRequests;

    // GUI
    QString smileyPack;
    int emojiFontPointSize;
    bool minimizeOnClose;
    QByteArray windowGeometry;
    QByteArray windowState;
    QByteArray splitterState;
    QByteArray dialogGeometry;
    QByteArray dialogSplitterState;
    QByteArray dialogSettingsGeometry;
    QString style;
    bool showSystemTray;

    // ChatView
    QFont chatMessageFont;
    StyleType stylePreference;
    int firstColumnHandlePos;
    int secondColumnHandlePosFromRight;
    QString timestampFormat;
    QString dateFormat;
    bool statusChangeNotificationEnabled;
    bool showConferenceJoinLeaveMessages;
    bool spellCheckingEnabled;
    bool hidePostNullSuffix;

    // Privacy
    bool typingNotification;
    Db::syncType dbSyncType;
    QStringList blockList;

    // Audio
    QString inDev;
    bool audioInDevEnabled;
    qreal audioInGainDecibel;
    qreal audioThreshold;
    QString outDev;
    bool audioOutDevEnabled;
    int outVolume;
    int audioBitrate;
    bool enableTestSound;

    // Video
    QString videoDev;
    QRect camVideoRes;
    QRect screenRegion;
    bool screenGrabbed;
    float camVideoFPS;

    struct FriendProp
    {
        FriendProp() = delete;
        explicit FriendProp(QString addr_)
            : addr(std::move(addr_))
        {
        }
        QString alias = "";
        QString addr = "";
        QString autoAcceptDir = "";
        QString note = "";
        int circleID = -1;
        QDateTime activity = QDateTime();
        AutoAcceptCallFlags autoAcceptCall;
        bool autoConferenceInvite = false;
    };

    struct CircleProp
    {
        QString name;
        bool expanded;
    };

    QHash<QByteArray, FriendProp> friendLst;

    QVector<CircleProp> circleLst;

    int themeColor;

    static QRecursiveMutex bigLock;
    static const QString globalSettingsFile;
    static QThread* settingsThread;
    Paths paths;
    int globalSettingsVersion = 0;
    int personalSettingsVersion = 0;
    IMessageBoxManager& messageBoxManager;
    const Profile* loadedProfile = nullptr;
};
