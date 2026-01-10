/**
 * BLE.cpp - Implementacja obsługi Bluetooth Low Energy dla Windows
 * 
 * Ta implementacja używa Windows SetupAPI do enumeracji urządzeń BLE
 * oraz bezpośredniego dostępu do urządzenia przez CreateFile.
 * 
 * Uwaga: Pełna implementacja BLE GATT wymaga Windows SDK z odpowiednimi
 * nagłówkami. Ta wersja jest uproszczona i może wymagać rozszerzenia
 * dla pełnej funkcjonalności.
 */

#include "BLE.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

// Biblioteki Windows
#include <initguid.h>
#include <setupapi.h>
#include <bluetoothapis.h>
#include <bthdef.h>

// GUID dla urządzeń BLE
// {781aee18-7733-4ce4-add0-91f41c67b592} - GUID_BLUETOOTHLE_DEVICE_INTERFACE
DEFINE_GUID(GUID_BTLE_DEVICE_INTERFACE,
    0x781aee18, 0x7733, 0x4ce4, 0xad, 0xd0, 0x91, 0xf4, 0x1c, 0x67, 0xb5, 0x92);

// UUID dla OWON OW18B
const std::wstring BLE::OWON_SERVICE_UUID = L"0000fff0-0000-1000-8000-00805f9b34fb";
const std::wstring BLE::OWON_NOTIFY_CHARACTERISTIC_UUID = L"0000fff4-0000-1000-8000-00805f9b34fb";
const std::wstring BLE::OWON_WRITE_CHARACTERISTIC_UUID = L"0000fff3-0000-1000-8000-00805f9b34fb";

// Implementacja BLEDevice::toString()
std::string BLEDevice::toString() const {
    std::string nameStr(name.begin(), name.end());
    std::string addrStr;
    
    // Wyciągnij tylko końcówkę adresu dla czytelności
    if (address.length() > 30) {
        addrStr = std::string(address.end() - 20, address.end());
    } else {
        addrStr = std::string(address.begin(), address.end());
    }
    
    std::stringstream ss;
    ss << nameStr;
    if (rssi != 0) {
        ss << " (RSSI: " << rssi << " dBm)";
    }
    return ss.str();
}

BLE::BLE() 
    : m_bleAvailable(false)
    , m_scanning(false)
    , m_connectionState(ConnectionState::DISCONNECTED)
    , m_stopScanThread(false)
    , m_stopConnectionThread(false)
    , m_stopNotificationThread(false)
    , m_deviceHandle(INVALID_HANDLE_VALUE)
    , m_serviceHandle(INVALID_HANDLE_VALUE)
    , m_notifyCharacteristicHandle(INVALID_HANDLE_VALUE)
    , m_writeCharacteristicHandle(INVALID_HANDLE_VALUE)
{
}

BLE::~BLE() {
    disconnect();
    stopScan();
    
    if (m_scanThread.joinable()) {
        m_stopScanThread = true;
        m_scanThread.join();
    }
    
    if (m_connectionThread.joinable()) {
        m_stopConnectionThread = true;
        m_connectionThread.join();
    }
    
    if (m_notificationThread.joinable()) {
        m_stopNotificationThread = true;
        m_notificationThread.join();
    }
}

bool BLE::init() {
    // Sprawdzenie czy adapter Bluetooth jest dostępny
    BLUETOOTH_FIND_RADIO_PARAMS btfrp = { sizeof(BLUETOOTH_FIND_RADIO_PARAMS) };
    HANDLE hRadio = NULL;
    HBLUETOOTH_RADIO_FIND hFind = BluetoothFindFirstRadio(&btfrp, &hRadio);
    
    if (hFind) {
        // Pobierz informacje o adapterze
        BLUETOOTH_RADIO_INFO radioInfo = { sizeof(BLUETOOTH_RADIO_INFO) };
        if (BluetoothGetRadioInfo(hRadio, &radioInfo) == ERROR_SUCCESS) {
            // Adapter znaleziony
            m_bleAvailable = true;
        } else {
            m_bleAvailable = true; // Zakładamy że jest, nawet jeśli nie możemy odczytać info
        }
        CloseHandle(hRadio);
        BluetoothFindRadioClose(hFind);
        return m_bleAvailable;
    }
    
    m_bleAvailable = false;
    return false;
}

bool BLE::startScan(int durationSeconds) {
    if (!m_bleAvailable) {
        if (m_onErrorCallback) {
            m_onErrorCallback(L"Bluetooth nie jest dostępny w systemie");
        }
        return false;
    }
    
    if (m_scanning) {
        return true; // Już skanujemy
    }
    
    // Zatrzymaj poprzednie skanowanie
    if (m_scanThread.joinable()) {
        m_stopScanThread = true;
        m_scanThread.join();
    }
    
    m_stopScanThread = false;
    m_scanning = true;
    m_discoveredDevices.clear();
    m_availableDeviceStrings.clear();
    
    // Uruchom skanowanie w osobnym wątku
    m_scanThread = std::thread([this, durationSeconds]() {
        scanThreadFunction();
        
        // Czekaj określony czas lub do zatrzymania
        auto startTime = std::chrono::steady_clock::now();
        while (!m_stopScanThread) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - startTime
            ).count();
            
            if (elapsed >= durationSeconds) {
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        m_scanning = false;
        
        if (m_onScanCompleteCallback) {
            m_onScanCompleteCallback();
        }
    });
    
    return true;
}

void BLE::stopScan() {
    if (m_scanning) {
        m_stopScanThread = true;
        m_scanning = false;
    }
}

void BLE::scanThreadFunction() {
    // Skanowanie urządzeń BLE przez Windows SetupAPI
    GUID bleGuid = GUID_BTLE_DEVICE_INTERFACE;
    
    HDEVINFO deviceInfoSet = SetupDiGetClassDevsW(
        &bleGuid,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
    );
    
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        if (m_onErrorCallback) {
            m_onErrorCallback(L"Nie można pobrać listy urządzeń BLE");
        }
        return;
    }
    
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    
    DWORD deviceIndex = 0;
    
    while (SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &bleGuid, deviceIndex, &deviceInterfaceData)) {
        if (m_stopScanThread) break;
        
        // Pobierz rozmiar potrzebny dla szczegółów interfejsu
        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetailW(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);
        
        if (requiredSize > 0) {
            PSP_DEVICE_INTERFACE_DETAIL_DATA_W detailData = 
                (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)malloc(requiredSize);
            
            if (detailData) {
                detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
                
                SP_DEVINFO_DATA devInfoData;
                devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                
                if (SetupDiGetDeviceInterfaceDetailW(deviceInfoSet, &deviceInterfaceData, 
                    detailData, requiredSize, NULL, &devInfoData)) {
                    
                    // Pobierz nazwę urządzenia
                    wchar_t friendlyName[256] = {0};
                    if (!SetupDiGetDeviceRegistryPropertyW(deviceInfoSet, &devInfoData,
                        SPDRP_FRIENDLYNAME, NULL, (PBYTE)friendlyName, sizeof(friendlyName), NULL)) {
                        // Jeśli nie ma friendly name, spróbuj device description
                        SetupDiGetDeviceRegistryPropertyW(deviceInfoSet, &devInfoData,
                            SPDRP_DEVICEDESC, NULL, (PBYTE)friendlyName, sizeof(friendlyName), NULL);
                    }
                    
                    std::wstring deviceName = friendlyName;
                    std::wstring devicePath = detailData->DevicePath;
                    
                    // Sprawdź czy to urządzenie OWON
                    std::wstring nameLower = deviceName;
                    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::towlower);
                    
                    std::wstring pathLower = devicePath;
                    std::transform(pathLower.begin(), pathLower.end(), pathLower.begin(), ::towlower);
                    
                    bool isOwon = nameLower.find(L"ow18b") != std::wstring::npos ||
                                  nameLower.find(L"owon") != std::wstring::npos ||
                                  pathLower.find(L"ow18b") != std::wstring::npos ||
                                  pathLower.find(L"owon") != std::wstring::npos;
                    
                    // Dodaj urządzenie do listy
                    BLEDevice device;
                    device.name = deviceName.empty() ? L"BLE Device" : deviceName;
                    device.address = devicePath;
                    device.rssi = 0; // Windows SetupAPI nie daje RSSI
                    device.isConnectable = true;
                    
                    // Jeśli to OWON, dodaj na początek listy
                    // Sprawdź czy urządzenie już jest na liście
                    bool alreadyExists = false;
                    for (const auto& d : m_discoveredDevices) {
                        if (d.address == device.address) {
                            alreadyExists = true;
                            break;
                        }
                    }
                    
                    if (!alreadyExists) {
                        if (isOwon) {
                            m_discoveredDevices.insert(m_discoveredDevices.begin(), device);
                        } else {
                            m_discoveredDevices.push_back(device);
                        }
                        updateAvailableDeviceStrings();
                        
                        if (m_onDeviceDiscoveredCallback) {
                            m_onDeviceDiscoveredCallback(device);
                        }
                    }
                }
                
                free(detailData);
            }
        }
        
        deviceIndex++;
    }
    
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
}

void BLE::updateAvailableDeviceStrings() {
    m_availableDeviceStrings.clear();
    for (const auto& device : m_discoveredDevices) {
        m_availableDeviceStrings.push_back(device.toString());
    }
}

bool BLE::connect(const std::wstring& deviceAddress) {
    if (!m_bleAvailable) {
        if (m_onErrorCallback) {
            m_onErrorCallback(L"Bluetooth nie jest dostępny");
        }
        return false;
    }
    
    if (m_connectionState == ConnectionState::CONNECTED) {
        disconnect();
    }
    
    m_selectedDeviceAddress = deviceAddress;
    m_connectionState = ConnectionState::CONNECTING;
    
    // Zatrzymaj poprzedni wątek połączenia
    if (m_connectionThread.joinable()) {
        m_stopConnectionThread = true;
        m_connectionThread.join();
    }
    
    m_stopConnectionThread = false;
    
    // Połączenie w osobnym wątku
    m_connectionThread = std::thread(&BLE::connectionThreadFunction, this);
    
    return true;
}

void BLE::connectionThreadFunction() {
    // Otwórz uchwyt do urządzenia BLE
    m_deviceHandle = CreateFileW(
        m_selectedDeviceAddress.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL
    );
    
    if (m_deviceHandle == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        m_connectionState = ConnectionState::CONNECTION_ERROR;
        
        std::wstringstream ss;
        ss << L"Nie można otworzyć urządzenia BLE (błąd: " << error << L")";
        
        if (m_onErrorCallback) {
            m_onErrorCallback(ss.str());
        }
        return;
    }
    
    // Urządzenie otwarte - oznacz jako połączone
    // Uwaga: Pełna implementacja wymagałaby użycia BluetoothGATT* API
    // które nie są dostępne w MinGW. Ta implementacja jest uproszczona.
    
    m_connectionState = ConnectionState::CONNECTED;
    
    if (m_onConnectCallback) {
        m_onConnectCallback();
    }
    
    // Uruchom wątek powiadomień
    m_stopNotificationThread = false;
    if (m_notificationThread.joinable()) {
        m_notificationThread.join();
    }
    m_notificationThread = std::thread(&BLE::notificationThreadFunction, this);
}

void BLE::notificationThreadFunction() {
    // Wątek odbierający dane z urządzenia BLE
    std::vector<uint8_t> buffer(256);
    OVERLAPPED overlapped = {0};
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    if (overlapped.hEvent == NULL) {
        return;
    }
    
    while (!m_stopNotificationThread && m_connectionState == ConnectionState::CONNECTED) {
        DWORD bytesRead = 0;
        
        // Próba odczytu z urządzenia
        if (ReadFile(m_deviceHandle, buffer.data(), (DWORD)buffer.size(), &bytesRead, &overlapped)) {
            if (bytesRead > 0 && m_onReceiveCallback) {
                std::vector<uint8_t> data(buffer.begin(), buffer.begin() + bytesRead);
                m_onReceiveCallback(data);
            }
        } else {
            DWORD error = GetLastError();
            if (error == ERROR_IO_PENDING) {
                // Czekaj na dane z timeout 100ms
                DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 100);
                if (waitResult == WAIT_OBJECT_0) {
                    if (GetOverlappedResult(m_deviceHandle, &overlapped, &bytesRead, FALSE)) {
                        if (bytesRead > 0 && m_onReceiveCallback) {
                            std::vector<uint8_t> data(buffer.begin(), buffer.begin() + bytesRead);
                            m_onReceiveCallback(data);
                        }
                    }
                }
                ResetEvent(overlapped.hEvent);
            } else if (error != ERROR_SUCCESS) {
                // Błąd - możliwe że urządzenie się rozłączyło
                break;
            }
        }
    }
    
    CloseHandle(overlapped.hEvent);
}

void BLE::disconnect() {
    if (m_connectionState == ConnectionState::DISCONNECTED) {
        return;
    }
    
    m_stopNotificationThread = true;
    if (m_notificationThread.joinable()) {
        m_notificationThread.join();
    }
    
    if (m_deviceHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_deviceHandle);
        m_deviceHandle = INVALID_HANDLE_VALUE;
    }
    
    m_connectionState = ConnectionState::DISCONNECTED;
    
    if (m_onDisconnectCallback) {
        m_onDisconnectCallback();
    }
}

void BLE::setDevice(const std::wstring& deviceNameOrAddress) {
    // Szukaj urządzenia po nazwie lub adresie
    for (const auto& device : m_discoveredDevices) {
        if (device.name == deviceNameOrAddress || device.address == deviceNameOrAddress) {
            m_selectedDeviceAddress = device.address;
            return;
        }
        // Sprawdź czy podana nazwa zawiera się w toString()
        std::string deviceStr = device.toString();
        std::string searchStr(deviceNameOrAddress.begin(), deviceNameOrAddress.end());
        if (deviceStr.find(searchStr) != std::string::npos) {
            m_selectedDeviceAddress = device.address;
            return;
        }
    }
    
    // Jeśli nie znaleziono, ustaw bezpośrednio
    m_selectedDeviceAddress = deviceNameOrAddress;
}

std::wstring BLE::getConnectionStateString() const {
    switch (m_connectionState) {
        case ConnectionState::DISCONNECTED:
            return L"Rozłączony";
        case ConnectionState::SCANNING:
            return L"Skanowanie...";
        case ConnectionState::CONNECTING:
            return L"Łączenie...";
        case ConnectionState::CONNECTED:
            return L"Połączony";
        case ConnectionState::CONNECTION_ERROR:
            return L"Błąd";
        default:
            return L"Nieznany";
    }
}

bool BLE::send(const std::vector<uint8_t>& data) {
    return write(data);
}

bool BLE::write(const std::vector<uint8_t>& data) {
    if (m_connectionState != ConnectionState::CONNECTED || 
        m_deviceHandle == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    DWORD bytesWritten = 0;
    OVERLAPPED overlapped = {0};
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    if (overlapped.hEvent == NULL) {
        return false;
    }
    
    bool success = false;
    
    if (WriteFile(m_deviceHandle, data.data(), (DWORD)data.size(), &bytesWritten, &overlapped)) {
        success = (bytesWritten == data.size());
    } else {
        DWORD error = GetLastError();
        if (error == ERROR_IO_PENDING) {
            // Czekaj na zakończenie zapisu
            if (WaitForSingleObject(overlapped.hEvent, 1000) == WAIT_OBJECT_0) {
                if (GetOverlappedResult(m_deviceHandle, &overlapped, &bytesWritten, FALSE)) {
                    success = (bytesWritten == data.size());
                }
            }
        }
    }
    
    CloseHandle(overlapped.hEvent);
    return success;
}

// Settery callbacków
void BLE::onConnect(std::function<void()> callback) {
    m_onConnectCallback = callback;
}

void BLE::onDisconnect(std::function<void()> callback) {
    m_onDisconnectCallback = callback;
}

void BLE::onReceive(std::function<void(const std::vector<uint8_t>&)> callback) {
    m_onReceiveCallback = callback;
}

void BLE::onDeviceDiscovered(std::function<void(const BLEDevice&)> callback) {
    m_onDeviceDiscoveredCallback = callback;
}

void BLE::onScanComplete(std::function<void()> callback) {
    m_onScanCompleteCallback = callback;
}

void BLE::onError(std::function<void(const std::wstring&)> callback) {
    m_onErrorCallback = callback;
}
