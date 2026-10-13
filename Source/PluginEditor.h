#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Settings.h"
#include "LightDisplay.h"
#include "HelpPage.h"
#include "Widgets.h"
#include "LightLookAndFeel.h"

//==============================================================================
class LightControlAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit LightControlAudioProcessorEditor (LightControlAudioProcessor& p);
    ~LightControlAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setSettingsVisible (bool visible);
    void setHelpOpen (bool open);
    void updateButtons();
    void layoutSettingsContent();
    void updateStatuses();

    LightControlAudioProcessor& audioProcessor;

    juce::SharedResourcePointer<LightLookAndFeel> laf;
    juce::TooltipWindow tooltipWindow { this, 500 };

    LightDisplay display;
    StatusChip openRgbChip, artNetChip;
    GlowButton settingsButton { "Settings" };
    GlowButton helpButton { "Help" };

    SettingsPage settingsPage;
    juce::Viewport settingsViewport;
    HelpPage helpPage;

    juce::ComponentBoundsConstrainer constrainer;
    bool settingsVisible = true;
    bool helpOpen = false;          // not saved: the panel shows Help instead of the settings while this is on
    int tick = 0;
    int lastChipsWidth = 0;
    juce::Colour lastLogoColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LightControlAudioProcessorEditor)
};
