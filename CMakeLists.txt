cmake_minimum_required(VERSION 3.22)
project(XboxMacroPad VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4)
FetchContent_MakeAvailable(JUCE)

if(DEFINED ENV{CI})
    set(COPY_AFTER_BUILD FALSE)
else()
    set(COPY_AFTER_BUILD TRUE)
endif()

juce_add_plugin(XboxMacroPad
    COMPANY_NAME "YourName"
    PLUGIN_MANUFACTURER_CODE Xbmp
    PLUGIN_CODE Xmcp
    FORMATS VST3
    PRODUCT_NAME "Xbox Macro Pad"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD ${COPY_AFTER_BUILD})

target_sources(XboxMacroPad PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(XboxMacroPad PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(XboxMacroPad
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Xbox 360 pads are read through XInput (Windows only)
if(WIN32)
    target_link_libraries(XboxMacroPad PRIVATE xinput)
endif()
