#pragma once
#include <JuceHeader.h>
#include "LightWorker.h"
#include "Widgets.h"
#include "LightLookAndFeel.h"

/** Pick the colour of one transport state, or preview it for a moment. */
class ColourRow : public juce::Component
{
public:
    ColourRow (const juce::String& labelText, LightWorker& worker, TransportState state);

    void resized() override;
    void refresh();

private:
    void openSelector();

    LightWorker& lights;
    TransportState transportState;
    juce::Label label;
    GlowButton swatch { {}, GlowButton::Style::Swatch };
    GlowButton previewButton { "Preview" };
};

class SettingsPage : public juce::Component,
                     private juce::Timer
{
public:
    explicit SettingsPage (LightWorker& worker);
    ~SettingsPage() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    /** Height this page needs when it is `width` pixels wide. */
    int getHeightForWidth (int width);

    /** Re-reads every control from the worker (after the settings were changed from outside). */
    void refreshFromWorker();

    void setStatusLines (const juce::StringArray& lines, const juce::Array<juce::Colour>& colours);

    /** Called when the page needs a different height (section expanded, device list changed). */
    std::function<void()> onContentSizeChanged;

    /** Set by the editor, which owns the access to the processor. */
    std::function<bool()> onSaveDefaults;
    std::function<void()> onLoadDefaults;

private:
    void timerCallback() override;
    void showDefaultsMessage (const juce::String& text, bool error);
    void applyServer();
    void applyArtNet();
    void refreshArtNetStatus();
    void setArtNetExpanded (bool expanded);
    void rebuildDeviceList (const juce::StringArray& names);
    int doLayout (int width, bool apply);

    juce::SharedResourcePointer<LightLookAndFeel> laf;
    LightWorker& lights;

    juce::Label statusHeader { {}, "Status" };
    juce::Label statusLines[3];

    juce::Label colourHeader { {}, "Colours" };
    ColourRow recordingRow { "Recording", lights, TransportState::Recording };
    ColourRow playingRow   { "Play",      lights, TransportState::Playing };
    ColourRow stoppedRow   { "Stop",      lights, TransportState::Stopped };

    juce::Label serverHeader { {}, "OpenRGB lights" };
    juce::ToggleButton openRgbToggle { "Use OpenRGB lights" };
    juce::Label hostLabel { {}, "Host" };
    juce::Label portLabel { {}, "Port" };
    juce::TextEditor hostEditor;
    juce::TextEditor portEditor;
    GlowButton applyButton { "Apply", GlowButton::Style::Primary };

    GlowButton artNetHeader { {} };           // click to expand/collapse the Art-Net section
    bool artNetExpanded = false;
    juce::ToggleButton artEnabled { "Send Art-Net (DMX)" };
    juce::Label artIpLabel { {}, "Node IP" };
    juce::TextEditor artIpEditor;
    juce::Label artUniverseLabel { {}, "Universe" };
    juce::TextEditor artUniverseEditor;
    juce::Label artStartLabel { {}, "Start ch." };
    juce::TextEditor artStartEditor;
    juce::Label artFixturesLabel { {}, "Fixtures" };
    juce::TextEditor artFixturesEditor;
    juce::Label artLayoutLabel { {}, "Layout" };
    juce::ComboBox artLayoutBox;
    juce::Label artStatus;

    juce::Label deviceHeader { {}, "Devices" };
    juce::Label noDevicesLabel { {}, "No devices to choose from. The on-screen light always follows the transport." };
    juce::OwnedArray<juce::ToggleButton> deviceToggles;
    juce::StringArray shownDevices;

    juce::Label defaultsHeader { {}, "Defaults" };
    GlowButton saveDefaultsButton { "Save as default", GlowButton::Style::Primary };
    GlowButton loadDefaultsButton { "Load defaults" };
    juce::Label defaultsMessage;
    int defaultsMessageTicks = 0;

    juce::Label about { {}, "LightControl - made by Michal Gochna" };

    juce::Array<juce::Rectangle<int>> cards;      // filled by doLayout()
    juce::Array<juce::Point<int>> headerDots;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsPage)
};
