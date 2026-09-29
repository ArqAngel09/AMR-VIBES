cmake_minimum_required(VERSION 3.22)
project(AMRVibes VERSION 1.0.0)
include(FetchContent)
FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 9.0.2 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(juce)

juce_add_plugin(AMRVibes
 COMPANY_NAME "Angel Music Records"
 BUNDLE_ID com.angelmusicrecords.amrvibes
 PLUGIN_MANUFACTURER_CODE AMRV
 PLUGIN_CODE AV2M
 FORMATS VST3 Standalone
 PRODUCT_NAME "AMR VIBES"
 DESCRIPTION "Voice and melody audio to MIDI converter"
 IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT TRUE IS_MIDI_EFFECT FALSE
 COPY_PLUGIN_AFTER_BUILD TRUE VST3_CATEGORIES "Tools")

juce_generate_juce_header(AMRVibes)

target_sources(AMRVibes PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_features(AMRVibes PRIVATE cxx_std_17)
target_compile_definitions(AMRVibes PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(AMRVibes PRIVATE juce::juce_audio_utils juce::juce_audio_formats juce::juce_audio_basics juce::juce_dsp juce::juce_gui_extra juce::juce_recommended_config_flags juce::juce_recommended_warning_flags juce::juce_recommended_lto_flags)
