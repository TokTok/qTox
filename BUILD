load("//tools:qt.bzl", "qt_moc", "qt_rcc", "qt_ui_library")

# User interface headers
# =========================================================

UI_FILES = glob(["src/**/*.ui"])

UI_HDRS = ["ui_" + ui[ui.rindex("/") + 1:-3] for ui in UI_FILES]

[qt_ui_library(
    name = "ui_" + ui[ui.rindex("/") + 1:-3],
    ui = ui,
) for ui in UI_FILES]

# MOC headers
# =========================================================

MOC_HDRS = [
    "src/audio/audio.h",
    "src/audio/backend/openal2.h",
    "src/audio/backend/openal.h",
    "src/chatlog/chatlinecontent.h",
    "src/chatlog/chatlinecontentproxy.h",
    "src/chatlog/chatlog.h",
    "src/chatlog/content/filetransferwidget.h",
    "src/chatlog/content/notificationicon.h",
    "src/chatlog/content/spinner.h",
    "src/chatlog/content/text.h",
    "src/chatlog/content/timestamp.h",
    "src/chatlog/customtextdocument.h",
    "src/core/coreav.h",
    "src/core/core.h",
    "src/ipc.h",
    "src/model/about/aboutfriend.h",
    "src/model/about/iaboutfriend.h",
    "src/model/contact.h",
    "src/model/friend.h",
    "src/model/group.h",
    "src/model/profile/iprofileinfo.h",
    "src/net/toxuri.h",
    "src/nexus.h",
    "src/persistence/db/rawdatabase.h",
    "src/persistence/offlinemsgengine.h",
    "src/persistence/profile.h",
    "src/persistence/settings.h",
    "src/persistence/smileypack.h",
    "src/platform/posixsignalnotifier.h",
    "src/video/camerasource.h",
    "src/video/corevideosource.h",
    "src/video/genericnetcamview.h",
    "src/video/netcamview.h",
    "src/video/videosource.h",
    "src/video/videosurface.h",
    "src/widget/about/aboutfriendform.h",
    "src/widget/categorywidget.h",
    "src/widget/chatformheader.h",
    "src/widget/circlewidget.h",
    "src/widget/contentdialog.h",
    "src/widget/emoticonswidget.h",
    "src/widget/form/addfriendform.h",
    "src/widget/form/chatform.h",
    "src/widget/form/filesform.h",
    "src/widget/form/genericchatform.h",
    "src/widget/form/groupchatform.h",
    "src/widget/form/groupinviteform.h",
    "src/widget/form/groupinvitewidget.h",
    "src/widget/form/loadhistorydialog.h",
    "src/widget/form/profileform.h",
    "src/widget/form/setpassworddialog.h",
    "src/widget/form/settings/aboutform.h",
    "src/widget/form/settings/advancedform.h",
    "src/widget/form/settings/avform.h",
    "src/widget/form/settings/generalform.h",
    "src/widget/form/settings/genericsettings.h",
    "src/widget/form/settings/privacyform.h",
    "src/widget/form/settings/userinterfaceform.h",
    "src/widget/form/settings/verticalonlyscroller.h",
    "src/widget/form/settingswidget.h",
    "src/widget/form/tabcompleter.h",
    "src/widget/friendlistlayout.h",
    "src/widget/friendlistwidget.h",
    "src/widget/friendwidget.h",
    "src/widget/genericchatitemwidget.h",
    "src/widget/genericchatroomwidget.h",
    "src/widget/groupwidget.h",
    "src/widget/gui.h",
    "src/widget/loginscreen.h",
    "src/widget/maskablepixmapwidget.h",
    "src/widget/notificationedgewidget.h",
    "src/widget/passwordedit.h",
    "src/widget/qrwidget.h",
    "src/widget/systemtrayicon.h",
    "src/widget/tool/activatedialog.h",
    "src/widget/tool/adjustingscrollarea.h",
    "src/widget/tool/callconfirmwidget.h",
    "src/widget/tool/chattextedit.h",
    "src/widget/tool/croppinglabel.h",
    "src/widget/tool/flyoutoverlaywidget.h",
    "src/widget/tool/friendrequestdialog.h",
    "src/widget/tool/profileimporter.h",
    "src/widget/tool/removefrienddialog.h",
    "src/widget/tool/screengrabberchooserrectitem.h",
    "src/widget/tool/screengrabberoverlayitem.h",
    "src/widget/tool/screenshotgrabber.h",
    "src/widget/tool/toolboxgraphicsitem.h",
    "src/widget/widget.h",
]

MOC_SRCS = [hdr[hdr.rindex("/") + 1:-2] for hdr in MOC_HDRS]

[qt_moc(
    name = hdr[hdr.rindex("/") + 1:-2],
    src = hdr,
    srcs = glob(["src/**/*.h"]),
    mocopts = ["-Iqtox"],
    deps = ["@qt//:qt_widgets"],
) for hdr in MOC_HDRS]

# Resources
# =========================================================

qt_rcc(
    name = "qtox_resources",
    srcs = [
        "res.qrc",
        "smileys/emojione.qrc",
        "smileys/smileys.qrc",
    ],
    data = glob([
        "smileys/*/*",
        "audio/**/*",
        "img/**/*",
        "ui/**/*",
        "res/**/*",
    ]),
)

# Main binary
# =========================================================

cc_library(
    name = "qtox_lib",
    srcs = MOC_SRCS + glob(
        [
            "src/**/*.cpp",
        ],
        exclude = ["src/main.cpp"],
    ),
    hdrs = glob([
        "src/**/*.h",
    ]),
    defines = [
        "GIT_DESCRIBE='\"master\"'",
        "GIT_VERSION='\"master\"'",
        "TIMESTAMP='\"2018\"'",
        "QTOX_PLATFORM_EXT",
    ],
    includes = ["."],
    linkopts = [
        "-lopenal",
        "-lsqlite3",
        "-lX11",
        "-lXss",
    ],
    visibility = ["//qtox:__subpackages__"],
    deps = UI_HDRS + [
        ":qtox_resources",
        "//c-toxcore",
        "@ffmpeg",
        "@filter_audio",
        "@libexif",
        "@libqrencode",
        "@qt//:qt_concurrent",
        "@qt//:qt_gui",
        "@qt//:qt_network",
        "@qt//:qt_svg",
        "@qt//:qt_widgets",
        "@qt//:qt_xml",
    ],
)

cc_binary(
    name = "qtox",
    srcs = ["src/main.cpp"],
    deps = [":qtox_lib"],
)
