/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class SimpleeqAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleeqAudioProcessor();
    ~SimpleeqAudioProcessor() override;

    //==============================================================================
    // prepareToPlay: called by host whenever the plugin is about to start playback
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif
    
    // processBlock: called when you hit play button in the transport control
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    // Note:
    // when the play button is hit, the host will send buffers at a regular rate
    // into your plugin, and it's the job of the plugin to return the audio after
    // it has been processed. if you add latency or interrupt the chain of events,
    // it can cause audio pops and glitches, which may lead to damaged speakers or
    // even worse, damaged ears! all work needs to be done in a fixed amount of time.

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    
    
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleeqAudioProcessor)
};


/*
 Notes from the tutorial:
 
 Audio plugins rely on parameters to control the parts of the DSP.
 JUCE uses the "AudioProcessorValueTreeState" (a class) to coordinate syncing the
 knobs on the GUI and the parameters of the DSP (It needs to be public!).
 
 
  
 
 
 */
