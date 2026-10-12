#pragma once
#include <JuceHeader.h>
#include "ArtNetOutput.h"
#include <array>
#include <atomic>

enum class TransportState
{
    Stopped = 0,    // also covers pause for now
    Playing,
    Recording
};

constexpr int numTransportStates = 3;

enum class OpenRgbStatus
{
    Off,                 // switched off in the settings
    Starting,            // not tried yet / reconnecting after a settings change
    Connected,
    ServerUnreachable,   // nothing is listening there (OpenRGB not running, SDK server off, wrong port)
    HostNotResolved,
    VersionNotSupported, // the server's protocol is too old for this client
    ConnectionLost,      // was connected, then requests started failing
    Error                // any other failure
};

/** Talks to the OpenRGB server on its own thread and sets the colour of all devices
    according to the transport state. Nothing in here blocks the audio or message thread. */
class LightWorker : private juce::Thread
{
public:
    LightWorker();
    ~LightWorker() override;   // turns the lights off and disconnects

    void start();

    static juce::Colour defaultColour (TransportState state) noexcept;
    void resetToFactorySettings();   // colours, server, device selection and Art-Net

    /** Real-time safe. Call once per processBlock. */
    void notifyBlock (TransportState state) noexcept;

    void setColour (TransportState state, juce::Colour colour) noexcept;
    juce::Colour getColour (TransportState state) const noexcept;

    /** The state shown/applied right now: a running preview wins over the host transport. */
    TransportState getRequestedState() const noexcept;

    /** Pretends the transport is in `state` for a while, so the user can see the colours without a host. */
    void previewState (TransportState state, int durationMs) noexcept;
    bool isPreviewing() const noexcept;

    void setOpenRgbEnabled (bool enabled) noexcept   { openRgbEnabled.store (enabled); }
    bool isOpenRgbEnabled() const noexcept           { return openRgbEnabled.load(); }
    bool isConnected() const noexcept        { return connected.load(); }
    OpenRgbStatus getOpenRgbStatus() const noexcept   { return OpenRgbStatus (openRgbStatus.load()); }
    bool hasReceivedAudio() const noexcept   { return receivedBlock.load(); }
    int getDeviceCount() const noexcept      { return deviceCount.load(); }

    // Message-thread API. The worker picks these changes up on its own thread.
    void setServer (const juce::String& newHost, int newPort);
    juce::String getHost() const;
    int getPort() const;

    juce::StringArray getDeviceNames() const;           // devices currently reported by OpenRGB
    void setDeviceEnabled (const juce::String& name, bool enabled);
    bool isDeviceEnabled (const juce::String& name) const;
    juce::StringArray getDisabledDevices() const;
    void setDisabledDevices (const juce::StringArray& names);

    void setArtNetConfig (const ArtNetConfig& newConfig);
    ArtNetConfig getArtNetConfig() const;
    bool isArtNetOk() const noexcept         { return artNetState.load() == 1; }
    bool hasArtNetFailed() const noexcept    { return artNetState.load() == 2; }

private:
    void run() override;
    TransportState getEffectiveState() const noexcept;

    std::atomic<int> requestedState { int (TransportState::Stopped) };
    std::atomic<juce::uint32> lastBlockMs { 0 };
    std::atomic<bool> receivedBlock { false };
    std::atomic<int> previewStateValue { 0 };
    std::atomic<juce::uint32> previewUntilMs { 0 };
    std::atomic<bool> previewActive { false };
    std::atomic<bool> openRgbEnabled { true };
    std::array<std::atomic<juce::uint32>, numTransportStates> colours;

    mutable juce::CriticalSection lock;                 // never taken on the audio thread
    juce::String host { "127.0.0.1" };
    int port = 6742;
    juce::StringArray deviceNames;
    juce::StringArray disabledDevices;
    std::atomic<int> configVersion { 0 };               // bumped when the device selection changes
    std::atomic<bool> reconnectRequested { false };

    ArtNetConfig artNetConfig;
    std::atomic<int> artNetState { 0 };                 // 0 = off / not sent yet, 1 = ok, 2 = send failed

    std::atomic<int> openRgbStatus { int (OpenRgbStatus::Starting) };
    std::atomic<bool> connected { false };
    std::atomic<int> deviceCount { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LightWorker)
};
