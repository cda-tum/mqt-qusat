# Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
# Copyright (c) 2025 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

# set required cmake version
cmake_minimum_required(VERSION 3.24...4.0)

project(
  qusat
  LANGUAGES CXX
  DESCRIPTION "A Tool for Utilizing SAT in Quantum Computing")

option(BUILD_MQT_QUSAT_BINDINGS "Build the MQT QUSAT Python bindings" OFF)
option(BUILD_MQT_QUSAT_TESTS "Also build tests for the MQT QUSAT project" ON)

if(BUILD_MQT_QUSAT_BINDINGS)
  # ensure that the BINDINGS option is set
  set(BINDINGS
      ON
      CACHE BOOL "Enable settings related to Python bindings" FORCE)
  # cmake-lint: disable=C0103
  set(Python_FIND_VIRTUALENV
      FIRST
      CACHE STRING "Give precedence to virtualenvs when searching for Python")
  # cmake-lint: disable=C0103
  set(Python_ARTIFACTS_INTERACTIVE
      ON
      CACHE BOOL "Prevent multiple searches for Python and instead cache the results.")

  if(DISABLE_GIL)
    message(STATUS "Disabling Python GIL")
    add_compile_definitions(Py_GIL_DISABLED)
  endif()

  # top-level call to find Python
  find_package(Python 3.9 REQUIRED COMPONENTS Interpreter Development.Module)
endif()

# Add path for custom modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(cmake/ExternalDependencies.cmake)

# add main library code
add_subdirectory(src)

# add test code
if(BUILD_MQT_QUSAT_TESTS)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(test)
endif()
