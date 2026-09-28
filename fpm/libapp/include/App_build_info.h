/* Copyright 2025, His Majesty the King in right of Canada */

#ifndef _BUILD_INFO_H
#define _BUILD_INFO_H

/* Hand written stand in for the App_build_info.h that the CMake build of the
 * App submodule generates with ec_build_info().  App/src/App.c includes this
 * header but does not use any of the values defined here, so a static file is
 * enough for the fpm build. */

#define PROJECT_NAME                 App
#define PROJECT_NAME_C_ID            App
#define PROJECT_NAME_STRING         "App"
#define PROJECT_VERSION_STRING      "App 20.0.0"
#define PROJECT_DESCRIPTION_STRING  "ECCC-MRD Application management library"
#define VERSION                     "20.0.0"
#define GIT_VERSION                 "20.0.0"
#define EC_ARCH                     ""
#define BUILD_TIMESTAMP             ""

#endif /* _BUILD_INFO_H */
