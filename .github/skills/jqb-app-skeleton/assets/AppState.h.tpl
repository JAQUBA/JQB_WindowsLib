#pragma once

#include <Core.h>
#include <Util/ConfigManager.h>
#include <Util/PollingManager.h>
#include <Util/TextLogger.h>

class AppState {
public:
    AppState();

    void load();
    void save();

    void afterUiReady();
    void tick();

    ConfigManager config;
    PollingManager polling;
    TextLogger logger;

    bool connected;
    std::string selectedPort;
};
