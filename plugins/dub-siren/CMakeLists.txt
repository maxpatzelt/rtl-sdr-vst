cmake_minimum_required(VERSION 3.15)

project(DubSiren VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE as subdirectory - assumes JUCE is in libs/JUCE
# Clone with: git submodule add https://github.com/juce-framework/JUCE.git libs/JUCE
# Or use FetchContent to download automatically (see README)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/JUCE/CMakeLists.txt")
    add_subdirectory(libs/JUCE)
else()
    message(FATAL_ERROR
        "JUCE not found! Please run:\n"
        "  git submodule add https://github.com/juce-framework/JUCE.git libs/JUCE\n"
        "  git submodule update --init --recursive\n"
        "Or see README.md for FetchContent alternative.")
endif()

# Create the plugin target
juce_add_plugin(DubSiren
    COMPANY_NAME "Max Patzelt"
    PLUGIN_MANUFACTURER_CODE MxPz
    PLUGIN_CODE Syn1
    FORMATS VST3
    PRODUCT_NAME "Dub Siren"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST2_CATEGORY kPlugSynthInstrument)

# Add binary resources
juce_add_binary_data(DubSirenResources
    SOURCES
        Resources/panel.jpg
        Resources/background.jfif
        Resources/ds01.wav
        Resources/ds02.wav
        Resources/ds03.wav
        Resources/ds04.wav
        Resources/ds05.wav
        Resources/ds06.wav
        Resources/ds07.wav
        Resources/ds08.wav
        Resources/ds09.wav
        Resources/ds10.wav
        Resources/ds11.wav
        Resources/ds12.wav
        Resources/ds13.wav
        Resources/ds14.wav)

# Add source files
target_sources(DubSiren
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/DSP/Oscillator.cpp
        Source/DSP/Oscillator.h
        Source/DSP/Envelope.cpp
        Source/DSP/Envelope.h
        Source/DSP/Voice.cpp
        Source/DSP/Voice.h
        Source/DSP/DubOscillator.cpp
        Source/DSP/DubOscillator.h
        Source/DSP/LFO.cpp
        Source/DSP/LFO.h
        Source/DSP/DubDelay.cpp
        Source/DSP/DubDelay.h
        Source/DSP/Common.h
        Source/DSP/SamplePlayer.h)

# Compile definitions
target_compile_definitions(DubSiren
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0)

# Set C++17 and enable warnings
target_compile_features(DubSiren PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(DubSiren PRIVATE /W4)
else()
    target_compile_options(DubSiren PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Link JUCE modules
target_link_libraries(DubSiren
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_dsp
        DubSirenResources
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Enable testing
enable_testing()
add_subdirectory(Tests)
