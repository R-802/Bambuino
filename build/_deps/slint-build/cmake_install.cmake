# Install script for directory: C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Bambuino")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/msys64/ucrt64/bin/objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/corrosion-build/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Slint/SlintTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Slint/SlintTargets.cmake"
         "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/CMakeFiles/Export/002ce537dcf861259db4455970114bb1/SlintTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Slint/SlintTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Slint/SlintTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Slint" TYPE FILE FILES "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/CMakeFiles/Export/002ce537dcf861259db4455970114bb1/SlintTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/slint" TYPE FILE FILES
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint-interpreter.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint-platform.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint-stm32.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint-testing.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_brush.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_callbacks.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_color.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_config.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_image.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_interpreter.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_pathdata.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_point.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_properties.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_sharedvector.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_size.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_string.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_tests_helpers.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_timer.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/slint_window.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/include/vtable.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_brush_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_builtin_structs.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_builtin_structs_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_color_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_enums.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_enums_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_generated_public.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_image_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_pathdata_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_platform_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_properties_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_qt_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_sharedvector_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_string_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_timer_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_interpreter_internal.h"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/generated_include/slint_interpreter_generated_public.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/slint_cpp.dll")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/libslint_cpp.dll.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/slint-compiler.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Slint" TYPE FILE FILES
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/lib/cmake/Slint/SlintConfig.cmake"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/lib/cmake/Slint/SlintConfigVersion.cmake"
    "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-src/api/cpp/cmake/SlintMacro.cmake"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/shema/Documents/PlatformIO/Projects/Bambuino/build/_deps/slint-build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
