

What to do research on:
1) : and ::, like "juce::Slider::SliderStyle" vs "CustomRotarySlider() : juce::Slider"
2) meaning of keywords:
    - "override"
    - "const" inside of a method header (like:
    '''
        bool SimpleeqAudioProcessor::hasEditor() const
        {
            return true; // (change this to false if you choose to not supply an editor)
        })
    '''
    or 
    '''
    void SimpleeqAudioProcessor::setStateInformation (const void\* data, int sizeInBytes)
    '''
    or even
    '''
    bool acceptsMidi() const override;
    '''
    - "auto"
    - "static_cast"
    - "template"
    - "using"
    
3) meaning of symbols like \*, &, and others
    - "juce::Component\*"
    - void SimpleeqAudioProcessor::setStateInformation (const void\* data, int sizeInBytes)
    - '''
        void SimpleeqAudioProcessor::updateCoefficients(Coefficients &old, const Coefficients &replacements)
        {
            \*old = \*replacements;
        }
      '''
    - "&highCutFreqSlider"
    - "for (auto& comp : getComps())" like why & first vs & last?
4) Atomic Values

      
      
    
    
