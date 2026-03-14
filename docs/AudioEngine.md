# AudioEngine

Audio I/O engine for Windows — manages `waveOut` (playback) and `waveIn` (recording) with multi-threaded triple-buffering, automatic sample rate negotiation, and thread-safe snapshot access for UI rendering.

## Include

```cpp
#include <IO/Audio/AudioEngine.h>
```

> **Requires:** `winmm` library. Linked automatically by `compile_resources.py`.

## Constants

| Constant | Default Value | Description |
|----------|-------|-------------|
| `AUDIO_SAMPLE_RATE` | 48000 | Default/fallback sample rate (Hz) |
| `AUDIO_CHANNELS` | 1 | Mono |
| `AUDIO_BITS` | 16 | 16-bit PCM |
| `AUDIO_BUFFER_SAMPLES` | 4096 | Samples per buffer |
| `AUDIO_NUM_BUFFERS` | 3 | Triple-buffering |
| `AUDIO_DOWNSAMPLE` | 8 | Default downsample factor |
| `AUDIO_SNAPSHOT_SIZE` | `AUDIO_BUFFER_SAMPLES` | Snapshot size (4096 at downsample=1) |

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

### Sample Rate

```cpp
engine.setSampleRate(192000);           // Set preferred rate (before startOutput/startInput)
uint32_t rate = engine.getActualSampleRate(); // Query negotiated rate
```

`startOutput()` and `startInput()` auto-negotiate the sample rate: they try the preferred rate first, then fall back through 192000 → 96000 → 48000 → 44100 Hz until the device accepts. Use `getActualSampleRate()` after starting to get the actual rate used.

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
- **Triple-buffering** — 3 buffers per direction to prevent audio stutter
- **Auto-negotiation** — `startOutput()` / `startInput()` try multiple sample rates (192k → 96k → 48k → 44.1k)

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

// Request high sample rate (auto-negotiates down if unsupported)
engine.setSampleRate(192000);

// Start I/O
engine.startOutput(0);
engine.startInput(0);

// Check actual rate
uint32_t rate = engine.getActualSampleRate(); // e.g. 48000

// In loop():
double buf[AUDIO_SNAPSHOT_SIZE];
int count;
if (engine.getOutputSnapshot(buf, AUDIO_SNAPSHOT_SIZE, count)) {
    double durationMs = (double)AUDIO_BUFFER_SAMPLES / rate * 1000.0;
    outputChart->addDataPoints(buf, count, durationMs);
}
if (engine.getInputSnapshot(buf, AUDIO_SNAPSHOT_SIZE, count)) {
    double durationMs = (double)AUDIO_BUFFER_SAMPLES / rate * 1000.0;
    inputChart->addDataPoints(buf, count, durationMs);
}
```

## Notes

- Always call `stopOutput()` / `stopInput()` before destroying the engine
- Device indices match the order from `enumOutputDevices()` / `enumInputDevices()`
- Output thread automatically calls `WaveGen::fillBuffer()` for continuous playback
- Snapshot normalization: `sample / 32768.0` → range [-1.0, +1.0]
