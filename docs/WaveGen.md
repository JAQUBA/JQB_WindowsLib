# WaveGen

Audio waveform generator — produces PCM 16-bit mono samples for sine, sawtooth, triangle, square, and white noise waveforms.

## Include

```cpp
#include <IO/Audio/WaveGen.h>
```

## Waveform Types

| Enum | Type |
|------|------|
| `WAVE_SINE` | Sine wave |
| `WAVE_SAWTOOTH` | Sawtooth wave |
| `WAVE_TRIANGLE` | Triangle wave |
| `WAVE_SQUARE` | Square wave |
| `WAVE_WHITE_NOISE` | White noise |
| `WAVE_COUNT` | Total count (for iteration) |

## API

### Constructor

```cpp
WaveGen gen;    // Default: sine, 440 Hz, 50% amplitude
```

### Configuration

```cpp
gen.setWaveform(WAVE_SINE);     // Set waveform type
gen.setFrequency(1000.0);       // Set frequency in Hz
gen.setAmplitude(0.8);          // Set amplitude (0.0 – 1.0)
gen.resetPhase();               // Reset phase accumulator
```

### Getters

```cpp
WaveformType type = gen.getWaveform();
double freq       = gen.getFrequency();
double amp        = gen.getAmplitude();
```

### Buffer Generation

```cpp
int16_t buffer[2048];
gen.fillBuffer(buffer, 2048, 44100);    // numSamples, sampleRate
```

`fillBuffer()` maintains a continuous phase accumulator between calls, producing glitch-free audio across consecutive buffer fills.

## Usage Example

```cpp
#include <IO/Audio/WaveGen.h>

WaveGen gen;
gen.setWaveform(WAVE_TRIANGLE);
gen.setFrequency(880.0);
gen.setAmplitude(0.6);

int16_t buffer[1024];
gen.fillBuffer(buffer, 1024, 44100);
// buffer now contains 1024 PCM samples of 880 Hz triangle wave
```

## Notes

- Amplitude is clamped to [-32768, 32767] (int16_t range)
- White noise uses `rand()` — seeded once with `time(NULL)` on first construction
- Phase accumulator wraps at 1.0, ensuring continuous signal
