# Xbox Macro Pad

A VST3 plugin that reads an Xbox 360 controller and turns each joystick into an XY pad controlling two macros.

- Left stick: Macro 1 (X), Macro 2 (Y)
- Right stick: Macro 1 (X), Macro 2 (Y)
- Four automatable parameters, also sent as MIDI CC 20-23 (channel 1)
- Optional "Latch on release" (otherwise macros spring back to center)
- Audio passes through untouched

Windows only for now (XInput, controller #1).

## Build

Requires CMake 3.22+ and Visual Studio 2022.

    cmake -B build
    cmake --build build --config Release

The VST3 is copied to your VST3 folder after a local build. Every push also builds on GitHub Actions; download the plugin from the run's Artifacts.

## License

Add a LICENSE file before publishing. Note that JUCE is dual-licensed (AGPLv3 / commercial), so an open-source plugin built with it should be released under a compatible license such as AGPLv3 or GPLv3.
