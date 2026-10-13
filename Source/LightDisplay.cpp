#include "LightDisplay.h"
#include "Widgets.h"


namespace
{
    constexpr double twoPi = juce::MathConstants<double>::twoPi;
    constexpr float cardRadius = 26.0f;
}

LightDisplay::LightDisplay (LightWorker& worker) : lights (worker)
{
   #if JUCE_DEBUG || LIGHTCONTROL_DEBUG_HOOKS
    freezeTime = juce::SystemStats::getEnvironmentVariable ("LIGHTCONTROL_DEBUG_FREEZE", {}) == "1";
   #endif

    setOpaque (true);      // the base image covers every pixel, so the parent doesn't need to paint behind us
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

    shown = freezeTime ? lights.getColour (state) : shown.interpolatedWith (lights.getColour (state), 0.2f);

    const double speed = state == TransportState::Playing ? 0.35 : (state == TransportState::Recording ? 0.08 : 0.0);
    rotation = freezeTime ? 0.4 : std::fmod (rotation + dt * speed * twoPi, twoPi);

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

void LightDisplay::moved()   { layers.valid = false; }
void LightDisplay::resized() { layers.valid = false; }

void LightDisplay::rebuildLayers (juce::Rectangle<float> card, juce::Point<float> centre, float baseR, juce::Colour c)
{
    const int w = getWidth(), h = getHeight();
    const float margin = 14.0f;

    // Base: exactly what the editor paints behind the lamp (its background gradient), plus the card.
    layers.base = juce::Image (juce::Image::RGB, w, h, false);
    {
        juce::Graphics g (layers.base);
        const float top = -(float) getY();
        g.setGradientFill (juce::ColourGradient (theme::bgTop, 0.0f, top, theme::bgBottom, 0.0f, top + (float) getParentHeight(), false));
        g.fillAll();

        g.setGradientFill (juce::ColourGradient (juce::Colours::white, 0.0f, card.getY(),
                                                 juce::Colours::white.interpolatedWith (c, 0.12f), 0.0f, card.getBottom(), false));
        g.fillRoundedRectangle (card, cardRadius);
    }

    // Overlay: the glow outside the card and the halo inside it, both scaled by the pulse when drawn.
    layers.overlay = juce::Image (juce::Image::ARGB, w, h, true);
    {
        juce::Graphics g (layers.overlay);

        {
            juce::Path outside;                              // everything except the card (even-odd)
            outside.setUsingNonZeroWinding (false);
            outside.addRectangle (getLocalBounds().toFloat());
            outside.addRoundedRectangle (card, cardRadius);
            g.saveState();
            g.reduceClipRegion (outside);
            theme::drawGlow (g, card, cardRadius, c, 0.9f, margin);
            g.restoreState();
        }

        juce::Path inside;
        inside.addRoundedRectangle (card, cardRadius);
        g.reduceClipRegion (inside);

        const float haloR = baseR * 3.8f;
        g.setGradientFill (juce::ColourGradient (c.withAlpha (0.55f), centre.x, centre.y,
                                                 c.withAlpha (0.0f), centre.x + haloR, centre.y, true));
        g.fillEllipse (centre.x - haloR, centre.y - haloR, haloR * 2.0f, haloR * 2.0f);
    }

    layers.orbs.clear();
    layers.colour = c;
    layers.bounds = getLocalBounds();
    layers.parentHeight = getParentHeight();
    layers.centre = centre;
    layers.baseR = baseR;
    layers.valid = true;
}

const juce::Image& LightDisplay::getOrb (float radius, juce::Point<float> centre, juce::Colour c, juce::Point<int>& origin)
{
    // The gradients are the costliest part of a frame and only depend on the size, which pulses in
    // quarter-pixel steps: draw each size once. The image is rendered with the lamp's exact sub-pixel
    // centre, so blitting it at a whole-pixel position gives the same pixels as drawing it directly.
    if (layers.orbCentre.getDistanceFrom (centre) > 0.01f)
    {
        layers.orbs.clear();
        layers.orbCentre = centre;
    }

    const int key = (int) std::lround (radius * 4.0f);
    const float R = (float) key * 0.25f;
    const int s = (int) std::ceil (R * 2.0f) + 8;

    origin = { (int) std::floor (centre.x) - s / 2, (int) std::floor (centre.y) - s / 2 };

    auto it = layers.orbs.find (key);
    if (it != layers.orbs.end())
        return it->second;

    if (layers.orbs.size() > 40)
        layers.orbs.clear();

    const float cx = centre.x - (float) origin.x, cy = centre.y - (float) origin.y;

    juce::Image img (juce::Image::ARGB, s, s, true);
    juce::Graphics g (img);

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

    return layers.orbs.emplace (key, std::move (img)).first->second;
}

void LightDisplay::paint (juce::Graphics& g)
{
    const double nowMs = freezeTime ? flashStartMs + 5000.0 : juce::Time::getMillisecondCounterHiRes();
    const double t = freezeTime ? 1.0 : nowMs / 1000.0;
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
    {
        g.fillAll (theme::bgTop);    // opaque component: every pixel must be painted
        return;
    }

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
        || std::abs (layers.centre.y - cy) > 0.5f || layers.parentHeight != getParentHeight()
        || colourDistance (layers.colour, c) >= 5)
        rebuildLayers (card, { cx, cy }, baseR, c);

    const float op = juce::jlimit (0.0f, 1.0f, intensity);
    const auto cardInt = card.toNearestInt();

    g.drawImageAt (layers.base, 0, 0);          // opaque copy, no blending
    g.setOpacity (op);
    g.drawImageAt (layers.overlay, 0, 0);
    g.setOpacity (1.0f);

    // Everything below stays inside the card; a rectangular clip is far cheaper than a rounded-path clip.
    g.saveState();
    g.reduceClipRegion (cardInt);

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
        juce::Point<int> origin;
        const auto& orb = getOrb (R, { cx, cy }, c, origin);
        g.drawImageAt (orb, origin.x, origin.y);
    }

    g.restoreState();

    // border
    g.setColour (c.withAlpha (0.35f));
    g.drawRoundedRectangle (card.reduced (0.75f), cardRadius, 1.5f);

    // labels: an icon in the colour of the state (circle = recording / playing, two bars = paused / stopped)
    const char* small = "Paused";
    switch (state)
    {
        case TransportState::Recording: small = "Recording"; break;
        case TransportState::Playing:   small = "Playing";   break;
        case TransportState::Stopped:   break;
    }
    if (waiting)
        small = "Waiting for the transport";

    auto textArea = juce::Rectangle<float> (card.getX() + 8.0f, cy + baseR * 1.3f, card.getWidth() - 16.0f, textBlockH);
    const auto iconRow = textArea.removeFromTop (bigH * 1.25f);

    if (waiting)
    {
        g.setColour (theme::text);
        g.setFont (theme::font (bigH, true));
        g.drawFittedText ("READY", iconRow.toNearestInt(), juce::Justification::centred, 1);
    }
    else
    {
        const float s = bigH * 0.95f;
        const auto icon = juce::Rectangle<float> (s, s).withCentre (iconRow.getCentre());
        const auto outline = c.darker (0.5f).withAlpha (0.8f);

        if (state == TransportState::Stopped)
        {
            const float barW = s * 0.30f;
            const auto left = juce::Rectangle<float> (icon.getX() + s * 0.10f, icon.getY(), barW, s);
            const auto right = left.withX (icon.getRight() - s * 0.10f - barW);
            for (const auto& bar : { left, right })
            {
                g.setColour (c);
                g.fillRoundedRectangle (bar, barW * 0.25f);
                g.setColour (outline);
                g.drawRoundedRectangle (bar, barW * 0.25f, 1.0f);
            }
        }
        else
        {
            g.setColour (c);
            g.fillEllipse (icon);
            g.setColour (outline);
            g.drawEllipse (icon.reduced (0.5f), 1.0f);
        }
    }

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
