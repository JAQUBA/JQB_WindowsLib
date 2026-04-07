# JQB_WindowsLib Examples

This directory contains compact example applications showing how the library is intended to be used in real Windows tools.

The examples are documentation-first: each file explains the scenario, the components involved, and the shape of the code you would typically write in an actual project.

## What You Will Find Here

| Example | Focus | Main Components |
|---------|-------|-----------------|
| [01_hello_window](01_hello_window.md) | minimal application skeleton | `SimpleWindow`, `Label`, `Button` |
| [02_serial_monitor](02_serial_monitor.md) | serial communication UI | `SimpleWindow`, `Select`, `Button`, `TextArea`, `Serial` |
| [03_form_inputs](03_form_inputs.md) | forms and input handling | `InputField`, `CheckBox`, `Select`, `Button` |
| [04_realtime_dashboard](04_realtime_dashboard.md) | live visualization | `Chart`, `ValueDisplay`, `ProgressBar` |
| [05_tabbed_app](05_tabbed_app.md) | multi-view layout | `TabControl`, `Label`, `Button` |
| [06_ble_scanner](06_ble_scanner.md) | BLE discovery and interaction | `BLE`, `Select`, `TextArea`, `Button` |
| [07_image_viewer](07_image_viewer.md) | image rendering | `ImageView`, `Button`, `Select` |

## Recommended Reading Order

1. Start with [01_hello_window](01_hello_window.md) to understand the base application pattern.
2. Move to [03_form_inputs](03_form_inputs.md) for standard controls.
3. Read [02_serial_monitor](02_serial_monitor.md) or [06_ble_scanner](06_ble_scanner.md) if you are building a hardware-facing tool.
4. Use [04_realtime_dashboard](04_realtime_dashboard.md) and [07_image_viewer](07_image_viewer.md) for richer visual workflows.

## How To Use These Examples

- copy the structural pattern, not necessarily every literal line
- keep your own project resources and branding separate
- pair the example docs with the matching API documents in `../`

If you are starting a new application, the fastest path is usually:

1. read [../QuickStart.md](../QuickStart.md)
2. copy the structure from [01_hello_window](01_hello_window.md)
3. add the components you need based on the remaining examples

## Related Docs

- [QuickStart](../QuickStart.md)
- [SimpleWindow](../SimpleWindow.md)
- [CanvasWindow](../CanvasWindow.md)
- [Serial](../Serial.md)
- [BLE](../BLE.md)
- [ImageView](../ImageView.md)

These examples are intentionally small so they remain easy to scan and easy to adapt.
