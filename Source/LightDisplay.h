#pragma once
#include <JuceHeader.h>
#include "LightWorker.h"
#include <map>

/** A big on-screen lamp that follows the transport: it pulses while recording, breathes and sweeps its rays
    while playing and glows quietly when stopped. It works without any real lights connected. */
class LightDisplay : public juce::Component,
                     private juce::Timer
{
public:
    explicit LightDisplay (LightWorker& worker);

    juce::Colour getShownColour() const noexcept    { return shown; }

    void paint (juce::Graphics&) override;
    void moved() override;
    void resized() override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

    std::function<void()> onDoubleClick;

private:
    void timerCallback() override;

    /** The expensive layers are drawn once into images and only composited each frame. */
    struct Layers
    {
        juce::Image base;       // opaque: window background + card. Static, copied each frame.
        juce::Image overlay;    // glow around the card + halo inside it. Faded with the pulse.
        juce::Colour colour;
        juce::Rectangle<int> bounds;
        int parentHeight = 0;
        juce::Point<float> centre;
        float baseR = 0.0f;
        bool valid = false;
        std::map<int, juce::Image> orbs;    // the lamp itself, per size in quarter-pixel steps
        juce::Point<float> orbCentre;       // centre the cached orbs were rendered for
    } layers;

    /** The lamp, pre-rendered. `origin` receives where to draw it so its centre lands exactly on `centre`. */
    const juce::Image& getOrb (float radius, juce::Point<float> centre, juce::Colour colour, juce::Point<int>& origin);

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
    bool freezeTime = false;     // debug builds only: deterministic frames for screenshot comparisons

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LightDisplay)
};
