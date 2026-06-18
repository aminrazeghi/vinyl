# Groovr

A frameless, always-on-top desktop music widget for Linux that visualizes the currently playing track as a spinning vinyl record. Controls Spotify (and any MPRIS-compatible player) via D-Bus.

![Groovr widget showing a spinning record with album art]

## Features

- Spinning vinyl record with procedural grooves and a tonearm that tracks playback progress
- Album art displayed as a circular label on the record
- Track title, artist, and album metadata
- Play / Pause / Previous / Next controls
- Seek bar
- MPRIS player selector — works with Spotify, VLC, Rhythmbox, and any other MPRIS2 player
- Always-on-top pin
- Adjustable window opacity (persisted across sessions)
- **Ad muter** — automatically silences Spotify ads via PulseAudio/PipeWire without pausing playback

## Requirements

| Dependency | Notes |
|---|---|
| Qt 6.4+ | Core, Gui, Widgets, Qml, Quick, DBus |
| CMake 3.16+ | |
| PulseAudio or PipeWire | For the ad muter (`pactl` must be in PATH) |
| An MPRIS2-compatible player | Spotify, VLC, etc. |

On Ubuntu/Debian:
```bash
sudo apt install qt6-base-dev qt6-declarative-dev libqt6dbus6 cmake build-essential
```

On Arch:
```bash
sudo pacman -S qt6-base qt6-declarative cmake
```

## Build

```bash
git clone <repo-url>
cd groovr
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Run

```bash
./build/groovr_app
```

The app runs under XWayland (`QT_QPA_PLATFORM=xcb`) so that window hints such as always-on-top and opacity work correctly on GNOME Wayland.

## Usage

| Control | Action |
|---|---|
| Drag the window | Move it anywhere on screen |
| 📌 pin button | Toggle always-on-top |
| 🔇 button | Toggle automatic Spotify ad muting |
| ⚙ button | Open settings (window opacity slider) |
| ✕ button | Hide to system tray |
| Tray icon click | Show / hide window |
| Tray → Quit | Exit |

Settings are persisted in `~/.config/GroovrApp/Groovr.conf`.

## Project Structure

```
src/
  main.cpp                    — App entry point, tray icon, wires modules together
  admuter/                    — Spotify ad detection and PulseAudio muting
  mediacontroller/            — Aggregates MPRIS state for QML
  mpris/                      — D-Bus MPRIS2 discovery and player interface
  settings/                   — QSettings wrapper exposed to QML
  qml/
    main.qml                  — Root window, header, footer layout
    GroovrRecord.qml          — Spinning disc + cover art + tonearm
    ControlBar.qml            — Play/Pause/Prev/Next buttons
    SeekBar.qml               — Seek / progress bar
    PlayerSelector.qml        — MPRIS player picker dropdown
    GlassPanel.qml            — Glassmorphism background panel
assets/
  default_cover.png           — Fallback cover art
```

## How the Ad Muter Works

1. Reads the current track's `mpris:trackid` from Spotify's MPRIS2 interface.
2. Classifies the track as an ad if the ID contains `:ad` / `/ad/`, is not a normal track/episode/local file, or the title is "Advertisement" or "Spotify".
3. On ad detection, finds Spotify's PulseAudio/PipeWire sink input via `pactl list sink-inputs` and mutes it with `pactl set-sink-input-mute <id> 1`.
4. Unmutes automatically when normal playback resumes.

The toggle is persistent — if you leave it enabled, it activates automatically on the next session.
