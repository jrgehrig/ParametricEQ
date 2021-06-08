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
    float getFrequencyForPosition(float pos);
    void changeListenerCallback(juce::ChangeBroadcaster* sender) override;
    
    class FilterEditor : public juce::GroupComponent, public juce::Slider::Listener
    {
    public:
        FilterEditor(ParametricEQAudioProcessor&, int);
        ~FilterEditor();
        void sliderValueChanged(juce::Slider* slider) override;
        void resized() override; 
        
        void setSliderAttachments(int index);
        void setButtonAttachments(int index); 
        void setActivesEnabled();

        juce::Slider* getCutoffDial();
        juce::Label* getCutoffLabel();

        juce::Slider* getGainDial();
        juce::Label* getGainLabel();

        juce::Slider* getQDial();
        juce::Label* getQLabel();

        juce::TextButton activeSwitch;
        juce::TextButton soloSwitch;

        juce::Path filterResponse;
        juce::Colour filterResponseColour;

    private:
        juce::Slider cutoffDial;
        juce::Label cutoffLabel;

        juce::Slider gainDial;
        juce::Label gainLabel;

        juce::Slider qDial;
        juce::Label qLabel;

        ParametricEQAudioProcessor& filterEditorProcessor;
        int index; 

        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> filterSliderAttachments;
        std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> activeAttachment;


        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterEditor)
    };

    void genFilter(FilterEditor&);
    
    void setButtonAttachments(juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment>&, FilterEditor&, int index);
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ParametricEQAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricEQAudioProcessorEditor);

    void updateFrequencyResponses();

    static float getPositionForFrequency(float freq);

    static float getPositionForGain(float gain, float top, float bottom);

    juce::Rectangle<int> plotFrame;
    juce::Path totalResponse;

    juce::OwnedArray<FilterEditor> bands; 

    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> filter1ButtonAttachments;

    
};
