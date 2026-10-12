#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr int headerHeight = 58;
    constexpr int minWidth = 300, minHeight = 240;

    struct StatusInfo
    {
        juce::String shortText, longText;
        juce::Colour colour;
    };

    StatusInfo describeOpenRgb (const LightWorker& lights)
    {
        const auto server = lights.getHost() + ":" + juce::String (lights.getPort());

        switch (lights.getOpenRgbStatus())
        {
            case OpenRgbStatus::Off:
                return { "OpenRGB: off", "OpenRGB: switched off", theme::neutral };
            case OpenRgbStatus::Connected:
            {
                const auto n = lights.getDeviceCount();
                if (n > 0)
                    return { "OpenRGB: " + juce::String (n) + (n == 1 ? " light" : " lights"),
                             "OpenRGB: connected, " + juce::String (n) + (n == 1 ? " device" : " devices"), theme::ok };
                return { "OpenRGB: no devices", "OpenRGB: connected, but it reports no devices", theme::warn };
            }
            case OpenRgbStatus::Starting:
                return { "OpenRGB: connecting", "OpenRGB: connecting to " + server + "...", theme::neutral };
            case OpenRgbStatus::ServerUnreachable:
                return { "OpenRGB: not found", "OpenRGB: not running at " + server + " (enable SDK Server)", theme::warn };
            case OpenRgbStatus::HostNotResolved:
                return { "OpenRGB: bad host", "OpenRGB: can't resolve host \"" + lights.getHost() + "\"", theme::error };
            case OpenRgbStatus::VersionNotSupported:
                return { "OpenRGB: too old", "OpenRGB: server protocol too old, update OpenRGB", theme::error };
            case OpenRgbStatus::ConnectionLost:
                return { "OpenRGB: reconnecting", "OpenRGB: connection lost, reconnecting...", theme::warn };
            case OpenRgbStatus::Error:
                return { "OpenRGB: error", "OpenRGB: error talking to " + server + ", retrying...", theme::error };
        }
        return {};
    }

    StatusInfo describeArtNet (const LightWorker& lights)
    {
        const auto art = lights.getArtNetConfig();

        if (! art.enabled)
            return { "Art-Net: off", "Art-Net: off", theme::neutral };
        if (! ArtNetOutput::isValidAddress (art.ip))
            return { "Art-Net: bad IP", "Art-Net: invalid node IP \"" + art.ip + "\" (e.g. 192.168.1.50)", theme::error };
        if (lights.isArtNetOk())
            return { "Art-Net: sending", "Art-Net: sending to " + art.ip + ", universe " + juce::String (art.universe), theme::ok };
        if (lights.hasArtNetFailed())
            return { "Art-Net: error", "Art-Net: can't send to \"" + art.ip + "\" (check the node IP)", theme::error };
        return { "Art-Net: starting", "Art-Net: starting...", theme::neutral };
    }
}

//==============================================================================
LightControlAudioProcessorEditor::LightControlAudioProcessorEditor (LightControlAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), display (p.lights), settingsPage (p.lights)
{
    setLookAndFeel (&laf.get());

    settingsPage.onSaveDefaults = [this] { return audioProcessor.saveAsDefaults(); };
    settingsPage.onLoadDefaults = [this] { audioProcessor.loadDefaults(); };
    settingsPage.onContentSizeChanged = [this] { layoutSettingsContent(); };

    settingsViewport.setViewedComponent (&settingsPage, false);
    settingsViewport.setScrollBarsShown (true, false);
    settingsViewport.setScrollBarThickness (8);

    display.onDoubleClick = [this] { setSettingsVisible (! settingsVisible); };

    settingsButton.onClick = [this] { setSettingsVisible (! settingsVisible); };
    settingsButton.setTooltip ("Show or hide the settings. Hidden, the window is just the lamp: resize it freely, e.g. for a second screen");

    addAndMakeVisible (display);
    addAndMakeVisible (openRgbChip);
    addAndMakeVisible (artNetChip);
    addAndMakeVisible (settingsButton);
    addAndMakeVisible (settingsViewport);

    settingsVisible = audioProcessor.uiSettingsVisible.load();
    settingsViewport.setVisible (settingsVisible);
    settingsButton.setButtonText (settingsVisible ? "Hide settings" : "Settings");

    constrainer.setMinimumSize (minWidth, minHeight);
    constrainer.setMaximumSize (4000, 3000);
    setConstrainer (&constrainer);
    setResizable (true, true);
    setSize (audioProcessor.uiWidth.load(), audioProcessor.uiHeight.load());

   #if JUCE_DEBUG || LIGHTCONTROL_DEBUG_HOOKS
    // Debug builds only (or -DLIGHTCONTROL_DEBUG_HOOKS), for screenshots and measurements without a DAW: LIGHTCONTROL_DEBUG_STATE=rec|play|stop, LIGHTCONTROL_DEBUG_HIDE=1
    {
        const auto state = juce::SystemStats::getEnvironmentVariable ("LIGHTCONTROL_DEBUG_STATE", {});
        if (state.isNotEmpty())
            audioProcessor.lights.previewState (state == "rec" ? TransportState::Recording
                                                : state == "play" ? TransportState::Playing : TransportState::Stopped, 600000);
        if (juce::SystemStats::getEnvironmentVariable ("LIGHTCONTROL_DEBUG_HIDE", {}) == "1")
            setSettingsVisible (false);
        if (juce::SystemStats::getEnvironmentVariable ("LIGHTCONTROL_DEBUG_CLICK", {}) == "1")
            juce::Timer::callAfterDelay (2500, [safe = juce::Component::SafePointer<GlowButton> (&settingsButton)]
                                         { if (safe != nullptr) safe->triggerClick(); });
    }
   #endif

    startTimerHz (15);
    updateStatuses();
}

LightControlAudioProcessorEditor::~LightControlAudioProcessorEditor()
{
    settingsViewport.setViewedComponent (nullptr, false);
    setLookAndFeel (nullptr);
}

void LightControlAudioProcessorEditor::setSettingsVisible (bool visible)
{
    settingsVisible = visible;
    audioProcessor.uiSettingsVisible = visible;
    settingsButton.setButtonText (visible ? "Hide settings" : "Settings");
    resized();
}

void LightControlAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (theme::bgTop, 0.0f, 0.0f, theme::bgBottom, 0.0f, (float) getHeight(), false));
    g.fillAll();

    // header: a little lamp and the title
    const auto logo = juce::Rectangle<float> (26.0f, 26.0f).withCentre ({ 32.0f, headerHeight * 0.5f });
    const auto c = display.getShownColour();
    theme::drawGlow (g, logo, 13.0f, c, 1.1f, 10.0f);
    g.setGradientFill (juce::ColourGradient (c.brighter (0.7f), logo.getX() + 8.0f, logo.getY() + 6.0f,
                                             c.darker (0.2f), logo.getRight(), logo.getBottom(), true));
    g.fillEllipse (logo);
    g.setColour (juce::Colours::white.withAlpha (0.8f));
    g.drawEllipse (logo, 1.5f);

    if (getWidth() >= 420)
    {
        g.setColour (theme::text);
        g.setFont (theme::font (21.0f, true));
        g.drawText ("LightControl", 54, 0, 200, headerHeight, juce::Justification::centredLeft);
    }
}

void LightControlAudioProcessorEditor::layoutSettingsContent()
{
    const int w = settingsViewport.getMaximumVisibleWidth();
    if (w <= 0)
        return;

    settingsPage.setSize (w, settingsPage.getHeightForWidth (w));
}

void LightControlAudioProcessorEditor::resized()
{
    auto r = getLocalBounds();
    const int width = getWidth(), height = getHeight();

    // header: [logo + title] ............ [chips] [settings button]
    auto header = r.removeFromTop (headerHeight);
    const int btnW = 126;
    settingsButton.setBounds (GlowButton::withGlowMargin (header.removeFromRight (btnW + 16).reduced (0, 12).withTrimmedRight (6)
                                                                .withSizeKeepingCentre (btnW, 32)));

    const bool showChips = width >= 520;
    openRgbChip.setVisible (showChips);
    artNetChip.setVisible (showChips);
    if (showChips)
    {
        header.removeFromRight (6);
        const int h = 28;
        auto chipRow = header.withSizeKeepingCentre (header.getWidth(), h);
        artNetChip.setBounds (chipRow.removeFromRight (artNetChip.getIdealWidth()));
        chipRow.removeFromRight (8);
        openRgbChip.setBounds (chipRow.removeFromRight (openRgbChip.getIdealWidth()));
    }

    auto body = r.reduced (8, 0).withTrimmedBottom (8);
    settingsViewport.setVisible (settingsVisible);

    if (! settingsVisible)
    {
        display.setBounds (body);
    }
    else if (width >= 640 && width >= height * 1.15)          // wide: lamp left, settings right
    {
        auto side = body.removeFromRight (juce::jlimit (320, 460, (int) (width * 0.38f)));
        body.removeFromRight (6);
        display.setBounds (body);
        settingsViewport.setBounds (side);
    }
    else                                                          // narrow: lamp on top, settings below
    {
        const int lampH = juce::jlimit (170, 460, (int) (body.getHeight() * 0.42f));
        display.setBounds (body.removeFromTop (lampH));
        body.removeFromTop (4);
        settingsViewport.setBounds (body);
    }

    layoutSettingsContent();

    audioProcessor.uiWidth = width;
    audioProcessor.uiHeight = height;
}

void LightControlAudioProcessorEditor::updateStatuses()
{
    auto& lights = audioProcessor.lights;

    const auto transport = [&]() -> StatusInfo
    {
        if (lights.isPreviewing())
            return { {}, "Transport: previewing a state", theme::accentDeep };
        if (! lights.hasReceivedAudio())
            return { {}, "Transport: waiting for the host (no audio processed yet)", theme::neutral };

        switch (lights.getRequestedState())
        {
            case TransportState::Recording: return { {}, "Transport: REC",  theme::error };
            case TransportState::Playing:   return { {}, "Transport: PLAY", theme::ok };
            case TransportState::Stopped:   break;
        }
        return { {}, "Transport: STOP", theme::text };
    }();

    const auto orgb = describeOpenRgb (lights);
    const auto art = describeArtNet (lights);

    openRgbChip.set (orgb.shortText, orgb.colour, orgb.longText);
    artNetChip.set (art.shortText, art.colour, art.longText);

    juce::Array<juce::Colour> colours { transport.colour, orgb.colour, art.colour };
    settingsPage.setStatusLines ({ transport.longText, orgb.longText, art.longText }, colours);

    // Is anything real being driven? If not, tell the user the lamp on screen is all there is.
    const bool orgbActive = lights.getOpenRgbStatus() == OpenRgbStatus::Connected && lights.getDeviceCount() > 0;
    const bool artActive = lights.isArtNetOk();

    if (orgbActive || artActive)
    {
        juce::StringArray parts;
        if (orgbActive)
            parts.add ("OpenRGB (" + juce::String (lights.getDeviceCount()) + ")");
        if (artActive)
            parts.add ("Art-Net");
        display.setCaption ("Driving lights: " + parts.joinIntoString (" + "));
    }
    else
    {
        display.setCaption ("On-screen light only - no lights connected");
    }

    // chip widths follow their text
    if (openRgbChip.isVisible() && (openRgbChip.getWidth() != openRgbChip.getIdealWidth()
                                    || artNetChip.getWidth() != artNetChip.getIdealWidth()))
        resized();
}

void LightControlAudioProcessorEditor::timerCallback()
{
    repaint (0, 0, 70, headerHeight);   // the little lamp in the header follows the state colour

    if (++tick % 4 == 0)
        updateStatuses();
}
