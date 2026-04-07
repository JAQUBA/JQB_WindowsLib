# Contributing to JQB_WindowsLib

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## How to Contribute

### Reporting Bugs

1. Check if the issue already exists in [Issues](https://github.com/JAQUBA/JQB_WindowsLib/issues)
2. Create a new issue with:
   - Clear title and description
   - Steps to reproduce
   - Expected vs actual behavior
   - Compiler version and Windows version

### Suggesting Features

Open an issue with the `enhancement` label describing:
- The use case
- Proposed API (if applicable)
- Why existing components don't cover it

### Pull Requests

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make your changes
4. Test with PlatformIO: `pio run -e native`
5. Submit a pull request against `master`

## Code Style

### General

- **Language:** C++17
- **Indentation:** 4 spaces (no tabs)
- **Line endings:** LF
- **Encoding:** UTF-8

### Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Classes | PascalCase | `SimpleWindow`, `ValueDisplay` |
| Methods | camelCase | `setText()`, `getHandle()` |
| Member variables | `m_` prefix | `m_hwnd`, `m_width` |
| Static members | `s_` prefix | `s_nextId`, `s_instance` |
| Constants/Defines | UPPER_SNAKE_CASE | `MAX_PATH`, `WM_TRAYICON` |

### WinAPI

- Always use explicit Wide (`W`) versions: `CreateWindowExW`, `MessageBoxW`, `LoadCursorW`
- Use `LoadCursorW(NULL, (LPCWSTR)IDC_ARROW)` cast for MinGW compatibility
- Dynamic loading for optional DLLs: `LoadLibrary` / `GetProcAddress` in `init()`

### MinGW.org Compatibility

Avoid these (not available in MinGW.org GCC 6.3.0):

| Avoid | Use instead |
|-------|-------------|
| `std::thread` | `CreateThread()` |
| `std::to_wstring()` | `jqb_compat::to_wstring()` |
| `swprintf_s()` | `_snwprintf()` |

### Component Pattern

New UI components should follow this structure:

```
src/UI/MyComponent/
├── MyComponent.h
└── MyComponent.cpp
```

- Inherit from `UIComponent`
- Implement `create(HWND parent)`, `getId()`, `getHandle()`
- Use auto-incrementing static ID starting at a unique range
- Document in `docs/MyComponent.md`

## Project Structure

- `src/` — library source code
- `docs/` — API documentation (one `.md` per component)
- `scripts/` — build scripts
- `.github/` — GitHub templates and Copilot instructions

## Testing

Currently, testing is done manually via example applications. When submitting changes:

1. Verify your changes compile: `pio run`
2. Test with a minimal example application
3. Verify no regressions in existing components

## License

By contributing, you agree that your contributions will be licensed under the [LGPL-3.0-or-later](LICENSE) license.
