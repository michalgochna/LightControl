#include "BroadcasterComponent.h"

void BroadcasterComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::slategrey);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("Broadcaster", getLocalBounds(), juce::Justification::topLeft, 1);
}

void BroadcasterComponent::mouseDown(const juce::MouseEvent& event)
{
    sendChangeMessage();
}


