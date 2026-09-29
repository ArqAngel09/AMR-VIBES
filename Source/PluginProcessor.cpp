#include "PluginProcessor.h"
#include "PluginEditor.h"
AMRVibesAudioProcessor::AMRVibesAudioProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)){
 formatManager.registerBasicFormats();
 apvts.createAndAddParameter("root","Root","Root",juce::NormalisableRange<float>(0,11,1),0,{},nullptr);
 apvts.createAndAddParameter("scale","Scale","Scale",juce::NormalisableRange<float>(0,2,1),0,{},nullptr);
 apvts.createAndAddParameter("bpm","BPM","BPM",juce::NormalisableRange<float>(40,240,1),120,{},nullptr);
 apvts.createAndAddParameter("quant","Quantize","Quantize",juce::NormalisableRange<float>(0,4,1),0,{},nullptr);
 apvts.createAndAddParameter("octave","Octave","Octave",juce::NormalisableRange<float>(-2,2,1),0,{},nullptr);
 apvts.createAndAddParameter("sensitivity","Sensitivity","Sensitivity",juce::NormalisableRange<float>(0.05f,1,0.01f),0.35f,{},nullptr);
 startTimerHz(8);
}
AMRVibesAudioProcessor::~AMRVibesAudioProcessor(){}
void AMRVibesAudioProcessor::prepareToPlay(double sr,int){sampleRate=sr;}
void AMRVibesAudioProcessor::releaseResources(){}
bool AMRVibesAudioProcessor::isBusesLayoutSupported(const BusesLayout& l)const{return l.getMainInputChannelSet()!=juce::AudioChannelSet::disabled();}
void AMRVibesAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& m){
 juce::ScopedNoDenormals no; m.clear();
 if(isRecording.load()){const int n=b.getNumSamples(); juce::ScopedLock lock(mutex); int old=audio.getNumSamples(); audio.setSize(1,old+n,true); const float* l=b.getReadPointer(0); const float* r=b.getNumChannels()>1?b.getReadPointer(1):l; float* d=audio.getWritePointer(0,old); for(int i=0;i<n;++i)d[i]=0.5f*(l[i]+r[i]); if(audio.getNumSamples()>(int)(sampleRate*120.0))isRecording=false;}
}
void AMRVibesAudioProcessor::loadAudio(){juce::FileChooser c("Load audio",{}, "*.wav;*.aif;*.aiff"); if(!c.browseForFileToOpen())return; std::unique_ptr<juce::AudioFormatReader> r(formatManager.createReaderFor(c.getResult())); if(!r)return; juce::AudioSampleBuffer x(1,(int)r->lengthInSamples); r->read(&x,0,x.getNumSamples(),0,true,false); {juce::ScopedLock l(mutex); audio=std::move(x);} statusText="Audio loaded"; analyze();}
void AMRVibesAudioProcessor::toggleRecording(){if(isRecording){isRecording=false;statusText="Recording stopped";}else{juce::ScopedLock l(mutex);audio.setSize(1,0);isRecording=true;statusText="Recording...";}}
float AMRVibesAudioProcessor::detectPitch(const float* x,int n,double sr,float& conf)const{
 double best=0; int bestLag=0; double e=0; for(int i=0;i<n;++i)e+=x[i]*x[i]; if(e<1e-5){conf=0;return 0;}
 int minLag=(int)(sr/1000), maxLag=(int)(sr/70); for(int lag=minLag;lag<=maxLag&&lag<n-1;++lag){double c=0; for(int i=lag;i<n;++i)c+=x[i]*x[i-lag]; c/=e; if(c>best){best=c;bestLag=lag;}} conf=(float)juce::jlimit(0.0,best,1.0); return bestLag?float(sr/bestLag):0;
}
int AMRVibesAudioProcessor::fitPitchClass(int pc)const{
 int root=(int)apvts.getRawParameterValue("root")->load(), sc=(int)apvts.getRawParameterValue("scale")->load();
 if(sc==2)return pc; static const int maj[]={0,2,4,5,7,9,11}; static const int min[]={0,2,3,5,7,8,10}; const int* s=sc==0?maj:min;
 int best=pc,b=99; for(int k:s){int cand=(root+k)%12,d=std::abs(cand-pc);d=std::min(d,12-d);if(d<b){b=d;best=cand;}} return best;
}
void AMRVibesAudioProcessor::analyze(){if(analyzing.exchange(true))return; juce::AudioSampleBuffer x; {juce::ScopedLock l(mutex); x=audio;} if(x.getNumSamples()<512){statusText="No audio";analyzing=false;return;} juce::Thread::launch([this,x=std::move(x)]()mutable{analyzeBuffer(std::move(x),sampleRate);});}
void AMRVibesAudioProcessor::analyzeBuffer(juce::AudioSampleBuffer x,double sr){
 juce::MidiMessageSequence out; const int hop=512, win=2048; int last=-1,start=0; double step=double(hop)/sr; float sens=apvts.getRawParameterValue("sensitivity")->load(); int oct=(int)apvts.getRawParameterValue("octave")->load();
 for(int pos=0;pos+win<=x.getNumSamples();pos+=hop){float c=0,p=detectPitch(x.getReadPointer(0,pos),win,sr,c); if(c<sens||p<=0)continue; int note=juce::jlimit(0,127,(int)std::lround(69+12*std::log2(p/440.0))+12*oct); int pc=fitPitchClass(note%12); note += pc-(note%12); note=juce::jlimit(0,127,note); if(last<0){last=note;start=pos;} else if(std::abs(note-last)>1){double t0=start/sr,t1=pos/sr; int vel=100; out.addEvent(juce::MidiMessage::noteOn(1,last,(juce::uint8)vel),t0); out.addEvent(juce::MidiMessage::noteOff(1,last),t1); last=note;start=pos;}}
 if(last>=0){double t0=start/sr,t1=x.getNumSamples()/sr;out.addEvent(juce::MidiMessage::noteOn(1,last,(juce::uint8)100),t0);out.addEvent(juce::MidiMessage::noteOff(1,last),t1);}
 out.updateMatchedPairs(); {juce::ScopedLock l(mutex);sequence=out;} statusText="Conversion complete"; analyzing=false;
}
bool AMRVibesAudioProcessor::exportMidi(){juce::MidiFile f;f.setTicksPerQuarterNote(480);f.addTrack(sequence);juce::FileChooser c("Export MIDI",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("AMR-VIBES.mid"),"*.mid");if(!c.browseForFileToSave(true))return false;juce::FileOutputStream s(c.getResult());if(!s.openedOk())return false;f.writeTo(s);statusText="MIDI exported";return true;}
juce::String AMRVibesAudioProcessor::status()const{return statusText;}
void AMRVibesAudioProcessor::timerCallback(){}
void AMRVibesAudioProcessor::getStateInformation(juce::MemoryBlock& d){auto x=apvts.copyState();std::unique_ptr<juce::XmlElement> e=x.createXml();copyXmlToBinary(*e,d);}
void AMRVibesAudioProcessor::setStateInformation(const void* d,int n){std::unique_ptr<juce::XmlElement> e(getXmlFromBinary(d,n));if(e&&e->hasTagName(apvts.state.getType()))apvts.replaceState(juce::ValueTree::fromXml(*e));}
juce::AudioProcessorEditor* AMRVibesAudioProcessor::createEditor(){return new AMRVibesAudioProcessorEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AMRVibesAudioProcessor();}
