#ifndef GODOTJS_VERSION_COMPARISON_H
#define GODOTJS_VERSION_COMPARISON_H

#define GODOT_VERSION_COMPARE(Current, MinExpected, ComparisonChain) (((Current) > (MinExpected)) || ((Current) == (MinExpected) && (ComparisonChain)))

#if JSB_GDEXTENSION
#   include <godot_cpp/core/version.hpp>
#   define GODOT_VERSION_NEWER_THAN(major, minor, patch) GODOT_VERSION_COMPARE(GODOT_VERSION_MAJOR, major, GODOT_VERSION_COMPARE(GODOT_VERSION_MINOR, minor, GODOT_VERSION_COMPARE(GODOT_VERSION_PATCH, patch, false)))
#else
#   include "core/version.h"
#ifndef VERSION_MAJOR
#define VERSION_MAJOR GODOT_VERSION_MAJOR
#endif
#ifndef VERSION_MINOR
#define VERSION_MINOR GODOT_VERSION_MINOR
#endif
#ifndef VERSION_PATCH
#define VERSION_PATCH GODOT_VERSION_PATCH
#endif
#ifndef VERSION_DOCS_URL
#define VERSION_DOCS_URL GODOT_VERSION_DOCS_URL
#endif
#   define GODOT_VERSION_NEWER_THAN(major, minor, patch) GODOT_VERSION_COMPARE(VERSION_MAJOR, major, GODOT_VERSION_COMPARE(VERSION_MINOR, minor, GODOT_VERSION_COMPARE(VERSION_PATCH, patch, false)))
#endif

#define GODOT_4_3_OR_NEWER GODOT_VERSION_NEWER_THAN(4, 3, -1)
#define GODOT_4_4_OR_NEWER GODOT_VERSION_NEWER_THAN(4, 4, -1)

#endif
