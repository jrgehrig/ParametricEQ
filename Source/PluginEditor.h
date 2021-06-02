/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.
    
    Filter ranges:
    Low 40 - 600
    LM  200 - 2k
    HM  600 - 7k
    High 1.5k - 22k
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ParametricEQAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::ChangeListener
{
public:
    ParametricEQAudioProcessorEditor (ParametricEQAudioProcessor&);
    ~ParametricEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster* sender) override;

    class FilterEditor : public juce::GroupComponent
    {
    public:
        FilterEditor(const juce::String&); 
        ~FilterEditor();

        void resized() override; 

        juce::Slider cutoffDial;
        juce::Label freqLabel;

        juce::Slider gainDial;
        juce::Label gainLabel;

        juce::Slider qDial;
        juce::Label qLabel;

        juce::TextButton activeSwitch;
        juce::TextButton soloSwitch;

    private:
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterEditor)
    };

    void genFilter(FilterEditor&);
    void setSliderAttachments(juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment>&, FilterEditor&, int index);
    void setButtonAttachments(juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment>&, FilterEditor&, int index);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ParametricEQAudioProcessor& audioProcessor;

    FilterEditor lowShelf{ "Low Shelf" };
    FilterEditor param1{ "Param 1" };
    FilterEditor param2{ "Param 2" };
    FilterEditor highShelf{ "High Shelf" };

    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> filter1SliderAttachments;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> filter2SliderAttachments;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> filter3SliderAttachments;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> filter4SliderAttachments;

    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> filter1ButtonAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessorEditor)
};
