#include "Util/FileDialogs.h"
#include "Util/StringUtils.h"

#include <commdlg.h>
#include <shlobj.h>

namespace FileDialogs {

std::string browseFolderUTF8(HWND owner, const wchar_t* title) {
    BROWSEINFOW bi = {};
    bi.hwndOwner = owner;
    bi.lpszTitle = title;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (!pidl)
        return "";

    wchar_t path[MAX_PATH] = {};
    std::string result;
    if (SHGetPathFromIDListW(pidl, path))
        result = StringUtils::wideToUtf8(path);

    CoTaskMemFree(pidl);
    return result;
}

std::string openFileDialogUTF8(
    HWND owner,
    const wchar_t* filter,
    const wchar_t* title,
    const std::string& initialDir)
{
    wchar_t file[MAX_PATH] = {};
    std::wstring wInitDir = StringUtils::utf8ToWide(initialDir);

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!wInitDir.empty())
        ofn.lpstrInitialDir = wInitDir.c_str();

    if (!GetOpenFileNameW(&ofn))
        return "";

    return StringUtils::wideToUtf8(file);
}

std::string saveFileDialogUTF8(
    HWND owner,
    const wchar_t* filter,
    const wchar_t* title,
    const wchar_t* defExt,
    const std::string& initialDir)
{
    wchar_t file[MAX_PATH] = {};
    std::wstring wInitDir = StringUtils::utf8ToWide(initialDir);

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title;
    ofn.lpstrDefExt = defExt;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    if (!wInitDir.empty())
        ofn.lpstrInitialDir = wInitDir.c_str();

    if (!GetSaveFileNameW(&ofn))
        return "";

    return StringUtils::wideToUtf8(file);
}

} // namespace FileDialogs
