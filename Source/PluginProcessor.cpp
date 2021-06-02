/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::String ParametricEQAudioProcessor::getFilterCutoffParamName(int index)
{
    switch (index)
    {
    case 0: return "Band0Cutoff"; break;
    case 1: return "Band1Cutoff"; break; 
    case 2: return "Band2Cutoff"; break;
    case 3: return "Band3Cutoff"; break;
    }
    return "invalid";
}

juce::String ParametricEQAudioProcessor::getFilterQParamName(int index)
{
    switch (index)
    {
    case 0: return "Band0Q"; break;
    case 1: return "Band1Q"; break;
    case 2: return "Band2Q"; break;
    case 3: return "Band3Q"; break;
    }
    return "invalid";
}

juce::String ParametricEQAudioProcessor::getFilterGainParamName(int index)
{
    switch (index)
    {
    case 0: return "Band0Gain"; break;
    case 1: return "Band1Gain"; break;
    case 2: return "Band2Gain"; break;
    case 3: return "Band3Gain"; break;
    }
    return "invalid";
}

juce::String ParametricEQAudioProcessor::getFilterBandName(int index)
{
    switch (index)
    {
    case 0: return "Band0"; break;
    case 1: return "Band1"; break;
    case 2: return "Band2"; break;
    case 3: return "Band3"; break;
    }
    return "invalid";
}

int ParametricEQAudioProcessor::getBandIndexFromID(juce::String paramID)
{
    for (int i = 0; i < 4; ++i)
        if (paramID.startsWith(getFilterBandName(i)))
        {
            return int(i);
        }
    return -1;
}

//==============================================================================
ParametricEQAudioProcessor::ParametricEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), tree(*this, nullptr, "PARAMS", createParameterLayout()), lastSampleRate(44100.0f)

#endif
{
    *filterChain.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(44100, 134.2f, 0.62f, 1.0f);
    *filterChain.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 883.9f, 5.7f, 1.0f);
    *filterChain.get<2>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 883.9f, 5.7f, 1.0f);
    *filterChain.get<3>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(44100, 6000.0f, 0.62f, 1.0f);
    for (int i = 0; i < 4; ++i) 
    {
        tree.addParameterListener(getFilterCutoffParamName(i), this);
        tree.addParameterListener(getFilterQParamName(i), this);
        tree.addParameterListener(getFilterGainParamName(i), this);
    }
    
}

ParametricEQAudioProcessor::~ParametricEQAudioProcessor()
{

}

juce::AudioProcessorValueTreeState::ParameterLayout ParametricEQAudioProcessor::createParameterLayout()
{
    juce::NormalisableRange<float> fullParamCutoffRange(20.0f, 20000.0f);
    fullParamCutoffRange.setSkewForCentre(883.9f);

    juce::NormalisableRange<float> fullParamQRange(1.0f, 32.0f);
    fullParamQRange.setSkewForCentre(5.7f);

    juce::NormalisableRange<float> lowShelfCutoffRange(20.0f, 600.0f);
    lowShelfCutoffRange.setSkewForCentre(134.2f);

    juce::NormalisableRange<float> lowShelfQRange(0.5f, 2.0f);
    lowShelfQRange.setSkewForCentre(1.0f);

    juce::NormalisableRange<float> highShelfCutoffRange(3000.0f, 12000.0f);
    highShelfCutoffRange.setSkewForCentre(6000.0f);

    juce::NormalisableRange<float> highShelfQRange(0.5f, 2.0f);
    highShelfQRange.setSkewForCentre(1.0f);

    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    for (int i = 0; i < 4; ++i)
    {
        switch (i)
        {
        case 0: 
        {
            auto cutoffParam = std::make_unique<juce::AudioParameterFloat>
                (getFilterCutoffParamName(i), getFilterCutoffParamName(i), lowShelfCutoffRange, 134.2f,
                    juce::String(), juce::AudioProcessorParameter::genericParameter,
                    [](float param, int) {return juce::String(param, 1); });
            
            
            auto qParam = std::make_unique<juce::AudioParameterFloat>
                (getFilterQParamName(i), getFilterQParamName(i), lowShelfQRange, 0.62f,
                    juce::String(), juce::AudioProcessorParameter::genericParameter,
                    [](float param, int) {return juce::String(param, 2); });

            params.push_back(std::move(cutoffParam));
            params.push_back(std::move(qParam));
            break;
        }
        case 1: 
        {
            auto cutoffParam = std::make_unique<juce::AudioParameterFloat>
                (getFilterCutoffParamName(i), getFilterCutoffParamName(i), fullParamCutoffRange, 883.9f,
                    juce::String(), juce::AudioProcessorParameter::genericParameter,
                    [](float param, int) {return juce::String(param, 1); });

            auto qParam = std::make_unique<juce::AudioParameterFloat>
                (getFilterQParamName(i), getFilterQParamName(i), fullParamQRange, 5.7f,
                    juce::String(), juce::AudioProcessorParameter::genericParameter,
                    [](float param, int) {return juce::String(param, 2); });

            params.push_back(std::move(cutoffParam));
            params.push_back(std::move(qParam));
            break;
        }
        case 2:
        {
            auto cutoffParam = std::make_unique<juce::AudioParameterFloat>
                (getFilterCutoffParamName(i), getFilterCutoffParamName(i), fullParamCutoffRange, 883.9f,
                    juce::String(), juce::AudioProcessorParameter::genericParameter,
                    [](float param, int) {return juce::String(param, 1); });

            auto qParam = std::make_unique<juce::AudioParameterFloat>
                (getFilterQParamName(i), getFilterQParamName(i), fullParamQRange, 5.7f,
                    juce::String(), juce::AudioProcessorParameter::genericParameter,
                    [](float param, int) {return juce::String(param, 2); });

            params.push_back(std::move(cutoffParam));
            params.push_back(std::move(qParam));
            break;
            
        }
        case 3:
        {
            auto cutoffParam = std::make_unique<juce::AudioParameterFloat>
                (getFilterCutoffParamName(i), getFilterCutoffParamName(i), highShelfCutoffRange, 6000.0f,
                    juce::String(), juce::AudioProcessorParameter::genericParameter,
                    [](float param, int) {return juce::String(param, 1); });

            auto qParam = std::make_unique<juce::AudioParameterFloat>
                (getFilterQParamName(i), getFilterQParamName(i), highShelfQRange, 0.62f,
                    juce::String(), juce::AudioProcessorParameter::genericParameter,
                    [](float param, int) {return juce::String(param, 2); });

            params.push_back(std::move(cutoffParam));
            params.push_back(std::move(qParam));
            break;
        }
        }
        auto gainParam = std::make_unique<juce::AudioParameterFloat>
            (getFilterGainParamName(i), getFilterGainParamName(i), -18.0f, 18.0f, 0.0f);

        params.push_back(std::move(gainParam));

    }
    return { params.begin(), params.end() };
}

void ParametricEQAudioProcessor::parameterChanged(const juce::String& parameter, float newValue)
{
    int index = getBandIndexFromID(parameter);
    updateFilter(index);
}

//==============================================================================
const juce::String ParametricEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ParametricEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ParametricEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ParametricEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ParametricEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ParametricEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ParametricEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ParametricEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ParametricEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void ParametricEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ParametricEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    lastSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    filterChain.prepare(spec);
    filterChain.reset();
}

void ParametricEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ParametricEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ParametricEQAudioProcessor::updateFilter(int index)
{
    juce::String cutoffID = getFilterCutoffParamName(index);
    juce::String qID = getFilterQParamName(index);
    juce::String gainID = getFilterGainParamName(index); 

    float cutoff = *tree.getRawParameterValue(cutoffID);
    float q = *tree.getRawParameterValue(qID);
    float gainDB = *tree.getRawParameterValue(gainID);
    float gain = juce::Decibels::decibelsToGain(gainDB);

    switch (index)
    {
    case 0: *filterChain.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(lastSampleRate, cutoff, q, gain); break;
    case 1: *filterChain.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(lastSampleRate, cutoff, q, gain); break;
    case 2: *filterChain.get<2>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(lastSampleRate, cutoff, q, gain); break;
    case 3: *filterChain.get<3>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(lastSampleRate, cutoff, q, gain); break;
    }

}

void ParametricEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block(buffer);
    filterChain.process(juce::dsp::ProcessContextReplacing<float>(block));
}

//==============================================================================
bool ParametricEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ParametricEQAudioProcessor::createEditor()
{
    return new ParametricEQAudioProcessorEditor (*this);
}

//==============================================================================
void ParametricEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ParametricEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParametricEQAudioProcessor();
}
