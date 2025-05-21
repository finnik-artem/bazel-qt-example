load("@rules_qt//:qt.bzl", "qt_cc_binary", "qt_cc_library")

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "questions_files",
    srcs = glob(["questions/**"]),
)

qt_cc_library(
    name = "main_window",
    srcs = ["mainwindow.cpp"],
    hdrs = ["mainwindow.h"],
    deps = [
        "@rules_qt//:qt_core",
        "@rules_qt//:qt_gui",
        "@rules_qt//:qt_sql",
        "@rules_qt//:qt_widgets",
    ],
    data = [":questions_files"],
)

qt_cc_binary(
    name = "duolingo",
    srcs = ["main.cpp"],
    deps = [
        ":main_window",
        "@rules_qt//:qt_widgets",
    ],
)

# Optional: If you created a questions/BUILD file for a filegroup
# And if you have a questions/BUILD like:
# package(default_visibility = ["//visibility:public"])
# filegroup(
#     name = "question_files",
#     srcs = glob(["*.txt"]),
# )
# Then in the qt_cc_binary above, data could be:
# data = ["//questions:question_files"],