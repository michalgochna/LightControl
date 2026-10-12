#pragma once
#include <JuceHeader.h>

enum class DmxLayout
{
    RGB = 0,
    RGBW,
    DimmerRGB,    // master dimmer, then R G B
    RGBDimmer     // R G B, then master dimmer
};

int channelsPerFixture (DmxLayout layout) noexcept;
const char* layoutName (DmxLayout layout) noexcept;

struct ArtNetConfig
{
    bool enabled = false;
    juce::String ip = "192.168.1.100";   // node address, or a broadcast address
    int universe = 0;                    // Art-Net 15-bit port-address (0-based)
    int startChannel = 1;                // 1-512
    int fixtures = 1;
    DmxLayout layout = DmxLayout::RGB;

    bool operator== (const ArtNetConfig& o) const noexcept
    {
        return enabled == o.enabled && ip == o.ip && universe == o.universe
            && startChannel == o.startChannel && fixtures == o.fixtures && layout == o.layout;
    }
    bool operator!= (const ArtNetConfig& o) const noexcept { return ! (*this == o); }
};

/** Sends ArtDmx packets (UDP port 6454). Not thread-safe: use from one thread only. */
class ArtNetOutput
{
public:
    static constexpr int port = 6454;

    /** Only dotted IPv4 addresses are accepted. Host names are refused on purpose:
        resolving them could block the worker thread for seconds. */
    static bool isValidAddress (const juce::String& address);

    /** Builds a complete ArtDmx packet that sets every configured fixture to the given colour. */
    static juce::MemoryBlock buildPacket (const ArtNetConfig& config, juce::Colour colour);

    /** Returns true if the whole packet was handed to the network stack. */
    bool send (const ArtNetConfig& config, juce::Colour colour);

private:
    juce::DatagramSocket socket { true };
};
