#include "Devices.h"


DevicesPage::DevicesPage(orgb::DeviceListResult& d) 
{
    //deviceButton.setButtonText("Device 1");
    devicesList = d;
    for (const orgb::Device& device :devicesList.devices)
        addAndMakeVisible(juce::TextButton(device.name));

}


DevicesPage::~DevicesPage() 
{

};

void DevicesPage::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
};

void DevicesPage::resized()
{

    juce::Rectangle <int> zeroBounds;
    zeroBounds.setBounds(0, 0, 0, 0);
    devicesMenu.performLayout(zeroBounds);

    //check for connectivity with OpenRGB server
    devicesMenu.flexWrap = juce::FlexBox::Wrap::wrap;
    devicesMenu.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    devicesMenu.alignContent = juce::FlexBox::AlignContent::spaceAround;

    for (auto* b : devicesButton)
        devicesMenu.items.add(juce::FlexItem(*b).withMinWidth(50.0f).withMinHeight(50.0f));

    juce::Rectangle <int> devicesBounds = juce::Component::getLocalBounds();
    devicesMenu.performLayout(devicesBounds);

};
