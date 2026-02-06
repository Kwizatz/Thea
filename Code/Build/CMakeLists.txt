#===============================================================================================================================
#
# Build script for the Thea library.
#
# Copyright (C) 2009, Siddhartha Chaudhuri/Stanford University
#
#===============================================================================================================================

# Set the minimum required CMake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.0...3.27.7)

# See cmake --help-policy CMP0003 for details on this one
IF(POLICY CMP0003)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(POLICY CMP0003)

PROJECT(Thea CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Avoid having to repeat condition after ELSE and ENDIF statements
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

# Postfix for debug builds
SET(CMAKE_DEBUG_POSTFIX "d")

# Set the default build type
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo")
ENDIF()

# We'll fix this later, right now we get an error on MSYS2/MinGW64,
# Either way bin/Thea is not a more of a standard location for executables than share/Thea/Build/Output/bin.
# https://stackoverflow.com/a/42697475
# MACRO(install_symlink filepath sympath)
#   INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink ${filepath} ${sympath})")
#   INSTALL(CODE "MESSAGE(STATUS \"Created symlink: ${sympath} -> ${filepath}\")")
# ENDMACRO(install_symlink)

# Are we building the library in a stripped-down more? (no
IF(LITE)
  MESSAGE(STATUS "")
  MESSAGE(STATUS "NOTE: The library is being built in LITE mode, with a minimal set of")
  MESSAGE(STATUS "dependencies and no additional tools unless explicitly included.")
  MESSAGE(STATUS "")
ENDIF()

# Enable plugins by default, unless explicitly omitted or in LITE mode
IF(NOT DEFINED WITH_PLUGINS)
  IF(LITE)
    SET(WITH_PLUGINS FALSE)
  ELSE()
    SET(WITH_PLUGINS TRUE)
  ENDIF()
ENDIF()

option(WITH_PLUGIN_ARPACK "Enable ARPACK plugin" ${WITH_PLUGINS})

option(WITH_PLUGIN_CSPARSE "Enable CSPARSE plugin" ${WITH_PLUGINS})

option(WITH_PLUGIN_GL "Enable GL plugin" ${WITH_PLUGINS})

# Enable tests by default, unless explicitly omitted or in LITE mode
IF(LITE)
  SET(WITH_TESTS_DEFAULT FALSE)
ELSE()
  SET(WITH_TESTS_DEFAULT TRUE)
ENDIF()
option(WITH_TESTS "Enable tests" ${WITH_TESTS_DEFAULT})

# Enable tools by default, unless explicitly omitted or in LITE mode
IF(LITE)
  SET(WITH_TOOLS_DEFAULT FALSE)
ELSE()
  SET(WITH_TOOLS_DEFAULT TRUE)
ENDIF()
option(WITH_TOOLS "Enable tools" ${WITH_TOOLS_DEFAULT})

# Subdirectories containing individual build targets
ADD_SUBDIRECTORY(Common)

IF(WITH_PLUGIN_ARPACK)
  ADD_SUBDIRECTORY(Plugins/ARPACK)
ENDIF()

IF(WITH_PLUGIN_CSPARSE)
  ADD_SUBDIRECTORY(Plugins/CSPARSE)
ENDIF()

IF(WITH_PLUGIN_GL)
  ADD_SUBDIRECTORY(Plugins/GL)
ENDIF()

IF(WITH_TESTS)
  MESSAGE(STATUS "")
  MESSAGE(STATUS "NOTE: Test programs run when you execute 'make test'.")
  MESSAGE(STATUS "NOTE: Some tests may be out-of-date with the current library.")
  MESSAGE(STATUS "")
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(Test)
ENDIF()

IF(WITH_TOOLS)
  ADD_SUBDIRECTORY(Tools)
ENDIF()

# Generate the version file
set(PACKAGE_VERSION "1.0.0") # Define your project's version
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/TheaConfigVersion.cmake"
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/TheaConfig.cmake.in" # Source template
    "${CMAKE_CURRENT_BINARY_DIR}/TheaConfig.cmake"         # Output file
    @ONLY
)

# Install the generated config and version files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/TheaConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/TheaConfigVersion.cmake"
    DESTINATION lib/cmake/Thea
)

install(EXPORT TheaExportSet
    FILE TheaTargets.cmake
    NAMESPACE Thea::
    DESTINATION lib/cmake/Thea
)
