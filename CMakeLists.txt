# CMAKE_COMPILER
# - Basic CMake setup
# Check version meets ou requirements
# Declare project, which will configure compiler for us
cmake_minimum_required(VERSION 3.3)
project(FalaiseDigitizationPlugin)

include(GNUInstallDirs)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(STATUS "${PROJECT_NAME} requires an out-of-source build.")
  message(STATUS "Please remove these files from ${CMAKE_BINARY_DIR} first:")
  message(STATUS "  CMakeCache.txt")
  message(STATUS "  CMakeFiles")
  message(STATUS "Once these files are removed, create a separate directory")
  message(STATUS "and run CMake from there, pointing it to:")
  message(STATUS "  ${CMAKE_SOURCE_DIR}")
  message(FATAL_ERROR "in-source build detected")
endif()

# - Validate that certain paths are relative, otherwise relocation may fail
foreach(_dir BINDIR LIBDIR INCLUDEDIR DATAROOTDIR)
  if(IS_ABSOLUTE "${CMAKE_INSTALL_${_dir}}")
    message(FATAL_ERROR "Absolute path for CMAKE_INSTALL_${_dir} not allowed")
  endif()
endforeach()

message(STATUS "[DEBUG] CMAKE_INSTALL_BINDIR      = ${CMAKE_INSTALL_BINDIR}")
message(STATUS "[DEBUG] CMAKE_INSTALL_LIBDIR      = ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "[DEBUG] CMAKE_INSTALL_INCLUDEDIR  = ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "[DEBUG] CMAKE_INSTALL_DATAROOTDIR = ${CMAKE_INSTALL_DATAROOTDIR}")
message(STATUS "[DEBUG] CMAKE_INSTALL_FULL_LIBDIR      = ${CMAKE_INSTALL_FULL_LIBDIR}")
message(STATUS "[DEBUG] CMAKE_INSTALL_FULL_DATAROOTDIR = ${CMAKE_INSTALL_FULL_DATAROOTDIR}")

# - Load Custom Modules
# list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake)

set(FalaiseDigi_VERSION_MAJOR 0)
set(FalaiseDigi_VERSION_MINOR 1)
set(FalaiseDigi_VERSION_PATCH 0)
set(FalaiseDigi_VERSION_REVISION 0)
set(FalaiseDigi_VERSION "${FalaiseDigi_VERSION_MAJOR}.${FalaiseDigi_VERSION_MINOR}.${FalaiseDigi_VERSION_PATCH}")
message( STATUS "[DEBUG] Digi version : ${FalaiseDigi_VERSION}")

# Resources directories based on name-version subdirs
set(FalaiseDigi_TAG "FalaiseDigi-${FalaiseDigi_VERSION}")
file(RELATIVE_PATH FalaiseDigi_LIBDIR_TO_RESOURCEDIR
  "${CMAKE_INSTALL_FULL_LIBDIR}"
  "${CMAKE_INSTALL_FULL_DATAROOTDIR}/${FalaiseDigi_TAG}/resources")
# file(RELATIVE_PATH FalaiseDigi_CMAKEDIR_TO_INCLUDEDIR
#   "${CMAKE_INSTALL_FULL_CMAKEDIR}/${FalaiseDigi_TAG}"
#   "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
message( STATUS "[DEBUG] FalaiseDigi_LIBDIR_TO_RESOURCEDIR : ${FalaiseDigi_LIBDIR_TO_RESOURCEDIR}")

find_package(Falaise 3.1)
message( STATUS "[DEBUG] Falaise ${Falaise_VERSION} was found")

############################################################################################
# - Digitization modules:
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/source/falaise)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/source)
#include_directories(${Falaise_INCLUDE_DIRS})
include_directories(${Bayeux_INCLUDE_DIRS})

# - Prepare files from skelettons:
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/digitization/version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/digitization/version.h
  @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/digitization/resource.cc.in
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/digitization/resource.cc
  @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/digitization/fldigi.cc.in
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/digitization/fldigi.cc
  @ONLY)

# - Generate binreloc header/source with mangled names, ensuring first
# character(s) are a valid for a C identifier
string(RANDOM LENGTH 13 MANGLE_BINRELOC_MAIN)
set(MANGLE_BINRELOC "MBFLDIGI${MANGLE_BINRELOC_MAIN}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/digitization/fldigi_binreloc.h.in ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/digitization/fldigi_binreloc.h @ONLY)

# - Headers:
list(APPEND FalaiseDigitizationPlugin_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/digitization/fldigi_binreloc.h
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/digitization/version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/digitization/resource.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/digitization/fldigi.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/digitization/detail/fldigi_sys.h
   )

# - Sources:
list(APPEND FalaiseDigitizationPlugin_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/digitization/fldigi_binreloc.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/digitization/version.cc
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/digitization/resource.cc
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/digitization/fldigi.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/digitization/detail/fldigi_sys.cc
  )

############################################################################################

# Build a dynamic library from our sources
add_library(Falaise_Digitization SHARED
  ${FalaiseDigitizationPlugin_HEADERS}
  ${FalaiseDigitizationPlugin_SOURCES})
target_include_directories(Falaise_Digitization
  PUBLIC
    ${PROJECT_SOURCE_DIR}/source
    ${PROJECT_SOURCE_DIR}/source/falaise
  )

target_link_libraries(Falaise_Digitization PUBLIC Falaise::FalaiseModule)

# target_compile_features(Falaise_Digitization PUBLIC ${FALAISE_CXX_COMPILE_FEATURES})
# target_compile_definitions(Falaise_Digitization PRIVATE ENABLE_BINRELOC)
# target_include_directories(Falaise_Digitization PUBLIC
#   $<BUILD_INTERFACE:${FALAISE_BUILD_INCLUDEDIR}>
#   $<BUILD_INTERFACE:${PROJECT_BUILD_INCLUDEDIR}/falaise>
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
#   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
#   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/falaise>
#   )
# target_link_libraries(Falaise PUBLIC Bayeux::Bayeux)
# target_clang_format(Falaise)

# Apple linker requires dynamic lookup of symbols, so we
# add link flags on this platform
if(APPLE)
  set_target_properties(Falaise_Digitization
    PROPERTIES LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()

# Install it:
install(TARGETS Falaise_Digitization DESTINATION ${CMAKE_INSTALL_LIBDIR}/Falaise/modules)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/resources/modules/Digitization/
  DESTINATION ${CMAKE_INSTALL_DATADIR}/Falaise/resources/modules/Digitization)

# Test support:
option(FalaiseDigitizationPlugin_ENABLE_TESTING "Build unit testing system for FalaiseDigitization" ON)
if(FalaiseDigitizationPlugin_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(testing)
endif()

# - end
