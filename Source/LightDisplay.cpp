#include "LightDisplay.h"
#include "Widgets.h"

namespace
{
    constexpr double twoPi = juce::MathConstants<double>::twoPi;
    constexpr float cardRadius = 26.0f;
}

LightDisplay::LightDisplay (LightWorker& worker) : lights (worker)
{
    lastTickMs = juce::Time::getMillisecondCounterHiRes();
    startTimerHz (30);
}

void LightDisplay::setCaption (const juce::String& text)
{
    if (text != caption)
    {
        caption = text;
        repaint();
    }
}

void LightDisplay::mouseDoubleClick (const juce::MouseEvent&)
{
    if (onDoubleClick)
        onDoubleClick();
}

void LightDisplay::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes();
    const double dt = juce::jlimit (0.0, 0.2, (now - lastTickMs) / 1000.0);
    lastTickMs = now;

    state = lights.getRequestedState();
    waiting = ! lights.hasReceivedAudio() && ! lights.isPreviewing();

    if (state != previousState)
    {
        previousState = state;
        flashStartMs = now;          // a ripple of light on every change
    }

    shown = shown.interpolatedWith (lights.getColour (state), 0.2f);

    const double speed = state == TransportState::Playing ? 0.35 : (state == TransportState::Recording ? 0.08 : 0.0);
    rotation = std::fmod (rotation + dt * speed * twoPi, twoPi);

    if (! isShowing())
        return;

    // Recording and playing animate continuously. A stopped, settled lamp only breathes very slowly,
    // so it needs just a few frames per second.
    const bool colourSettling = shown != lastRepaintColour;
    const bool rippleActive = (now - flashStartMs) < 1000.0;
    const bool busy = state != TransportState::Stopped || colourSettling || rippleActive;

    if (busy || ++idleTicks >= 5)
    {
        idleTicks = 0;
        lastRepaintColour = shown;
        repaint();
    }
}

void LightDisplay::rebuildLayers (juce::Rectangle<float> card, juce::Point<float> centre, float baseR, juce::Colour c)
{
    const int w = getWidth(), h = getHeight();
    const float margin = 14.0f;

    layers.glow = juce::Image (juce::Image::ARGB, w, h, true);
    {
        juce::Graphics g (layers.glow);
        theme::drawGlow (g, card, cardRadius, c, 0.9f, margin);
    }

    layers.halo = juce::Image (juce::Image::ARGB, w, h, true);
    {
        juce::Graphics g (layers.halo);
        juce::Path clip;
        clip.addRoundedRectangle (card, cardRadius);
        g.reduceClipRegion (clip);

        const float haloR = baseR * 3.8f;
        g.setGradientFill (juce::ColourGradient (c.withAlpha (0.55f), centre.x, centre.y,
                                                 c.withAlpha (0.0f), centre.x + haloR, centre.y, true));
        g.fillEllipse (centre.x - haloR, centre.y - haloR, haloR * 2.0f, haloR * 2.0f);
    }

    layers.card = juce::Image (juce::Image::ARGB, w, h, true);
    {
        juce::Graphics g (layers.card);
        g.setGradientFill (juce::ColourGradient (juce::Colours::white, 0.0f, card.getY(),
                                                 juce::Colours::white.interpolatedWith (c, 0.12f), 0.0f, card.getBottom(), false));
        g.fillRoundedRectangle (card, cardRadius);
    }

    layers.colour = c;
    layers.bounds = getLocalBounds();
    layers.centre = centre;
    layers.baseR = baseR;
    layers.valid = true;
}

void LightDisplay::paint (juce::Graphics& g)
{
    const double nowMs = juce::Time::getMillisecondCounterHiRes();
    const double t = nowMs / 1000.0;
    const auto c = shown;

    // How the lamp behaves in each state
    float pulseHz = 0.15f, intensity = 0.5f, scaleAmount = 0.0f;
    switch (state)
    {
        case TransportState::Recording: pulseHz = 1.3f;  intensity = 0.62f; scaleAmount = 0.05f; break;
        case TransportState::Playing:   pulseHz = 0.35f; intensity = 0.80f; scaleAmount = 0.025f; break;
        case TransportState::Stopped:   pulseHz = 0.15f; intensity = 0.50f; scaleAmount = 0.01f; break;
    }

    const float pulse = 0.5f + 0.5f * (float) std::sin (t * twoPi * pulseHz);
    if (state == TransportState::Recording)
        intensity += 0.38f * pulse;
    else
        intensity += 0.10f * pulse;
    if (waiting)
        intensity *= 0.6f;
    const float scale = 1.0f + scaleAmount * pulse;

    const float margin = 14.0f;
    auto card = getLocalBounds().toFloat().reduced (margin);
    if (card.getWidth() < 40.0f || card.getHeight() < 40.0f)
        return;

    // The expensive layers only change with the size or (noticeably) the colour.
    const auto colourDistance = [] (juce::Colour x, juce::Colour y)
    {
        return std::abs (x.getRed() - y.getRed()) + std::abs (x.getGreen() - y.getGreen()) + std::abs (x.getBlue() - y.getBlue());
    };

    // Vertical layout: [lamp] [big label] [small label] ... [caption]. Everything is sized from the space that is
    // left, so nothing overlaps in a small window.
    const bool showCaption = caption.isNotEmpty() && card.getHeight() > 150.0f;
    const float captionH = showCaption ? 34.0f : 0.0f;
    const float availH = card.getHeight() - captionH;

    const float bigH = juce::jlimit (16.0f, 72.0f, availH * 0.13f);
    const float smallH = juce::jlimit (11.0f, 22.0f, availH * 0.05f);
    const float textBlockH = bigH * 1.25f + smallH * 1.5f;

    const float maxR = (availH * 0.92f - textBlockH) / 2.6f;
    const float baseR = juce::jmax (12.0f, juce::jmin (card.getWidth() * 0.26f, availH * 0.26f, maxR));
    const float R = baseR * scale;

    const float groupH = baseR * 2.0f + baseR * 0.3f + textBlockH;
    const float groupTop = card.getY() + juce::jmax (4.0f, (availH - groupH) * 0.5f);
    const float cx = card.getCentreX();
    const float cy = groupTop + baseR;

    if (! layers.valid || layers.bounds != getLocalBounds() || std::abs (layers.baseR - baseR) > 0.5f
        || std::abs (layers.centre.y - cy) > 0.5f || colourDistance (layers.colour, c) >= 5)
        rebuildLayers (card, { cx, cy }, baseR, c);

    const float op = juce::jlimit (0.0f, 1.0f, intensity);
    const auto cardInt = card.toNearestInt();

    // glow: only the ring around the card is ever visible, so don't blend the part underneath it
    g.saveState();
    g.excludeClipRegion (card.reduced (cardRadius * 0.3f).toNearestInt());
    g.setOpacity (op);
    g.drawImageAt (layers.glow, 0, 0);
    g.restoreState();

    g.drawImageAt (layers.card, 0, 0);

    // Everything below stays inside the card. A rectangular clip is far cheaper than a rounded-path clip,
    // and the corners are baked into the cached halo image.
    g.saveState();
    g.reduceClipRegion (cardInt);

    g.setOpacity (op);
    g.drawImageAt (layers.halo, 0, 0);
    g.setOpacity (1.0f);

    // rays (a few thin triangles: cheap to fill directly, unlike rotating a big image)
    {
        juce::Path rays;
        const int count = 20;
        for (int i = 0; i < count; ++i)
        {
            const double a = rotation + i * twoPi / count;
            const float half = (i % 2 == 0) ? 0.05f : 0.035f;
            const float len = R * ((i % 2 == 0) ? 3.1f : 2.3f);
            const auto at = [&] (double ang, float r) { return juce::Point<float> (cx + (float) std::cos (ang) * r, cy + (float) std::sin (ang) * r); };
            rays.startNewSubPath (at (a - half, R * 1.05f));
            rays.lineTo (at (a, len));
            rays.lineTo (at (a + half, R * 1.05f));
            rays.closeSubPath();
        }

        g.setGradientFill (juce::ColourGradient (c.withAlpha (0.60f * intensity), cx, cy, c.withAlpha (0.0f), cx + R * 3.1f, cy, true));
        g.fillPath (rays);
    }

    // ripple when the state changes
    {
        const double age = (nowMs - flashStartMs) / 1000.0;
        if (age >= 0.0 && age < 1.0)
        {
            const float k = (float) age;
            const float r = R * (1.0f + k * 3.4f);
            g.setColour (c.withAlpha ((1.0f - k) * (1.0f - k) * 0.8f));
            g.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 1.5f + 4.0f * (1.0f - k));
        }
    }

    // the lamp itself
    {
        juce::ColourGradient body (c.brighter (0.7f), cx - R * 0.4f, cy - R * 0.5f,
                                   c.darker (0.25f), cx + R * 0.7f, cy + R * 0.9f, true);
        g.setGradientFill (body);
        g.fillEllipse (cx - R, cy - R, R * 2.0f, R * 2.0f);

        juce::ColourGradient gloss (juce::Colours::white.withAlpha (0.75f), cx - R * 0.3f, cy - R * 0.55f,
                                    juce::Colours::white.withAlpha (0.0f), cx - R * 0.3f, cy - R * 0.05f, false);
        g.setGradientFill (gloss);
        g.fillEllipse (cx - R * 0.65f, cy - R * 0.85f, R * 1.1f, R * 0.7f);

        g.setColour (juce::Colours::white.withAlpha (0.8f));
        g.drawEllipse (cx - R, cy - R, R * 2.0f, R * 2.0f, juce::jmax (1.5f, R * 0.04f));
    }

    g.restoreState();

    // border
    g.setColour (c.withAlpha (0.35f));
    g.drawRoundedRectangle (card.reduced (0.75f), cardRadius, 1.5f);

    // labels
    const char* big = "STOP";
    const char* small = "Stopped";
    switch (state)
    {
        case TransportState::Recording: big = "REC";  small = "Recording"; break;
        case TransportState::Playing:   big = "PLAY"; small = "Playing";   break;
        case TransportState::Stopped:   break;
    }
    if (waiting)
    {
        big = "READY";
        small = "Waiting for the transport";
    }

    auto textArea = juce::Rectangle<float> (card.getX() + 8.0f, cy + baseR * 1.3f, card.getWidth() - 16.0f, textBlockH);

    g.setColour (theme::text);
    g.setFont (theme::font (bigH, true));
    g.drawFittedText (big, textArea.removeFromTop (bigH * 1.25f).toNearestInt(), juce::Justification::centred, 1);

    g.setColour (theme::textMuted);
    g.setFont (theme::font (smallH));
    g.drawFittedText (small, textArea.removeFromTop (smallH * 1.5f).toNearestInt(), juce::Justification::centred, 1);

    if (showCaption)
    {
        g.setColour (theme::textMuted.withAlpha (0.9f));
        g.setFont (theme::font (juce::jlimit (10.0f, 14.0f, card.getHeight() * 0.04f)));
        g.drawFittedText (caption, card.removeFromBottom (30.0f).reduced (12.0f, 0.0f).toNearestInt(), juce::Justification::centred, 2);
    }
}
