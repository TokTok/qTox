load("//tools:qt.bzl", "qt_moc")

SRCS = glob(["*/*_test.cpp"])

[qt_moc(
    name = "%s_moc" % src[src.rindex("/") + 1:-4],
    src = src,
) for src in SRCS]

[cc_test(
    name = src[src.rindex("/") + 1:-4],
    srcs = [src],
    copts = ["-I$(GENDIR)/qtox/test/%s" % src[:src.rindex("/")]],
    deps = [
        ":%s_moc" % src[src.rindex("/") + 1:-4],
        "//qtox:qtox_lib",
        "@qt//:qt_test",
    ],
) for src in SRCS]
