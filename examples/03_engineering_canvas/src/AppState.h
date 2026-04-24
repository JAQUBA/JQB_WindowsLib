#pragma once

#include <windows.h>

#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/TextArea/TextArea.h>
#include <UI/ProgressBar/ProgressBar.h>
#include <UI/InputField/InputField.h>
#include <Util/ConfigManager.h>

#include <string>

class TreePanel;
class EngineeringCanvas;

extern SimpleWindow* g_window;
extern ConfigManager* g_settings;
extern TextArea* g_log;
extern ProgressBar* g_progress;
extern InputField* g_fldTolerance;
extern EngineeringCanvas* g_canvas;

extern HWND g_hLayerPanel;
extern TreePanel* g_tree;

extern bool g_isRunning;
extern bool g_showOutline;
extern bool g_showPaths;
extern bool g_layersExpanded;

constexpr UINT_PTR TIMER_PREVIEW = 9601;
constexpr UINT PREVIEW_DELAY_MS = 400;
constexpr int LAYER_PANEL_ID = 9500;

void logMsg(const std::string& msg);

void loadSettings();
void saveSettings();

void scheduleAutoRefresh();
void doRefreshPreview();
void doGenerateHeavy();
void rebuildLayerPanel();
