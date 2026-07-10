# Archetype Matrix

| Archetype | Best For | Core Modules | Key Skills | Key Agents |
|---|---|---|---|---|
| firmware-configurator | Device firmware + desktop config app | protocol, app state/ui, compatibility layer | jqb-dual-runtime-product, jqb-protocol-contract | JQB Firmware App Bridge |
| audio-realtime | Generator/analyzer with live charts | audio engine wiring, realtime loop, chart pipeline | jqb-app-skeleton, jqb-audio-analysis-app | JQB Audio DSP UI |
| cam-pipeline | File parser -> geometry -> export | parser, pipeline, preview canvas, exporter | jqb-cam-pipeline-app, jqb-export-validation | JQB CAM Pipeline Engineer |

## Selection Heuristic
- If a hardware protocol is central and versioned, start with firmware-configurator.
- If continuous sampled data and waveform UX are central, start with audio-realtime.
- If geometry processing and deterministic file export are central, start with cam-pipeline.
