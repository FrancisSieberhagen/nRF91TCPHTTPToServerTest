#
# Copyright (c) 2019 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

cmake_minimum_required(VERSION 3.8.2)

# This sample runs as a non-secure application on nRF91. Therefore, it
# requires the secure_partition_manager that prepares the required
# peripherals to be available for the application.
#
# Configure the SPM image to enable the peripherals that this sample
# needs.

set(ENV{GNUARMEMB_TOOLCHAIN_PATH} /opt/gnuarmemb)

set(ENV{BOARD} nrf9160_pca10090ns)
set(ENV{BOARD_DIR} $ENV{ZEPHYR_BASE}/boards/arm/nrf9160_pca10090)

set(ENV{ZEPHYR_TOOLCHAIN_VARIANT} gnuarmemb)

set(ENV{CMAKE_AR} $ENV{GNUARMEMB_TOOLCHAIN_PATH}/bin/arm-none-eabi-ar)
set(ENV{CMAKE_AS} $ENV{GNUARMEMB_TOOLCHAIN_PATH}/bin/arm-none-eabi-as)
set(ENV{CMAKE_ASM_COMPILER} $ENV{GNUARMEMB_TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc)
set(ENV{CMAKE_ASM_COMPILER_AR} $ENV{GNUARMEMB_TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc-ar)
set(ENV{CMAKE_ASM_COMPILER_RANLIB} $ENV{GNUARMEMB_TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc-ranlib)

set(ENV{CMAKE_ASM_FLAGS_DEBUG} -g)
set(ENV{CMAKE_ASM_FLAGS_MINSIZEREL} -Os -DNDEBUG)
set(ENV{CMAKE_ASM_FLAGS_RELEASE} -O3 -DNDEBUG)
set(ENV{CMAKE_ASM_FLAGS_RELWITHDEBINFO} -O2 -g -DNDEBUG)

set(ENV{CMAKE_CXX_COMPILER} $ENV{GNUARMEMB_TOOLCHAIN_PATH}/bin/arm-none-eabi-g++)
set(ENV{CMAKE_CXX_COMPILER_AR} $ENV{GNUARMEMB_TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc-ar)
set(ENV{CMAKE_CXX_COMPILER_RANLIB} $ENV{GNUARMEMB_TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc-ranlib)

set(spm_CONF_FILE
  prj.conf
  ${CMAKE_CURRENT_LIST_DIR}/child_secure_partition_manager.conf
  )
include($ENV{ZEPHYR_BASE}/../nrf/cmake/boilerplate.cmake)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(test1)

target_sources(app PRIVATE src/main.c)

zephyr_include_directories(src)
