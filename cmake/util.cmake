# add executables with project library
macro(zetton_cc_excutable dirname name)
  add_executable(${dirname}_${name}
                 ${CMAKE_CURRENT_SOURCE_DIR}/${dirname}/${name}.cc)
  target_link_libraries(${dirname}_${name} ${PROJECT_NAME})
  install(TARGETS ${dirname}_${name}
          RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
endmacro()

macro(zetton_cc_excutables dirname)
  file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/${dirname}/*.cc")
  foreach(file ${files})
    get_filename_component(name ${file} NAME_WE)
    zetton_cc_excutable(${dirname} ${name})
  endforeach()
endmacro()

macro(zetton_cc_apps)
  zetton_cc_excutables("apps")
endmacro()

macro(zetton_cc_examples)
  zetton_cc_excutables("examples")
endmacro()

# add tests with project library
macro(zetton_cc_test dirname name)
  add_executable(${dirname}_${name}
                 ${CMAKE_CURRENT_SOURCE_DIR}/tests/${dirname}/${name}.cc)
  target_link_libraries(${dirname}_${name} ${PROJECT_NAME}
                        Catch2::Catch2WithMain)
  add_test(NAME ${dirname}_${name} COMMAND ${dirname}_${name})
  # install(TARGETS ${name} DESTINATION bin)
endmacro()

macro(zetton_cc_tests dirname)
  file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/tests/${dirname}/*.cc")
  foreach(file ${files})
    get_filename_component(name ${file} NAME_WE)
    zetton_cc_test(${dirname} ${name})
  endforeach()
endmacro()

macro(zetton_cc_metadata)
  # read the package manifest.
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/package.xml" package_xml_str)

  # extract project name.
  if(NOT package_xml_str MATCHES "<name>([A-Za-z0-9_]+)</name>")
    message(
      FATAL_ERROR
        "Could not parse project name from package manifest (aborting)")
  else()
    set(extracted_name ${CMAKE_MATCH_1})
  endif()

  # extract project version.
  if(NOT package_xml_str MATCHES "<version>([0-9]+.[0-9]+.[0-9]+)</version>")
    message(
      FATAL_ERROR
        "Could not parse project version from package manifest (aborting)")
  else()
    # at this point we either have a proper version string, or we've errored out
    # with a FATAL_ERROR above. So assume CMAKE_MATCH_1 contains our package's
    # version.
    set(extracted_version ${CMAKE_MATCH_1})
  endif()
endmacro()

macro(zetton_cc_settings)
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
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  add_definitions(-O2)
endmacro()

# zetton_cc_library()
# cmake-format: off
#
# CMake function to imitate Bazel's cc_library rule.
#
# Parameters:
# NAME: name of target
# HDRS: List of public header files for the library
# SRCS: List of source files for the library
# INCLUDES: List of other headers to be included in to the binary targets
# DEPS: List of other libraries to be linked in to the binary targets
# COPTS: List of private compile options
# DEFINES: List of public defines
# LINKOPTS: List of link options
#
# Note:
# By default, zetton_cc_library will always create a library named zetton_${NAME},
# and alias target zetton::${NAME}.
# The zetton:: form should always be used.
# This is to reduce namespace pollution.
#
# zetton_cc_library(
#   NAME
#     awesome
#   HDRS
#     "a.h"
#   SRCS
#     "a.cc"
# )
# zetton_cc_library(
#   NAME
#     fantastic_lib
#   SRCS
#     "b.cc"
#   DEPS
#     zetton::awesome # not "awesome" !
#   PUBLIC
# )
#
# zetton_cc_library(
#   NAME
#     main_lib
#   ...
#   DEPS
#     zetton::fantastic_lib
# )
# cmake-format: on
function(zetton_cc_library)
  cmake_parse_arguments(
    ZETTON_CC_LIB "" "NAME" "HDRS;SRCS;COPTS;DEFINES;LINKOPTS;INCLUDES;DEPS"
    ${ARGN})

  set(_NAME "${ZETTON_CC_LIB_NAME}")

  # ==============#
  # Build targets #
  # ==============#

  # common library
  add_library(${_NAME} ${ZETTON_CC_LIB_SRCS} ${ZETTON_CC_LIB_HDRS})
  add_library(${PROJECT_NAME}::${_NAME} ALIAS ${_NAME})

  # generate a header with export macros, which is written to the
  # CMAKE_CURRENT_BINARY_DIR location.
  generate_export_header(${_NAME})

  target_include_directories(
    ${_NAME}
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
           "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>"
           ${ZETTON_CC_LIB_INCLUDES})

  target_link_libraries(
    ${_NAME}
    PUBLIC ${ZETTON_CC_LIB_DEPS}
    PRIVATE ${ZETTON_CC_LIB_LINKOPTS})

  # compiling options
  target_compile_options(${_NAME} PRIVATE ${ZETTON_CC_LIB_COPTS})
  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${_NAME} PRIVATE -Wall -Wextra -Wpedantic
                                            -Wno-unused-parameter)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${_NAME} PRIVATE /W4)
  endif()
  if(MSVC)
    # Force include the export header when using Microsoft Visual C++ compiler.
    target_compile_options(
      ${_NAME} PUBLIC "/FI${CMAKE_CURRENT_BINARY_DIR}/${_NAME}_export.h")
  endif()

  # compiling definitions
  target_compile_definitions(${_NAME} PUBLIC ${ZETTON_CC_LIB_DEFINES})
  if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${_NAME} PUBLIC "ZETTON_COMMON_STATIC_DEFINE")
  endif()

  # compiling features
  target_compile_features(${_NAME} PRIVATE cxx_std_14)

  # =============================#
  # CMake package configurations #
  # =============================#

  include(CMakePackageConfigHelpers)

  # Create the ${_NAME}Config.cmake file, which is used by other packages to
  # find this package and its dependencies.
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
    "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${_NAME}Config.cmake" @ONLY)

  # Create the ${_NAME}ConfigVersion.cmake.
  write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${_NAME}ConfigVersion.cmake"
    COMPATIBILITY AnyNewerVersion)

  # ==============#
  # Install files #
  # ==============#

  include(GNUInstallDirs)

  install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${_NAME}_export.h"
          DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${_NAME}")

  install(
    FILES
      "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${_NAME}Config.cmake"
      "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${_NAME}ConfigVersion.cmake"
      "${CMAKE_CURRENT_SOURCE_DIR}/package.xml"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${_NAME}")

  install(
    TARGETS ${_NAME}
    EXPORT EXPORT_${_NAME}
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

  install(
    EXPORT EXPORT_${_NAME}
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${_NAME}"
    NAMESPACE ${PROJECT_NAME}::
    FILE ${_NAME}Targets.cmake)

  # ===============#
  # Export targets #
  # ===============#

  export(
    EXPORT EXPORT_${_NAME}
    NAMESPACE ${PROJECT_NAME}::
    FILE "${PROJECT_BINARY_DIR}/${_NAME}Targets.cmake")

endfunction()

# remove all matching elements from the list
macro(zetton_list_filterout lst regex)
  foreach(item ${${lst}})
    if(item MATCHES "${regex}")
      list(REMOVE_ITEM ${lst} "${item}")
    endif()
  endforeach()
endmacro()
