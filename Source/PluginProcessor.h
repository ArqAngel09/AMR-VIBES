#pragma once
#include <JuceHeader.h>
class AMRVibesAudioProcessor : public juce::AudioProcessor, private juce::Timer {
public:
 AMRVibesAudioProcessor();
 ~AMRVibesAudioProcessor() override;
 void prepareToPlay(double sampleRate,int samplesPerBlock) override;
 void releaseResources() override;
 bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override { return true; }
 const juce::String getName() const override { return "AMR VIBES"; }
 bool acceptsMidi() const override { return false; } bool producesMidi() const override { return true; }
 bool isMidiEffect() const override { return false; }
 double getTailLengthSeconds() const override { return 0; }
 int getNumPrograms() override { return 1; } int getCurrentProgram() override { return 0; }
 void setCurrentProgram(int) override {} const juce::String getProgramName(int) override { return {}; } void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
 void loadAudio(); void toggleRecording(); void analyze(); bool exportMidi(); void clearAudio();
 juce::String status() const; bool recording() const { return isRecording.load(); }
 juce::AudioProcessorValueTreeState& params() { return apvts; }
private:
 void timerCallback() override; void analyzeBuffer(juce::AudioBuffer<float>,double);
 float detectPitch(const float*,int,double,float& confidence) const;
 int fitPitchClass(int pc) const;
 juce::AudioProcessorValueTreeState apvts;
 juce::AudioFormatManager formatManager; juce::AudioSampleBuffer audio; juce::MidiMessageSequence sequence;
 std::unique_ptr<juce::FileChooser> fileChooser;
 juce::CriticalSection mutex; std::atomic<bool> isRecording{false}; std::atomic<bool> analyzing{false};
 double sampleRate=44100; juce::String statusText="Ready"; juce::File lastMidi;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AMRVibesAudioProcessor)
};