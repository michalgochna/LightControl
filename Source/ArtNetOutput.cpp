#include "ArtNetOutput.h"

int channelsPerFixture (DmxLayout layout) noexcept
{
    switch (layout)
    {
        case DmxLayout::RGB:       return 3;
        case DmxLayout::RGBW:      return 4;
        case DmxLayout::DimmerRGB: return 4;
        case DmxLayout::RGBDimmer: return 4;
    }
    return 3;
}

const char* layoutName (DmxLayout layout) noexcept
{
    switch (layout)
    {
        case DmxLayout::RGB:       return "RGB";
        case DmxLayout::RGBW:      return "RGBW";
        case DmxLayout::DimmerRGB: return "Dimmer + RGB";
        case DmxLayout::RGBDimmer: return "RGB + Dimmer";
    }
    return "RGB";
}

juce::MemoryBlock ArtNetOutput::buildPacket (const ArtNetConfig& config, juce::Colour colour)
{
    constexpr int headerSize = 18;
    constexpr int maxChannels = 512;

    const int footprint = channelsPerFixture (config.layout);
    const int firstChannel = juce::jlimit (1, maxChannels, config.startChannel) - 1;   // 0-based

    juce::uint8 dmx[maxChannels] = {};
    int usedChannels = 0;

    for (int f = 0; f < juce::jmax (1, config.fixtures); ++f)
    {
        const int base = firstChannel + f * footprint;
        if (base + footprint > maxChannels)
            break;

        const juce::uint8 r = colour.getRed(), g = colour.getGreen(), b = colour.getBlue();

        switch (config.layout)
        {
            case DmxLayout::RGB:
                dmx[base] = r;  dmx[base + 1] = g;  dmx[base + 2] = b;
                break;

            case DmxLayout::RGBW:
            {
                // The common part of R, G and B goes to the white emitter.
                const juce::uint8 w = juce::jmin (r, g, b);
                dmx[base] = (juce::uint8) (r - w);  dmx[base + 1] = (juce::uint8) (g - w);
                dmx[base + 2] = (juce::uint8) (b - w);  dmx[base + 3] = w;
                break;
            }

            case DmxLayout::DimmerRGB:
                dmx[base] = 255;  dmx[base + 1] = r;  dmx[base + 2] = g;  dmx[base + 3] = b;
                break;

            case DmxLayout::RGBDimmer:
                dmx[base] = r;  dmx[base + 1] = g;  dmx[base + 2] = b;  dmx[base + 3] = 255;
                break;
        }

        usedChannels = base + footprint;
    }

    // The DMX length must be even and at least 2.
    int length = juce::jmax (2, usedChannels);
    if (length % 2 != 0)
        ++length;
    length = juce::jmin (length, maxChannels);

    const int universe = juce::jlimit (0, 32767, config.universe);

    juce::MemoryBlock packet ((size_t) (headerSize + length), true);
    auto* p = static_cast<juce::uint8*> (packet.getData());

    std::memcpy (p, "Art-Net", 7);                     // ID, terminated by 0 (already zeroed)
    p[8]  = 0x00;  p[9]  = 0x50;                       // OpOutput / ArtDmx = 0x5000, low byte first
    p[10] = 0;     p[11] = 14;                         // protocol version 14, high byte first
    p[12] = 0;                                         // sequence (0 = disabled)
    p[13] = 0;                                         // physical input port
    p[14] = (juce::uint8) (universe & 0xff);           // SubUni
    p[15] = (juce::uint8) ((universe >> 8) & 0x7f);    // Net
    p[16] = (juce::uint8) (length >> 8);               // length, high byte first
    p[17] = (juce::uint8) (length & 0xff);
    std::memcpy (p + headerSize, dmx, (size_t) length);

    return packet;
}

bool ArtNetOutput::isValidAddress (const juce::String& address)
{
    const auto parts = juce::StringArray::fromTokens (address.trim(), ".", "");

    if (parts.size() != 4)
        return false;

    for (const auto& part : parts)
        if (part.isEmpty() || part.length() > 3 || ! part.containsOnly ("0123456789") || part.getIntValue() > 255)
            return false;

    return true;
}

bool ArtNetOutput::send (const ArtNetConfig& config, juce::Colour colour)
{
    if (! isValidAddress (config.ip))
        return false;

    const auto packet = buildPacket (config, colour);
    const auto written = socket.write (config.ip.trim(), port, packet.getData(), (int) packet.getSize());
    return written == (int) packet.getSize();
}
