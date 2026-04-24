# JQB_WindowsLib Documentation Hub

Use this page as the central index for building real Windows desktop tools with JQB_WindowsLib.

## Start Here

1. [Quick Start](QuickStart.md) — first project setup and minimal app skeleton.
2. [Examples Overview](examples/README.md) — ready-to-adapt application patterns.
3. [Project Blueprint](ProjectBlueprint.md) — production-grade project structure inspired by real apps.

## AI-Assisted Workflow

- [AI Assistants Guide](AI_Assistants.md) — practical prompt templates for GitHub Copilot and Claude Code.
- [Copilot Instructions](../.github/copilot-instructions.md) — repository-specific coding rules for GitHub Copilot.
- [Claude Project Instructions](../CLAUDE.md) — repository-specific rules for Claude Code.

## Component Documentation

### Core and Windowing

- [Core](Core.md)
- [SimpleWindow](SimpleWindow.md)
- [OverlayWindow](OverlayWindow.md)
- [CanvasWindow](CanvasWindow.md)
- [LogWindow](LogWindow.md)

### Standard Controls

- [Button](Button.md)
- [Label](Label.md)
- [InputField](InputField.md)
- [Select](Select.md)
- [CheckBox](CheckBox.md)
- [TextArea](TextArea.md)

### Visual and Advanced Controls

- [ProgressBar](ProgressBar.md)
- [Chart](Chart.md)
- [ValueDisplay](ValueDisplay.md)
- [ImageView](ImageView.md)
- [TabControl](TabControl.md)
- [TreePanel](TreePanel.md)

### Device I/O

- [Serial](Serial.md)
- [BLE](BLE.md)
- [HID](HID.md)
- [AudioEngine](AudioEngine.md)
- [WaveGen](WaveGen.md)

### Utilities

- [ConfigManager](ConfigManager.md)
- [StringUtils](StringUtils.md)
- [FileDialogs](FileDialogs.md)
- [TimerUtils](TimerUtils.md)

## Production App Patterns

These patterns come from real projects built on this library:

- gerber2gcode: CAD/CAM desktop workflow, layered canvas, heavy processing pipeline.
- WektoroweLitery2: document editor + preview canvas + export workflow.

Use these references:

- [Project Blueprint](ProjectBlueprint.md)
- [Example 08 — Engineering Canvas App](examples/08_engineering_canvas_app.md)
- [Example 09 — Document Editor + Preview Canvas](examples/09_document_editor_canvas.md)

## Suggested Learning Path

1. Build and run the sample from [Quick Start](QuickStart.md).
2. Read [Example 01](examples/01_hello_window.md), [Example 03](examples/03_form_inputs.md), and [Example 04](examples/04_realtime_dashboard.md).
3. Move to [Example 08](examples/08_engineering_canvas_app.md) and [Example 09](examples/09_document_editor_canvas.md).
4. Apply [Project Blueprint](ProjectBlueprint.md) in your own repository.
5. Use [AI Assistants Guide](AI_Assistants.md) to accelerate refactors and feature work.
