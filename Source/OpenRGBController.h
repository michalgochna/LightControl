#pragma once
#include "OpenRGB/Client.hpp"
#include <JuceHeader.h>
using orgb::ConnectStatus;
using orgb::DeviceListResult;
using orgb::RequestStatus;
using orgb::DeviceType;



#include <JuceHeader.h>


class OpenRGBController : public juce::Component, public juce::ChangeListener
{
public:
	OpenRGBController();
	void connect();
	void getDevices();

	void paint(juce::Graphics& g) override;
	void resized() override;
	void changeListenerCallback(juce::ChangeBroadcaster* source) override;

	void setColor(juce::Colour);
	orgb::DeviceListResult getDeviceList();

private:
	orgb::Client client;
	orgb::ConnectStatus connection;
	orgb::DeviceListResult deviceList;
	
};
