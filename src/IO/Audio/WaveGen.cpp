#include "WaveGen.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

WaveGen::WaveGen()
    : m_type(WAVE_SINE)
    , m_frequency(440.0)
    , m_amplitude(0.5)
    , m_phase(0.0)
{
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }
}

void WaveGen::fillBuffer(int16_t* buffer, uint32_t numSamples, uint32_t sampleRate) {
    double phaseInc = m_frequency / (double)sampleRate;

    for (uint32_t i = 0; i < numSamples; i++) {
        double sample = 0.0;

        switch (m_type) {
            case WAVE_SINE:
                sample = sin(2.0 * M_PI * m_phase);
                break;

            case WAVE_SAWTOOTH:
                sample = 2.0 * m_phase - 1.0;
                break;

            case WAVE_TRIANGLE:
                sample = (m_phase < 0.5)
                    ? (4.0 * m_phase - 1.0)
                    : (3.0 - 4.0 * m_phase);
                break;

            case WAVE_SQUARE:
                sample = (m_phase < 0.5) ? 1.0 : -1.0;
                break;

            case WAVE_WHITE_NOISE:
                sample = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
                break;

            default:
                sample = 0.0;
                break;
        }

        double val = sample * m_amplitude * 32767.0;
        if (val > 32767.0)  val = 32767.0;
        if (val < -32768.0) val = -32768.0;
        buffer[i] = (int16_t)val;

        m_phase += phaseInc;
        if (m_phase >= 1.0) m_phase -= 1.0;
    }
}
