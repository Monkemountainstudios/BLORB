# BLORB

**Binaural Location & Orbital Rotation Box** is a JUCE spatial-motion effect for
headphones and stereo playback. It can act as a practical orbiting autopanner,
a listener-centred rotary-speaker effect, or an intentionally unruly spatial
sound-design machine.

This is an early beta intended for trusted testing.

## Highlights

- Continuous front, rear, left and right orbital motion with a sample-accurate display.
- Width and Depth controls ranging from conventional stereo motion to exaggerated distance.
- Head-shadow filtering, interaural delay, front reflections and rear spectral shaping.
- Doppler shadow modes: Off, Half and Full.
- Free-running speed from 0.02 Hz through an extreme 64 Hz region.
- DAW-tempo divisions from whole notes to 1/32-note triplets.
- **Swirl** continuously moves from 0.5x to 2x speed. Its modulation rate follows
  one tenth of the instantaneous orbit speed.
- Probability events: half speed, double speed, speed run, reversal, or deliberately nothing.
- Input, sine, saw, noise and loaded-WAV test sources in the hidden calibration panel.
- Amber/brass interface with a real-time phosphor trail and listener-calibration controls.

## Beta installation

### Windows

Copy `BLORB.vst3` to:

```text
C:\Program Files\Common Files\VST3
```

The standalone `BLORB.exe` can run from any folder. The beta is not code-signed,
so Windows may show an unknown-publisher warning.

### macOS

Copy `BLORB.vst3` to:

```text
~/Library/Audio/Plug-Ins/VST3
```

Move `BLORB.app` to Applications if the standalone is wanted. The beta is not
signed or notarized; testers may need to Control-click the app or plug-in and
choose **Open** the first time. Some hosts may require a plug-in rescan.

## Build locally

BLORB requires CMake 3.22 or newer. By default CMake fetches the pinned JUCE
version automatically. A local JUCE checkout can be supplied with
`-DORBIT_JUCE_SOURCE_DIR=/path/to/JUCE`.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target OrbitDspSmoke Orbit_Standalone Orbit_VST3
```

Run the smoke test after building:

```sh
./build/OrbitDspSmoke_artefacts/Release/OrbitDspSmoke
```

On Windows the executable ends in `.exe`.

## Automated builds and releases

GitHub Actions builds and tests Windows x64 and universal macOS binaries on every
push to `main`. Pushing a version tag such as `v0.1.0-beta.1` creates a prerelease
containing both platform packages.

## Current calibration defaults

- Rear Blend: 75%
- Spectral Arc: 110 degrees
- Front Space: 50%
- Interaural delay: enabled
- Dual-lobe front model: enabled

Click the illuminated **O** in BLORB to open the test/calibration panel. Its
Factory Reset button restores these spatial defaults without changing the main
musical controls.
