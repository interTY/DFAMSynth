cmake_minimum_required(VERSION 3.22)
project(DFAMSynth VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE as a subdirectory (override with -DJUCE_DIR=path)
if(NOT DEFINED JUCE_DIR)
    if(EXISTS "C:/JUCE/CMakeLists.txt")
        set(JUCE_DIR "C:/JUCE")
    elseif(EXISTS "$ENV{HOME}/JUCE/CMakeLists.txt")
        set(JUCE_DIR "$ENV{HOME}/JUCE")
    else()
        message(FATAL_ERROR "JUCE not found. Set -DJUCE_DIR=<path-to-JUCE>")
    endif()
endif()
add_subdirectory("${JUCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/JUCE")

juce_add_plugin(DFAMSynth
    COMPANY_NAME "DFAMSynth"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Dfam
    PLUGIN_CODE Dfam
    FORMATS VST3 Standalone
    PRODUCT_NAME "DFAM Synth"
)

juce_generate_juce_header(DFAMSynth)

target_sources(DFAMSynth
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/DSP/Oscillator.cpp
        Source/DSP/Envelope.cpp
        Source/DSP/NoiseGenerator.cpp
        Source/DSP/LadderFilter.cpp
        Source/Sequencer/Sequencer.cpp
)

target_include_directories(DFAMSynth
    PRIVATE
        Source
)

target_compile_definitions(DFAMSynth
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(DFAMSynth
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
