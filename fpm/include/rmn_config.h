/*
 * Copyright 2025, His Majesty the King in right of Canada
 *
 * Compile time definitions for the C sources of librmn.
 *
 * The CMake build gets these from cmake_rpn/modules/ec_compiler_presets
 * (add_definitions(-DLittle_Endian)) and from the top level CMakeLists.txt
 * (add_compile_definitions(_${CMAKE_SYSTEM_NAME}_ _GNU_SOURCE)).  fpm has no
 * pre-build step, so this header is force included into every C source with
 * "-include rmn_config.h" (see fpm.toml) and does the same job.
 */

#ifndef RMN_CONFIG_H
#define RMN_CONFIG_H

/* Request the GNU flavour of the C library, as the CMake build does. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

/* Little_Endian selects the native bit field layout of the packed formats, see
 * include/bitPacking.h and src/PUBLIC_INCLUDES/rmn/rpnmacros.h.  The CMake
 * presets hard code it for every platform they support; here it is derived
 * from the compiler so that big endian platforms are handled correctly. */
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#ifndef Little_Endian
#define Little_Endian
#endif
#endif
#endif

#endif /* RMN_CONFIG_H */
