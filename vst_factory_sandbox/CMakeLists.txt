cmake_minimum_required(VERSION 3.22)

project(vst_factory_sandbox VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch JUCE Framework from GitHub
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.3
)
FetchContent_MakeAvailable(juce)

# Create the Audio Plugin
juce_add_plugin(vst_factory_sandbox
    PRODUCT_NAME "VST_Factory_Sandbox"
    PLUGIN_NAME "VST_Factory_Sandbox"
    COMPANY_NAME "SonicDesign"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    FORMATS VST3 AU Standalone
)

# Target compile sources
target_sources(vst_factory_sandbox PRIVATE
    Source/Core/PluginProcessor.h
    Source/Core/PluginProcessor.cpp
    Source/Core/PluginEditor.h
    Source/Core/PluginEditor.cpp
    Source/Core/Parameters.h
)

# Header search paths
target_include_directories(vst_factory_sandbox PRIVATE
    Source
)

# Libraries to link
target_link_libraries(vst_factory_sandbox PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
)
