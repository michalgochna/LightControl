#pragma once
#include <JuceHeader.h>
#include "HelpContent.h"
#include "Widgets.h"

/** One help section laid out as headings, paragraphs, bullets, code boxes and links. */
class HelpArticle : public juce::Component
{
public:
    void setBlocks (std::vector<help::Block> newBlocks);

    /** Lays the text out for `width` and returns the height it needs. */
    int layoutFor (int width);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Laid
    {
        help::Block block;
        juce::TextLayout layout;
        juce::Rectangle<float> area;      // the whole block
        juce::Rectangle<float> textArea;  // where the text goes inside it
    };

    juce::OwnedArray<juce::HyperlinkButton> linkButtons;
    std::vector<Laid> laid;
    std::vector<help::Block> blocks;
    int laidWidth = -1;
};

/** The Help panel: pick a section, read it. */
class HelpPage : public juce::Component
{
public:
    HelpPage();

    void paint (juce::Graphics&) override;
    void resized() override;

    void showSection (help::Section);

private:
    void layoutArticle();

    GlowButton sectionButtons[help::numSections] { GlowButton ("-"), GlowButton ("-"), GlowButton ("-"), GlowButton ("-") };
    help::Section current = help::Section::Installation;
    int buttonsHeight = 40;

    HelpArticle article;
    juce::Viewport viewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelpPage)
};
