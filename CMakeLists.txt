cmake_minimum_required(VERSION 3.5)

# ##############################################################################
# CMake Project ##
# ##############################################################################

# include useful macros
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/util.cmake)

# read metadata from package.xml and setup project
zetton_cc_metadata()

# set project name and version
project(
  ${extracted_name}
  VERSION ${extracted_version}
  LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# =========#
# Settings #
# =========#

# introduce default building settings
zetton_cc_settings()

# simd flags
include(FindSIMD)
option(WITH_AVX "Compile zetton-stream with AVX intrinsics" ${AVX_FOUND})
if(WITH_AVX AND AVX_FOUND)
  set(SIMD_FLAG ${AVX_FLAG})
  add_definitions(-DWITH_AVX)
  if(AVX2_FOUND)
    set(SIMD_FLAG "${SIMD_FLAG} ${AVX2_FLAG}")
  endif()
elseif(SSE3_FOUND AND NOT WIN32)
  set(SIMD_FLAG ${SSE3_FLAG})
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SIMD_FLAG}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SIMD_FLAG}")

# =============#
# Dependencies #
# =============#

# ----------------------#
# Third-party libraries #
# ----------------------#

find_package(Threads REQUIRED)
find_package(OpenCV 4 REQUIRED)
find_package(FFmpeg REQUIRED)

# -----------------#
# Zetton libraries #
# -----------------#

find_package(zetton_common REQUIRED)

# ##############################################################################
# Build ##
# ##############################################################################

# ==============#
# Build targets #
# ==============#

include(GenerateExportHeader)

# ---------------#
# Library target #
# ---------------#

# find all source files
file(GLOB_RECURSE src_files ${PROJECT_SOURCE_DIR}/src/*.cc
     ${PROJECT_SOURCE_DIR}/src/*/*.cc)

# common library
zetton_cc_library(
  NAME
  ${PROJECT_NAME}
  SRCS
  ${src_files}
  INCLUDES
  ${OpenCV_INCLUDE_DIRS}
  ${FFMPEG_INCLUDE_DIR}
  DEPS
  Threads::Threads
  ${OpenCV_LIBS}
  ${FFMPEG_LIBRARIES}
  zetton_common::zetton_common)

# ------------------#
# Executable targets #
# ------------------#

zetton_cc_apps()
zetton_cc_examples()

# -------------#
# Test targets #
# -------------#
zetton_cc_tests("stream")
