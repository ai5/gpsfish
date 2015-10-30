project (gpsshogi CXX)
set (CMAKE_VERBOSE_MAKEFILE on)

# Tweak build mode
if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE MATCHES "^[Dd][Ee][Bb][Uu][Gg]")
        add_definitions(-DQT_NO_DEBUG_OUTPUT)
        set(CMAKE_BUILD_RELEASE TRUE)
else()
        set(CMAKE_BUILD_RELEASE FALSE)
endif()

# Installation directory
if(APPLE AND CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
        set(CMAKE_INSTALL_PREFIX "/Applications")
endif()

message(STATUS "Building ${PROJECT_NAME} in ${CMAKE_BUILD_TYPE} mode")
cmake_minimum_required(VERSION 2.8.8)
set (CMAKE_VERBOSE_MAKEFILE on)

ENABLE_TESTING()

# Basic build options
if(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR APPLE)
  #if(CMAKE_BUILD_RELEASE)
  #        add_definitions(-Werror)
  #endif()
  set (CMAKE_CXX_FLAGS "-Wall -D_REENTRANT")
  set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
  set (CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -funroll-loops -funswitch-loops -fpeel-loops --param large-function-growth=48000 --param inline-unit-growth=8000 --param max-inline-insns-single=8000 -fomit-frame-pointer")

  add_definitions(-DOSL_SMP=1 -DOSL_DFPN_SMP_SEARCH -DMORE_CHECKMATE_IF_CAPTURE_MAJOR -DUSE_TCMALLOC=0 -DUSE_BOOST_POOL_ALLOCATOR=1)
  add_definitions(-DBOOST_FILESYSTEM_VERSION=3) 
endif()

# Compiler specific build options
if(MINGW)
  set (CMAKE_CXX_FLAGS "-Wall -D_REENTRANT -D_MT -mthreads -mwin32 -msse2 -static-libgcc -static-libstdc++ -DWIN32_LEAN_AND_MEAN")
  add_definitions(-march=pentium4)
  add_definitions(-D_WIN32_WINNT=0x0500)

  set (BOOST_ROOT "$ENV{HOME}/local/src/boost_1_46_1/")
  find_library(Winsock2_LIBRARY ws2_32)
elseif (APPLE)
  set (CMAKE_CXX_COMPILER "/usr/local/bin/g++-4.7")
  set (CMAKE_CXX_FLAGS "-Wall -D_REENTRANT -m64 -fno-common")
  add_definitions(-march=core2)

  set (BOOST_ROOT "/usr/local/Cellar/boost/1.54.0")
endif()

add_definitions(-DOSL_HOME="/Applications/${PROJECT_NAME}.app")

include_directories ("${PROJECT_SOURCE_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}/../include")
include_directories ("${PROJECT_SOURCE_DIR}/../../osl/include")

#
# Third-party libraries
#

if (MINGW)
  ## bzip2
  find_library(Bz2_LIBRARY bz2 PATH "/usr/i686-w64-mingw32/lib"
               NO_DEFAULT_PATH)

  ## psapi
  find_library(Psapi_LIBRARY psapi PATH "/usr/i686-w64-mingw32/lib"
               NO_DEFAULT_PATH)
else()
  find_library(Bz2_LIBRARY bz2)
  find_library(Iconv_LIBRARY iconv)
endif()

## Boost thing
#set(Boost_DEBUG ON)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)
if (MINGW)
  find_package (Boost 1.46.1 COMPONENTS program_options date_time filesystem serialization iostreams thread_win32 system)
else()
  find_package (Boost 1.54.0 COMPONENTS program_options date_time filesystem serialization iostreams thread system)
endif()
if (Boost_FOUND)
  include_directories (${Boost_INCLUDE_DIRS})
  message (STATUS ${Boost_INCLUDE_DIRS})
else()
  message (FATAL_ERROR "Boost installation not found")
endif()

## Import libosl.a
add_library (osl STATIC IMPORTED)
set_property (TARGET osl PROPERTY IMPORTED_LOCATION_DEBUG   ${PROJECT_SOURCE_DIR}/../../osl/build-Debug/libosl.a)
set_property (TARGET osl PROPERTY IMPORTED_LOCATION_RELEASE ${PROJECT_SOURCE_DIR}/../../osl/build-Release/libosl.a)

## gpsshogi
set(GPSSHOGI_HEADERS  options.h)
set(GPSSHOGI_SRCS     gpsshogi.cc)
set(GPSSHIGI_ALL_SRCS ${GPSSHOGI_HEADERS} ${GPSSHOGI_SRCS})
add_executable(gpsshogi MACOSX_BUNDLE ${GPSSHIGI_ALL_SRCS})
target_link_libraries (gpsshogi osl)
target_link_libraries (gpsshogi ${Boost_PROGRAM_OPTIONS_LIBRARY})
target_link_libraries (gpsshogi ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries (gpsshogi ${Boost_SERIALIZATION_LIBRARY})
target_link_libraries (gpsshogi ${Boost_IOSTREAMS_LIBRARY})
target_link_libraries (gpsshogi ${Boost_SYSTEM_LIBRARY})
target_link_libraries (gpsshogi ${Bz2_LIBRARY})
if (MINGW)
  target_link_libraries (gpsshogi ${Boost_THREAD_WIN32_LIBRARY})
  target_link_libraries (gpsshogi ${Psapi_LIBRARY})
  target_link_libraries (gpsshogi ${Winsock2_LIBRARY})
else()
  target_link_libraries (gpsshogi ${Boost_THREAD_LIBRARY})
  target_link_libraries (gpsshogi ${Iconv_LIBRARY})
endif()

## gpsusi
set(GPSUSI_HEADERS  options.h)
set(GPSUSI_SRCS     gpsusi.cc)
set(GPSUSI_ALL_SRCS ${GPSUSI_HEADERS} ${GPSUSI_SRCS})
add_executable(gpsusi MACOSX_BUNDLE ${GPSUSI_ALL_SRCS})
target_link_libraries (gpsusi osl)
target_link_libraries (gpsusi ${Boost_PROGRAM_OPTIONS_LIBRARY})
target_link_libraries (gpsusi ${Boost_DATE_TIME_LIBRARY})
target_link_libraries (gpsusi ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries (gpsusi ${Boost_SERIALIZATION_LIBRARY})
target_link_libraries (gpsusi ${Boost_IOSTREAMS_LIBRARY})
target_link_libraries (gpsusi ${Boost_SYSTEM_LIBRARY})
target_link_libraries (gpsusi ${Bz2_LIBRARY})
if (MINGW)
  target_link_libraries (gpsusi ${Boost_THREAD_WIN32_LIBRARY})
  target_link_libraries (gpsusi ${Psapi_LIBRARY})
  target_link_libraries (gpsusi ${Winsock2_LIBRARY})
else()
  target_link_libraries (gpsusi ${Boost_THREAD_LIBRARY})
  target_link_libraries (gpsusi ${Iconv_LIBRARY})
endif()

## installer
set(PROJECT_VERSION_MAJOR "20130810")
set(PROJECT_VERSION_MINOR "1")
set(PROJECT_VERSION_PATCH "0")
set(PROJECT_VERSION_COUNT 3)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PROJECT_VENDOR "TeamGPS")
set(PROJECT_COPYRIGHT_YEAR "2013")

set(ICONS_DIR "${PROJECT_SOURCE_DIR}/icons")
if(APPLE)
  set(BIN_INSTALL_DIR ".")
  set(DOC_INSTALL_DIR "${PROJECT_NAME}.app")
  set(DATA_INSTALL_DIR "${PROJECT_NAME}.app/data")
else()
  string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
  set(BIN_INSTALL_DIR "smp")
  set(DOC_INSTALL_DIR "smp")
  set(DATA_INSTALL_DIR "data")
  set(DOC_INSTALL_DIR "share/doc/${PROJECT_NAME_LOWERCASE}/")
endif()

if(APPLE)
  set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_NAME} ${PROJECT_VERSION}")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
  set(MACOSX_BUNDLE_COPYRIGHT "${PROJECT_COPYRIGHT_YEAR} ${PROJECT_VENDOR}")
  #set(MACOSX_BUNDLE_ICON_FILE "audio-input-microphone.icns")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "gps.tanaka.ecc.u-tokyo.ac.jp")
  set(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}")
  set(MACOSX_BUNDLE_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources")
  #set(MACOSX_BUNDLE_ICON "${ICONS_DIR}/${MACOSX_BUNDLE_ICON_FILE}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${MACOSX_BUNDLE_RESOURCES})
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MACOSX_BUNDLE_ICON} ${MACOSX_BUNDLE_RESOURCES})
endif()

if(APPLE)
        set(CMAKE_INSTALL_PREFIX "/Applications")
endif()

message(STATUS "${PROJECT_NAME} will be installed to ${CMAKE_INSTALL_PREFIX}")


# CPack

set(CPACK_GENERATOR "TBZ2")
set (CPACK_PACKAGE_VENDOR "${PROJECT_VENDOR}")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "GPSShogi by TeamGPS")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../sample/binary-dist/gpsshogi_setup/README.ja.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../sample/binary-dist/GPL-2")
#set (CPACK_PACKAGE_EXECUTABLES gpsshogi;gpsshogi) # do not create start menu
set (CPACK_PACKAGE_INSTALL_DIRECTORY "gpsshogi")

# Specific to NSIS (Windows)
if (MINGW)
  set (CPACK_GENERATOR "NSIS")
  #set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY HKCU "/Environment/OSL_HOME" ${CPACK_NSIS_INSTALL_ROOT})
  set (CPACK_NSIS_PACKAGE_NAME "GPSShogi")
  set (CPACK_NSIS_DISPLAY_NAME "GPSShogi")
  set (CPACK_NSIS_URL_INFO_ABOUT "http://gps.tanaka.ecc.u-tokyo.ac.jp/gpsshogi/")
  set (CPACK_NSIS_HELP_LINK "http://gps.tanaka.ecc.u-tokyo.ac.jp/gpsshogi/")
  ## registry key for OSL_HOME environment variable
  set (CPACK_NSIS_EXTRA_INSTALL_COMMANDS   "WriteRegStr HKCU \\\"Environment\\\" \\\"OSL_HOME\\\" \\\"$INSTDIR\\\"")
  set (CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "DeleteRegValue HKCU \\\"Environment\\\" \\\"OSL_HOME\\\"")
elseif (APPLE)
  set(CMAKE_INSTALL_PREFIX "/Applications")
  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_DMG_FORMAT "UDBZ")
  set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
  set(CPACK_SYSTEM_NAME "OSX")
  set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
  #set(CPACK_PACKAGE_ICON "${ICONS_DIR}/DMG.icns")
  #set(CPACK_DMG_DS_STORE "${ICONS_DIR}/DMGDSStore")
  #set(CPACK_DMG_BACKGROUND_IMAGE "${ICONS_DIR}/DMGBackground.png")
elseif(UNIX)
  set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
endif()

## Binary
install (TARGETS gpsshogi gpsusi DESTINATION ${BIN_INSTALL_DIR})
## Data files
file (GLOB datafiles "${CMAKE_CURRENT_SOURCE_DIR}/../data/*.dat"
                     "${CMAKE_CURRENT_SOURCE_DIR}/../data/*.bin"
                     "${CMAKE_CURRENT_SOURCE_DIR}/../data/*.txt"
                     "${CMAKE_CURRENT_SOURCE_DIR}/../data/*.pair")
file (GLOB dataratingfiles "${CMAKE_CURRENT_SOURCE_DIR}/../data/rating/*.txt")
install (FILES ${datafiles}       DESTINATION "${DATA_INSTALL_DIR}")
install (FILES ${dataratingfiles} DESTINATION "${DATA_INSTALL_DIR}/rating")
## README
file (GLOB readmefiles "${CMAKE_CURRENT_SOURCE_DIR}/../sample/binary-dist/gpsshogi_setup/README*.txt")
install (FILES ${readmefiles} DESTINATION "./")
## licenses for data
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../AUTHORS"
               "${CMAKE_CURRENT_SOURCE_DIR}/../COPYING"
               "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE"
         DESTINATION "${DATA_INSTALL_DIR}")
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../sample/binary-dist/GPL-2"
         DESTINATION "${DATA_INSTALL_DIR}")
## licenses for binaries
### OSL
set (OSL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../osl")
install (FILES "${OSL_SOURCE_DIR}/AUTHORS" DESTINATION "${DOC_INSTALL_DIR}" RENAME "AUTHORS.osl")
install (FILES "${OSL_SOURCE_DIR}/LICENSE" DESTINATION "${DOC_INSTALL_DIR}" RENAME "LICENSE.osl")
### GPS
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../AUTHORS" DESTINATION "${DOC_INSTALL_DIR}" RENAME "AUTHORS")
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../COPYING" DESTINATION "${DOC_INSTALL_DIR}" RENAME "COPYING")
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE" DESTINATION "${DOC_INSTALL_DIR}" RENAME "LICENSE")
### GPL
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../sample/binary-dist/GPL-2" DESTINATION "${DOC_INSTALL_DIR}")
if (MINGW)
  ### Boost
  install (FILES "${BOOST_ROOT}/LICENSE_1_0.txt" DESTINATION "${DOC_INSTALL_DIR}" RENAME "copyright.boost") 
  ### bzip2
  install (FILES "$ENV{HOME}/local/src/bzip2-1.0.6/LICENSE" DESTINATION "${DOC_INSTALL_DIR}" RENAME "copyright.bzip2") 
  ### Mingw
  install (FILES "/usr/share/doc/mingw-w64/copyright" DESTINATION "${DOC_INSTALL_DIR}" RENAME "copyright.mingw")
endif()

if(APPLE)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${BIN_INSTALL_DIR}")
  include(InstallRequiredSystemLibraries)

  set(EXECUTABLE "${PROJECT_NAME}.app")
elseif(WIN32)
  set(EXECUTABLE "${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
else()
  set(EXECUTABLE "${BIN_INSTALL_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
endif()

include (CPack)
