#pragma once
#include <JuceHeader.h>
#include <vector>

/** The text of the built-in Help, as simple blocks that HelpPage knows how to lay out. */
namespace help
{
    enum class Section { Installation = 0, Configuration, Usage, Licences };
    constexpr int numSections = 4;

    const char* sectionTitle (Section);

    struct Block
    {
        enum class Type { Heading, Subheading, Paragraph, Bullet, Code, Note, Link };

        Type type;
        juce::String text;
        juce::String url;      // Link only
    };

    std::vector<Block> getBlocks (Section);
}
