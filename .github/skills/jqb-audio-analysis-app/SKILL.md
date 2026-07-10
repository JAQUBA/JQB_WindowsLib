---
name: jqb-audio-analysis-app
description: "Implement an audio signal generator/analyzer desktop app with AudioEngine, WaveGen, real-time charts, trigger sync, sample-rate negotiation, and phase comparison workflows."
argument-hint: "Describe generators, analyzers, and required chart/measurement behavior"
user-invocable: true
---
# JQB Audio Analysis App

Builds realtime audio tools for generation, acquisition, and visualization.

## Use When
- Creating oscilloscopic signal monitoring applications.
- Building loopback tests and phase/inversion analysis.
- Integrating waveform controls and live charting.

## Procedure
1. Setup AudioEngine output/input lifecycle and device selectors.
2. Configure WaveGen controls (waveform/frequency/amplitude).
3. Feed charts in batches using sample-rate-aware timing.
4. Enable trigger-sync chart mode for stable waveforms.
5. Add comparison channel (output vs input or inverted input).
6. Surface runtime status and negotiated sample rate.

## Assets
- [audio layout checklist](./assets/audio-layout-checklist.md)

## Acceptance Criteria
- Start/stop is stable for both output and input paths.
- UI remains responsive while streaming data.
- Charts do not produce diagonal stitching artifacts between batches.
