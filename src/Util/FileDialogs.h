#pragma once

#include <string>
#include <windows.h>

namespace FileDialogs {

std::string browseFolderUTF8(HWND owner, const wchar_t* title);

std::string openFileDialogUTF8(
    HWND owner,
    const wchar_t* filter,
    const wchar_t* title,
    const std::string& initialDir = "");

std::string saveFileDialogUTF8(
    HWND owner,
    const wchar_t* filter,
    const wchar_t* title,
    const wchar_t* defExt,
    const std::string& initialDir = "");

} // namespace FileDialogs
