#include <Core.h>
#include "AppState.h"

AppState::AppState()
    : config("settings.ini"), connected(false), selectedPort("COM1") {}

void AppState::load() {
    selectedPort = config.getValue("port", "COM1");
}

void AppState::save() {
    config.setValue("port", selectedPort);
}

void AppState::afterUiReady() {
    polling.addGroup("monitor", 250, [this]() {
        if (!connected) return;
        logger.info(L"Polling tick");
    });
}

void AppState::tick() {
    polling.tick();
}
