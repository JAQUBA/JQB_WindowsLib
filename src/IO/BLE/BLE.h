#ifndef BLE_H
#define BLE_H

#include "Core.h"
#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <map>

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

// Struktura reprezentująca urządzenie BLE
struct BLEDevice {
    std::wstring name;           // Nazwa urządzenia
    std::wstring address;        // Adres MAC urządzenia
    int rssi;                    // Siła sygnału (RSSI)
    bool isConnectable;          // Czy urządzenie można połączyć
    
    // Konwersja do string dla Select
    std::string toString() const;
};

// Klasa do obsługi komunikacji przez Bluetooth Low Energy
class BLE {
public:
    // Stan połączenia BLE
    enum class ConnectionState {
        DISCONNECTED,
        SCANNING,
        CONNECTING,
        CONNECTED,
        CONNECTION_ERROR  // Zmieniono z ERROR - konflikt z makrem Windows
    };

    BLE();
    ~BLE();

    // Inicjalizacja adaptera BLE
    bool init();
    
    // Sprawdzenie czy BLE jest dostępne w systemie
    bool isAvailable() const { return m_bleAvailable; }
    
    // Skanowanie urządzeń BLE
    bool startScan(int durationSeconds = 10);
    void stopScan();
    bool isScanning() const { return m_scanning; }
    
    // Połączenie z urządzeniem
    bool connect(const std::wstring& deviceAddress);
    void disconnect();
    bool isConnected() const { return m_connectionState == ConnectionState::CONNECTED; }
    
    // Ustawienie urządzenia do połączenia (po nazwie lub adresie)
    void setDevice(const std::wstring& deviceNameOrAddress);
    
    // Pobieranie listy znalezionych urządzeń
    const std::vector<BLEDevice>& getDiscoveredDevices() const { return m_discoveredDevices; }
    
    // Lista urządzeń jako stringi (dla kompatybilności z Select)
    const std::vector<std::string>& getAvailableDevices() const { return m_availableDeviceStrings; }
    
    // Pobieranie aktualnego stanu
    ConnectionState getConnectionState() const { return m_connectionState; }
    std::wstring getConnectionStateString() const;
    
    // Wysyłanie danych do urządzenia
    bool send(const std::vector<uint8_t>& data);
    bool write(const std::vector<uint8_t>& data);
    
    // Callbacki dla zdarzeń
    void onConnect(std::function<void()> callback);
    void onDisconnect(std::function<void()> callback);
    void onReceive(std::function<void(const std::vector<uint8_t>&)> callback);
    void onDeviceDiscovered(std::function<void(const BLEDevice&)> callback);
    void onScanComplete(std::function<void()> callback);
    void onError(std::function<void(const std::wstring&)> callback);
    
    // UUID serwisów i charakterystyk dla OWON OW18B
    static const std::wstring OWON_SERVICE_UUID;
    static const std::wstring OWON_NOTIFY_CHARACTERISTIC_UUID;
    static const std::wstring OWON_WRITE_CHARACTERISTIC_UUID;

private:
    // Wewnętrzne metody
    void scanThreadFunction();
    void connectionThreadFunction();
    void notificationThreadFunction();
    void updateAvailableDeviceStrings();
    bool setupNotifications();
    
    // Stan adaptera i połączenia
    bool m_bleAvailable;
    std::atomic<bool> m_scanning;
    std::atomic<ConnectionState> m_connectionState;
    std::wstring m_selectedDeviceAddress;
    
    // Wątki (Windows API zamiast std::thread)
    HANDLE m_scanThread;
    HANDLE m_connectionThread;
    HANDLE m_notificationThread;
    std::atomic<bool> m_stopScanThread;
    std::atomic<bool> m_stopConnectionThread;
    std::atomic<bool> m_stopNotificationThread;

    static DWORD WINAPI scanThreadWrapperStatic(LPVOID param);
    static DWORD WINAPI connectionThreadWrapperStatic(LPVOID param);
    static DWORD WINAPI notificationThreadWrapperStatic(LPVOID param);

    // Parametry wątku skanowania
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
    
    // Handlery Windows BLE
    HANDLE m_deviceHandle;
    HANDLE m_serviceHandle;
    HANDLE m_notifyCharacteristicHandle;
    HANDLE m_writeCharacteristicHandle;
    
    // Bufor do odbierania danych
    std::vector<uint8_t> m_receiveBuffer;

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
