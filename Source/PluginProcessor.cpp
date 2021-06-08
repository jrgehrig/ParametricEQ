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

juce::String ParametricEQAudioProcessor::getFilterBandNum(int index)
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

juce::String ParametricEQAudioProcessor::getFilterBandName(int index)
{
    switch (index)
    {
    case 0: return "Low Shelf"; break;
    case 1: return "Param 1"; break;
    case 2: return "Param 2"; break;
    case 3: return "High Shelf"; break;
    }
    return "invalid";
}

juce::String ParametricEQAudioProcessor::getFilterMagnitudeName(int index)
{
    switch (index)
    {
    case 0: return "Band0Magnitude"; break;
    case 1: return "Band1Magnitude"; break;
    case 2: return "Band2Magnitude"; break;
    case 3: return "Band3Magnitude"; break;
    }
    return "invalid";
}

juce::String ParametricEQAudioProcessor::getFilterActiveName(int index)
{
    switch (index)
    {
    case 0: return "Band0Active"; break;
    case 1: return "Band1Active"; break;
    case 2: return "Band2Active"; break;
    case 3: return "Band3Active"; break;
    }
    return "invalid";
}
juce::String ParametricEQAudioProcessor::getFilterSoloName(int index)
{
    switch (index)
    {
    case 0: return "Band0Solo"; break;
    case 1: return "Band1Solo"; break;
    case 2: return "Band2Solo"; break;
    case 3: return "Band3Solo"; break;
    }
    return "invalid";
}

int ParametricEQAudioProcessor::getBandIndexFromID(juce::String paramID)
{
    for (int i = 0; i < 4; ++i)
        if (paramID.startsWith(getFilterBandNum(i)))
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
                       ), tree(*this, nullptr, "PARAMS", createParameterLayout()), lastSampleRate(44100.0f),
    soloedBand(-1)

#endif
{
    frequencies.resize(300);
    for (int i = 0; i < frequencies.size(); ++i) {
        frequencies[i] = 20.0 * std::pow(2.0, i / 30.0);
    }
    //Ensures all vectors set to same size, 
    magnitudes.resize(frequencies.size());
    lowShelfMagnitudes.resize(frequencies.size());
    lowMidsMagnitudes.resize(frequencies.size());
    highMidsMagnitudes.resize(frequencies.size());
    highShelfMagnitudes.resize(frequencies.size());

    *filterChain.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(44100, 134.2f, 0.62f, 1.0f);
    *filterChain.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 883.9f, 5.7f, 1.0f);
    *filterChain.get<2>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 883.9f, 5.7f, 1.0f);
    *filterChain.get<3>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(44100, 6000.0f, 0.62f, 1.0f);
    
    filterChain.get<0>().state->getMagnitudeForFrequencyArray(frequencies.data(),
        lowShelfMagnitudes.data(),
        frequencies.size(), lastSampleRate);
    filterChain.get<1>().state->getMagnitudeForFrequencyArray(frequencies.data(),
        lowMidsMagnitudes.data(),
        frequencies.size(), lastSampleRate);
    filterChain.get<2>().state->getMagnitudeForFrequencyArray(frequencies.data(),
        highMidsMagnitudes.data(),
        frequencies.size(), lastSampleRate);
    filterChain.get<3>().state->getMagnitudeForFrequencyArray(frequencies.data(),
        highShelfMagnitudes.data(),
        frequencies.size(), lastSampleRate);
    

    //Set all filters to bypassed by default 
    filterChain.setBypassed<0>(true);
    filterChain.setBypassed<1>(true);
    filterChain.setBypassed<2>(true);
    filterChain.setBypassed<3>(true);

    //Add all APVTS parameter listeners 
    for (int i = 0; i < 4; ++i) 
    {
        tree.addParameterListener(getFilterCutoffParamName(i), this);
        tree.addParameterListener(getFilterQParamName(i), this);
        tree.addParameterListener(getFilterGainParamName(i), this);
        tree.addParameterListener(getFilterActiveName(i), this);
    }

    updatePlots();
    
}

void ParametricEQAudioProcessor::updatePlots()
{
    //Update total response 
    juce::FloatVectorOperations::fill(magnitudes.data(), 1, magnitudes.size());
    for (int i = 0; i < 4; ++i)
    {
        if (!bypassedBands[i])
        {
            juce::FloatVectorOperations::multiply(magnitudes.data(), getMagnitudes(i).data(), magnitudes.size());
        }
    }
    sendChangeMessage();
}

const std::vector<double>& ParametricEQAudioProcessor::getMagnitudes(int index)
{
    switch (index)
    {
    case 0: return lowShelfMagnitudes; break;
    case 1: return lowMidsMagnitudes; break;
    case 2: return highMidsMagnitudes; break;
    case 3: return highShelfMagnitudes; break; 
    case 4: return magnitudes; break;
    }
    
}

void ParametricEQAudioProcessor::createFrequencyPlot(juce::Path& p, const std::vector<double>& mags, const juce::Rectangle<int> bounds, float pixelsPerDouble)
{

    p.startNewSubPath(float(bounds.getX()), mags[0] > 0 ? float(bounds.getCentreY() - pixelsPerDouble * std::log(mags[0]) / std::log(2.0)) : bounds.getBottom());
    const auto xFactor = static_cast<double> (bounds.getWidth()) / frequencies.size(); //spacing between points 
    for (size_t i = 1; i < frequencies.size(); ++i)
    {
        p.lineTo(float(bounds.getX() + i * xFactor),
            float(mags[i] > 0 ? bounds.getCentreY() - pixelsPerDouble * std::log(mags[i]) / std::log(2.0) : bounds.getBottom()));
    }
    
}

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
    case 0: 
        *filterChain.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(lastSampleRate, cutoff, q, gain);
        filterChain.get<0>().state->getMagnitudeForFrequencyArray(frequencies.data(), lowShelfMagnitudes.data(),
            frequencies.size(), lastSampleRate);
        break;
    case 1: 
        *filterChain.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(lastSampleRate, cutoff, q, gain); 
        filterChain.get<1>().state->getMagnitudeForFrequencyArray(frequencies.data(), lowMidsMagnitudes.data(),
            frequencies.size(), lastSampleRate);
        break;
    case 2: 
        *filterChain.get<2>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(lastSampleRate, cutoff, q, gain); 
        filterChain.get<2>().state->getMagnitudeForFrequencyArray(frequencies.data(), highMidsMagnitudes.data(),
            frequencies.size(), lastSampleRate);
        break;
    case 3: 
        *filterChain.get<3>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(lastSampleRate, cutoff, q, gain); 
        filterChain.get<3>().state->getMagnitudeForFrequencyArray(frequencies.data(), highShelfMagnitudes.data(),
            frequencies.size(), lastSampleRate);
        break;
    }
    updatePlots();

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
            (getFilterGainParamName(i), getFilterGainParamName(i), -24.0f, 24.0f, 0.0f);

        auto activeParam = std::make_unique<juce::AudioParameterBool>
            (getFilterActiveName(i), getFilterActiveName(i), false, juce::String(), nullptr, nullptr);

        params.push_back(std::move(gainParam));
        params.push_back(std::move(activeParam));

    }
    return { params.begin(), params.end() };
}

void ParametricEQAudioProcessor::parameterChanged(const juce::String& parameter, float newValue)
{
    int index = getBandIndexFromID(parameter);
    updateFilter(index);
}

void ParametricEQAudioProcessor::updateActiveBands(int index)
{
    //If index was bypassed and button clicked, set index active and vice versa
    bool newBypassedState = bypassedBands[index] ? false : true;
    bypassedBands[index] = newBypassedState;

    filterChain.setBypassed<0>(bypassedBands[0]);
    filterChain.setBypassed<1>(bypassedBands[1]);
    filterChain.setBypassed<2>(bypassedBands[2]);
    filterChain.setBypassed<3>(bypassedBands[3]);

    sendChangeMessage();
}

void ParametricEQAudioProcessor::updateSoloedBand(int index)
{
    soloedBand = index;
    filterChain.setBypassed<0>(!(soloedBand == 0));
    filterChain.setBypassed<1>(!(soloedBand == 1));
    filterChain.setBypassed<2>(!(soloedBand == 2));
    filterChain.setBypassed<3>(!(soloedBand == 3));
}

int ParametricEQAudioProcessor::getSoloedBand()
{
    //Returns index of soloed band, return -1 if no band soloed 
    return soloedBand;
}

bool ParametricEQAudioProcessor::isBypassed(int index) 
{
    return bypassedBands[index];
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
    auto state = tree.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ParametricEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(tree.state.getType()))
            tree.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParametricEQAudioProcessor();
}
