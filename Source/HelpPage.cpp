#include "HelpPage.h"

namespace
{
    using Type = help::Block::Type;

    constexpr int sidePad = 12;
    constexpr float codeFontSize = 12.5f;

    juce::Font fontFor (Type t)
    {
        switch (t)
        {
            case Type::Heading:    return theme::font (20.0f, true);
            case Type::Subheading: return theme::font (15.0f, true);
            case Type::Code:       return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), codeFontSize, juce::Font::plain));
            case Type::Paragraph:
            case Type::Bullet:
            case Type::Note:
            case Type::Link:       break;
        }
        return theme::font (14.0f);
    }

    juce::Colour colourFor (Type t)
    {
        switch (t)
        {
            case Type::Subheading: return theme::accentDeep.darker (0.2f);
            case Type::Heading:
            case Type::Paragraph:
            case Type::Bullet:
            case Type::Code:
            case Type::Note:
            case Type::Link:       break;
        }
        return theme::text;
    }

    float spaceBefore (Type t, bool first)
    {
        if (first)
            return 0.0f;

        switch (t)
        {
            case Type::Heading:    return 0.0f;
            case Type::Subheading: return 18.0f;
            case Type::Bullet:     return 4.0f;
            case Type::Link:       return 4.0f;
            case Type::Code:
            case Type::Note:       return 8.0f;
            case Type::Paragraph:  return 8.0f;
        }
        return 8.0f;
    }
}

//==============================================================================
void HelpArticle::setBlocks (std::vector<help::Block> newBlocks)
{
    blocks = std::move (newBlocks);
    laidWidth = -1;

    linkButtons.clear();

    for (const auto& b : blocks)
    {
        if (b.type != Type::Link)
            continue;

        auto* button = linkButtons.add (new juce::HyperlinkButton (b.text, juce::URL (b.url)));
        button->setFont (theme::font (14.0f, true), false);
        button->setColour (juce::HyperlinkButton::textColourId, juce::Colour (0xff2563b8));
        button->setJustificationType (juce::Justification::centredLeft);
        button->setTooltip (b.url);
        addAndMakeVisible (button);
    }
}

int HelpArticle::layoutFor (int width)
{
    laid.clear();
    laidWidth = width;

    const float x = (float) sidePad, w = (float) juce::jmax (80, width - sidePad * 2);
    float y = 8.0f;
    bool first = true;
    int linkIndex = 0;

    for (const auto& b : blocks)
    {
        Laid item { b, {}, {}, {} };
        y += spaceBefore (b.type, first);
        first = false;

        float indentLeft = 0.0f, padding = 0.0f;
        if (b.type == Type::Bullet)  indentLeft = 16.0f;
        if (b.type == Type::Code)    padding = 8.0f;
        if (b.type == Type::Note)    { padding = 10.0f; indentLeft = 4.0f; }

        if (b.type == Type::Link)
        {
            item.area = { x + indentLeft, y, w - indentLeft, 22.0f };
            if (auto* button = linkButtons[linkIndex++])
                button->setBounds (item.area.toNearestInt().withTrimmedLeft (b.type == Type::Link ? 16 : 0));
            y = item.area.getBottom();
            laid.push_back (std::move (item));
            continue;
        }

        juce::AttributedString text;
        text.setWordWrap (juce::AttributedString::byWord);
        text.append (b.type == Type::Bullet ? b.text : b.text, fontFor (b.type), colourFor (b.type));

        const float textW = w - indentLeft - padding * 2.0f;
        item.layout.createLayout (text, juce::jmax (40.0f, textW));

        const float textH = item.layout.getHeight();
        item.area = { x + indentLeft, y, w - indentLeft, textH + padding * 2.0f };
        item.textArea = item.area.reduced (padding);
        y = item.area.getBottom();
        laid.push_back (std::move (item));
    }

    return (int) std::ceil (y) + 16;
}

void HelpArticle::resized()
{
    if (getWidth() != laidWidth)
    {
        const int h = layoutFor (getWidth());

        if (h != getHeight())
            setSize (getWidth(), h);

        repaint();
    }
}

void HelpArticle::paint (juce::Graphics& g)
{
    for (const auto& item : laid)
    {
        switch (item.block.type)
        {
            case Type::Code:
                g.setColour (juce::Colour (0xfff4f1e8));
                g.fillRoundedRectangle (item.area, 8.0f);
                g.setColour (theme::cardEdge);
                g.drawRoundedRectangle (item.area.reduced (0.5f), 8.0f, 1.0f);
                break;

            case Type::Note:
                g.setColour (theme::accent.withAlpha (0.14f));
                g.fillRoundedRectangle (item.area, 8.0f);
                g.setColour (theme::accent);
                g.fillRoundedRectangle (item.area.withWidth (4.0f), 2.0f);
                break;

            case Type::Bullet:
            {
                const auto dot = juce::Rectangle<float> (6.0f, 6.0f).withCentre ({ item.area.getX() - 8.0f, item.area.getY() + 9.0f });
                g.setColour (theme::accent);
                g.fillEllipse (dot);
                break;
            }

            case Type::Link:
                g.setColour (theme::textMuted);
                g.setFont (theme::font (13.0f));
                g.drawText (juce::String::fromUTF8 ("\xe2\x86\x92"), item.area.withWidth (16.0f), juce::Justification::centredLeft);
                continue;

            case Type::Heading:
            case Type::Subheading:
            case Type::Paragraph:
                break;
        }

        item.layout.draw (g, item.textArea.isEmpty() ? item.area : item.textArea);
    }
}

//==============================================================================
HelpPage::HelpPage()
{
    for (int i = 0; i < help::numSections; ++i)
    {
        auto& b = sectionButtons[i];
        b.setButtonText (help::sectionTitle ((help::Section) i));
        b.onClick = [this, i] { showSection ((help::Section) i); };
        addAndMakeVisible (b);
    }

    viewport.setViewedComponent (&article, false);
    viewport.setScrollBarsShown (true, false);
    viewport.setScrollBarThickness (8);
    addAndMakeVisible (viewport);

    showSection (help::Section::Installation);
}

void HelpPage::showSection (help::Section s)
{
    current = s;

    for (int i = 0; i < help::numSections; ++i)
        sectionButtons[i].setStyle (i == (int) s ? GlowButton::Style::Primary : GlowButton::Style::Soft);

    article.setBlocks (help::getBlocks (s));
    layoutArticle();
    viewport.setViewPosition (0, 0);
}

void HelpPage::layoutArticle()
{
    const int w = viewport.getMaximumVisibleWidth();
    if (w > 0)
        article.setSize (w, article.layoutFor (w));
}

void HelpPage::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (8.0f, 4.0f);
    b.removeFromTop (buttonsHeight);

    g.setColour (juce::Colour (0x14000000));
    g.fillRoundedRectangle (b.translated (0.0f, 2.5f).expanded (0.5f), 16.0f);
    g.setColour (theme::card);
    g.fillRoundedRectangle (b, 16.0f);
    g.setColour (theme::cardEdge);
    g.drawRoundedRectangle (b.reduced (0.5f), 16.0f, 1.0f);
}

void HelpPage::resized()
{
    auto area = getLocalBounds().reduced (8, 4);

    // the section buttons, wrapping to more rows when the panel is narrow
    constexpr int btnH = 30, gap = 6;
    int x = 0, y = 0;
    const int rowW = area.getWidth();

    juce::Array<int> widths;
    for (auto& b : sectionButtons)
    {
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText (theme::font (14.0f, true), b.getButtonText(), 0.0f, 0.0f);
        widths.add ((int) std::ceil (glyphs.getBoundingBox (0, glyphs.getNumGlyphs(), true).getWidth()) + 34);
    }

    for (int i = 0; i < help::numSections; ++i)
    {
        if (x > 0 && x + widths[i] > rowW)
        {
            x = 0;
            y += btnH + gap;
        }

        sectionButtons[i].setBounds (GlowButton::withGlowMargin ({ area.getX() + x, area.getY() + y, widths[i], btnH }));
        x += widths[i] + gap;
    }

    buttonsHeight = y + btnH + gap + 4;
    area.removeFromTop (buttonsHeight);
    viewport.setBounds (area.reduced (2, 6));
    layoutArticle();
    repaint();
}
