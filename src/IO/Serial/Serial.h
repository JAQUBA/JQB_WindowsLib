#ifndef SERIAL_H
#define SERIAL_H

#include "Core.h"
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>

class Serial {
public:
    Serial();
    ~Serial();

    bool init();
    bool connect();
    void disconnect();
    bool isConnected() const { return m_connected; }
    
    void setPort(const char* portName);
    void updateComPorts();
    const std::vector<std::string>& getAvailablePorts() const { return m_availablePorts; }
    
    bool write(const std::vector<uint8_t>& data);
    bool read(std::vector<uint8_t>& data, size_t bytesToRead);

    // Funkcje obsługi zdarzeń
    void onConnect(std::function<void()> callback);
    void onDisconnect(std::function<void()> callback);
    void onReceive(std::function<void(const std::vector<uint8_t>&)> callback);
    bool send(const std::vector<uint8_t>& data);

private:
    HANDLE m_serialHandle;
    bool m_connected;
    std::string m_portName;
    std::vector<std::string> m_availablePorts;
    
    // Callbacki
    std::function<void()> m_onConnectCallback;
    std::function<void()> m_onDisconnectCallback;
    std::function<void(const std::vector<uint8_t>&)> m_onReceiveCallback;
    
    // Wątek odczytu danych
    std::thread m_readThread;
    std::atomic<bool> m_stopReadThread;
    void readThreadFunction();

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