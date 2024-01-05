#pragma once
#include <JuceHeader.h>
#include "OpenRGBController.h"



class DevicesPage : public juce::Component
{
public:
    DevicesPage(orgb::DeviceListResult& d);
    ~DevicesPage();

    void paint(juce::Graphics&) override;
    void resized() override;

private:

    juce::TextButton deviceButton;
    juce::OwnedArray <juce::TextButton> devicesButton;
    juce::FlexBox devicesMenu;
    orgb::DeviceListResult devicesList;
    

};