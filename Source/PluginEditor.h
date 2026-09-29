#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class AMRVibesAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
 explicit AMRVibesAudioProcessorEditor(AMRVibesAudioProcessor&);
 ~AMRVibesAudioProcessorEditor() override = default;
 void paint(juce::Graphics&) override; void resized() override;
private:
 AMRVibesAudioProcessor& p;
 juce::TextButton load{"Load Audio"}, record{"Record"}, analyze{"Analyze / Convert"}, clear{"Clear"}, exportBtn{"Export MIDI"};
 juce::ComboBox root, scale; juce::Slider bpm, sensitivity, octave, quantize;
 juce::Label status;
 void timerCallback() override;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AMRVibesAudioProcessorEditor)
};
