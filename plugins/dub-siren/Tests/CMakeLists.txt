# Test executable configuration
add_executable(SimpleSynth_Tests
    test_Main.cpp
    test_Oscillator.cpp
    test_Envelope.cpp
    # Include DSP sources directly for testing
    ../Source/DSP/Oscillator.cpp
    ../Source/DSP/Envelope.cpp
    ../Source/DSP/Voice.cpp)

# Link minimal JUCE modules needed for tests
target_link_libraries(SimpleSynth_Tests
    PRIVATE
        juce::juce_audio_basics
        juce::juce_dsp)

target_compile_features(SimpleSynth_Tests PRIVATE cxx_std_17)

# Add include path for DSP headers
target_include_directories(SimpleSynth_Tests PRIVATE ../Source)

if(MSVC)
    target_compile_options(SimpleSynth_Tests PRIVATE /W4)
else()
    target_compile_options(SimpleSynth_Tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Register test with CTest
add_test(NAME SimpleSynth_Tests COMMAND SimpleSynth_Tests)
