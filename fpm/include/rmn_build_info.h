/*! @file rmn_build_info.h Build information for the fpm build of librmn.
 *
 *  The CMake build generates this header into the build directory (see
 *  cmake_rpn/modules/build_info.h.in).  fpm has no mechanism to generate files
 *  before the build starts, so this hand-written equivalent is provided for the
 *  fpm build instead.  Only the macros that librmn actually uses are defined;
 *  keep VERSION in sync with the version in fpm.toml.  The CMake build fills
 *  these macros in from the git state, which is not possible here.
 */

#ifndef _RMN_BUILD_INFO_H
#define _RMN_BUILD_INFO_H

#define PROJECT_NAME            "rmn"
#define PROJECT_NAME_C_ID       "rmn"
#define PROJECT_NAME_STRING     "rmn"
#define PROJECT_VERSION_STRING  "rmn 20.0.0 fpm build"
#define PROJECT_DESCRIPTION_STRING "ECCC-MRD Collection of routines for numerical weather prediction"
#define VERSION                 "20.0.0"
#define GIT_VERSION             "fpm"
#define GIT_COMMIT              "unknown"
#define GIT_COMMIT_TIMESTAMP    "unknown"
#define GIT_STATUS              "unknown"
#define EC_ARCH                 "native"
#define BUILD_USER              "unknown"
#define BUILD_TIMESTAMP         "unknown"

#define C_COMPILER_ID            "unknown"
#define C_COMPILER_VERSION       "unknown"
#define CXX_COMPILER_ID          "unknown"
#define CXX_COMPILER_VERSION     "unknown"
#define FORTRAN_COMPILER_ID      "unknown"
#define FORTRAN_COMPILER_VERSION "unknown"

#endif /* _RMN_BUILD_INFO_H */
