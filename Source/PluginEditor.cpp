/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
static float maxDB = 24.0f;
ParametricEQAudioProcessorEditor::FilterEditor::FilterEditor(ParametricEQAudioProcessor& p, int i) 
    : filterEditorProcessor(p), index(i)
{
    setText(filterEditorProcessor.getFilterBandName(index));
    setTextLabelPosition(juce::Justification::centredTop);

    addAndMakeVisible(cutoffDial);
    addAndMakeVisible(cutoffLabel);

    addAndMakeVisible(gainDial);
    addAndMakeVisible(gainLabel);

    addAndMakeVisible(qDial);
    addAndMakeVisible(qLabel);
    
    
    activeSwitch.setClickingTogglesState(true);
    activeSwitch.setToggleState(!filterEditorProcessor.isBypassed(index),juce::NotificationType::dontSendNotification);
    activeSwitch.setColour(juce::TextButton::buttonOnColourId, juce::Colours::powderblue);
    activeSwitch.onClick = [this]() { filterEditorProcessor.updateActiveBands(index); filterEditorProcessor.updatePlots(); };
    addAndMakeVisible(activeSwitch);
    
    //addAndMakeVisible(soloSwitch);
    soloSwitch.setClickingTogglesState(true);
    activeSwitch.setColour(juce::TextButton::buttonOnColourId, juce::Colours::powderblue);
    soloSwitch.onClick = [this]() { filterEditorProcessor.updateSoloedBand(index); };

    switch (index)
    {
    case 0: filterResponseColour = juce::Colours::grey; break;
    case 1: filterResponseColour = juce::Colours::cornflowerblue; break;
    case 2: filterResponseColour = juce::Colours::lightgreen; break;
    case 3: filterResponseColour = juce::Colours::indianred; break;
    }

    setColour(juce::GroupComponent::outlineColourId, filterResponseColour);
    

}

ParametricEQAudioProcessorEditor::FilterEditor::~FilterEditor()
{

}


void ParametricEQAudioProcessorEditor::FilterEditor::resized() 
{
    cutoffDial.setBounds(10, 10, 80, 80);
    cutoffLabel.setBounds(10, 95, 80, 10);

    gainDial.setBounds(10, 110, 80, 80);
    gainLabel.setBounds(10, 195, 80, 10);

    qDial.setBounds(10, 210, 80, 80);
    qLabel.setBounds(10, 295, 80, 10);

    soloSwitch.setBounds(70, 310, 20, 20);
    activeSwitch.setBounds(10, 310, 20, 20);
}

juce::Slider* ParametricEQAudioProcessorEditor::FilterEditor::getCutoffDial()
{
    return &cutoffDial;
}
juce::Label* ParametricEQAudioProcessorEditor::FilterEditor::getCutoffLabel()
{
    return &cutoffLabel;
}

juce::Slider* ParametricEQAudioProcessorEditor::FilterEditor::getGainDial()
{
    return &gainDial;
}
juce::Label* ParametricEQAudioProcessorEditor::FilterEditor::getGainLabel()
{
    return &gainLabel;
}

juce::Slider* ParametricEQAudioProcessorEditor::FilterEditor::getQDial()
{
    return &qDial;
}
juce::Label* ParametricEQAudioProcessorEditor::FilterEditor::getQLabel()
{
    return &qLabel;
}

void ParametricEQAudioProcessorEditor::FilterEditor::setSliderAttachments(int index)
{
    filterSliderAttachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment
    (filterEditorProcessor.tree, filterEditorProcessor.getFilterCutoffParamName(index), cutoffDial));

    filterSliderAttachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment
    (filterEditorProcessor.tree, filterEditorProcessor.getFilterGainParamName(index), gainDial));

    filterSliderAttachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment
    (filterEditorProcessor.tree, filterEditorProcessor.getFilterQParamName(index), qDial));

}

void ParametricEQAudioProcessorEditor::FilterEditor::setButtonAttachments(int index)
{
    activeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>
    (filterEditorProcessor.tree, filterEditorProcessor.getFilterActiveName(index), activeSwitch);
}

void ParametricEQAudioProcessorEditor::FilterEditor::setActivesEnabled()
{

}

void ParametricEQAudioProcessorEditor::FilterEditor::sliderValueChanged(juce::Slider* slider)
{
    
}


void ParametricEQAudioProcessorEditor::genFilter(ParametricEQAudioProcessorEditor::FilterEditor& filter)
{
    filter.getCutoffDial()->setRange(20.0f, 20000.0f, 0.1f);
    filter.getCutoffDial()->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    filter.getCutoffDial()->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 15);

    filter.getGainDial()->setRange(-24.0f, 24.0f, 0.1f);
    filter.getGainDial()->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    filter.getGainDial()->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 15);

    filter.getQDial()->setRange(1.0f, 32.0f, 0.1f);
    filter.getQDial()->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    filter.getQDial()->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 15);

    filter.getCutoffLabel()->setText(juce::String("Frequency"), juce::NotificationType::dontSendNotification);
    filter.getCutoffLabel()->setJustificationType(juce::Justification::horizontallyCentred);
    filter.getCutoffLabel()->setFont(juce::Font(11.5f));

    filter.getGainLabel()->setText(juce::String("Gain"), juce::NotificationType::dontSendNotification);
    filter.getGainLabel()->setJustificationType(juce::Justification::horizontallyCentred);
    filter.getGainLabel()->setFont(juce::Font(11.5f));

    filter.getQLabel()->setText(juce::String("Q"), juce::NotificationType::dontSendNotification);
    filter.getQLabel()->setJustificationType(juce::Justification::horizontallyCentred);
    filter.getQLabel()->setFont(juce::Font(11.5f));

    filter.activeSwitch.setButtonText("A");

    filter.soloSwitch.setButtonText("S");
}

//==============================================================================
ParametricEQAudioProcessorEditor::ParametricEQAudioProcessorEditor (ParametricEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    

    for (int i = 0; i < 4; ++i) {
        auto* bandEditor = bands.add(new FilterEditor(audioProcessor,i));
        addAndMakeVisible(bandEditor);
        genFilter(*bandEditor);
        bandEditor->setSliderAttachments(i);
        bandEditor->setButtonAttachments(i);
    }

    audioProcessor.addChangeListener(this);
    setSize(965, 360);

    updateFrequencyResponses();
}

ParametricEQAudioProcessorEditor::~ParametricEQAudioProcessorEditor()
{
    audioProcessor.removeChangeListener(this);
}

void ParametricEQAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* sender)
{
    ignoreUnused(sender);
    updateFrequencyResponses();
    repaint(); 
}

//==============================================================================
void ParametricEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setFont(juce::Font(16.0f));
    g.setColour(juce::Colours::white);

    //
    g.setFont(12.0f);
    g.setColour(juce::Colours::silver);
    g.drawRoundedRectangle(plotFrame.toFloat(), 5, 2);

    //Vertical grid lines
    for (int i = 0; i < 10; ++i) {
        g.setColour(juce::Colours::silver.withAlpha(0.3f));
        auto x = plotFrame.getX() + plotFrame.getWidth() * i * 0.1f;
        if (i > 0) g.drawVerticalLine(juce::roundToInt(x), float(plotFrame.getY()), float(plotFrame.getBottom()));

        g.setColour(juce::Colours::silver);
        auto freq = getFrequencyForPosition(i * 0.1f);
        g.drawFittedText((freq < 1000) ? juce::String(freq) + " Hz"
            : juce::String(freq / 1000, 1) + " kHz",
            juce::roundToInt(x + 3), plotFrame.getBottom() - 18, 50, 15, juce::Justification::left, 1);
    }

    //Horizontal grid lines 
    g.setColour(juce::Colours::silver.withAlpha(0.3f));
    g.drawHorizontalLine(juce::roundToInt(plotFrame.getY() + 0.25 * plotFrame.getHeight()), float(plotFrame.getX()), float(plotFrame.getRight()));
    g.drawHorizontalLine(juce::roundToInt(plotFrame.getY() + 0.50 * plotFrame.getHeight()), float(plotFrame.getX()), float(plotFrame.getRight()));
    g.drawHorizontalLine(juce::roundToInt(plotFrame.getY() + 0.75 * plotFrame.getHeight()), float(plotFrame.getX()), float(plotFrame.getRight()));

    //Text 
    g.setColour(juce::Colours::silver);
    g.drawFittedText(juce::String(maxDB) + " dB", plotFrame.getX() + 3, plotFrame.getY() + 2, 50, 14, juce::Justification::left, 1);
    g.drawFittedText(juce::String(maxDB / 2) + " dB", plotFrame.getX() + 3, juce::roundToInt(plotFrame.getY() + 2 + 0.25 * plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);
    g.drawFittedText(" 0 dB", plotFrame.getX() + 3, juce::roundToInt(plotFrame.getY() + 2 + 0.5 * plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);
    g.drawFittedText(juce::String(-maxDB / 2) + " dB", plotFrame.getX() + 3, juce::roundToInt(plotFrame.getY() + 2 + 0.75 * plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);

    g.reduceClipRegion(plotFrame);

    for (int i = 0; i < 4; ++i) {
        auto* filterEditor = bands.getUnchecked(int(i));
        juce::String cutoffID = audioProcessor.getFilterCutoffParamName(i);
        juce::String gainID = audioProcessor.getFilterGainParamName(i);
        g.setColour(!audioProcessor.isBypassed(i) ? filterEditor->filterResponseColour : filterEditor->filterResponseColour.withAlpha(0.3f));
        g.strokePath(filterEditor->filterResponse, juce::PathStrokeType(1.5f));
    }
    g.setColour(juce::Colours::yellow);
    g.strokePath(totalResponse, juce::PathStrokeType(1.5f));
}

void ParametricEQAudioProcessorEditor::updateFrequencyResponses()
{
    auto pixelsPerDouble = 2.0f * plotFrame.getHeight() / juce::Decibels::decibelsToGain(maxDB);

    for (int i = 0; i < 4; ++i)
    {
        auto* filterEditor = bands.getUnchecked(i);
        filterEditor->filterResponse.clear();
        audioProcessor.createFrequencyPlot(filterEditor->filterResponse, audioProcessor.getMagnitudes(i), plotFrame.withX(plotFrame.getX() + 1), pixelsPerDouble);
    }

    totalResponse.clear();
    audioProcessor.createFrequencyPlot(totalResponse, audioProcessor.getMagnitudes(4), plotFrame.withX(plotFrame.getX() + 1), pixelsPerDouble);

}

float ParametricEQAudioProcessorEditor::getPositionForFrequency(float freq)
{
    return (std::log(freq / 20.0f) / std::log(2.0f)) / 10.0f;
}

float ParametricEQAudioProcessorEditor::getPositionForGain(float gain, float top, float bottom)
{
    return juce::jmap(juce::Decibels::gainToDecibels(gain, -maxDB), -maxDB, maxDB, bottom, top);
}

void ParametricEQAudioProcessorEditor::resized()
{
    bands[0]->setBounds(10, 10, 100, 340);
    bands[1]->setBounds(120, 10, 100, 340);
    bands[2]->setBounds(230, 10, 100, 340);
    bands[3]->setBounds(340, 10, 100, 340);
    plotFrame.setBounds(450, 20, 500, 326);
}

float ParametricEQAudioProcessorEditor::getFrequencyForPosition(float pos)
{
    return 20.0f * std::pow(2.0f, pos * 10.0f);
}
