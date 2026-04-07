// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

#ifndef BLE_H
#define BLE_H

#include "Core.h"
#include <string>
#include <vector>
#include <functional>
#include <atomic>

/* Forward declaration for setupapi types (loaded dynamically) */
#ifndef _SETUPAPI_H_
typedef PVOID HDEVINFO;
#endif

/* Manually defined Bluetooth structs (avoids dependency on bluetoothapis.h link) */
typedef struct _BLUETOOTH_FIND_RADIO_PARAMS_BLE {
    DWORD dwSize;
} BLUETOOTH_FIND_RADIO_PARAMS_BLE;

typedef struct _BLUETOOTH_RADIO_INFO_BLE {
    DWORD dwSize;
    BYTE  address[8];       // BLUETOOTH_ADDRESS (8 bytes)
    WCHAR szName[248];
    ULONG ulClassofDevice;
    USHORT lmpSubversion;
    USHORT manufacturer;
} BLUETOOTH_RADIO_INFO_BLE;

/* GATT implementation (pimpl — defined in BLE.cpp) */
struct BLEGattImpl;

// =====================================================================
// BLEDevice — informacja o znalezionym urządzeniu BLE
// =====================================================================
struct BLEDevice {
    std::wstring name;           // Nazwa urządzenia
    std::wstring address;        // Ścieżka urządzenia (device path)
    int rssi;                    // Siła sygnału (RSSI), 0 jeśli niedostępne
    bool isConnectable;          // Czy urządzenie można połączyć

    // Konwersja do string (dla Select)
    std::string toString() const;
};

// =====================================================================
// BLEScanFilter — filtr urządzeń podczas skanowania
// =====================================================================
struct BLEScanFilter {
    std::wstring nameContains;   // Filtruj po fragmencie nazwy (case-insensitive)
    std::wstring pathContains;   // Filtruj po fragmencie ścieżki (case-insensitive)

    BLEScanFilter() {}

    // Filtr po nazwie
    BLEScanFilter(const std::wstring& nameFilter)
        : nameContains(nameFilter) {}

    // Filtr po nazwie i ścieżce
    BLEScanFilter(const std::wstring& nameFilter, const std::wstring& pathFilter)
        : nameContains(nameFilter), pathContains(pathFilter) {}

    bool isEmpty() const { return nameContains.empty() && pathContains.empty(); }
};

// =====================================================================
// BLE — uniwersalna klasa komunikacji Bluetooth Low Energy
// =====================================================================
class BLE {
    friend struct BLEGattImpl;

public:
    // Stan połączenia BLE
    enum class ConnectionState {
        DISCONNECTED,
        SCANNING,
        CONNECTING,
        CONNECTED,
        CONNECTION_ERROR
    };

    BLE();
    ~BLE();

    // =================================================================
    // Inicjalizacja
    // =================================================================

    bool init();
    bool isAvailable() const { return m_bleAvailable; }

    // =================================================================
    // Skanowanie urządzeń
    // =================================================================

    bool startScan(int durationSeconds = 10);
    bool startScan(const BLEScanFilter& filter, int durationSeconds = 10);
    void stopScan();
    bool isScanning() const { return m_scanning; }

    const std::vector<BLEDevice>& getDiscoveredDevices() const { return m_discoveredDevices; }
    const std::vector<std::string>& getAvailableDevices() const { return m_availableDeviceStrings; }

    // =================================================================
    // Konfiguracja GATT (wywoływane przed connect)
    // =================================================================

    // UUID serwisu GATT do którego się łączymy
    void setServiceUUID(const std::wstring& uuid);

    // UUID charakterystyki notify (odbiór danych)
    void setNotifyCharacteristicUUID(const std::wstring& uuid);

    // UUID charakterystyki write (wysyłanie danych)
    void setWriteCharacteristicUUID(const std::wstring& uuid);

    // =================================================================
    // Połączenie
    // =================================================================

    bool connect(const std::wstring& deviceAddress);
    void disconnect();
    bool isConnected() const { return m_connectionState == ConnectionState::CONNECTED; }

    void setDevice(const std::wstring& deviceNameOrAddress);
    ConnectionState getConnectionState() const { return m_connectionState; }

    // =================================================================
    // Transmisja danych
    // =================================================================

    bool write(const std::vector<uint8_t>& data);
    bool send(const std::vector<uint8_t>& data);

    // =================================================================
    // Callbacki zdarzeń
    // =================================================================

    void onConnect(std::function<void()> callback);
    void onDisconnect(std::function<void()> callback);
    void onReceive(std::function<void(const std::vector<uint8_t>&)> callback);
    void onDeviceDiscovered(std::function<void(const BLEDevice&)> callback);
    void onScanComplete(std::function<void()> callback);
    void onError(std::function<void(const std::wstring&)> callback);

    // =================================================================
    // Priorytetyzacja urządzeń (opcjonalna)
    // =================================================================

    void addPriorityFilter(const std::wstring& nameOrPathFragment);
    void clearPriorityFilters();

private:
    // =====================================================================
    // Marshaling callbacków na wątek UI przez PostMessage
    // =====================================================================
    HWND m_callbackHwnd;
    bool createCallbackWindow();
    void destroyCallbackWindow();
    static LRESULT CALLBACK callbackWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Wątki
    void scanThreadFunction();
    void connectionThreadFunction();
    void updateAvailableDeviceStrings();
    void signalAndJoinThread(HANDLE& hThread, std::atomic<bool>& stopFlag, DWORD timeoutMs);

    // GATT setup (wywoływane z connectionThreadFunction)
    bool setupGatt();

    // Filtrowanie
    bool matchesScanFilter(const std::wstring& name, const std::wstring& path) const;
    bool isPriorityDevice(const std::wstring& name, const std::wstring& path) const;

    // Stan adaptera i połączenia
    bool m_bleAvailable;
    std::atomic<bool> m_scanning;
    std::atomic<ConnectionState> m_connectionState;
    std::wstring m_selectedDeviceAddress;

    // Konfiguracja UUID GATT
    std::wstring m_serviceUUID;
    std::wstring m_notifyCharUUID;
    std::wstring m_writeCharUUID;

    // Filtr skanowania
    BLEScanFilter m_scanFilter;
    std::vector<std::wstring> m_priorityFilters;

    // Wątki
    HANDLE m_scanThread;
    HANDLE m_connectionThread;
    std::atomic<bool> m_stopScanThread;
    std::atomic<bool> m_stopConnectionThread;
    HANDLE m_stopEvent;

    static DWORD WINAPI scanThreadWrapperStatic(LPVOID param);
    static DWORD WINAPI connectionThreadWrapperStatic(LPVOID param);

    int m_scanDurationSeconds;

    // Lista odkrytych urządzeń
    std::vector<BLEDevice> m_discoveredDevices;
    std::vector<std::string> m_availableDeviceStrings;

    // Callbacki
    std::function<void()> m_onConnectCallback;
    std::function<void()> m_onDisconnectCallback;
    std::function<void(const std::vector<uint8_t>&)> m_onReceiveCallback;
    std::function<void(const BLEDevice&)> m_onDeviceDiscoveredCallback;
    std::function<void()> m_onScanCompleteCallback;
    std::function<void(const std::wstring&)> m_onErrorCallback;

    // Handle urządzenia BLE
    HANDLE m_deviceHandle;

    // GATT (pimpl — implementacja w BLE.cpp)
    BLEGattImpl* m_gatt;

    // --- Dynamic library loading (bthprops.cpl) ---
    HMODULE m_bthpropsDll;

    typedef HANDLE (WINAPI *fn_BluetoothFindFirstRadio)(
        const BLUETOOTH_FIND_RADIO_PARAMS_BLE*, HANDLE*);
    typedef BOOL   (WINAPI *fn_BluetoothFindRadioClose)(HANDLE);
    typedef DWORD  (WINAPI *fn_BluetoothGetRadioInfo)(HANDLE, BLUETOOTH_RADIO_INFO_BLE*);

    fn_BluetoothFindFirstRadio  pBluetoothFindFirstRadio;
    fn_BluetoothFindRadioClose  pBluetoothFindRadioClose;
    fn_BluetoothGetRadioInfo    pBluetoothGetRadioInfo;

    // --- Dynamic library loading (setupapi.dll) ---
    HMODULE m_setupapiDll;

    typedef HDEVINFO (WINAPI *fn_SetupDiGetClassDevsW)(const GUID*, PCWSTR, HWND, DWORD);
    typedef BOOL     (WINAPI *fn_SetupDiEnumDeviceInterfaces)(HDEVINFO, void*, const GUID*, DWORD, void*);
    typedef BOOL     (WINAPI *fn_SetupDiGetDeviceInterfaceDetailW)(HDEVINFO, void*, void*, DWORD, DWORD*, void*);
    typedef BOOL     (WINAPI *fn_SetupDiGetDeviceRegistryPropertyW)(HDEVINFO, void*, DWORD, DWORD*, BYTE*, DWORD, DWORD*);
    typedef BOOL     (WINAPI *fn_SetupDiDestroyDeviceInfoList)(HDEVINFO);

    fn_SetupDiGetClassDevsW               pSetupDiGetClassDevsW;
    fn_SetupDiEnumDeviceInterfaces        pSetupDiEnumDeviceInterfaces;
    fn_SetupDiGetDeviceInterfaceDetailW   pSetupDiGetDeviceInterfaceDetailW;
    fn_SetupDiGetDeviceRegistryPropertyW  pSetupDiGetDeviceRegistryPropertyW;
    fn_SetupDiDestroyDeviceInfoList       pSetupDiDestroyDeviceInfoList;
};

#endif // BLE_H
