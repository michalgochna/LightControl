#include "HelpContent.h"

namespace help
{
    using Type = Block::Type;

    namespace
    {
        Block heading (const char* t)               { return { Type::Heading, juce::String::fromUTF8 (t), {} }; }
        Block sub (const char* t)                   { return { Type::Subheading, juce::String::fromUTF8 (t), {} }; }
        Block para (const char* t)                  { return { Type::Paragraph, juce::String::fromUTF8 (t), {} }; }
        Block bullet (const char* t)                { return { Type::Bullet, juce::String::fromUTF8 (t), {} }; }
        Block code (const char* t)                  { return { Type::Code, juce::String::fromUTF8 (t), {} }; }
        Block note (const char* t)                  { return { Type::Note, juce::String::fromUTF8 (t), {} }; }
        Block link (const char* t, const char* u)   { return { Type::Link, juce::String::fromUTF8 (t), u }; }

        std::vector<Block> installation()
        {
            return {
                heading ("Installing LightControl"),
                para ("LightControl is a VST3 plugin. Put it in your VST3 folder, then let your DAW scan for new plugins."),

                sub ("Windows"),
                bullet ("Copy the whole LightControl.vst3 folder to:"),
                code ("C:\\Program Files\\Common Files\\VST3"),
                bullet ("Writing there needs administrator rights."),

                sub ("macOS"),
                bullet ("Copy LightControl.vst3 to ~/Library/Audio/Plug-Ins/VST3 (just for you) or /Library/Audio/Plug-Ins/VST3 (all users)."),
                bullet ("A file downloaded from the internet is blocked by macOS. Unblock it once in Terminal:"),
                code ("xattr -cr ~/Library/Audio/Plug-Ins/VST3/LightControl.vst3"),

                sub ("Linux"),
                bullet ("Copy LightControl.vst3 to ~/.vst3/ (just for you) or /usr/lib/vst3/ (all users)."),

                sub ("Make your DAW find it"),
                bullet ("Reaper: Options > Preferences > Plug-ins > VST. Make sure the folder above is in the list of paths, then click Re-scan (or Clear cache/re-scan)."),
                bullet ("Other DAWs: use their plugin manager to rescan VST3 plugins."),
                bullet ("Add LightControl to any track. It does not process audio, so it can go anywhere in the chain."),

                sub ("Optional: OpenRGB for keyboards, bulbs and other lights"),
                para ("To control lights through OpenRGB, install OpenRGB, a separate free program. LightControl does not include it."),
                link ("Download OpenRGB (openrgb.org)", "https://openrgb.org"),
                bullet ("Start OpenRGB, open its SDK Server tab and click Start Server. LightControl talks to this server."),
                bullet ("Linux: install the OpenRGB udev rules so it can reach USB devices without root."),
                link ("OpenRGB udev rules", "https://openrgb.org/udev"),
                bullet ("Windows: some devices may need OpenRGB to be started as administrator."),
                bullet ("Philips WiZ bulbs: in OpenRGB open Settings > Philips Wiz Devices and add the bulb's IP address."),

                sub ("Optional: DMX lights over Art-Net"),
                para ("Nothing to install. You need an Art-Net node (an Art-Net to DMX interface) on the same network, and its IP address."),
            };
        }

        std::vector<Block> configuration()
        {
            return {
                heading ("Configuring LightControl"),
                para ("Open the settings with the Settings button (top right). Every change takes effect immediately and is saved with your project."),

                sub ("Colours"),
                bullet ("There is one colour for Recording, one for Play and one for Stop."),
                bullet ("Click a colour to open the colour picker."),
                bullet ("Preview shows that state for 3 seconds on the lamp and on your lights, so you can see the colour without touching the transport."),

                sub ("OpenRGB lights"),
                bullet ("Use OpenRGB lights: switch it off if you have no lights to control (the lamp on the screen keeps working)."),
                bullet ("Host and Port tell LightControl where OpenRGB's SDK server is. The defaults, 127.0.0.1 and 6742, are right when OpenRGB runs on the same computer."),
                bullet ("Apply connects again, but only if you changed the host or port."),
                bullet ("In OpenRGB, start the server on the SDK Server tab (Start Server)."),
                bullet ("Devices: tick the devices that should follow the transport. Devices you leave unticked are never touched."),

                sub ("Art-Net (DMX)"),
                para ("Click the Art-Net header to open the section, then switch on Send Art-Net (DMX)."),
                bullet ("Node IP: the IPv4 address of your Art-Net node, for example 192.168.1.50. Host names are not accepted."),
                bullet ("Universe: Art-Net counts from 0. If your node's manual counts from 1, subtract one."),
                bullet ("Start ch.: the first DMX channel of the first light (1 to 512)."),
                bullet ("Fixtures: how many identical lights there are. They sit one after another from the start channel."),
                bullet ("Layout: RGB uses 3 channels. RGBW uses 4 and puts the part common to red, green and blue into the white channel. Dimmer + RGB and RGB + Dimmer use 4 channels and keep the dimmer at full."),
                bullet ("The frame is sent again every second, because many nodes switch off when it stops."),

                sub ("Defaults"),
                bullet ("The settings live in the project: open it again and everything is as you left it."),
                bullet ("A new plugin starts from your defaults. Save as default stores the current settings for that. Load defaults brings them back (or the factory settings, if you never saved any)."),
                para ("The defaults file is called defaults.xml and lives in:"),
                code ("Windows  %APPDATA%\\LightControl\nmacOS    ~/Library/Application Support/LightControl\nLinux    ~/.config/LightControl"),

                sub ("Reaper"),
                note ("Reaper normally stops calling a plugin when the transport stops. LightControl then treats half a second of silence as Stop, so it still works, but the lights react faster if Reaper keeps running the plugin while stopped. Look in Reaper's Preferences > Audio for the option that keeps FX running when the transport is stopped."),
            };
        }

        std::vector<Block> usage()
        {
            return {
                heading ("Using LightControl"),
                para ("LightControl watches your DAW's transport and sets the lights to a colour for each state:"),
                bullet ("Recording: the DAW is recording (red by default)."),
                bullet ("Play: the DAW is playing (green)."),
                bullet ("Stop: the transport is stopped or paused (amber). Pause counts as Stop for now."),

                sub ("The lamp"),
                para ("The big lamp always shows the current state, with or without real lights. It pulses while recording, breathes and turns its rays while playing, and glows quietly when stopped. A ripple of light runs out whenever the state changes. Below the lamp an icon in the state's colour shows it without any text: a circle for recording, a triangle for playing, two bars for stop / pause."),

                sub ("The status chips"),
                para ("Next to the buttons at the top, two chips show OpenRGB and Art-Net: green is working, amber means waiting or no devices, red is an error, grey is off. Hover over a chip for the details, which are also listed on the Settings page under Status."),

                sub ("No lights? Use it as a screen"),
                bullet ("Switch off Use OpenRGB lights if you have no lights. The lamp keeps working."),
                bullet ("Click Hide settings (or double-click the lamp) to show only the lamp."),
                bullet ("Drag the corner of the window to any size, for example to fill a second monitor, so the whole room can see the state."),

                sub ("When you close the project"),
                para ("The lights are switched off when the plugin is removed or the project closes."),

                sub ("If something doesn't work"),
                bullet ("The lamp says Ready, waiting for the transport: your DAW is not calling the plugin yet. Put it on a track. In Reaper, check Preferences > Audio for the option that keeps FX running while stopped."),
                bullet ("OpenRGB: not found: start OpenRGB and click Start Server on its SDK Server tab. Check Host and Port."),
                bullet ("OpenRGB: no devices: OpenRGB is running but sees no lights. On Linux install the udev rules, on Windows try running OpenRGB as administrator, and check the device is supported."),
                bullet ("Lights stay dark although the chip is green: check that the device is ticked under Devices."),
                bullet ("Art-Net: bad IP: type an IPv4 address, not a name."),
                bullet ("Art-Net says sending but nothing happens: check the universe number (counted from 0), the start channel, and that the lights' DMX address matches."),
                bullet ("The colours never change to Recording: some DAWs do not tell plugins that they are recording. Reaper does."),
            };
        }

        std::vector<Block> licences()
        {
            return {
                heading ("Licences"),
                para ("LightControl, made by Michal Gochna, is built with the open source software below. Their licences apply to those parts."),

                sub ("JUCE"),
                para ("The user interface and the plugin framework come from JUCE 9, copyright Raw Material Software Limited. JUCE is dual-licensed: under the GNU Affero General Public License v3 (AGPLv3) or under the commercial JUCE licence."),
                link ("JUCE 9 licence", "https://juce.com/legal/juce-9-licence/"),
                link ("GNU AGPLv3", "https://www.gnu.org/licenses/agpl-3.0.html"),

                sub ("OpenRGB C++ SDK (MIT)"),
                para ("The client that talks to OpenRGB is the OpenRGB C++ SDK by Youda008. It includes two small helper libraries by the same author, CppUtils-Essential and CppUtils-Network."),
                link ("OpenRGB C++ SDK on GitHub", "https://github.com/Youda008/OpenRGB-CppSDK"),
                code ("MIT License\n\nCopyright (c) 2021 Youda008\n\nPermission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\n\nThe above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\n\nTHE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE."),

                sub ("Libraries inside JUCE"),
                para ("JUCE contains the following third-party libraries. Each keeps its own licence, which is shipped with JUCE."),
                bullet ("FLAC, Ogg, Vorbis and Opus: BSD-style licences (Xiph.org Foundation and others)"),
                bullet ("zlib: zlib licence"),
                bullet ("libpng: libpng licence"),
                bullet ("IJG JPEG library: IJG licence"),
                bullet ("libwebp: BSD-style licence (Google)"),
                bullet ("HarfBuzz: \"Old MIT\" licence"),
                bullet ("SheenBidi: Apache License 2.0"),
                bullet ("lunasvg: MIT licence"),
                para ("On Linux, FreeType and fontconfig are used from the system."),

                sub ("OpenRGB"),
                para ("OpenRGB itself is a separate program under the GNU GPL v2. It is not part of LightControl and is not included with it: LightControl only talks to it over the network."),
                link ("OpenRGB project", "https://gitlab.com/CalcProgrammer1/OpenRGB"),

                sub ("Trademarks"),
                para ("VST is a registered trademark of Steinberg Media Technologies GmbH."),
            };
        }
    }

    const char* sectionTitle (Section s)
    {
        switch (s)
        {
            case Section::Installation:  return "Installation";
            case Section::Configuration: return "Configuration";
            case Section::Usage:         return "Usage";
            case Section::Licences:      return "Licences";
        }
        return "";
    }

    std::vector<Block> getBlocks (Section s)
    {
        switch (s)
        {
            case Section::Installation:  return installation();
            case Section::Configuration: return configuration();
            case Section::Usage:         return usage();
            case Section::Licences:      return licences();
        }
        return {};
    }
}
