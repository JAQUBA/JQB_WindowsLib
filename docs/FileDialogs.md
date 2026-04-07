# FileDialogs

Helpers for native Windows folder/file selection dialogs with UTF-8 return values.

## Header

```cpp
#include <Util/FileDialogs.h>
```

## API

```cpp
namespace FileDialogs {
    std::string browseFolderUTF8(HWND owner, const wchar_t* title);
    std::string openFileDialogUTF8(HWND owner,
                                   const wchar_t* filter,
                                   const wchar_t* title,
                                   const std::string& initialDir = "");
    std::string saveFileDialogUTF8(HWND owner,
                                   const wchar_t* filter,
                                   const wchar_t* title,
                                   const wchar_t* defExt,
                                   const std::string& initialDir = "");
}
```

- `browseFolderUTF8(...)` opens a folder picker (`SHBrowseForFolderW`) and returns UTF-8 path.
- `openFileDialogUTF8(...)` opens file picker (`GetOpenFileNameW`) and returns UTF-8 path.
- `saveFileDialogUTF8(...)` opens save-file dialog (`GetSaveFileNameW`) and returns UTF-8 path.
- Empty string means cancel or no valid selection.

## Example

```cpp
#include <Util/FileDialogs.h>

void doExport(HWND hwnd) {
    std::string out = FileDialogs::saveFileDialogUTF8(
        hwnd,
        L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0",
        L"Export",
        L"txt");
    if (out.empty()) return;
    // Save file to `out`
}
```
