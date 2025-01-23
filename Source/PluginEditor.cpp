/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

ResponseCurveComponent::ResponseCurveComponent(SimpleeqAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    
    for (auto& param : params)
    {
        param->addListener(this);
    }
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for (auto& param : params)
    {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback()
{
    DBG("params changed");
    if (parametersChanged.compareAndSetBool(false, true))
    {
        // update the monochain
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
        
        updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
        
        auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
        
        updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, (Slope)chainSettings.lowCutSlope);
        updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, (Slope)chainSettings.highCutSlope);
        
        // signal a repaint
        repaint();
    }
}

void ResponseCurveComponent::paint (juce::Graphics& g)
{
    using namespace juce;
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
    
    auto responseArea = getLocalBounds();
    auto width = responseArea.getWidth();
    
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<double> mags;
    
    mags.resize(width);
    
    for (int i = 0; i < width; ++i)
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(width), 20.0, 20000.0);
        
        if (monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (lowcut.isBypassed<0>())
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (lowcut.isBypassed<1>())
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (lowcut.isBypassed<2>())
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (lowcut.isBypassed<3>())
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (highcut.isBypassed<0>())
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (highcut.isBypassed<1>())
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (highcut.isBypassed<2>())
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (highcut.isBypassed<3>())
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        mags[i] = Decibels::gainToDecibels(mag);
    }
    
    Path responseCurve;
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    
    for (size_t i=1; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }
    
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);
    
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
}


//==============================================================================
SimpleeqAudioProcessorEditor::SimpleeqAudioProcessorEditor (SimpleeqAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p),

peakFreqSlider(*audioProcessor.apvts.getParameter("peakfreq"), "Hz"),
peakGainSlider(*audioProcessor.apvts.getParameter("peakgain"), "dB"),
peakQualitySlider(*audioProcessor.apvts.getParameter("peakquality"), ""),
lowCutFreqSlider(*audioProcessor.apvts.getParameter("lowcutfreq"), "Hz"),
highCutFreqSlider(*audioProcessor.apvts.getParameter("highcutfreq"), "Hz"),
lowCutSlopeSlider(*audioProcessor.apvts.getParameter("lowcutslope"), "dB/Oct"),
highCutSlopeSlider(*audioProcessor.apvts.getParameter("highcutslope"), "dB/Oct"),

responseCurveComponent(audioProcessor),
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
    using namespace juce;
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
    
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
    
    responseCurveComponent.setBounds(responseArea);
    
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
        &highCutSlopeSlider,
        &responseCurveComponent
    };
}
