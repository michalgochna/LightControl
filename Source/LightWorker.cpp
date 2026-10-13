#include "LightWorker.h"
#include "OpenRGB/Client.hpp"
#include <set>

namespace
{
    constexpr int reconnectDelayMs = 2000;
    constexpr int pollIntervalMs = 20;

    // Once nothing has changed for a while there is no need to look at the transport so often.
    constexpr int idlePollIntervalMs = 60;
    constexpr juce::uint32 idleAfterMs = 2000;

    // Hosts call setStateInformation() shortly after creating the plugin. Waiting a little before the first output
    // keeps the lights from flashing the default settings of a project that has its own.
    constexpr int startupGraceMs = 400;
    constexpr juce::uint32 artNetRefreshMs = 1000;   // many nodes blank their output when frames stop coming

    // Hosts may stop calling processBlock when the transport is stopped (e.g. Reaper without "Run FX when stopped").
    constexpr juce::uint32 blockTimeoutMs = 500;

    /** Sets one colour on every enabled device. Returns false if no device accepted it (connection lost). */
    bool applyColour (orgb::Client& client, const orgb::DeviceList& devices, juce::Colour c,
                      const juce::StringArray& disabled, std::set<uint32_t>* switchedToCustom)
    {
        int total = 0, ok = 0;
        for (const orgb::Device& device : devices)
        {
            if (disabled.contains (juce::String (device.name)))
                continue;

            ++total;

            if (switchedToCustom != nullptr && switchedToCustom->insert (device.idx).second)
            {
                client.switchToCustomMode (device);   // some devices don't support it, that's fine
                juce::Thread::sleep (10);
            }

            if (client.setDeviceColor (device, { c.getRed(), c.getGreen(), c.getBlue() }) == orgb::RequestStatus::Success)
                ++ok;
        }
        return total == 0 || ok > 0;
    }
}

LightWorker::LightWorker() : juce::Thread ("LightControl OpenRGB")
{
    for (int i = 0; i < numTransportStates; ++i)
        colours[(size_t) i].store (defaultColour (TransportState (i)).getARGB());
}

LightWorker::~LightWorker()
{
    stopThread (3000);
}

juce::Colour LightWorker::defaultColour (TransportState state) noexcept
{
    switch (state)
    {
        case TransportState::Recording: return juce::Colour (0xffff0000);   // red
        case TransportState::Playing:   return juce::Colour (0xff00ff00);   // green
        case TransportState::Stopped:   break;
    }
    return juce::Colour (0xffffa000);                                       // amber
}

void LightWorker::resetToFactorySettings()
{
    for (int i = 0; i < numTransportStates; ++i)
        setColour (TransportState (i), defaultColour (TransportState (i)));

    setOpenRgbEnabled (true);
    setServer ("127.0.0.1", 6742);
    setDisabledDevices ({});
    setArtNetConfig ({});
}

void LightWorker::start()
{
    startThread();
}

void LightWorker::notifyBlock (TransportState state) noexcept
{
    requestedState.store (int (state));
    lastBlockMs.store (juce::Time::getMillisecondCounter());
    receivedBlock.store (true);
}

void LightWorker::setColour (TransportState state, juce::Colour colour) noexcept
{
    colours[(size_t) state].store (colour.getARGB());
}

juce::Colour LightWorker::getColour (TransportState state) const noexcept
{
    return juce::Colour (colours[(size_t) state].load());
}

TransportState LightWorker::getRequestedState() const noexcept
{
    return getEffectiveState();
}

void LightWorker::previewState (TransportState state, int durationMs) noexcept
{
    previewStateValue.store (int (state));
    previewUntilMs.store (juce::Time::getMillisecondCounter() + (juce::uint32) durationMs);
    previewActive.store (true);
}

bool LightWorker::isPreviewing() const noexcept
{
    return previewActive.load() && (juce::int32) (previewUntilMs.load() - juce::Time::getMillisecondCounter()) > 0;
}

void LightWorker::setServer (const juce::String& newHost, int newPort)
{
    bool changed = false;
    {
        const juce::ScopedLock sl (lock);
        const auto trimmedHost = newHost.trim();
        const auto clampedPort = juce::jlimit (1, 65535, newPort);

        changed = trimmedHost != host || clampedPort != port;
        host = trimmedHost;
        port = clampedPort;
    }

    // Reconnecting to the same address would only drop a working connection.
    if (changed)
        reconnectRequested.store (true);
}

juce::String LightWorker::getHost() const
{
    const juce::ScopedLock sl (lock);
    return host;
}

int LightWorker::getPort() const
{
    const juce::ScopedLock sl (lock);
    return port;
}

juce::StringArray LightWorker::getDeviceNames() const
{
    const juce::ScopedLock sl (lock);
    return deviceNames;
}

void LightWorker::setDeviceEnabled (const juce::String& name, bool enabled)
{
    {
        const juce::ScopedLock sl (lock);
        if (enabled)
            disabledDevices.removeString (name);
        else
            disabledDevices.addIfNotAlreadyThere (name);
    }
    ++configVersion;
}

bool LightWorker::isDeviceEnabled (const juce::String& name) const
{
    const juce::ScopedLock sl (lock);
    return ! disabledDevices.contains (name);
}

juce::StringArray LightWorker::getDisabledDevices() const
{
    const juce::ScopedLock sl (lock);
    return disabledDevices;
}

void LightWorker::setDisabledDevices (const juce::StringArray& names)
{
    {
        const juce::ScopedLock sl (lock);
        disabledDevices = names;
    }
    ++configVersion;
}

void LightWorker::setArtNetConfig (const ArtNetConfig& newConfig)
{
    {
        const juce::ScopedLock sl (lock);
        artNetConfig = newConfig;
    }
    ++artNetVersion;
}

ArtNetConfig LightWorker::getArtNetConfig() const
{
    const juce::ScopedLock sl (lock);
    return artNetConfig;
}

TransportState LightWorker::getEffectiveState() const noexcept
{
    if (isPreviewing())
        return TransportState (previewStateValue.load());

    if (receivedBlock.load() && juce::Time::getMillisecondCounter() - lastBlockMs.load() > blockTimeoutMs)
        return TransportState::Stopped;

    return TransportState (requestedState.load());
}

void LightWorker::run()
{
    orgb::Client client ("LightControl");
    orgb::DeviceList devices;
    std::set<uint32_t> switchedToCustom;

    bool isUp = false;
    bool haveApplied = false;
    TransportState appliedState = TransportState::Stopped;
    juce::uint32 appliedColour = 0;
    int appliedConfig = -1;
    juce::uint32 nextConnectAttempt = 0;

    juce::uint32 lastChangeMs = juce::Time::getMillisecondCounter();
    TransportState seenState = TransportState::Stopped;
    juce::uint32 seenColour = 0;
    int seenConfig = -1, seenArtVersion = -1, artSeenVersion = -1;

    ArtNetOutput artNet;
    ArtNetConfig artCfg;                     // local copy of the Art-Net settings
    ArtNetConfig artSent;                    // config of the last frames we sent
    bool artActive = false;                  // have we sent anything that is still lit?
    TransportState artState = TransportState::Stopped;
    juce::uint32 artColour = 0, artLastSendMs = 0;

    const auto dropConnection = [&] (juce::uint32 retryAfterMs, OpenRgbStatus why)
    {
        client.disconnect();
        devices = {};
        switchedToCustom.clear();
        isUp = false;
        haveApplied = false;
        openRgbStatus.store (int (why));
        deviceCount.store (0);
        {
            const juce::ScopedLock sl (lock);
            deviceNames.clear();
        }
        nextConnectAttempt = juce::Time::getMillisecondCounter() + retryAfterMs;
    };

    wait (startupGraceMs);

    while (! threadShouldExit())
    {
        if (reconnectRequested.exchange (false))
            dropConnection (0, OpenRgbStatus::Starting);

        const auto now = juce::Time::getMillisecondCounter();
        const auto state = getEffectiveState();
        const auto colour = colours[(size_t) state].load();
        const auto config = configVersion.load();
        const auto artVersion = artNetVersion.load();

        if (state != seenState || colour != seenColour || config != seenConfig || artVersion != seenArtVersion)
        {
            seenState = state;
            seenColour = colour;
            seenConfig = config;
            seenArtVersion = artVersion;
            lastChangeMs = now;
        }

        const bool useOpenRgb = openRgbEnabled.load();

        if (! useOpenRgb)
        {
            if (isUp)
            {
                applyColour (client, devices, juce::Colours::black, getDisabledDevices(), nullptr);
                dropConnection (0, OpenRgbStatus::Off);
            }

            openRgbStatus.store (int (OpenRgbStatus::Off));
        }

        if (useOpenRgb && ! isUp && (juce::int32) (juce::Time::getMillisecondCounter() - nextConnectAttempt) >= 0)
        {
            juce::String serverHost;
            int serverPort;
            {
                const juce::ScopedLock sl (lock);
                serverHost = host;
                serverPort = port;
            }

            const auto connectStatus = client.connect (serverHost.toStdString(), (uint16_t) serverPort);

            if (connectStatus == orgb::ConnectStatus::Success)
            {
                client.setTimeout (std::chrono::milliseconds (500));
                auto result = client.requestDeviceList();

                if (result.status == orgb::RequestStatus::Success)
                {
                    devices = std::move (result.devices);

                    juce::StringArray names;
                    for (const orgb::Device& device : devices)
                        names.addIfNotAlreadyThere (juce::String (device.name));

                    {
                        const juce::ScopedLock sl (lock);
                        deviceNames = names;
                    }

                    isUp = true;
                    deviceCount.store ((int) devices.size());
                    openRgbStatus.store (int (OpenRgbStatus::Connected));
                }
                else
                {
                    dropConnection (reconnectDelayMs, OpenRgbStatus::Error);
                }
            }
            else
            {
                openRgbStatus.store (int (connectStatus == orgb::ConnectStatus::HostNotResolved     ? OpenRgbStatus::HostNotResolved
                                          : connectStatus == orgb::ConnectStatus::VersionNotSupported ? OpenRgbStatus::VersionNotSupported
                                          : connectStatus == orgb::ConnectStatus::ConnectFailed       ? OpenRgbStatus::ServerUnreachable
                                                                                                      : OpenRgbStatus::Error));

                nextConnectAttempt = juce::Time::getMillisecondCounter() + (juce::uint32) reconnectDelayMs;
            }
        }

        if (isUp)
        {
            if (! haveApplied || state != appliedState || colour != appliedColour || config != appliedConfig)
            {
                if (applyColour (client, devices, juce::Colour (colour), getDisabledDevices(), &switchedToCustom))
                {
                    haveApplied = true;
                    appliedState = state;
                    appliedColour = colour;
                    appliedConfig = config;
                }
                else
                {
                    dropConnection (reconnectDelayMs, OpenRgbStatus::ConnectionLost);
                }
            }
        }

        // Art-Net is independent of the OpenRGB connection.
        {
            if (artVersion != artSeenVersion)
            {
                artSeenVersion = artVersion;
                const juce::ScopedLock sl (lock);
                artCfg = artNetConfig;
            }

            if (artCfg.enabled)
            {
                if (! artActive || artCfg != artSent || state != artState || colour != artColour
                    || now - artLastSendMs >= artNetRefreshMs)
                {
                    if (artActive && artCfg != artSent)
                        artNet.send (artSent, juce::Colours::black);   // don't leave the previous fixtures lit

                    artNetState.store (artNet.send (artCfg, juce::Colour (colour)) ? 1 : 2);
                    artSent = artCfg;
                    artState = state;
                    artColour = colour;
                    artLastSendMs = now;
                    artActive = true;
                }
            }
            else if (artActive)
            {
                artNet.send (artSent, juce::Colours::black);   // turned off in the GUI: lights off
                artActive = false;
                artNetState.store (0);
            }
        }

        wait (now - lastChangeMs > idleAfterMs ? idlePollIntervalMs : pollIntervalMs);
    }

    if (artActive)
    {
        for (int i = 0; i < 3; ++i)   // UDP is lossy, repeat the final blackout
        {
            artNet.send (artSent, juce::Colours::black);
            juce::Thread::sleep (10);
        }
    }

    if (isUp)
        applyColour (client, devices, juce::Colours::black, getDisabledDevices(), nullptr);   // lights off when the plugin goes away

    client.disconnect();
}
