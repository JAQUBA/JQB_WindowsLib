#include "AudioEngine.h"
#include <cstring>

// ============================================================================
// Constructor / Destructor
// ============================================================================
AudioEngine::AudioEngine()
    : m_hWaveOut(NULL)
    , m_outputRunning(false)
    , m_outputEvent(NULL)
    , m_outputThread(NULL)
    , m_hWaveIn(NULL)
    , m_inputRunning(false)
    , m_inputEvent(NULL)
    , m_inputThread(NULL)
    , m_downsample(AUDIO_DOWNSAMPLE)
    , m_outputSnapshotCount(0)
    , m_inputSnapshotCount(0)
    , m_newOutputData(false)
    , m_newInputData(false)
{
    for (int i = 0; i < AUDIO_NUM_BUFFERS; i++) {
        m_outBuffers[i] = nullptr;
        m_inBuffers[i]  = nullptr;
        ZeroMemory(&m_outHeaders[i], sizeof(WAVEHDR));
        ZeroMemory(&m_inHeaders[i], sizeof(WAVEHDR));
    }
    ZeroMemory(m_outputSnapshot, sizeof(m_outputSnapshot));
    ZeroMemory(m_inputSnapshot, sizeof(m_inputSnapshot));

    InitializeCriticalSection(&m_csOutput);
    InitializeCriticalSection(&m_csInput);
    initFormat();
}

AudioEngine::~AudioEngine() {
    stopOutput();
    stopInput();
    DeleteCriticalSection(&m_csOutput);
    DeleteCriticalSection(&m_csInput);
}

void AudioEngine::initFormat() {
    ZeroMemory(&m_wfx, sizeof(m_wfx));
    m_wfx.wFormatTag      = WAVE_FORMAT_PCM;
    m_wfx.nChannels        = AUDIO_CHANNELS;
    m_wfx.nSamplesPerSec   = AUDIO_SAMPLE_RATE;
    m_wfx.wBitsPerSample   = AUDIO_BITS;
    m_wfx.nBlockAlign      = (AUDIO_CHANNELS * AUDIO_BITS) / 8;
    m_wfx.nAvgBytesPerSec  = AUDIO_SAMPLE_RATE * m_wfx.nBlockAlign;
    m_wfx.cbSize           = 0;
}

// ============================================================================
// Device Enumeration
// ============================================================================
std::vector<std::string> AudioEngine::enumOutputDevices() {
    std::vector<std::string> devices;
    UINT count = waveOutGetNumDevs();
    for (UINT i = 0; i < count; i++) {
        WAVEOUTCAPSW caps;
        if (waveOutGetDevCapsW(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            char name[128];
            WideCharToMultiByte(CP_UTF8, 0, caps.szPname, -1, name, 128, NULL, NULL);
            devices.push_back(name);
        }
    }
    return devices;
}

std::vector<std::string> AudioEngine::enumInputDevices() {
    std::vector<std::string> devices;
    UINT count = waveInGetNumDevs();
    for (UINT i = 0; i < count; i++) {
        WAVEINCAPSW caps;
        if (waveInGetDevCapsW(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            char name[128];
            WideCharToMultiByte(CP_UTF8, 0, caps.szPname, -1, name, 128, NULL, NULL);
            devices.push_back(name);
        }
    }
    return devices;
}

// ============================================================================
// Output — waveOut
// ============================================================================
bool AudioEngine::startOutput(int deviceIndex) {
    if (m_outputRunning) return false;

    m_outputEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_outputEvent) return false;

    MMRESULT result = waveOutOpen(&m_hWaveOut, (UINT)deviceIndex, &m_wfx,
                                   (DWORD_PTR)m_outputEvent, 0, CALLBACK_EVENT);
    if (result != MMSYSERR_NOERROR) {
        CloseHandle(m_outputEvent);
        m_outputEvent = NULL;
        return false;
    }

    for (int i = 0; i < AUDIO_NUM_BUFFERS; i++) {
        m_outBuffers[i] = new int16_t[AUDIO_BUFFER_SAMPLES];
        ZeroMemory(&m_outHeaders[i], sizeof(WAVEHDR));
        m_outHeaders[i].lpData         = (LPSTR)m_outBuffers[i];
        m_outHeaders[i].dwBufferLength = AUDIO_BUFFER_SAMPLES * sizeof(int16_t);

        waveOutPrepareHeader(m_hWaveOut, &m_outHeaders[i], sizeof(WAVEHDR));

        m_waveGen.fillBuffer(m_outBuffers[i], AUDIO_BUFFER_SAMPLES, AUDIO_SAMPLE_RATE);
        waveOutWrite(m_hWaveOut, &m_outHeaders[i], sizeof(WAVEHDR));
    }

    m_outputRunning = true;
    m_outputThread = CreateThread(NULL, 0, outputThreadProc, this, 0, NULL);
    return true;
}

void AudioEngine::stopOutput() {
    if (!m_outputRunning) return;

    m_outputRunning = false;
    if (m_outputEvent) SetEvent(m_outputEvent);

    if (m_outputThread) {
        WaitForSingleObject(m_outputThread, 2000);
        CloseHandle(m_outputThread);
        m_outputThread = NULL;
    }

    if (m_hWaveOut) {
        waveOutReset(m_hWaveOut);
        for (int i = 0; i < AUDIO_NUM_BUFFERS; i++) {
            if (m_outBuffers[i]) {
                waveOutUnprepareHeader(m_hWaveOut, &m_outHeaders[i], sizeof(WAVEHDR));
                delete[] m_outBuffers[i];
                m_outBuffers[i] = nullptr;
            }
        }
        waveOutClose(m_hWaveOut);
        m_hWaveOut = NULL;
    }

    if (m_outputEvent) {
        CloseHandle(m_outputEvent);
        m_outputEvent = NULL;
    }
}

DWORD WINAPI AudioEngine::outputThreadProc(LPVOID param) {
    AudioEngine* self = (AudioEngine*)param;
    while (self->m_outputRunning) {
        WaitForSingleObject(self->m_outputEvent, 100);
        if (!self->m_outputRunning) break;

        for (int i = 0; i < AUDIO_NUM_BUFFERS; i++) {
            if (self->m_outHeaders[i].dwFlags & WHDR_DONE) {
                self->refillOutputBuffer(i);
            }
        }
    }
    return 0;
}

void AudioEngine::refillOutputBuffer(int index) {
    m_waveGen.fillBuffer(m_outBuffers[index], AUDIO_BUFFER_SAMPLES, AUDIO_SAMPLE_RATE);

    int snapCount = AUDIO_BUFFER_SAMPLES / m_downsample;
    EnterCriticalSection(&m_csOutput);
    for (int i = 0; i < snapCount; i++) {
        m_outputSnapshot[i] = m_outBuffers[index][i * m_downsample] / 32768.0;
    }
    m_outputSnapshotCount = snapCount;
    m_newOutputData = true;
    LeaveCriticalSection(&m_csOutput);

    waveOutWrite(m_hWaveOut, &m_outHeaders[index], sizeof(WAVEHDR));
}

// ============================================================================
// Input — waveIn
// ============================================================================
bool AudioEngine::startInput(int deviceIndex) {
    if (m_inputRunning) return false;

    m_inputEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_inputEvent) return false;

    MMRESULT result = waveInOpen(&m_hWaveIn, (UINT)deviceIndex, &m_wfx,
                                  (DWORD_PTR)m_inputEvent, 0, CALLBACK_EVENT);
    if (result != MMSYSERR_NOERROR) {
        CloseHandle(m_inputEvent);
        m_inputEvent = NULL;
        return false;
    }

    for (int i = 0; i < AUDIO_NUM_BUFFERS; i++) {
        m_inBuffers[i] = new int16_t[AUDIO_BUFFER_SAMPLES];
        ZeroMemory(&m_inHeaders[i], sizeof(WAVEHDR));
        m_inHeaders[i].lpData         = (LPSTR)m_inBuffers[i];
        m_inHeaders[i].dwBufferLength = AUDIO_BUFFER_SAMPLES * sizeof(int16_t);

        waveInPrepareHeader(m_hWaveIn, &m_inHeaders[i], sizeof(WAVEHDR));
        waveInAddBuffer(m_hWaveIn, &m_inHeaders[i], sizeof(WAVEHDR));
    }

    waveInStart(m_hWaveIn);

    m_inputRunning = true;
    m_inputThread = CreateThread(NULL, 0, inputThreadProc, this, 0, NULL);
    return true;
}

void AudioEngine::stopInput() {
    if (!m_inputRunning) return;

    m_inputRunning = false;
    if (m_inputEvent) SetEvent(m_inputEvent);

    if (m_inputThread) {
        WaitForSingleObject(m_inputThread, 2000);
        CloseHandle(m_inputThread);
        m_inputThread = NULL;
    }

    if (m_hWaveIn) {
        waveInStop(m_hWaveIn);
        waveInReset(m_hWaveIn);
        for (int i = 0; i < AUDIO_NUM_BUFFERS; i++) {
            if (m_inBuffers[i]) {
                waveInUnprepareHeader(m_hWaveIn, &m_inHeaders[i], sizeof(WAVEHDR));
                delete[] m_inBuffers[i];
                m_inBuffers[i] = nullptr;
            }
        }
        waveInClose(m_hWaveIn);
        m_hWaveIn = NULL;
    }

    if (m_inputEvent) {
        CloseHandle(m_inputEvent);
        m_inputEvent = NULL;
    }
}

DWORD WINAPI AudioEngine::inputThreadProc(LPVOID param) {
    AudioEngine* self = (AudioEngine*)param;
    while (self->m_inputRunning) {
        WaitForSingleObject(self->m_inputEvent, 100);
        if (!self->m_inputRunning) break;

        for (int i = 0; i < AUDIO_NUM_BUFFERS; i++) {
            if (self->m_inHeaders[i].dwFlags & WHDR_DONE) {
                self->processInputBuffer(i);
            }
        }
    }
    return 0;
}

void AudioEngine::processInputBuffer(int index) {
    int snapCount = AUDIO_BUFFER_SAMPLES / m_downsample;
    EnterCriticalSection(&m_csInput);
    for (int i = 0; i < snapCount; i++) {
        m_inputSnapshot[i] = m_inBuffers[index][i * m_downsample] / 32768.0;
    }
    m_inputSnapshotCount = snapCount;
    m_newInputData = true;
    LeaveCriticalSection(&m_csInput);

    waveInAddBuffer(m_hWaveIn, &m_inHeaders[index], sizeof(WAVEHDR));
}

// ============================================================================
// Thread-safe snapshot access
// ============================================================================
bool AudioEngine::getOutputSnapshot(double* buffer, int maxSamples, int& outCount) {
    EnterCriticalSection(&m_csOutput);
    if (!m_newOutputData) {
        LeaveCriticalSection(&m_csOutput);
        return false;
    }
    int count = m_outputSnapshotCount;
    if (count > maxSamples) count = maxSamples;
    memcpy(buffer, m_outputSnapshot, count * sizeof(double));
    outCount = count;
    m_newOutputData = false;
    LeaveCriticalSection(&m_csOutput);
    return true;
}

bool AudioEngine::getInputSnapshot(double* buffer, int maxSamples, int& outCount) {
    EnterCriticalSection(&m_csInput);
    if (!m_newInputData) {
        LeaveCriticalSection(&m_csInput);
        return false;
    }
    int count = m_inputSnapshotCount;
    if (count > maxSamples) count = maxSamples;
    memcpy(buffer, m_inputSnapshot, count * sizeof(double));
    outCount = count;
    m_newInputData = false;
    LeaveCriticalSection(&m_csInput);
    return true;
}
