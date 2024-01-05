/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LightControlAudioProcessorEditor::LightControlAudioProcessorEditor (LightControlAudioProcessor& p, orgb::DeviceListResult d)
    : AudioProcessorEditor (&p), audioProcessor (p) 
{
    //TabbedComponent tabs;
    auto colour = juce::Component::findColour(juce::ResizableWindow::backgroundColourId);

    //tabs.addTab("Devices", colour, &devicesPage, true);

    tabs.addTab("Settings", colour, &settingsPage, true);
    addAndMakeVisible(tabs);

    
    juce::Component::setSize(400, 600);
   
}

LightControlAudioProcessorEditor::~LightControlAudioProcessorEditor()
{

}

void LightControlAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::grey);
}


void LightControlAudioProcessorEditor::resized()
{

    tabs.setBounds(juce::Component::getLocalBounds());


    //settingsButton.setBounds(getLocalBounds().removeFromBottom(100));
    //devicesButton.setBounds(getLocalBounds().removeFromTop(100));

}

