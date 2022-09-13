cmake_minimum_required(VERSION 3.5)

############
# Metadata #
############

# Read the package manifest.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/package.xml" package_xml_str)

# Extract project name.
if(NOT package_xml_str MATCHES "<name>([A-Za-z0-9_]+)</name>")
  message(FATAL_ERROR "Could not parse project name from package manifest (aborting)")
else()
  set(extracted_name ${CMAKE_MATCH_1})
endif()

# Extract project version.
if(NOT package_xml_str MATCHES "<version>([0-9]+.[0-9]+.[0-9]+)</version>")
  message(FATAL_ERROR "Could not parse project version from package manifest (aborting)")
else()
  # At this point we either have a proper version string, or we've errored
  # out with a FATAL_ERROR above. So assume CMAKE_MATCH_1 contains our
  # package's version.
  set(extracted_version ${CMAKE_MATCH_1})
endif()

#################
# CMake Project #
#################

project(${extracted_name} VERSION ${extracted_version} LANGUAGES CXX)

#==============#
# Dependencies #
#==============#

#-----------------------#
# Third-party libraries #
#-----------------------#

find_package(Threads REQUIRED)
find_package(OpenCV 4 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-app-1.0
    gstreamer-rtsp-server-1.0)

#------------------#
# Zetton libraries #
#------------------#

find_package(zetton_common REQUIRED)

#==========#
# Settings #
#==========#

# shared libraries
if(NOT DEFINED BUILD_SHARED_LIBS)
  message(STATUS "${PROJECT_NAME}: Building dynamically-linked binaries")
  option(BUILD_SHARED_LIBS "Build dynamically-linked binaries" ON)
  set(BUILD_SHARED_LIBS ON)
endif()

# build type
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  message(STATUS "${PROJECT_NAME}: Defaulting build type to RelWithDebInfo")
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# win32
if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# global compilations
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-O2)

# useful macros
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/util.cmake)

###########
## Build ##
###########

#===============#
# Build targets #
#===============#

include(GenerateExportHeader)

#----------------#
# Library target #
#----------------#

# find all source files
file(GLOB_RECURSE
    src_files
    ${PROJECT_SOURCE_DIR}/src/*.cc
    ${PROJECT_SOURCE_DIR}/src/*/*.cc
    )

# common library
add_library(${PROJECT_NAME} ${src_files})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# generate a header with export macros, which is written
# to the CMAKE_CURRENT_BINARY_DIR location.
generate_export_header(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>"
  ${OpenCV_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Threads::Threads
    ${OpenCV_LIBS}
    ${GST_LIBRARIES}
    zetton_common::zetton_common
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(${PROJECT_NAME} PUBLIC "ZETTON_COMMON_STATIC_DEFINE")
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()

if(MSVC)
  # Force include the export header when using Microsoft Visual C++ compiler.
  target_compile_options(${PROJECT_NAME} PUBLIC "/FI${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_export.h")
endif()

#############
## Install ##
#############

#==============================#
# CMake package configurations #
#==============================#

include(CMakePackageConfigHelpers)

# Create the ${PROJECT_NAME}Config.cmake file, which is used by
# other packages to find this package and its dependencies.
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
  "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
  @ONLY
)

# Create the ${PROJECT_NAME}ConfigVersion.cmake.
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}ConfigVersion.cmake"
  COMPATIBILITY AnyNewerVersion
)

#===============#
# Install files #
#===============#

include(GNUInstallDirs)

install(
  DIRECTORY include/
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_export.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
)

install(
  FILES
    "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
    "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}ConfigVersion.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/package.xml"
  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}"
)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT EXPORT_${PROJECT_NAME}
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(
  EXPORT EXPORT_${PROJECT_NAME}
  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}"
  NAMESPACE ${PROJECT_NAME}::
  FILE ${PROJECT_NAME}Targets.cmake
)

add_simple_apps()

#================#
# Export targets #
#================#

export(
  EXPORT EXPORT_${PROJECT_NAME}
  NAMESPACE ${PROJECT_NAME}::
  FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake"
)