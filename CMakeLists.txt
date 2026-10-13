cmake_minimum_required(VERSION 3.22)

project(LightControl VERSION 0.1.0)

# A Debug build of JUCE's GUI is many times slower; build optimised unless told otherwise.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(third_party/JUCE)
add_subdirectory(third_party/OpenRGB-CppSDK)

juce_add_plugin(LightControl
    COMPANY_NAME "Michal Gochna"
    PLUGIN_MANUFACTURER_CODE Mgoc
    PLUGIN_CODE Lctl
    FORMATS VST3 Standalone
    BUNDLE_ID com.mgochna.lightcontrol
    PRODUCT_NAME "LightControl"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

# The legacy sources include <JuceHeader.h>, which Projucer used to generate.
juce_generate_juce_header(LightControl)

target_sources(LightControl PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/LightWorker.cpp
    Source/ArtNetOutput.cpp
    Source/Settings.cpp
    Source/Widgets.cpp
    Source/LightLookAndFeel.cpp
    Source/LightDisplay.cpp
    Source/HelpContent.cpp
    Source/HelpPage.cpp)

target_include_directories(LightControl PRIVATE Source)

# The OpenRGB SDK's headers aren't ours to fix: keep their warnings out of our build output.
get_target_property(ORGB_INCLUDE_DIRS orgbsdk INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(LightControl SYSTEM PRIVATE ${ORGB_INCLUDE_DIRS})

target_compile_definitions(LightControl PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(LightControl
    PRIVATE
        juce::juce_audio_utils
        orgbsdk
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
