#include "AppUI.h"

#include "AppState.h"
#include "Canvas/DocumentCanvas.h"

#include <Core.h>
#include <UI/Button/Button.h>
#include <UI/Label/Label.h>
#include <UI/InputField/InputField.h>
#include <Util/TimerUtils.h>

#include <commctrl.h>
#include <richedit.h>

static LRESULT CALLBACK MainSubclassProc(HWND hwnd, UINT msg, WPARAM wParam,
                                         LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (msg == WM_SIZE) {
        doResize(LOWORD(lParam), HIWORD(lParam));
        return 0;
    }

    if (msg == WM_TIMER && wParam == TIMER_RENDER) {
        TimerUtils::stopTimer(hwnd, TIMER_RENDER);
        doRenderPreview();
        return 0;
    }

    if (msg == WM_COMMAND && HIWORD(wParam) == EN_CHANGE && (HWND)lParam == g_editor) {
        scheduleRender();
        return 0;
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

void createUI(SimpleWindow* window) {
    window->add(new Button(10, 8, 110, 26, "Render", [](Button*) {
        doRenderPreview();
    }));

    window->add(new Button(130, 8, 110, 26, "Export", [](Button*) {
        doExportPreview();
    }));

    window->add(new Button(250, 8, 90, 26, "Browse", [](Button*) {
        doBrowseOutput();
    }));

    window->add(new Label(10, 46, 52, 22, L"Output:"));

    g_outputField = new InputField(64, 42, 430, 26, g_outputFile.c_str(),
        [](InputField*, const char* text) {
            g_outputFile = text;
        });
    window->add(g_outputField);

    g_status = new Label(508, 46, 700, 22, L"Status: Ready");
    window->add(g_status);

    LoadLibraryW(L"Msftedit.dll");

    g_editor = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"RICHEDIT50W", L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL |
            ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL,
        0, 0, 100, 100,
        window->getHandle(), NULL, _core.hInstance, NULL);

    SendMessageW(g_editor, EM_SETEVENTMASK, 0,
        SendMessageW(g_editor, EM_GETEVENTMASK, 0, 0) | ENM_CHANGE);

    SendMessageW(g_editor, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(20, 24, 34));

    CHARFORMAT2W cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_FACE | CFM_SIZE | CFM_COLOR;
    cf.yHeight = 220;
    cf.crTextColor = RGB(230, 235, 245);
    lstrcpynW(cf.szFaceName, L"Consolas", LF_FACESIZE);
    SendMessageW(g_editor, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

    g_canvas = new DocumentCanvas();
    g_canvas->create(window->getHandle(), 0, 0, 100, 100);
    g_canvas->setBackgroundColor(RGB(18, 22, 31));
    g_canvas->setGridColor(RGB(55, 65, 85));
    g_canvas->setGridVisible(true);
    g_canvas->setGridExtent(200.0, 140.0);
    g_canvas->setDefaultZoom(0.35);
    g_canvas->setDefaultPan(18.0, 18.0);
    g_canvas->resetView();

    setEditorText(
        "# Example layout\n"
        "plate;100;30;HELLO\n"
        "plate;120;30;JQB\n"
        "plate;90;25;WINDOWSLIB\n");

    SetWindowSubclass(window->getHandle(), MainSubclassProc, 1, 0);

    RECT rc;
    GetClientRect(window->getHandle(), &rc);
    doResize(rc.right, rc.bottom);
}
