#include "LightLookAndFeel.h"
#include "Widgets.h"

LightLookAndFeel::LightLookAndFeel()
{
    setColourScheme (juce::LookAndFeel_V4::ColourScheme (0xfffffbf0, 0xffffffff, 0xffffffff, 0xffe0d8c4, 0xff262a36,
                                                         0xffffb300, 0xff262a36, 0xffffb300, 0xff262a36));

    setColour (juce::Label::textColourId, theme::text);
    setColour (juce::TextEditor::textColourId, theme::text);
    setColour (juce::TextEditor::backgroundColourId, theme::card);
    setColour (juce::TextEditor::highlightColourId, theme::accent.withAlpha (0.35f));
    setColour (juce::TextEditor::highlightedTextColourId, theme::text);
    setColour (juce::CaretComponent::caretColourId, theme::accentDeep);
    setColour (juce::ComboBox::textColourId, theme::text);
    setColour (juce::ComboBox::arrowColourId, theme::accentDeep);
    setColour (juce::PopupMenu::backgroundColourId, theme::card);
    setColour (juce::PopupMenu::textColourId, theme::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, theme::accent.withAlpha (0.25f));
    setColour (juce::PopupMenu::highlightedTextColourId, theme::text);
    setColour (juce::ScrollBar::thumbColourId, theme::accent.withAlpha (0.55f));
    setColour (juce::TooltipWindow::backgroundColourId, theme::text);
    setColour (juce::TooltipWindow::textColourId, juce::Colours::white);
    setColour (juce::TooltipWindow::outlineColourId, theme::text);
}

juce::Font LightLookAndFeel::getComboBoxFont (juce::ComboBox&)   { return theme::font (14.0f); }
juce::Font LightLookAndFeel::getLabelFont (juce::Label& l)       { return l.getFont(); }

void LightLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool highlighted, bool)
{
    const auto bounds = button.getLocalBounds().toFloat();
    const bool on = button.getToggleState();

    // a little light switch: pill track, glowing knob when on
    const float trackH = 20.0f, trackW = 38.0f;
    auto track = juce::Rectangle<float> (trackW, trackH).withCentre ({ bounds.getX() + 8.0f + trackW * 0.5f, bounds.getCentreY() });

    if (on)
        theme::drawGlow (g, track, trackH * 0.5f, theme::accent, highlighted ? 1.0f : 0.7f, 8.0f);

    g.setGradientFill (on ? juce::ColourGradient (theme::accent.brighter (0.2f), 0.0f, track.getY(),
                                                   theme::accentDeep, 0.0f, track.getBottom(), false)
                          : juce::ColourGradient (juce::Colour (0xffd9dbe4), 0.0f, track.getY(),
                                                   juce::Colour (0xffc4c7d4), 0.0f, track.getBottom(), false));
    g.fillRoundedRectangle (track, trackH * 0.5f);

    const float knobD = trackH - 4.0f;
    const float knobX = on ? track.getRight() - knobD - 2.0f : track.getX() + 2.0f;
    g.setColour (juce::Colours::white);
    g.fillEllipse (knobX, track.getY() + 2.0f, knobD, knobD);

    g.setColour (button.findColour (juce::ToggleButton::textColourId).withAlpha (button.isEnabled() ? 1.0f : 0.5f));
    g.setColour (theme::text.withAlpha (button.isEnabled() ? 1.0f : 0.5f));
    g.setFont (theme::font (14.0f));
    g.drawFittedText (button.getButtonText(), bounds.toNearestInt().withTrimmedLeft ((int) trackW + 20),
                      juce::Justification::centredLeft, 1);
}

void LightLookAndFeel::fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor& e)
{
    g.setColour (e.findColour (juce::TextEditor::backgroundColourId));
    g.fillRoundedRectangle (0.0f, 0.0f, (float) width, (float) height, 8.0f);
}

void LightLookAndFeel::drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& e)
{
    auto b = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);

    if (e.hasKeyboardFocus (true) && ! e.isReadOnly())
    {
        g.setColour (theme::accent.withAlpha (0.25f));
        g.drawRoundedRectangle (b, 8.0f, 3.5f);
        g.setColour (theme::accent);
        g.drawRoundedRectangle (b.reduced (1.0f), 7.0f, 1.5f);
    }
    else
    {
        g.setColour (theme::cardEdge);
        g.drawRoundedRectangle (b, 8.0f, 1.2f);
    }
}

void LightLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    auto b = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);
    g.setColour (theme::card);
    g.fillRoundedRectangle (b, 8.0f);
    g.setColour (box.hasKeyboardFocus (true) ? theme::accent : theme::cardEdge);
    g.drawRoundedRectangle (b, 8.0f, box.hasKeyboardFocus (true) ? 1.8f : 1.2f);

    juce::Path arrow;
    const float cx = (float) width - 16.0f, cy = (float) height * 0.5f;
    arrow.addTriangle (cx - 4.5f, cy - 2.0f, cx + 4.5f, cy - 2.0f, cx, cy + 3.0f);
    g.setColour (theme::accentDeep);
    g.fillPath (arrow);
}
