#################################################
# (c) martin stolz at noizplay.com
#################################################

# preconditions
cmake_minimum_required (VERSION 3.31.3)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS ON) # ON: use "-std=gnu++20" / OFF: use "-std=c++20"

# project config ################################
project (AudioPlugin VERSION 1.0.0)
set     (preRelease "alpha") # alpha, beta, final

set (Target       ${PROJECT_NAME})
set (ProductName  "audio plugin")
set (pluginCode   "NP01") # noizplay plugin 01
set (bundleId     "com.noizplay.${Target}")

set (companyName      "noizplay")
set (companyURL       "https://noizplay.com")
set (manufacturerCode "NZPL")
#################################################

# init
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property (GLOBAL PROPERTY USE_FOLDERS YES)
option (JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

# we are Mac ?
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set (CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum OS X deployment version" FORCE)
    set (CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE INTERNAL "")
    set (MACOSX_BUNDLE TRUE) # create .app bundle for mac
    message (">> Building for Mac")

    # or Win ?
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # static linking in Windows
    message (">> Building for Windows")

    # nope -> abort
else()
    message (FATAL_ERROR "No viable platform found")
endif()

# fetch external repos
include (FetchContent)

message (">> Fetching JUCE")
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.10
)
FetchContent_MakeAvailable (JUCE)

# Adds all the module sources so they appear correctly in the IDE
set_property (GLOBAL PROPERTY USE_FOLDERS YES)
option (JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

# predefine the plugin behaviour
set (isSynth            TRUE)
set (isMidiEffect       FALSE)
set (needsMidiInput     TRUE)
set (needsMidiOutput    FALSE)
set (wantsKeyboardFocus FALSE)

message (">> Add plugin")
juce_add_plugin ("${Target}"
    PRODUCT_NAME "${ProductName}"

    COMPANY_NAME    "${companyName}"
    COMPANY_WEBSITE "${companyURL}"

    PLUGIN_MANUFACTURER_CODE "${manufacturerCode}"
    PLUGIN_CODE              "${pluginCode}"
    BUNDLE_ID                "${bundleId}"

    IS_SYNTH                    ${isSynth}
    IS_MIDI_EFFECT              ${isMidiEffect}
    NEEDS_MIDI_INPUT            ${needsMidiInput}
    NEEDS_MIDI_OUTPUT           ${needsMidiOutput}
    EDITOR_WANTS_KEYBOARD_FOCUS ${wantsKeyboardFocus}

    COPY_PLUGIN_AFTER_BUILD TRUE
    FORMATS AU VST3 Standalone
)

juce_generate_juce_header ("${Target}")

# add binary assets
message (">> Add binary assets")
set (BinaryAssets "${Target}-BinaryAssets")
juce_add_binary_data ("${BinaryAssets}"

    HEADER_NAME
    "binaryAssets.h"

    NAMESPACE
    "binaryAssets"

    SOURCES

    #sounds
    # "asset/sound/..."
)

target_sources ("${Target}"
    PRIVATE
        source/Main.cpp
)

target_link_libraries ("${Target}"
    PRIVATE
        "${BinaryAssets}"

        juce::juce_audio_utils
        juce::juce_audio_processors

        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions ("${Target}"
    PUBLIC
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# create a build number from "several infos"
execute_process (
    COMMAND git rev-list --count HEAD # get git commit number (NOT the commit hash !)
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE gitCommitCount
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process (
    COMMAND git diff --quiet HEAD -- # working folder clean ?
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE gitWorkdirDirty
)

string (TIMESTAMP buildDate "%Y%m%d")
if (preRelease STREQUAL "final")
    set (buildVersion "${PROJECT_VERSION}")
else()
    set (buildVersion "${PROJECT_VERSION}-${preRelease}+${buildDate}.${gitCommitCount}")
    if (gitWorkdirDirty EQUAL 1)
        set (buildVersion "${buildVersion}_dirty")
    endif()
endif()
message (">> Build Version is: ${buildVersion}")

# create cmakeVars.h
message (">> Building cmakeVar.h")
set (generatedDir "${CMAKE_BINARY_DIR}/source/generated")
configure_file (template/cmakeVar.h.tpl "${generatedDir}/cmakeVar.h" )

# make it available in project source
target_include_directories (${Target}
    PRIVATE
        ${generatedDir}
)
