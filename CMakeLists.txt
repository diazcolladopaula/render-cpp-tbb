cmake_minimum_required(VERSION 3.28)
project(render_project LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
  GSL
  GIT_REPOSITORY https://github.com/microsoft/GSL.git
  GIT_TAG        v4.1.0
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
  GIT_SHALLOW    TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(GSL googletest)

enable_testing()
include(GoogleTest)
find_package(TBB REQUIRED)
include(cmake/TestUtils.cmake)

if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy-20 clang-tidy)
  if(CLANG_TIDY_EXE)
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE};--header-filter=.*")
    # Optional: point clang-tidy at a non-default GCC toolchain, e.g.
    # cmake --preset clang-tidy -DCLANG_TIDY_GCC_TOOLCHAIN=/path/to/gcc
    if(CLANG_TIDY_GCC_TOOLCHAIN)
      list(APPEND CLANG_TIDY_COMMAND
           "--extra-arg=--gcc-toolchain=${CLANG_TIDY_GCC_TOOLCHAIN}")
    endif()
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
  else()
    message(STATUS "clang-tidy not found; skipping static analysis")
  endif()
endif()

add_subdirectory(common)
add_subdirectory(utcommon)
add_subdirectory(par)
add_subdirectory(utpar)
