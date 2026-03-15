// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

#ifndef SERIAL_H
#define SERIAL_H

#include "Core.h"
#include <string>
#include <vector>
#include <functional>
#include <atomic>

/* Forward declaration for setupapi types (loaded dynamically) */
#ifndef _SETUPAPI_H_
typedef PVOID HDEVINFO;
#endif

class Serial {
public:
    Serial();
    ~Serial();

    bool init();
    bool connect();
    void disconnect();
    bool isConnected() const { return m_connected; }
    
    void setPort(const char* portName);
    void setBaudRate(DWORD baudRate);
    void updateComPorts();
    const std::vector<std::string>& getAvailablePorts() const { return m_availablePorts; }
    
    bool write(const std::vector<uint8_t>& data);
    bool read(std::vector<uint8_t>& data, size_t bytesToRead);

    // Funkcje obsługi zdarzeń
    void onConnect(std::function<void()> callback);
    void onDisconnect(std::function<void()> callback);
    void onReceive(std::function<void(const std::vector<uint8_t>&)> callback);
    void onError(std::function<void()> callback);  // Callback błędu połączenia
    bool send(const std::vector<uint8_t>& data);
    
    // Sprawdza czy połączenie zostało utracone (do wywoływania w loop)
    bool isConnectionLost() const { return m_connectionLost; }
    void clearConnectionLost() { m_connectionLost = false; }

private:
    HANDLE m_serialHandle;
    bool m_connected;
    std::string m_portName;
    DWORD m_baudRate;
    std::vector<std::string> m_availablePorts;
    
    // Callbacki
    std::function<void()> m_onConnectCallback;
    std::function<void()> m_onDisconnectCallback;
    std::function<void(const std::vector<uint8_t>&)> m_onReceiveCallback;
    std::function<void()> m_onErrorCallback;  // Callback błędu (np. port odłączony)
    
    // Wątek odczytu danych (Windows API zamiast std::thread)
    HANDLE m_readThread;
    volatile bool m_stopReadThread;
    volatile bool m_connectionLost;  // Sygnalizuje utratę połączenia (do sprawdzenia w loop)
    void readThreadFunction();
    static DWORD WINAPI readThreadWrapper(LPVOID param);
    void stopReadThread();  // Bezpieczne zatrzymanie wątku

    // --- Dynamic library loading (setupapi.dll) ---
    HMODULE m_setupapiDll;

    typedef HDEVINFO (WINAPI *fn_SetupDiGetClassDevsA)(const GUID*, PCSTR, HWND, DWORD);
    typedef BOOL     (WINAPI *fn_SetupDiEnumDeviceInfo)(HDEVINFO, DWORD, void*);
    typedef BOOL     (WINAPI *fn_SetupDiGetDeviceRegistryPropertyA)(HDEVINFO, void*, DWORD, DWORD*, BYTE*, DWORD, DWORD*);
    typedef BOOL     (WINAPI *fn_SetupDiDestroyDeviceInfoList)(HDEVINFO);

    fn_SetupDiGetClassDevsA               pSetupDiGetClassDevsA;
    fn_SetupDiEnumDeviceInfo              pSetupDiEnumDeviceInfo;
    fn_SetupDiGetDeviceRegistryPropertyA  pSetupDiGetDeviceRegistryPropertyA;
    fn_SetupDiDestroyDeviceInfoList       pSetupDiDestroyDeviceInfoList;

    bool loadSetupApi();
};

#endif // SERIAL_H