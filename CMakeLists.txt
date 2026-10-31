cmake_minimum_required(VERSION 3.22)

project(Orbit VERSION 0.1.0 LANGUAGES C CXX)

set(ORBIT_JUCE_SOURCE_DIR "" CACHE PATH "Optional path to a local JUCE checkout")

if(ORBIT_JUCE_SOURCE_DIR AND EXISTS "${ORBIT_JUCE_SOURCE_DIR}/CMakeLists.txt")
    add_subdirectory("${ORBIT_JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 9.0.1
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(Orbit
    VERSION ${PROJECT_VERSION}
    COMPANY_NAME "MonkeMountainStudios"
    COMPANY_WEBSITE "https://monkemountainstudios.neocities.org"
    BUNDLE_ID "com.monkemountainstudios.orbit"
    PLUGIN_MANUFACTURER_CODE Mnke
    PLUGIN_CODE Orb1
    FORMATS VST3 Standalone
    PRODUCT_NAME "BLORB"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Assets/BlorbEyeIcon.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Assets/BlorbEyeIcon.png"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(Orbit)

target_sources(Orbit PRIVATE
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h)

target_compile_features(Orbit PRIVATE cxx_std_17)

target_compile_definitions(Orbit PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Orbit PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

juce_add_console_app(OrbitDspSmoke PRODUCT_NAME "BLORB DSP Smoke Test")
juce_generate_juce_header(OrbitDspSmoke)
target_sources(OrbitDspSmoke PRIVATE Tests/OrbitDspSmoke.cpp)
target_include_directories(OrbitDspSmoke PRIVATE Source)
target_compile_features(OrbitDspSmoke PRIVATE cxx_std_17)
target_compile_definitions(OrbitDspSmoke PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0)
target_link_libraries(OrbitDspSmoke PRIVATE
    Orbit
    juce::juce_audio_utils
    juce::juce_dsp)
