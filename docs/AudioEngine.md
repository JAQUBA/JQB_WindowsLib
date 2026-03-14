# AudioEngine

Audio I/O engine for Windows — manages `waveOut` (playback) and `waveIn` (recording) with multi-threaded double-buffering and thread-safe snapshot access for UI rendering.

## Include

```cpp
#include <IO/Audio/AudioEngine.h>
```

> **Requires:** `winmm` library. Add `build_flags = -lwinmm` to `platformio.ini`.

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `AUDIO_SAMPLE_RATE` | 44100 | Sample rate (Hz) |
| `AUDIO_CHANNELS` | 1 | Mono |
| `AUDIO_BITS` | 16 | 16-bit PCM |
| `AUDIO_BUFFER_SAMPLES` | 2048 | Samples per buffer |
| `AUDIO_NUM_BUFFERS` | 2 | Double-buffering |
| `AUDIO_DOWNSAMPLE` | 8 | Default downsample factor |
| `AUDIO_SNAPSHOT_SIZE` | 256 | Snapshot size (2048/8) |

## API

### Constructor / Destructor

```cpp
AudioEngine engine;     // Initializes format, critical sections
// Destructor stops all streams and cleans up
```

### Device Enumeration

```cpp
std::vector<std::string> outputs = engine.enumOutputDevices();
std::vector<std::string> inputs  = engine.enumInputDevices();
// Returns UTF-8 device names
```

### Output (Playback)

```cpp
engine.startOutput(0);          // Start playback on device index 0
engine.isOutputRunning();       // Check state
engine.stopOutput();            // Stop and cleanup
```

### Input (Recording)

```cpp
engine.startInput(0);           // Start recording on device index 0
engine.isInputRunning();        // Check state
engine.stopInput();             // Stop and cleanup
```

### Waveform Generator

```cpp
WaveGen& gen = engine.getWaveGen();
gen.setWaveform(WAVE_SINE);
gen.setFrequency(1000.0);
gen.setAmplitude(0.8);
```

The built-in `WaveGen` automatically fills output buffers. Modify its parameters at any time — changes take effect on the next buffer fill.

### Snapshot Access (Thread-Safe)

```cpp
double buffer[256];
int count = 0;

if (engine.getOutputSnapshot(buffer, 256, count)) {
    // New output data available — count samples in buffer
    for (int i = 0; i < count; i++)
        chart->addDataPoint(buffer[i], L"");
}

if (engine.getInputSnapshot(buffer, 256, count)) {
    // New input data available
}
```

Snapshots contain downsampled audio data (normalized to -1.0 ... +1.0). Each call returns `true` only once per new buffer — subsequent calls return `false` until new data arrives.

### Configuration

```cpp
engine.setDownsampleFactor(4);      // Change downsample (default: 8)
int size = engine.getSnapshotSize(); // AUDIO_BUFFER_SAMPLES / factor
```

## Architecture

```
┌─────────────────────────────────────────┐
│  Main Thread (UI)                       │
│  ┌───────────────────┐                  │
│  │ loop() polling    │                  │
│  │ getOutputSnapshot │◄──── CRITICAL    │
│  │ getInputSnapshot  │      SECTION     │
│  └───────────────────┘                  │
└─────────────────────────────────────────┘
         ▲                        ▲
         │                        │
┌────────┴────────┐    ┌─────────┴────────┐
│ Output Thread   │    │ Input Thread     │
│ WaitForEvent    │    │ WaitForEvent     │
│ fillBuffer      │    │ processBuffer    │
│ downsample→snap │    │ downsample→snap  │
│ waveOutWrite    │    │ waveInAddBuffer  │
└─────────────────┘    └──────────────────┘
```

- **CALLBACK_EVENT** — waveOut/waveIn signal events when buffers complete
- **CreateThread** — dedicated threads wait on events and refill/process buffers
- **CRITICAL_SECTION** — protects snapshot data for thread-safe UI access

## Usage Example

```cpp
#include <IO/Audio/AudioEngine.h>

AudioEngine engine;

// Enumerate and select devices
auto outputs = engine.enumOutputDevices();
auto inputs  = engine.enumInputDevices();

// Configure generator
engine.getWaveGen().setWaveform(WAVE_SINE);
engine.getWaveGen().setFrequency(440.0);
engine.getWaveGen().setAmplitude(0.5);

// Start I/O
engine.startOutput(0);
engine.startInput(0);

// In loop():
double buf[256];
int count;
if (engine.getOutputSnapshot(buf, 256, count)) {
    for (int i = 0; i < count; i++)
        outputChart->addDataPoint(buf[i], L"");
}
if (engine.getInputSnapshot(buf, 256, count)) {
    for (int i = 0; i < count; i++)
        inputChart->addDataPoint(buf[i], L"");
}
```

## Notes

- Always call `stopOutput()` / `stopInput()` before destroying the engine
- Device indices match the order from `enumOutputDevices()` / `enumInputDevices()`
- Output thread automatically calls `WaveGen::fillBuffer()` for continuous playback
- Snapshot normalization: `sample / 32768.0` → range [-1.0, +1.0]
