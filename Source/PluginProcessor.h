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
class ParametricEQAudioProcessor : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener,
    public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    ParametricEQAudioProcessor();
    ~ParametricEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //Function to update a filter when a parameter is changed
    void updateFilter(int index);

    //Get filter parameter/band name/ID functions 
    juce::String getFilterCutoffParamName(int index);
    juce::String getFilterQParamName(int index);
    juce::String getFilterGainParamName(int index); 
    juce::String getFilterBandName(int index);
    int getBandIndexFromID(juce::String paramID);

    //Audio processor value tree state management stuff 
    juce::AudioProcessorValueTreeState tree;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout(); 
    void parameterChanged(const juce::String& parameter, float newValue) override;

private:
    using FilterProcessor = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients <float>>;
    FilterProcessor lowShelf;
    FilterProcessor lowMids;
    FilterProcessor highMids;
    FilterProcessor highShelf;
    juce::dsp::ProcessorChain<FilterProcessor, FilterProcessor, FilterProcessor, FilterProcessor> filterChain;

    float lastSampleRate;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessor)
};