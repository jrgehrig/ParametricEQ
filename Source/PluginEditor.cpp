/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ParametricEQAudioProcessorEditor::FilterEditor::FilterEditor(const juce::String& filterType)
{
    setText(filterType);
    setTextLabelPosition(juce::Justification::centredTop);

    addAndMakeVisible(cutoffDial);
    addAndMakeVisible(freqLabel);

    addAndMakeVisible(gainDial);
    addAndMakeVisible(gainLabel);

    addAndMakeVisible(qDial);
    addAndMakeVisible(qLabel);
    
    addAndMakeVisible(activeSwitch);
    activeSwitch.setClickingTogglesState(true);
    activeSwitch.onClick = [this]() { DBG(activeSwitch.getState()); };

    addAndMakeVisible(soloSwitch);
    soloSwitch.setClickingTogglesState(true);
    soloSwitch.onClick = [this]() {DBG("solo"); };
}

ParametricEQAudioProcessorEditor::FilterEditor::~FilterEditor()
{

}


void ParametricEQAudioProcessorEditor::FilterEditor::resized() 
{
    cutoffDial.setBounds(10, 10, 80, 80);
    freqLabel.setBounds(10, 95, 80, 10);

    gainDial.setBounds(10, 110, 80, 80);
    gainLabel.setBounds(10, 195, 80, 10);

    qDial.setBounds(10, 210, 80, 80);
    qLabel.setBounds(10, 295, 80, 10);

    soloSwitch.setBounds(70, 310, 20, 20);
    activeSwitch.setBounds(10, 310, 20, 20);

}


void ParametricEQAudioProcessorEditor::setSliderAttachments
(juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment>& arr, FilterEditor& filter, int i)
{
    arr.add(new juce::AudioProcessorValueTreeState::SliderAttachment
    (audioProcessor.tree, audioProcessor.getFilterCutoffParamName(i), filter.cutoffDial));
    arr.add(new juce::AudioProcessorValueTreeState::SliderAttachment
    (audioProcessor.tree, audioProcessor.getFilterQParamName(i), filter.qDial));
    arr.add(new juce::AudioProcessorValueTreeState::SliderAttachment
    (audioProcessor.tree, audioProcessor.getFilterGainParamName(i), filter.gainDial));
}

/*
void ParametricEQAudioProcessorEditor::setButtonAttachments
(juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment>& arr, FilterEditor& filter, int i)
{
    arr.add(new juce::AudioProcessorValueTreeState::ButtonAttachment
    (audioProcessor.tree, audioProcessor.getFilterCutoffParamName(i), filter.cutoffDial));
    arr.add(new juce::AudioProcessorValueTreeState::ButtonAttachment
    (audioProcessor.tree, audioProcessor.getFilterQParamName(i), filter.qDial));
}*/

void ParametricEQAudioProcessorEditor::genFilter(ParametricEQAudioProcessorEditor::FilterEditor& filter)
{
    filter.cutoffDial.setRange(20.0f, 20000.0f, 0.1f);
    filter.cutoffDial.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    filter.cutoffDial.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 15);

    filter.gainDial.setRange(-18.0f, 18.0f, 0.1f);
    filter.gainDial.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    filter.gainDial.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 15);

    filter.qDial.setRange(1.0f, 32.0f, 0.1f);
    filter.qDial.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    filter.qDial.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 15);

    filter.freqLabel.setText(juce::String("Frequency"), juce::NotificationType::dontSendNotification);
    filter.freqLabel.setJustificationType(juce::Justification::horizontallyCentred);
    filter.freqLabel.setFont(juce::Font(11.5f));

    filter.gainLabel.setText(juce::String("Gain"), juce::NotificationType::dontSendNotification);
    filter.gainLabel.setJustificationType(juce::Justification::horizontallyCentred);
    filter.gainLabel.setFont(juce::Font(11.5f));

    filter.qLabel.setText(juce::String("Q"), juce::NotificationType::dontSendNotification);
    filter.qLabel.setJustificationType(juce::Justification::horizontallyCentred);
    filter.qLabel.setFont(juce::Font(11.5f));

    
    filter.activeSwitch.setButtonText("A");

    filter.soloSwitch.setButtonText("S");
}

//==============================================================================
ParametricEQAudioProcessorEditor::ParametricEQAudioProcessorEditor (ParametricEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(800, 360);

    addAndMakeVisible(lowShelf);
    genFilter(lowShelf);

    addAndMakeVisible(param1);
    genFilter(param1);

    addAndMakeVisible(param2);
    genFilter(param2);

    addAndMakeVisible(highShelf);
    genFilter(highShelf);

    setSliderAttachments(filter1SliderAttachments, lowShelf, 0);
    setSliderAttachments(filter2SliderAttachments, param1, 1);
    setSliderAttachments(filter3SliderAttachments, param2, 2);
    setSliderAttachments(filter4SliderAttachments, highShelf, 3);

    audioProcessor.addChangeListener(this);
}

ParametricEQAudioProcessorEditor::~ParametricEQAudioProcessorEditor()
{
    audioProcessor.removeChangeListener(this);
}

void ParametricEQAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* sender)
{
    
}

//==============================================================================
void ParametricEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setFont(juce::Font(16.0f));
    g.setColour(juce::Colours::white);
}

void ParametricEQAudioProcessorEditor::resized()
{
    lowShelf.setBounds(10, 10, 100, 340);
    param1.setBounds(110, 10, 100, 340);
    param2.setBounds(220, 10, 100, 340);
    highShelf.setBounds(330, 10, 100, 340);


}
