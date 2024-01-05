#include "Device.h"



DevicePage::DevicePage()
{
    device.setButtonText("Device 1");
 
}


DevicePage::~DevicePage()
{

};

void DevicePage::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgreen);
};

void DevicePage::resized()
{

    juce::Rectangle <int> zeroBounds;
    zeroBounds.setBounds(0, 0, 0, 0);

    //check for connectivity with OpenRGB server
    deviceFlex.flexWrap = juce::FlexBox::Wrap::wrap;
    deviceFlex.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    deviceFlex.alignContent = juce::FlexBox::AlignContent::spaceAround;

    deviceFlex.items.add(juce::FlexItem(device).withMinWidth(50.0f).withMinHeight(50.0f));

    juce::Rectangle <int> devicesBounds = juce::Component::getLocalBounds().removeFromLeft(90);
    deviceFlex.performLayout(devicesBounds);

};
