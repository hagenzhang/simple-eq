/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>

enum Slope : int
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

struct ChainSettings
{
    float peakFreq { 0 }, peakGainInDecibels{ 0 }, peakQuality{ 1.f };
    float lowCutFreq { 0 }, highCutFreq { 0 };
    int lowCutSlope { Slope::Slope_12 }, highCutSlope { Slope::Slope_12 };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

// JUCE DSP namespace uses a lot of template metaprogramming and nested namespaces, so we're gonna
// create some type aliases to make things simpler.

// filter aliases:
using Filter = juce::dsp::IIR::Filter<float>;

// Slope of cut filter is a multiple 12, each of the filters in the IIR Filter class has a response of 12db
// per octave when configured as a low/high pass filter.
// If we want a chain with a response of 48 db per octave, we need 4 filters.

// We define a Chain, and pass in a processing context which runs through each element of the Chain
// automatically. We put 4 filters in a processing Chain and pass in 1 single processing context,
// and it will run through all 4 of the filters automatically.
using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
// We can also configure these filters to work as a peak filter, shelf, notch, bandpass, etc.

// We define a chain to represent 1 mono signal path: LowCut -> Parametric -> HighCut.
using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;


enum ChainPositions // all of the filters we have in a Mono Chain
{
    LowCut,
    Peak,
    HighCut
};

using Coefficients = Filter::CoefficientsPtr; // alias for convenience
void updateCoefficients(Coefficients& old, const Coefficients& replacements);

Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate);

template<int Index, typename ChainType, typename CoefficientType>
void update(ChainType& chain, const CoefficientType& coefficients)
{
    updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
    chain.template setBypassed<Index>(false);
}

template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& chain,
                     const CoefficientType& coefficients,
                     const Slope& slope)
{
    // bypassing all of the links in the chain
    chain.template setBypassed<0>(true);
    chain.template setBypassed<1>(true);
    chain.template setBypassed<2>(true);
    chain.template setBypassed<3>(true);
    
    switch (slope)
    {
        case Slope_48:
            update<3>(chain, coefficients);
        case Slope_36:
            update<2>(chain, coefficients);
        case Slope_24:
            update<1>(chain, coefficients);
        case Slope_12:
            update<0>(chain, coefficients);
    }
}

inline auto makeLowCutFilter(const ChainSettings& chainSettings, double sampleRate)
{
    // Refer to the implementation of the designIIRHighpassHighOrderButterworthMethod function for how this works.
    // Take a look at the logic for even number orders.
    // It will create 1 IIR filter coefficient object for every 2 orders.
    // We need to produce required number of filter coefficient objects based on the slope param of the filter.
    // This slope parameter had 4 choices, as multiples of 12 (slope -> db/oct, 0 -> 12, 1 -> 24, 2 -> 35,  3 -> 48).
    // So, for a slope of 12, we would need an order of 2 for 1 IIR filter object,
    // for a slope of 24 we would need an order of 4 for 2 IIR filter objects, and so on and so forth.
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                       sampleRate,
                                                                                       2 * (chainSettings.lowCutSlope + 1));
}

inline auto makeHighCutFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
                                                                                      sampleRate,
                                                                                      2 * (chainSettings.highCutSlope + 1));
}


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
    MonoChain leftChain, rightChain; // two chains for Stereo out.
    
    void updatePeakFilter(const ChainSettings& chainSettings);
    void updateLowCutFilters(const ChainSettings& chainSettings);
    void updateHighCutFilters(const ChainSettings& chainSettings);
    void updateFilters();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleeqAudioProcessor)
};


/*
 Notes from the tutorial:
 
 Audio plugins rely on parameters to control the parts of the DSP (Digital Signal Processor).
 JUCE uses the "AudioProcessorValueTreeState" (a class) to coordinate syncing the
 knobs on the GUI and the parameters of the DSP (It needs to be public!).
 
 Our plugin will run stereo audio, which will require 2 channels of audio. The signal processing
 classes in the DSP namespace by default run 1 channel of audio, so we will have to duplicate a
 lot of things for 2 channels of audio.
 */
