#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Settings.h"
#include "LightDisplay.h"
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
    void layoutSettingsContent();
    void updateStatuses();

    LightControlAudioProcessor& audioProcessor;

    juce::SharedResourcePointer<LightLookAndFeel> laf;
    juce::TooltipWindow tooltipWindow { this, 500 };

    LightDisplay display;
    StatusChip openRgbChip, artNetChip;
    GlowButton settingsButton { "Settings" };

    SettingsPage settingsPage;
    juce::Viewport settingsViewport;

    juce::ComponentBoundsConstrainer constrainer;
    bool settingsVisible = true;
    int tick = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LightControlAudioProcessorEditor)
};
