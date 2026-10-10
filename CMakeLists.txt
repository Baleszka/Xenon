cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(Xenon VERSION 1.0.0)

# Add all source files inside src (recursively)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Set up the mod binary
add_library(${PROJECT_NAME} SHARED ${SOURCES}
    src/ui/gui.cpp
    src/ui/gui.hpp
    src/info/playerInfo.cpp
    src/info/playerInfo.hpp
    src/bot/frameHook.hpp
    src/bot/xe.cpp
    src/bot/xe.hpp
    src/bot/replay.cpp
    src/bot/replay.hpp
    src/calculator/calculator.cpp
    src/calculator/calculator.hpp
    src/checkpoint/checkpoint.cpp
    src/checkpoint/checkpoint.hpp
    src/bot/cbf.cpp
    src/bot/cbf.hpp
    src/assist/hitboxes.cpp
    src/assist/hitboxes.hpp
    src/bot/frameStepper.cpp
    src/bot/frameStepper.hpp
    src/bot/speedhack.cpp
    src/bot/speedhack.hpp
    src/patches/releaseFix.cpp
    src/bot/tpsBypass.cpp
    src/bot/tpsBypass.hpp
    src/bot/midhook.hpp
    src/bot/midhook.cpp
    src/patches/releaseFix.hpp
    src/assist/autoclicker.cpp
    src/assist/autoclicker.hpp)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

CPMAddPackage("gh:matcool/gd-imgui-cocos#b93f08ccef778a53ebba09b20c347f6a63980119")
target_link_libraries(${PROJECT_NAME} imgui-cocos)

CPMAddPackage("gh:cursey/safetyhook#2f28386")
target_link_libraries(${PROJECT_NAME} safetyhook)

# Set up dependencies, resources, and link Geode.
setup_geode_mod(${PROJECT_NAME})
