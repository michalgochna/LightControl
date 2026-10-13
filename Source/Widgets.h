#pragma once
#include <JuceHeader.h>

//==============================================================================
// Palette and drawing helpers: a bright, warm "stage lighting" look.
namespace theme
{
    inline const juce::Colour bgTop       { 0xfffffbf0 };
    inline const juce::Colour bgBottom    { 0xffeef3ff };
    inline const juce::Colour card        { 0xffffffff };
    inline const juce::Colour cardEdge    { 0xffe9e2d0 };
    inline const juce::Colour text        { 0xff262a36 };
    inline const juce::Colour textMuted   { 0xff6f7487 };
    inline const juce::Colour accent      { 0xffffb300 };   // lamp amber
    inline const juce::Colour accentDeep  { 0xffff8f00 };
    inline const juce::Colour ok          { 0xff18a957 };
    inline const juce::Colour warn        { 0xffe08a00 };
    inline const juce::Colour error       { 0xffe5484d };
    inline const juce::Colour neutral     { 0xff9aa0b4 };

    juce::Font font (float height, bool bold = false);

    /** Soft halo around a rounded rectangle, drawn inside `reach` pixels outside of it. */
    void drawGlow (juce::Graphics&, juce::Rectangle<float> bounds, float cornerRadius,
                   juce::Colour colour, float intensity, float reach);
}

//==============================================================================
/** A button that glows when hovered and flares up when clicked. The glow is drawn
    inside the button's bounds, so give it `margin` pixels of room on every side. */
class GlowButton : public juce::Button,
                   private juce::Timer
{
public:
    enum class Style { Soft, Primary, Swatch };

    static constexpr int margin = 9;
    static juce::Rectangle<int> withGlowMargin (juce::Rectangle<int> r) { return r.expanded (margin); }

    explicit GlowButton (const juce::String& label, Style buttonStyle = Style::Soft);

    void setAccent (juce::Colour newAccent);

    void paintButton (juce::Graphics&, bool isMouseOver, bool isButtonDown) override;

private:
    void buttonStateChanged() override;
    using juce::Button::clicked;
    void clicked() override;
    void timerCallback() override;

    Style style;
    juce::Colour accent { theme::accent };
    float level = 0.0f;     // smoothed hover/press glow
    float pulse = 0.0f;     // flare after a click

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlowButton)
};

//==============================================================================
/** Small pill with a glowing status dot. The detailed message goes into the tooltip. */
class StatusChip : public juce::Component,
                   public juce::SettableTooltipClient
{
public:
    void set (const juce::String& newText, juce::Colour newColour, const juce::String& tooltip);
    int getIdealWidth() const;

    void paint (juce::Graphics&) override;

private:
    juce::String text;
    juce::Colour colour { theme::neutral };
};
