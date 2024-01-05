/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BroadcasterComponent.h"
#include "ListenerComponent.h"
#include "Devices.h"
#include "Settings.h"
#include "OpenRGBController.h"



//==============================================================================
/**
*/
class LightControlAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    LightControlAudioProcessorEditor (LightControlAudioProcessor& p, orgb::DeviceListResult d);
    ~LightControlAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.

    ListenerComponent listenerComponent;
    BroadcasterComponent broadcasterComponent;
    LightControlAudioProcessor& audioProcessor;

    juce::TabbedComponent tabs{ juce::TabbedButtonBar::Orientation::TabsAtTop }; // juce::TabbedButtonBar::Orientation::TabsAtTop );


    //DevicesPage devicesPage;
    SettingsPage settingsPage;
    




    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LightControlAudioProcessorEditor)
};
