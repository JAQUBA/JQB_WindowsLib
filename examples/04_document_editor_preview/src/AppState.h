#pragma once

#include <windows.h>

#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <UI/InputField/InputField.h>
#include <Util/ConfigManager.h>

#include <string>

class DocumentCanvas;

extern SimpleWindow* g_window;
extern ConfigManager* g_settings;
extern Label* g_status;
extern InputField* g_outputField;
extern HWND g_editor;
extern DocumentCanvas* g_canvas;

extern std::string g_outputFile;

constexpr UINT_PTR TIMER_RENDER = 9801;
constexpr UINT RENDER_DELAY_MS = 300;

void loadSettings();
void saveSettings();

std::string getEditorText();
void setEditorText(const std::string& text);

void scheduleRender();
void doRenderPreview();
void doBrowseOutput();
void doExportPreview();
void doResize(int width, int height);
