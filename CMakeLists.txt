cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

project(
  qusat
  LANGUAGES CXX
  DESCRIPTION "A Tool for Utilizing SAT in Quantum Computing")

# check whether the submodule ``modulename`` is correctly cloned in the ``/extern`` directory.
macro(CHECK_SUBMODULE_PRESENT modulename)
  if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/${modulename}/CMakeLists.txt")
    message(
      FATAL_ERROR
        "${modulename} submodule not cloned properly. \
        Please run `git submodule update --init --recursive` \
        from the main project directory")
  endif()
endmacro()

check_submodule_present(mqt-core)

# Add path for custom modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# search for Z3
find_package(Z3 REQUIRED)

# add main library code
add_subdirectory(src)

# add test code
option(BUILD_QUSAT_TESTS "Also build tests for QUSAT project")
if(BUILD_QUSAT_TESTS)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(test)
endif()

# add Python binding code
option(BINDINGS "Configure for building Python bindings")
if(BINDINGS)
  add_subdirectory(mqt/qusat)
endif()
