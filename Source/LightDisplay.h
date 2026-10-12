#pragma once
#include <JuceHeader.h>
#include "LightWorker.h"

/** A big on-screen lamp that follows the transport: it pulses while recording, breathes and sweeps its rays
    while playing and glows quietly when stopped. It works without any real lights connected. */
class LightDisplay : public juce::Component,
                     private juce::Timer
{
public:
    explicit LightDisplay (LightWorker& worker);

    void setCaption (const juce::String& text);
    juce::Colour getShownColour() const noexcept    { return shown; }

    void paint (juce::Graphics&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

    std::function<void()> onDoubleClick;

private:
    void timerCallback() override;

    /** The expensive layers are drawn once into images and only composited each frame. */
    struct Layers
    {
        juce::Image glow, halo, card;
        juce::Colour colour;
        juce::Rectangle<int> bounds;
        juce::Point<float> centre;
        float baseR = 0.0f;
        bool valid = false;
    } layers;

    void rebuildLayers (juce::Rectangle<float> card, juce::Point<float> centre, float baseR, juce::Colour colour);

    LightWorker& lights;
    TransportState state = TransportState::Stopped;
    TransportState previousState = TransportState::Stopped;
    bool waiting = true;
    juce::Colour shown { 0xffffa000 };     // eases towards the colour of the current state
    double rotation = 0.0;
    double lastTickMs = 0.0;
    double flashStartMs = -10000.0;
    juce::Colour lastRepaintColour;
    int idleTicks = 0;
    juce::String caption;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LightDisplay)
};
