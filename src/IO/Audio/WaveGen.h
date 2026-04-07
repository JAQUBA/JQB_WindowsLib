// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

#ifndef WAVEGEN_H
#define WAVEGEN_H

#include <cstdint>

enum WaveformType {
    WAVE_SINE = 0,
    WAVE_SAWTOOTH,
    WAVE_TRIANGLE,
    WAVE_SQUARE,
    WAVE_WHITE_NOISE,
    WAVE_COUNT
};

class WaveGen {
public:
    WaveGen();

    void setWaveform(WaveformType type) { m_type = type; }
    void setFrequency(double hz)        { m_frequency = hz; }
    void setAmplitude(double amp)       { m_amplitude = amp; }

    WaveformType getWaveform()  const { return m_type; }
    double       getFrequency() const { return m_frequency; }
    double       getAmplitude() const { return m_amplitude; }

    // Fill a PCM 16-bit mono buffer with the selected waveform
    void fillBuffer(int16_t* buffer, uint32_t numSamples, uint32_t sampleRate);

    // Reset phase accumulator to zero
    void resetPhase() { m_phase = 0.0; }

private:
    WaveformType m_type;
    double m_frequency;
    double m_amplitude;
    double m_phase;
};

#endif // WAVEGEN_H
