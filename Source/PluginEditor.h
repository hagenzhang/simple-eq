/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
};

struct ResponseCurveComponent: juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer
{
    ResponseCurveComponent(SimpleeqAudioProcessor&);
    
    ~ResponseCurveComponent();
    
    void parameterValueChanged(int parameterIndex, float newValue) override;
    
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { };
    
    void timerCallback() override;
    
    void paint(juce::Graphics& g) override;
    
private:
    SimpleeqAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged { false };
    
    MonoChain monoChain;
};

//==============================================================================
// This is where we set up all of our visual components.
class SimpleeqAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SimpleeqAudioProcessorEditor (SimpleeqAudioProcessor&);
    ~SimpleeqAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleeqAudioProcessor& audioProcessor;
    
    ResponseCurveComponent responseCurveComponent;
    
    CustomRotarySlider peakFreqSlider,
    peakGainSlider,
    peakQualitySlider,
    lowCutFreqSlider,
    highCutFreqSlider,
    lowCutSlopeSlider,
    highCutSlopeSlider;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    Attachment peakFreqSliderAttachment,
        peakGainSliderAttachment,
        peakQualitySliderAttachment,
        lowCutFreqSliderAttachment,
        highCutFreqSliderAttachment,
        lowCutSlopeSliderAttachment,
        highCutSlopeSliderAttachment;
    
    
    
    
    // When you have a list of objects that you will do the same thing with, you can add them all to a vector
    // so you can iterate through them all easily.
    std::vector<juce::Component*> getComps();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleeqAudioProcessorEditor)
};
