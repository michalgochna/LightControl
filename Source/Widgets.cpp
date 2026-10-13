#include "Widgets.h"

namespace theme
{
    juce::Font font (float height, bool bold)
    {
        return juce::Font (juce::FontOptions (height, bold ? juce::Font::bold : juce::Font::plain));
    }

    void drawGlow (juce::Graphics& g, juce::Rectangle<float> bounds, float cornerRadius,
                   juce::Colour colour, float intensity, float reach)
    {
        if (intensity <= 0.001f)
            return;

        constexpr int layers = 9;

        for (int i = layers; i >= 1; --i)
        {
            const float t = (float) i / (float) layers;                 // 1 = outermost
            const float alpha = intensity * 0.20f * (1.0f - t) * (1.0f - t * 0.4f);
            g.setColour (colour.withAlpha (juce::jlimit (0.0f, 1.0f, alpha)));
            g.fillRoundedRectangle (bounds.expanded (reach * t), cornerRadius + reach * t);
        }
    }
}

//==============================================================================
GlowButton::GlowButton (const juce::String& label, Style buttonStyle) : juce::Button (label), style (buttonStyle)
{
    setButtonText (label);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void GlowButton::setAccent (juce::Colour newAccent)
{
    accent = newAccent;
    repaint();
}

void GlowButton::buttonStateChanged()
{
    startTimerHz (30);
}

void GlowButton::clicked()
{
    pulse = 1.6f;
    startTimerHz (30);
}

void GlowButton::timerCallback()
{
    const float target = isDown() ? 1.0f : (isOver() ? 0.45f : 0.0f);
    level += (target - level) * 0.35f;
    pulse = juce::jmax (0.0f, pulse - 0.05f);

    if (std::abs (level - target) < 0.01f && pulse <= 0.0f)
    {
        level = target;
        stopTimer();
    }

    repaint();
}

void GlowButton::paintButton (juce::Graphics& g, bool, bool isButtonDown)
{
    auto body = getLocalBounds().toFloat().reduced ((float) margin);
    const float radius = body.getHeight() * 0.5f;
    const float glow = juce::jlimit (0.0f, 1.6f, level + pulse);

    if (isEnabled())
        theme::drawGlow (g, body, radius, accent, glow, (float) margin);

    if (isButtonDown)
        body = body.reduced (0.8f);

    juce::Colour textColour = theme::text;

    switch (style)
    {
        case Style::Primary:
            g.setGradientFill (juce::ColourGradient (accent.brighter (0.25f), 0.0f, body.getY(),
                                                     accent.darker (0.12f), 0.0f, body.getBottom(), false));
            g.fillRoundedRectangle (body, radius);
            g.setColour (juce::Colours::white.withAlpha (0.35f));
            g.drawRoundedRectangle (body.reduced (0.5f), radius, 1.0f);
            textColour = juce::Colour (0xff3a2600);
            break;

        case Style::Swatch:
            g.setColour (accent);
            g.fillRoundedRectangle (body, radius);
            g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.35f), 0.0f, body.getY(),
                                                     juce::Colours::transparentWhite, 0.0f, body.getCentreY(), false));
            g.fillRoundedRectangle (body, radius);
            g.setColour (juce::Colours::white.withAlpha (0.85f));
            g.drawRoundedRectangle (body.reduced (1.0f), radius, 2.0f);
            textColour = accent.contrasting (0.85f);
            break;

        case Style::Soft:
            g.setColour (theme::card.interpolatedWith (accent, juce::jmin (glow, 1.0f) * 0.16f));
            g.fillRoundedRectangle (body, radius);
            g.setColour (accent.withAlpha (0.45f + 0.4f * juce::jmin (glow, 1.0f)));
            g.drawRoundedRectangle (body.reduced (0.5f), radius, 1.4f);
            break;
    }

    g.setColour (isEnabled() ? textColour : textColour.withAlpha (0.4f));
    g.setFont (theme::font (juce::jlimit (11.0f, 15.0f, body.getHeight() * 0.5f), true));
    g.drawFittedText (getButtonText(), body.toNearestInt().reduced (6, 0), juce::Justification::centred, 1);
}

//==============================================================================
void StatusChip::set (const juce::String& newText, juce::Colour newColour, const juce::String& tooltip)
{
    if (newText != text || newColour != colour)
    {
        text = newText;
        colour = newColour;
        repaint();
    }

    setTooltip (tooltip);
}

int StatusChip::getIdealWidth() const
{
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText (theme::font (13.0f, true), text, 0.0f, 0.0f);
    return (int) std::ceil (glyphs.getBoundingBox (0, glyphs.getNumGlyphs(), true).getWidth()) + 38;
}

void StatusChip::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (1.0f);

    g.setColour (theme::card);
    g.fillRoundedRectangle (b, b.getHeight() * 0.5f);
    g.setColour (colour.withAlpha (0.45f));
    g.drawRoundedRectangle (b, b.getHeight() * 0.5f, 1.2f);

    const auto dot = juce::Rectangle<float> (10.0f, 10.0f).withCentre ({ b.getX() + 15.0f, b.getCentreY() });
    theme::drawGlow (g, dot, 5.0f, colour, 1.2f, 6.0f);
    g.setColour (colour);
    g.fillEllipse (dot);

    g.setColour (theme::text);
    g.setFont (theme::font (13.0f, true));
    g.drawFittedText (text, b.toNearestInt().withTrimmedLeft (28).withTrimmedRight (8), juce::Justification::centredLeft, 1);
}
