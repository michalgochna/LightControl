# LightControl

A VST3 plugin that sets the colour of your lights to follow the DAW transport: one colour for **recording**,
one for **play** and one for **stop/pause**. It also shows a large on-screen lamp that does the same, so it is
useful even without any lights (hide the settings and stretch the window onto a second screen).

Lights are driven through:

- **OpenRGB** (keyboards, mice, motherboards, Philips WiZ bulbs, E1.31 devices, ...). Start OpenRGB with the
  *SDK Server* enabled; the plugin connects to `127.0.0.1:6742` by default.
- **Art-Net** (DMX over the network) sent directly to a node's IP address, no extra software needed.

Settings are saved with the project. *Save as default* stores them for new instances of the plugin
(`defaults.xml` in the user's application data folder).

## Building

Requires CMake 3.22+ and a C++17 compiler. JUCE 9 and the OpenRGB C++ SDK are git submodules.

```
git clone --recurse-submodules <repo-url>
cd LightControl
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The plugin is in `build/LightControl_artefacts/Release/VST3/`. Install it to:

| OS      | Folder                                  |
|---------|-----------------------------------------|
| Windows | `C:\Program Files\Common Files\VST3\`   |
| macOS   | `~/Library/Audio/Plug-Ins/VST3/`        |
| Linux   | `~/.vst3/`                              |

On Linux the JUCE build needs: `libasound2-dev libfreetype-dev libfontconfig1-dev libx11-dev libxcomposite-dev
libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev libxi-dev libcurl4-openssl-dev
mesa-common-dev libglu1-mesa-dev`.

For Reaper, enable *Run FX when stopped* on the track if you want the lights to react the moment the transport
stops (the plugin also assumes "stopped" when the host stops calling it).

## Licensing

JUCE is used under the AGPLv3 (or a commercial JUCE licence). The OpenRGB C++ SDK is MIT licensed.
OpenRGB itself is a separate program (GPL-2.0) and is not part of this project.
