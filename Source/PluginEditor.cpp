/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleeqAudioProcessorEditor::SimpleeqAudioProcessorEditor (SimpleeqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
peakFreqSliderAttachment(audioProcessor.apvts, "peakfreq", peakFreqSlider),
peakGainSliderAttachment(audioProcessor.apvts, "peakgain", peakGainSlider),
peakQualitySliderAttachment(audioProcessor.apvts, "peakquality", peakQualitySlider),
lowCutFreqSliderAttachment(audioProcessor.apvts, "lowcutfreq", lowCutFreqSlider),
highCutFreqSliderAttachment(audioProcessor.apvts, "highcutfreq", highCutFreqSlider),
lowCutSlopeSliderAttachment(audioProcessor.apvts, "lowcutslope", lowCutSlopeSlider),
highCutSlopeSliderAttachment(audioProcessor.apvts, "highcutslope", highCutSlopeSlider)
{
    
    for (auto& comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
}

SimpleeqAudioProcessorEditor::~SimpleeqAudioProcessorEditor()
{
}

//==============================================================================
void SimpleeqAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleeqAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    // JUCE_LIVE_CONSTANT() is very useful for dialing in specific positions of your components while your component is visible
    
    // total bounding area
    auto bounds = getLocalBounds();
    
    // area allocated for the response curve
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    
    // lowcut stuff on left, highcut on right
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    // we take 0.5 instead of 0.33 because the lowCutArea took away 0.33 already, so now
    // the bounds area only represents 66% of the total available screen.
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    
    // setting the bounds of the sliders
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);
    
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);
    
    
    // adding peak sliders to the top 0.33 and top 0.66 of the screen
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualitySlider.setBounds(bounds);
}


std::vector<juce::Component*> SimpleeqAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider
    };
}
