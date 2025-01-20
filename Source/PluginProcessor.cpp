/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleeqAudioProcessor::SimpleeqAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                  )
#endif
{
}

SimpleeqAudioProcessor::~SimpleeqAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleeqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleeqAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SimpleeqAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SimpleeqAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SimpleeqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleeqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleeqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleeqAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleeqAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleeqAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
// This is where all of the pre-playback initialization is done.

void SimpleeqAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // We need to prepare our filters before we use them.
    // This is achieved by passing a process spec object to the chain.
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1; // mono audio can only handle 1 channel.
    spec.sampleRate = sampleRate;
    
    // we have to prepare both the left and right chains.
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    updateFilters();

}

void SimpleeqAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleeqAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif


// This is where we receieve and process the blocks of audio data!
// All of the guts of our plugin should be in this function (specifically, in the second for loop).
// We need to make sure all the operations in here finish in a fixed amount of time!
void SimpleeqAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Processor chain requires a processing context to be passed into it to run audio through the
    // links in the Chain. To make a processing context, we need to supply it with an audio block instance.
    
    // The processBlock function is called by the host, and is given a buffer whichh can have any
    // number of channels. We need to extract the left and right channels (channels 0 and 1, respectively),
    // from this buffer.
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    
    // Tip from tutorial: always update your audio process parameters before you run audio through them.
    updateFilters();
    
    
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    juce::dsp::AudioBlock<float> block(buffer); // start by initializing an AudioBlock, wrapping the buffer.
    
    // retrieve the left and right audio blocks
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    // initialize the left and right audio contexts to pass into the chain (see PluginProcessor.h for notes).
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool SimpleeqAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleeqAudioProcessor::createEditor()
{
    // before officially implementing the GUI, we can visualize our implemented parameters by
    // using the generic audio processor editor.
    return new juce::GenericAudioProcessorEditor(*this);
    
    // return new SimpleeqAudioProcessorEditor (*this);
}

//==============================================================================
void SimpleeqAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    // Note: state is stored as a JUCE Value Tree, and trees serialize to memory very easily
    // We can use memory output streams to write apvts state to the memory block (destData).
    
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void SimpleeqAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        updateFilters();
    }
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    // apvts.getParameter("ParamName")->getValue(); <-- this gives us a normalized value, which is bad.
    // All of the functions that produce coefficients for our filter expects "real world" values, not
    // normalized values, so we use getRawParameterValue instead to retrieve values from apvts.
    // The values from this function are "atomic", which is handy when interacting with the GUI, and the
    // thread safety is a plus.
    
    settings.lowCutFreq = apvts.getRawParameterValue("lowcutfreq") -> load();
    settings.highCutFreq = apvts.getRawParameterValue("highcutfreq") -> load();
    settings.peakFreq = apvts.getRawParameterValue("peakfreq") -> load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("peakgain") -> load();
    settings.peakQuality = apvts.getRawParameterValue("peakquality") -> load();
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope") -> load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope") -> load());
    
    return settings;
}

// Updates all of the settings in the peak filter chain.
void SimpleeqAudioProcessor::updatePeakFilter(const ChainSettings &chainSettings)
{
    // IIR::Coefficients are reference-counted objects that own a juce::Array<float>.
    // These helper functions return instances allocated on the heap.
    // You need to dereference them to copy the underlying coefficients array.
    // Tip from tutorial: allocating on the heap in an audio callback is bad, but we will ignore that poor design decision here.
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                                chainSettings.peakFreq,
                                                                                chainSettings.peakQuality,
                                                                                juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
    
    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
}


// We update Coefficients a lot, so this is a helper function to achieve that.
void SimpleeqAudioProcessor::updateCoefficients(Coefficients &old, const Coefficients &replacements)
{
    *old = *replacements;
}


void SimpleeqAudioProcessor::updateLowCutFilters(const ChainSettings &chainSettings)
{
    // Refer to the implementation of the designIIRHighpassHighOrderButterworthMethod function for how this works.
    // Take a look at the logic for even number orders.
    // It will create 1 IIR filter coefficient object for every 2 orders.
    // We need to produce required number of filter coefficient objects based on the slope param of the filter.
    // This slope parameter had 4 choices, as multiples of 12 (slope -> db/oct, 0 -> 12, 1 -> 24, 2 -> 35,  3 -> 48).
    // So, for a slope of 12, we would need an order of 2 for 1 IIR filter object,
    // for a slope of 24 we would need an order of 4 for 2 IIR filter objects, and so on and so forth.
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                                       getSampleRate(),
                                                                                                       2 * (chainSettings.lowCutSlope + 1));
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
    
    updateCutFilter(leftLowCut, lowCutCoefficients, (Slope)chainSettings.lowCutSlope);
    updateCutFilter(rightLowCut, lowCutCoefficients, (Slope)chainSettings.lowCutSlope);
}

void SimpleeqAudioProcessor::updateHighCutFilters(const ChainSettings &chainSettings)
{
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
                                                                                                          getSampleRate(),
                                                                                                           2 * (chainSettings.highCutSlope + 1));
    
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
    
    updateCutFilter(leftHighCut, highCutCoefficients, (Slope)chainSettings.highCutSlope);
    updateCutFilter(rightHighCut, highCutCoefficients, (Slope)chainSettings.highCutSlope);
}

void SimpleeqAudioProcessor::updateFilters()
{
    auto chainSettings = getChainSettings(apvts);
    
    // after you have your chain settings, you can start producing coefficients using the static helper functions
    // that are part of the IIR coefficients class.
    
    updateLowCutFilters(chainSettings);
    updatePeakFilter(chainSettings);
    updateHighCutFilters(chainSettings);
}



juce::AudioProcessorValueTreeState::ParameterLayout SimpleeqAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"lowcutfreq", 1},
                                                           "LowCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           20.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"highcutfreq", 2},
                                                           "HighCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           200000.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"peakfreq", 3},
                                                           "Peak Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           750.f));
    
    // here we're using db instead of frequency (hz), so the values change accordingly
    // a typical range to use is +- 24 db, and the step change is 0.05 of a decible.
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"peakgain", 4},
                                                           "Peak Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 0.25f),
                                                           0.0f));
    
    // how "tight" or "wide" the band is, or Q value
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"peakquality", 5},
                                                           "Peak Quality",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 0.25f),
                                                           1.f));
    
    
    // for lowcut and highcut filters, we want to adjust steepness, we'll get 4 different options.
    // cut option responses are expressed as decibles per octave, and the math typically rounds out
    // to multiples of 6 or 12 db/octave
    // we will use: 12, 24, 36, 48
    // because our options are more limited, we use the AudioParameterChoice instead.
    juce::StringArray stringArray;
    for (int i = 0; i < 4; i++)
    {
        juce::String str;
        str << (12 + i*12);
        str << " db/Oct";
        stringArray.add(str);
    }
    
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));
    
    // we have our parameters setup now in a ParameterLayout, so we can just pass the layout to the
    // AudioProcessorValueTreeState constructor (code is in the header file);
    return layout;
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleeqAudioProcessor();
}
