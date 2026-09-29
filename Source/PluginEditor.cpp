#include "PluginEditor.h"
AMRVibesAudioProcessorEditor::AMRVibesAudioProcessorEditor(AMRVibesAudioProcessor& x):AudioProcessorEditor(&x),p(x){
 setSize(620,420);
 for(auto* b:{&load,&record,&analyze,&clear,&exportBtn}){addAndMakeVisible(b);}
 load.onClick=[this]{p.loadAudio();}; record.onClick=[this]{p.toggleRecording();}; analyze.onClick=[this]{p.analyze();}; clear.onClick=[this]{analyze.setEnabled(true);};
 exportBtn.onClick=[this]{p.exportMidi();};
 addAndMakeVisible(root); const char* roots[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};for(auto s:roots)root.addItem(s,root.getNumItems()+1);root.setSelectedId(1);root.onChange=[this]{p.params().getParameter("root")->setValueNotifyingHost((root.getSelectedId()-1)/11.0f);};
 addAndMakeVisible(scale);scale.addItem("Major",1);scale.addItem("Minor",2);scale.addItem("Chromatic",3);scale.setSelectedId(1);scale.onChange=[this]{p.params().getParameter("scale")->setValueNotifyingHost((scale.getSelectedId()-1)/2.0f);};
 auto setup=[this](juce::Slider& s,double lo,double hi,double val){addAndMakeVisible(s);s.setRange(lo,hi,1);s.setValue(val);s.setSliderStyle(juce::Slider::LinearHorizontal);};
 setup(bpm,40,240,120);setup(sensitivity,.05,1,.35);setup(octave,-2,2,0);setup(quantize,0,4,0);
 bpm.onValueChange=[this]{p.params().getParameter("bpm")->setValueNotifyingHost((float)((bpm.getValue()-40)/200));};
 sensitivity.onValueChange=[this]{p.params().getParameter("sensitivity")->setValueNotifyingHost((float)((sensitivity.getValue()-.05)/.95));};
 octave.onValueChange=[this]{p.params().getParameter("octave")->setValueNotifyingHost((float)((octave.getValue()+2)/4));};
 startTimerHz(8);
}
void AMRVibesAudioProcessorEditor::paint(juce::Graphics& g){g.fillAll(juce::Colour(0xff171717));g.setColour(juce::Colours::white);g.setFont(26);g.drawText("AMR VIBES",24,18,300,36,juce::Justification::left);g.setFont(14);g.drawText("Voice / melody → MIDI",24,52,300,24,juce::Justification::left);g.drawText("Root",24,92,100,22,juce::Justification::left);g.drawText("Scale",220,92,100,22,juce::Justification::left);g.drawText("BPM",24,180,100,22,juce::Justification::left);g.drawText("Sensitivity",220,180,100,22,juce::Justification::left);g.drawText("Octave",420,180,100,22,juce::Justification::left);}
void AMRVibesAudioProcessorEditor::resized(){load.setBounds(24,70,130,34);record.setBounds(164,70,100,34);analyze.setBounds(274,70,160,34);clear.setBounds(444,70,70,34);exportBtn.setBounds(524,70,80,34);root.setBounds(70,112,120,28);scale.setBounds(270,112,150,28);bpm.setBounds(24,202,170,28);sensitivity.setBounds(220,202,170,28);octave.setBounds(420,202,170,28);status.setBounds(24,300,560,30);}
void AMRVibesAudioProcessorEditor::timerCallback(){repaint();}
