# cmake_minimum_required(VERSION 3.16.0)
# include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# project(Bambuino)
cmake_minimum_required(VERSION 3.21)
project(BambuinoOS LANGUAGES CXX)

# Set the Rust compiler path
set(Rust_COMPILER)

find_package(Slint QUIET)
if (NOT Slint_FOUND)
  message("Slint could not be located in the CMake module search path. Downloading it from Git and building it locally")
  include(FetchContent)
  FetchContent_Declare(
    Slint
    GIT_REPOSITORY https://github.com/slint-ui/slint.git
    # `release/1` will auto-upgrade to the latest Slint >= 1.0.0 and < 2.0.0
    # `release/1.0` will auto-upgrade to the latest Slint >= 1.0.0 and < 1.1.0
    GIT_TAG release/1
    SOURCE_SUBDIR api/cpp
  )
  FetchContent_MakeAvailable(Slint)
endif (NOT Slint_FOUND)

add_executable(BambuinOS src/main.cpp)
target_include_directories(BambuinOS PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(BambuinOS PRIVATE Slint::Slint)
slint_target_sources(BambuinOS ui/app-window.slint)
# On Windows, copy the Slint DLL next to the application binary so that it's found.
if (WIN32)
    add_custom_command(TARGET BambuinOS POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:BambuinOS> $<TARGET_FILE_DIR:BambuinOS> COMMAND_EXPAND_LISTS)
endif()
