#pragma once
#include <JuceHeader.h>

class BroadcasterComponent : public juce::Component, public juce::ChangeBroadcaster
{
public:

	void paint(juce::Graphics& g) override;
	void mouseDown(const juce::MouseEvent &event) override;

private:
};
