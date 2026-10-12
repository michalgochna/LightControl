#include "Settings.h"

namespace
{
    constexpr int outerPad = 8;
    constexpr int cardPad = 14;
    constexpr int rowGap = 8;
    constexpr int rowH = 30;
    constexpr int headerH = 26;
    constexpr int cardGap = 10;

    void styleHeader (juce::Label& l)
    {
        l.setFont (theme::font (15.0f, true));
        l.setColour (juce::Label::textColourId, theme::text);
    }

    void styleLabel (juce::Label& l)
    {
        l.setFont (theme::font (13.0f));
        l.setColour (juce::Label::textColourId, theme::textMuted);
    }
}

//==============================================================================
ColourRow::ColourRow (const juce::String& labelText, LightWorker& worker, TransportState state)
    : lights (worker), transportState (state)
{
    label.setText (labelText, juce::dontSendNotification);
    label.setFont (theme::font (14.0f, true));
    addAndMakeVisible (label);

    swatch.onClick = [this] { openSelector(); };
    addAndMakeVisible (swatch);

    previewButton.onClick = [this] { lights.previewState (transportState, 3000); };
    previewButton.setTooltip ("Show this state for 3 seconds on the lamp and on the connected lights");
    addAndMakeVisible (previewButton);

    refresh();
}

void ColourRow::resized()
{
    auto area = getLocalBounds();
    label.setBounds (area.removeFromLeft (92));

    const int h = rowH;
    auto line = area.withHeight (h).withY (area.getY() + (area.getHeight() - h) / 2);
    swatch.setBounds (GlowButton::withGlowMargin (line.removeFromLeft (112)));
    line.removeFromLeft (4);
    previewButton.setBounds (GlowButton::withGlowMargin (line.removeFromLeft (86)));
}

void ColourRow::refresh()
{
    const auto colour = lights.getColour (transportState);
    swatch.setAccent (colour);
    swatch.setButtonText (colour.toDisplayString (false));
    previewButton.setAccent (colour);
}

void ColourRow::openSelector()
{
    struct Popup : public juce::ColourSelector, private juce::ChangeListener
    {
        Popup (juce::Colour initial, std::function<void (juce::Colour)> cb)
            : juce::ColourSelector (showSliders | showColourspace), onChange (std::move (cb))
        {
            setLookAndFeel (&laf.get());
            setCurrentColour (initial, juce::dontSendNotification);
            addChangeListener (this);
            setSize (280, 320);
        }

        ~Popup() override
        {
            removeChangeListener (this);
            setLookAndFeel (nullptr);
        }

        void changeListenerCallback (juce::ChangeBroadcaster*) override
        {
            onChange (getCurrentColour().withAlpha (1.0f));
        }

        juce::SharedResourcePointer<LightLookAndFeel> laf;
        std::function<void (juce::Colour)> onChange;
    };

    juce::Component::SafePointer<ColourRow> safeThis (this);

    auto popup = std::make_unique<Popup> (lights.getColour (transportState), [safeThis] (juce::Colour c)
    {
        if (safeThis != nullptr)
        {
            safeThis->lights.setColour (safeThis->transportState, c);
            safeThis->lights.previewState (safeThis->transportState, 1500);   // show what you're picking
            safeThis->refresh();
        }
    });

    juce::CallOutBox::launchAsynchronously (std::move (popup), swatch.getScreenBounds().reduced (GlowButton::margin), nullptr);
}

//==============================================================================
SettingsPage::SettingsPage (LightWorker& worker) : lights (worker)
{
    setLookAndFeel (&laf.get());

    for (auto* l : { &statusHeader, &colourHeader, &serverHeader, &deviceHeader, &defaultsHeader })
    {
        styleHeader (*l);
        addAndMakeVisible (*l);
    }

    for (auto& l : statusLines)
    {
        l.setFont (theme::font (13.0f));
        addAndMakeVisible (l);
    }

    addAndMakeVisible (recordingRow);
    addAndMakeVisible (playingRow);
    addAndMakeVisible (stoppedRow);

    // OpenRGB
    openRgbToggle.setToggleState (lights.isOpenRgbEnabled(), juce::dontSendNotification);
    openRgbToggle.onClick = [this] { lights.setOpenRgbEnabled (openRgbToggle.getToggleState()); };
    openRgbToggle.setTooltip ("Switch off if you have no lights to control: the lamp on the screen keeps working");
    addAndMakeVisible (openRgbToggle);

    for (auto* l : { &hostLabel, &portLabel, &artIpLabel, &artUniverseLabel, &artStartLabel, &artFixturesLabel, &artLayoutLabel })
    {
        styleLabel (*l);
        addAndMakeVisible (*l);
    }

    // Text typed in before an editor has a parent keeps the colour of the default look, so set ours explicitly.
    for (auto* e : { &hostEditor, &portEditor, &artIpEditor, &artUniverseEditor, &artStartEditor, &artFixturesEditor })
    {
        e->setColour (juce::TextEditor::textColourId, theme::text);
        e->setJustification (juce::Justification::centredLeft);
        e->setIndents (10, 0);
        e->setFont (theme::font (14.0f));
    }

    hostEditor.setText (lights.getHost(), false);
    portEditor.setText (juce::String (lights.getPort()), false);
    portEditor.setInputRestrictions (5, "0123456789");
    hostEditor.onReturnKey = portEditor.onReturnKey = [this] { applyServer(); };
    applyButton.onClick = [this] { applyServer(); };
    addAndMakeVisible (hostEditor);
    addAndMakeVisible (portEditor);
    addAndMakeVisible (applyButton);

    // Art-Net
    const auto art = lights.getArtNetConfig();
    artEnabled.setToggleState (art.enabled, juce::dontSendNotification);
    artEnabled.onClick = [this] { applyArtNet(); };
    artIpEditor.setText (art.ip, false);
    artUniverseEditor.setText (juce::String (art.universe), false);
    artStartEditor.setText (juce::String (art.startChannel), false);
    artFixturesEditor.setText (juce::String (art.fixtures), false);
    artUniverseEditor.setInputRestrictions (5, "0123456789");
    artStartEditor.setInputRestrictions (3, "0123456789");
    artFixturesEditor.setInputRestrictions (3, "0123456789");

    for (int i = 0; i < 4; ++i)
        artLayoutBox.addItem (layoutName ((DmxLayout) i), i + 1);
    artLayoutBox.setSelectedId ((int) art.layout + 1, juce::dontSendNotification);
    artLayoutBox.onChange = [this] { applyArtNet(); };

    for (auto* e : { &artIpEditor, &artUniverseEditor, &artStartEditor, &artFixturesEditor })
    {
        e->onReturnKey = [this] { applyArtNet(); };
        e->onFocusLost = [this] { applyArtNet(); };
        addAndMakeVisible (*e);
    }

    artStatus.setFont (theme::font (13.0f));
    artNetHeader.onClick = [this] { setArtNetExpanded (! artNetExpanded); };
    addAndMakeVisible (artNetHeader);

    for (auto* c : std::initializer_list<juce::Component*> { &artEnabled, &artLayoutBox, &artStatus })
        addAndMakeVisible (*c);

    // Devices
    noDevicesLabel.setFont (theme::font (13.0f));
    noDevicesLabel.setColour (juce::Label::textColourId, theme::textMuted);
    addAndMakeVisible (noDevicesLabel);

    // Defaults
    saveDefaultsButton.setTooltip ("Use the current settings for every new instance of the plugin");
    loadDefaultsButton.setTooltip ("Replace the current settings with the saved defaults (or factory settings)");
    saveDefaultsButton.onClick = [this]
    {
        if (onSaveDefaults)
        {
            const bool ok = onSaveDefaults();
            showDefaultsMessage (ok ? "Saved." : "Could not save the file.", ! ok);
        }
    };
    loadDefaultsButton.onClick = [this]
    {
        if (onLoadDefaults)
        {
            onLoadDefaults();
            refreshFromWorker();
            showDefaultsMessage ("Loaded.", false);
        }
    };
    addAndMakeVisible (saveDefaultsButton);
    addAndMakeVisible (loadDefaultsButton);
    addAndMakeVisible (defaultsMessage);

    about.setFont (theme::font (12.0f));
    about.setColour (juce::Label::textColourId, theme::textMuted);
    addAndMakeVisible (about);

    setArtNetExpanded (false);   // collapsed by default
    startTimerHz (2);
    timerCallback();
}

SettingsPage::~SettingsPage()
{
    setLookAndFeel (nullptr);
}

void SettingsPage::paint (juce::Graphics& g)
{
    for (auto card : cards)
    {
        auto b = card.toFloat();
        g.setColour (juce::Colour (0x14000000));
        g.fillRoundedRectangle (b.translated (0.0f, 2.5f).expanded (0.5f), 16.0f);
        g.setColour (theme::card);
        g.fillRoundedRectangle (b, 16.0f);
        g.setColour (theme::cardEdge);
        g.drawRoundedRectangle (b.reduced (0.5f), 16.0f, 1.0f);
    }

    for (auto p : headerDots)
    {
        const auto dot = juce::Rectangle<float> (8.0f, 8.0f).withCentre (p.toFloat());
        theme::drawGlow (g, dot, 4.0f, theme::accent, 1.0f, 6.0f);
        g.setColour (theme::accent);
        g.fillEllipse (dot);
    }
}

int SettingsPage::doLayout (int width, bool apply)
{
    cards.clear();
    headerDots.clear();

    const int cardX = outerPad;
    const int cardW = juce::jmax (120, width - outerPad * 2);
    const int ix = cardX + cardPad;
    const int iw = cardW - cardPad * 2;
    int y = outerPad;
    int cardTop = 0;

    auto place = [&] (juce::Component& c, juce::Rectangle<int> r) { if (apply) c.setBounds (r); };
    auto placeGlow = [&] (GlowButton& b, juce::Rectangle<int> r) { if (apply) b.setBounds (GlowButton::withGlowMargin (r)); };
    auto hide = [&] (juce::Component& c, bool hidden) { if (apply) c.setVisible (! hidden); };

    auto beginCard = [&] { cardTop = y; y += cardPad; };
    auto endCard = [&]
    {
        y += cardPad;
        cards.add ({ cardX, cardTop, cardW, y - cardTop });
        y += cardGap;
    };
    auto header = [&] (juce::Label& l)
    {
        headerDots.add ({ ix + 4, y + headerH / 2 });
        place (l, { ix + 16, y, iw - 16, headerH });
        y += headerH + rowGap;
    };

    // Status
    beginCard();
    header (statusHeader);
    for (auto& l : statusLines)
    {
        place (l, { ix, y, iw, 20 });
        y += 22;
    }
    y -= 2;
    endCard();

    // Colours
    beginCard();
    header (colourHeader);
    for (auto* row : { &recordingRow, &playingRow, &stoppedRow })
    {
        place (*row, { ix, y, iw, rowH + 6 });
        y += rowH + 6 + 2;
    }
    y -= 2;
    endCard();

    // OpenRGB
    beginCard();
    header (serverHeader);
    place (openRgbToggle, { ix, y, iw, rowH });
    y += rowH + rowGap;
    {
        const int labelW = 44;
        place (hostLabel, { ix, y, labelW, rowH });
        place (hostEditor, { ix + labelW, y, iw - labelW, rowH });
        y += rowH + rowGap;
        place (portLabel, { ix, y, labelW, rowH });
        place (portEditor, { ix + labelW, y, 80, rowH });
        placeGlow (applyButton, { ix + labelW + 80 + 12, y, 92, rowH });
        y += rowH;
    }
    endCard();

    // Art-Net (collapsible)
    beginCard();
    placeGlow (artNetHeader, { ix, y, juce::jmin (iw, 210), headerH + 4 });
    y += headerH + 4;

    for (auto* c : std::initializer_list<juce::Component*> { &artEnabled, &artStatus, &artIpLabel, &artIpEditor, &artUniverseLabel,
                                                             &artUniverseEditor, &artStartLabel, &artStartEditor, &artFixturesLabel,
                                                             &artFixturesEditor, &artLayoutLabel, &artLayoutBox })
        hide (*c, ! artNetExpanded);

    if (artNetExpanded)
    {
        y += rowGap;
        place (artEnabled, { ix, y, iw / 2 + 20, rowH });
        place (artStatus, { ix + iw / 2 + 24, y, iw / 2 - 24, rowH });
        y += rowH + rowGap;

        const int labelW = 62;
        place (artIpLabel, { ix, y, labelW, rowH });
        place (artIpEditor, { ix + labelW, y, iw - labelW, rowH });
        y += rowH + rowGap;

        const int halfW = (iw - 12) / 2;
        place (artUniverseLabel, { ix, y, labelW, rowH });
        place (artUniverseEditor, { ix + labelW, y, juce::jmax (40, halfW - labelW), rowH });
        place (artStartLabel, { ix + halfW + 12, y, labelW, rowH });
        place (artStartEditor, { ix + halfW + 12 + labelW, y, juce::jmax (40, halfW - labelW), rowH });
        y += rowH + rowGap;

        place (artFixturesLabel, { ix, y, labelW, rowH });
        place (artFixturesEditor, { ix + labelW, y, juce::jmax (40, halfW - labelW), rowH });
        place (artLayoutLabel, { ix + halfW + 12, y, labelW, rowH });
        place (artLayoutBox, { ix + halfW + 12 + labelW, y, juce::jmax (60, halfW - labelW), rowH });
        y += rowH;
    }
    endCard();

    // Devices
    beginCard();
    header (deviceHeader);
    const bool noDevices = deviceToggles.isEmpty();
    hide (noDevicesLabel, ! noDevices);
    if (noDevices)
    {
        place (noDevicesLabel, { ix, y, iw, 38 });
        y += 38;
    }
    else
    {
        for (auto* toggle : deviceToggles)
        {
            place (*toggle, { ix, y, iw, rowH });
            y += rowH + 2;
        }
        y -= 2;
    }
    endCard();

    // Defaults + about
    beginCard();
    header (defaultsHeader);
    placeGlow (saveDefaultsButton, { ix, y, 132, rowH });
    placeGlow (loadDefaultsButton, { ix + 132 + 14, y, 120, rowH });
    y += rowH + 4;
    place (defaultsMessage, { ix, y, iw, 20 });
    y += 20 + 4;
    place (about, { ix, y, iw, 18 });
    y += 18;
    endCard();

    return y - cardGap + outerPad;
}

void SettingsPage::resized()
{
    doLayout (getWidth(), true);
}

int SettingsPage::getHeightForWidth (int width)
{
    return doLayout (width, false);
}

void SettingsPage::setStatusLines (const juce::StringArray& lines, const juce::Array<juce::Colour>& colours)
{
    for (int i = 0; i < 3 && i < lines.size(); ++i)
    {
        statusLines[i].setText (lines[i], juce::dontSendNotification);
        statusLines[i].setColour (juce::Label::textColourId, colours[i]);
    }
}

void SettingsPage::showDefaultsMessage (const juce::String& text, bool error)
{
    defaultsMessage.setText (text, juce::dontSendNotification);
    defaultsMessage.setColour (juce::Label::textColourId, error ? theme::error : theme::ok);
    defaultsMessageTicks = 6;   // timer runs at 2 Hz: hide after ~3 s
}

void SettingsPage::refreshFromWorker()
{
    recordingRow.refresh();
    playingRow.refresh();
    stoppedRow.refresh();

    openRgbToggle.setToggleState (lights.isOpenRgbEnabled(), juce::dontSendNotification);
    hostEditor.setText (lights.getHost(), false);
    portEditor.setText (juce::String (lights.getPort()), false);

    const auto art = lights.getArtNetConfig();
    artEnabled.setToggleState (art.enabled, juce::dontSendNotification);
    artIpEditor.setText (art.ip, false);
    artUniverseEditor.setText (juce::String (art.universe), false);
    artStartEditor.setText (juce::String (art.startChannel), false);
    artFixturesEditor.setText (juce::String (art.fixtures), false);
    artLayoutBox.setSelectedId ((int) art.layout + 1, juce::dontSendNotification);

    rebuildDeviceList (lights.getDeviceNames());   // re-applies the enabled/disabled checkboxes
}

void SettingsPage::applyServer()
{
    lights.setServer (hostEditor.getText(), portEditor.getText().getIntValue());
    hostEditor.setText (lights.getHost(), false);
    portEditor.setText (juce::String (lights.getPort()), false);
}

void SettingsPage::setArtNetExpanded (bool expanded)
{
    artNetExpanded = expanded;
    artNetHeader.setButtonText (juce::String (expanded ? "-  " : "+  ") + "Art-Net (DMX)");
    resized();

    if (onContentSizeChanged)
        onContentSizeChanged();
}

void SettingsPage::applyArtNet()
{
    ArtNetConfig cfg;
    cfg.enabled      = artEnabled.getToggleState();
    cfg.ip           = artIpEditor.getText().trim();
    cfg.universe     = juce::jlimit (0, 32767, artUniverseEditor.getText().getIntValue());
    cfg.startChannel = juce::jlimit (1, 512, artStartEditor.getText().getIntValue());
    cfg.fixtures     = juce::jlimit (1, 512, artFixturesEditor.getText().getIntValue());
    cfg.layout       = (DmxLayout) juce::jlimit (0, 3, artLayoutBox.getSelectedId() - 1);

    lights.setArtNetConfig (cfg);

    // show the values that were actually accepted
    artUniverseEditor.setText (juce::String (cfg.universe), false);
    artStartEditor.setText (juce::String (cfg.startChannel), false);
    artFixturesEditor.setText (juce::String (cfg.fixtures), false);
}

void SettingsPage::refreshArtNetStatus()
{
    const auto cfg = lights.getArtNetConfig();
    juce::String text;
    juce::Colour colour = theme::textMuted;

    if (cfg.enabled)
    {
        if (lights.isArtNetOk())             { text = "sending";                colour = theme::ok; }
        else if (lights.hasArtNetFailed())   { text = "send failed (check IP)"; colour = theme::error; }
    }

    artStatus.setText (text, juce::dontSendNotification);
    artStatus.setColour (juce::Label::textColourId, colour);
}

void SettingsPage::rebuildDeviceList (const juce::StringArray& names)
{
    shownDevices = names;
    deviceToggles.clear();

    for (const auto& name : names)
    {
        auto* toggle = deviceToggles.add (new juce::ToggleButton (name));
        toggle->setToggleState (lights.isDeviceEnabled (name), juce::dontSendNotification);
        toggle->onClick = [this, toggle, name] { lights.setDeviceEnabled (name, toggle->getToggleState()); };
        addAndMakeVisible (toggle);
    }

    resized();

    if (onContentSizeChanged)
        onContentSizeChanged();
}

void SettingsPage::timerCallback()
{
    if (defaultsMessageTicks > 0 && --defaultsMessageTicks == 0)
        defaultsMessage.setText ({}, juce::dontSendNotification);

    refreshArtNetStatus();

    const auto names = lights.getDeviceNames();

    if (names != shownDevices)
        rebuildDeviceList (names);
}
