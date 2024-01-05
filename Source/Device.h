#pragma once
#include <JuceHeader.h>

class DevicePage :
    public juce::Component, juce::TextButton
{
public:
    DevicePage();
    ~DevicePage();

    void paint(juce::Graphics&) override;
    void resized() override;

private:

    juce::FlexBox deviceFlex;
    juce::TextButton device;

};

