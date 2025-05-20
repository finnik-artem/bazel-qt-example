load("@rules_qt//:qt.bzl", "qt_cc_binary", "qt_cc_library")

package(default_visibility = ["//visibility:public"])

qt_cc_library(
    name = "main_window",
    srcs = ["mainwindow.cpp"],
    hdrs = ["mainwindow.h"],
    deps = [
        "@rules_qt//:qt_core",
        "@rules_qt//:qt_gui",
        "@rules_qt//:qt_sql",    # <<< MAKE SURE THIS IS PRESENT
        "@rules_qt//:qt_widgets",
    ],
)

qt_cc_binary(
    name = "duolingo", # Or english_tutor
    srcs = ["main.cpp"],
    data = glob(["questions/*.txt"]),
    deps = [
        ":main_window",
        "@rules_qt//:qt_widgets", # Often covered by main_window, but doesn't hurt
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