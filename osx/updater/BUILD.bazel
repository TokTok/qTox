load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library")

go_library(
    name = "go_default_library",
    srcs = ["updater.go"],
    importpath = "github.com/TokTok/qtox/osx/updater",
    visibility = ["//visibility:private"],
    deps = ["@org_bitbucket_kardianos_osext//:go_default_library"],
)

go_binary(
    name = "updater",
    embed = [":go_default_library"],
    importpath = "github.com/TokTok/qtox/osx/updater",
    visibility = ["//visibility:public"],
)
