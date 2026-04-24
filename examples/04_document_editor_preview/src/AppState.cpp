#include "AppState.h"

#include "Canvas/DocumentCanvas.h"

#include <Util/FileDialogs.h>
#include <Util/StringUtils.h>
#include <Util/TimerUtils.h>

#include <fstream>

SimpleWindow* g_window = nullptr;
ConfigManager* g_settings = nullptr;
Label* g_status = nullptr;
InputField* g_outputField = nullptr;
HWND g_editor = NULL;
DocumentCanvas* g_canvas = nullptr;

std::string g_outputFile = "preview_export.txt";

void loadSettings() {
    if (!g_settings) return;

    g_outputFile = g_settings->getValue("output_file", "preview_export.txt");
    if (g_outputField) {
        g_outputField->setText(g_outputFile.c_str());
    }
}

void saveSettings() {
    if (!g_settings) return;

    if (g_outputField) {
        g_outputFile = g_outputField->getText();
    }

    g_settings->setValue("output_file", g_outputFile);
}

std::string getEditorText() {
    if (!g_editor) return "";

    int len = GetWindowTextLengthW(g_editor);
    if (len <= 0) return "";

    std::wstring wbuf((size_t)len + 1, L'\0');
    GetWindowTextW(g_editor, &wbuf[0], len + 1);
    wbuf.resize((size_t)len);

    return StringUtils::wideToUtf8(wbuf);
}

void setEditorText(const std::string& text) {
    if (!g_editor) return;

    std::wstring w = StringUtils::utf8ToWide(text);
    SetWindowTextW(g_editor, w.c_str());
}

void scheduleRender() {
    if (!g_window) return;
    TimerUtils::restartDebounceTimer(g_window->getHandle(), TIMER_RENDER, RENDER_DELAY_MS);
}

void doRenderPreview() {
    if (!g_canvas) return;

    std::string src = getEditorText();
    g_canvas->setSource(src);
    g_canvas->redraw();

    if (g_status) {
        g_status->setText(L"Status: Preview updated");
    }
}

void doBrowseOutput() {
    if (!g_window) return;

    std::string p = FileDialogs::saveFileDialogUTF8(
        g_window->getHandle(),
        L"Text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0",
        L"Select output file",
        L"txt");

    if (!p.empty()) {
        g_outputFile = p;
        if (g_outputField) g_outputField->setText(p.c_str());
    }
}

void doExportPreview() {
    if (g_outputField) {
        g_outputFile = g_outputField->getText();
    }

    if (g_outputFile.empty()) {
        if (g_status) g_status->setText(L"Status: Set output file first");
        return;
    }

    std::ofstream out(g_outputFile.c_str(), std::ios::binary);
    if (!out.is_open()) {
        if (g_status) g_status->setText(L"Status: Cannot open output file");
        return;
    }

    std::string src = getEditorText();
    out << "# Export from Example 04 - Document Editor Preview\n";
    out << "# --- Begin Content ---\n";
    out << src;
    out << "\n# --- End Content ---\n";
    out.close();

    if (g_status) g_status->setText(L"Status: Exported");
}

void doResize(int width, int height) {
    if (!g_editor || !g_canvas) return;

    const int margin = 10;
    const int top = 76;
    const int editorW = 460;

    int bodyH = height - top - margin;
    if (bodyH < 120) bodyH = 120;

    MoveWindow(g_editor, margin, top, editorW, bodyH, TRUE);

    int canvasX = margin + editorW + margin;
    int canvasW = width - canvasX - margin;
    if (canvasW < 200) canvasW = 200;

    MoveWindow(g_canvas->getHandle(), canvasX, top, canvasW, bodyH, TRUE);
}
