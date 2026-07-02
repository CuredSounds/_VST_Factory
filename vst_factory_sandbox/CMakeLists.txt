cmake_minimum_required(VERSION 3.22)

project(vst_factory_sandbox VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build Catch2 DSP test suite" ON)

include(FetchContent)

FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.3
)
FetchContent_MakeAvailable(juce)

add_library(vst_factory_dsp STATIC
    Source/DSP/EffectModules/GainProcessor/GainProcessor.cpp
)
target_include_directories(vst_factory_dsp PUBLIC Source)
target_compile_options(vst_factory_dsp PRIVATE -Wall -Wextra -Werror)

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

target_sources(vst_factory_sandbox PRIVATE
    Source/Core/PluginProcessor.cpp
    Source/UI/PluginEditor.cpp
)

target_include_directories(vst_factory_sandbox PRIVATE Source)

target_compile_definitions(vst_factory_sandbox
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(vst_factory_sandbox
    PRIVATE
        vst_factory_dsp
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra
)

set_source_files_properties(
    Source/Core/PluginProcessor.cpp
    Source/UI/PluginEditor.cpp
    PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra;-Werror"
)

if(BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.4
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(DSP_Tests
        Tests/DSP_Tests/GainProcessorTests.cpp
        Tests/DSP_Tests/NullTest.cpp
        Tests/DSP_Tests/AllocationTest.cpp
        Tests/DSP_Tests/ProcessorIntegration_test.cpp
        Tests/Helpers/AllocationCounter.cpp
        Source/Core/PluginProcessor.cpp
        Source/UI/PluginEditor.cpp
    )

    target_include_directories(DSP_Tests PRIVATE Source Tests)

    target_compile_definitions(DSP_Tests
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
            JUCE_STANDALONE_APPLICATION=1
    )

    target_link_libraries(DSP_Tests
        PRIVATE
            vst_factory_dsp
            Catch2::Catch2WithMain
            juce::juce_audio_utils
            juce::juce_audio_processors
            juce::juce_gui_extra
    )

    set_source_files_properties(
        Tests/DSP_Tests/GainProcessorTests.cpp
        Tests/DSP_Tests/NullTest.cpp
        Tests/DSP_Tests/AllocationTest.cpp
        Tests/DSP_Tests/ProcessorIntegration_test.cpp
        Tests/Helpers/AllocationCounter.cpp
        Source/Core/PluginProcessor.cpp
        Source/UI/PluginEditor.cpp
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra;-Werror"
    )

    include(CTest)
    include(Catch)
    catch_discover_tests(DSP_Tests)
endif()
