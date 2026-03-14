#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <string>
#include <cstdint>
#include "WaveGen.h"

#define AUDIO_SAMPLE_RATE     44100
#define AUDIO_CHANNELS        1
#define AUDIO_BITS            16
#define AUDIO_BUFFER_SAMPLES  4096
#define AUDIO_NUM_BUFFERS     3
#define AUDIO_DOWNSAMPLE      8
#define AUDIO_SNAPSHOT_SIZE   AUDIO_BUFFER_SAMPLES

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    // Device enumeration
    std::vector<std::string> enumOutputDevices();
    std::vector<std::string> enumInputDevices();

    // Output (waveOut)
    bool startOutput(int deviceIndex);
    void stopOutput();
    bool isOutputRunning() const { return m_outputRunning; }

    // Input (waveIn)
    bool startInput(int deviceIndex);
    void stopInput();
    bool isInputRunning() const { return m_inputRunning; }

    // Waveform generator access
    WaveGen& getWaveGen() { return m_waveGen; }

    // Thread-safe snapshot access for UI/chart rendering
    // Returns true if new data was available (resets flag)
    bool getOutputSnapshot(double* buffer, int maxSamples, int& outCount);
    bool getInputSnapshot(double* buffer, int maxSamples, int& outCount);

    // Configuration
    void setDownsampleFactor(int factor) { m_downsample = factor; }
    int  getDownsampleFactor() const     { return m_downsample; }
    int  getSnapshotSize()     const     { return AUDIO_BUFFER_SAMPLES / m_downsample; }

private:
    WaveGen m_waveGen;
    WAVEFORMATEX m_wfx;
    int m_downsample;

    // --- Output ---
    HWAVEOUT m_hWaveOut;
    WAVEHDR  m_outHeaders[AUDIO_NUM_BUFFERS];
    int16_t* m_outBuffers[AUDIO_NUM_BUFFERS];
    volatile bool m_outputRunning;
    HANDLE   m_outputEvent;
    HANDLE   m_outputThread;

    // --- Input ---
    HWAVEIN  m_hWaveIn;
    WAVEHDR  m_inHeaders[AUDIO_NUM_BUFFERS];
    int16_t* m_inBuffers[AUDIO_NUM_BUFFERS];
    volatile bool m_inputRunning;
    HANDLE   m_inputEvent;
    HANDLE   m_inputThread;

    // --- Snapshot data (downsampled for charts) ---
    CRITICAL_SECTION m_csOutput;
    CRITICAL_SECTION m_csInput;
    double   m_outputSnapshot[AUDIO_SNAPSHOT_SIZE];
    double   m_inputSnapshot[AUDIO_SNAPSHOT_SIZE];
    int      m_outputSnapshotCount;
    int      m_inputSnapshotCount;
    bool     m_newOutputData;
    bool     m_newInputData;

    void initFormat();
    void refillOutputBuffer(int index);
    void processInputBuffer(int index);

    static DWORD WINAPI outputThreadProc(LPVOID param);
    static DWORD WINAPI inputThreadProc(LPVOID param);
};

#endif // AUDIO_ENGINE_H
