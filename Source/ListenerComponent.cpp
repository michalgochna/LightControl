#include "ListenerComponent.h"


ListenerComponent::ListenerComponent()
{
    receiverButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkred);
    addAndMakeVisible(receiverButton);


}

void ListenerComponent::paint(juce::Graphics& g)
{
    g.fillAll (juce::Colours::slategrey);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("Listener", getLocalBounds(), juce::Justification::topLeft, 1);
}

void ListenerComponent::resized()
{
    receiverButton.setBounds(getLocalBounds().reduced(30));
}

void ListenerComponent::changeListenerCallback(juce::ChangeBroadcaster*)
{
    receiverButton.triggerClick();
    receiverButton.setButtonText("YES");
}
