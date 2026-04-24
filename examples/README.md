# Runnable Example Projects

These are ready-to-run PlatformIO projects based on JQB_WindowsLib.

## Projects

1. [01_hello_window](01_hello_window/README.md)
2. [02_serial_monitor](02_serial_monitor/README.md)
3. [03_engineering_canvas](03_engineering_canvas/README.md)
4. [04_document_editor_preview](04_document_editor_preview/README.md)

## How To Run Any Example

1. Open the selected example folder in VS Code.
2. Run `pio run` to build.
3. Run `pio run --target exec` to launch (if your local setup supports it).

Each project uses local library dependency path:

```ini
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
```

Optional local development variant (outside this repository tree):

```ini
lib_deps =
    ../JQB_WindowsLib
```

Using a local path from inside this repository's own `examples/` folders is not recommended,
because PlatformIO package installation may recurse into active `.pio` lock files.
