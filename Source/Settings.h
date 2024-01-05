#pragma once
#include <JuceHeader.h>

class SettingsPage : 
    public juce::Component, juce::TextButton
{
public:
    SettingsPage();
    ~SettingsPage();

    void paint(juce::Graphics&) override;
    void resized() override;

private:

    juce::FlexBox settingsMenu;
    juce::Label serverIp;
    juce::Label serverIpLabel;
    juce::Label serverPort;
    juce::Label serverPortLabel;
    juce::Label aboutLabel;
    juce::Label about;


};
