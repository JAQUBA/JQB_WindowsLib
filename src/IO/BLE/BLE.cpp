// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

/**
 * BLE.cpp — Uniwersalna obsługa Bluetooth Low Energy dla Windows
 *
 * Architektura bezpieczna wątkowo:
 *   - Wątki robocze (scan, connection, notification) NIGDY nie wywołują callbacków
 *     bezpośrednio. Zamiast tego wysyłają PostMessage do ukrytego okna (m_callbackHwnd).
 *   - WindowProc ukrytego okna wywołuje callbacki na wątku UI.
 *   - Dzięki temu SetWindowText / SendMessage w callbackach NIE powodują deadlocków.
 *   - Stop event (m_stopEvent) natychmiastowo budzi wątki czekające na I/O.
 */

#include "BLE.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

// Biblioteki Windows — headers only, DLLs loaded dynamically
#include <initguid.h>
#include <setupapi.h>

// GUID dla urządzeń BLE
// {781aee18-7733-4ce4-add0-91f41c67b592} - GUID_BLUETOOTHLE_DEVICE_INTERFACE
DEFINE_GUID(GUID_BTLE_DEVICE_INTERFACE,
    0x781aee18, 0x7733, 0x4ce4, 0xad, 0xd0, 0x91, 0xf4, 0x1c, 0x67, 0xb5, 0x92);

// =====================================================================
// Komunikaty PostMessage wątki → wątek UI
// =====================================================================
// WPARAM = 0, LPARAM = new BLEDevice (heap)
#define WM_BLE_DEVICE_DISCOVERED  (WM_APP + 100)
// WPARAM = 0, LPARAM = 0
#define WM_BLE_SCAN_COMPLETE      (WM_APP + 101)
// WPARAM = 0, LPARAM = 0
#define WM_BLE_CONNECTED          (WM_APP + 102)
// WPARAM = 0, LPARAM = 0
#define WM_BLE_DISCONNECTED       (WM_APP + 103)
// WPARAM = 0, LPARAM = new std::vector<uint8_t> (heap)
#define WM_BLE_DATA_RECEIVED      (WM_APP + 104)
// WPARAM = 0, LPARAM = new std::wstring (heap)
#define WM_BLE_ERROR              (WM_APP + 105)

// =====================================================================
// Windows BLE GATT types (bthledef.h equivalents for MinGW)
// =====================================================================
typedef struct _BTH_LE_UUID {
    BOOLEAN IsShortUuid;
    union {
        USHORT ShortUuid;
        GUID LongUuid;
    } Value;
} BTH_LE_UUID, *PBTH_LE_UUID;

typedef struct _BTH_LE_GATT_SERVICE {
    BTH_LE_UUID ServiceUuid;
    USHORT AttributeHandle;
} BTH_LE_GATT_SERVICE, *PBTH_LE_GATT_SERVICE;

typedef struct _BTH_LE_GATT_CHARACTERISTIC {
    USHORT ServiceHandle;
    BTH_LE_UUID CharacteristicUuid;
    USHORT AttributeHandle;
    USHORT CharacteristicValueHandle;
    BOOLEAN IsBroadcastable;
    BOOLEAN IsReadable;
    BOOLEAN IsWritable;
    BOOLEAN IsWritableWithoutResponse;
    BOOLEAN IsSignedWritable;
    BOOLEAN IsNotifiable;
    BOOLEAN IsIndicatable;
    BOOLEAN HasExtendedProperties;
} BTH_LE_GATT_CHARACTERISTIC, *PBTH_LE_GATT_CHARACTERISTIC;

typedef struct _BTH_LE_GATT_CHARACTERISTIC_VALUE {
    ULONG DataSize;
    UCHAR Data[1];
} BTH_LE_GATT_CHARACTERISTIC_VALUE, *PBTH_LE_GATT_CHARACTERISTIC_VALUE;

typedef enum _BTH_LE_GATT_DESCRIPTOR_TYPE {
    CharacteristicExtendedProperties,
    CharacteristicUserDescription,
    ClientCharacteristicConfiguration,
    ServerCharacteristicConfiguration,
    CharacteristicFormat,
    CharacteristicAggregateFormat,
    CustomDescriptor
} BTH_LE_GATT_DESCRIPTOR_TYPE;

typedef struct _BTH_LE_GATT_DESCRIPTOR {
    USHORT ServiceHandle;
    BTH_LE_UUID CharacteristicUuid;
    BTH_LE_GATT_DESCRIPTOR_TYPE DescriptorType;
    BTH_LE_UUID DescriptorUuid;
    USHORT AttributeHandle;
} BTH_LE_GATT_DESCRIPTOR, *PBTH_LE_GATT_DESCRIPTOR;

typedef struct _BTH_LE_GATT_DESCRIPTOR_VALUE {
    BTH_LE_GATT_DESCRIPTOR_TYPE DescriptorType;
    BTH_LE_UUID DescriptorUuid;
    union {
        struct {
            BOOLEAN IsReliableWriteEnabled;
            BOOLEAN IsAuxiliariesWritable;
        } CharacteristicExtendedProperties;
        struct {
            BOOLEAN IsSubscribeToNotification;
            BOOLEAN IsSubscribeToIndication;
        } ClientCharacteristicConfiguration;
        struct {
            BOOLEAN IsBroadcast;
        } ServerCharacteristicConfiguration;
        struct {
            UCHAR Format;
            UCHAR Exponent;
            BTH_LE_UUID Unit;
            UCHAR NameSpace;
            BTH_LE_UUID Description;
        } CharacteristicFormat;
    };
    ULONG DataSize;
    UCHAR Data[1];
} BTH_LE_GATT_DESCRIPTOR_VALUE, *PBTH_LE_GATT_DESCRIPTOR_VALUE;

typedef enum _BTH_LE_GATT_EVENT_TYPE {
    CharacteristicValueChangedEvent
} BTH_LE_GATT_EVENT_TYPE;

typedef struct _BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION {
    USHORT NumCharacteristics;
    BTH_LE_GATT_CHARACTERISTIC Characteristics[1];
} BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION;

typedef struct _BLUETOOTH_GATT_VALUE_CHANGED_EVENT {
    USHORT ChangedAttributeHandle;
    size_t CharacteristicValueDataSize;
    PBTH_LE_GATT_CHARACTERISTIC_VALUE CharacteristicValue;
} BLUETOOTH_GATT_VALUE_CHANGED_EVENT;

typedef PVOID BLUETOOTH_GATT_EVENT_HANDLE;

typedef VOID (CALLBACK *PFNBLUETOOTH_GATT_EVENT_CALLBACK)(
    BTH_LE_GATT_EVENT_TYPE EventType,
    PVOID EventOutParameter,
    PVOID Context
);

#define BLUETOOTH_GATT_FLAG_NONE                   0x00000000
#define BLUETOOTH_GATT_FLAG_WRITE_WITHOUT_RESPONSE 0x00000020

// Function pointer types for BluetoothApis.dll
typedef HRESULT (WINAPI *fn_BluetoothGATTGetServices)(
    HANDLE, USHORT, PBTH_LE_GATT_SERVICE, USHORT*, DWORD);
typedef HRESULT (WINAPI *fn_BluetoothGATTGetCharacteristics)(
    HANDLE, PBTH_LE_GATT_SERVICE, USHORT, PBTH_LE_GATT_CHARACTERISTIC, USHORT*, DWORD);
typedef HRESULT (WINAPI *fn_BluetoothGATTGetDescriptors)(
    HANDLE, PBTH_LE_GATT_CHARACTERISTIC, USHORT, PBTH_LE_GATT_DESCRIPTOR, USHORT*, DWORD);
typedef HRESULT (WINAPI *fn_BluetoothGATTSetDescriptorValue)(
    HANDLE, PBTH_LE_GATT_DESCRIPTOR, PBTH_LE_GATT_DESCRIPTOR_VALUE, DWORD);
typedef HRESULT (WINAPI *fn_BluetoothGATTGetCharacteristicValue)(
    HANDLE, PBTH_LE_GATT_CHARACTERISTIC, ULONG, PBTH_LE_GATT_CHARACTERISTIC_VALUE, USHORT*, DWORD);
typedef HRESULT (WINAPI *fn_BluetoothGATTSetCharacteristicValue)(
    HANDLE, PBTH_LE_GATT_CHARACTERISTIC, PBTH_LE_GATT_CHARACTERISTIC_VALUE, ULONG, DWORD);
typedef HRESULT (WINAPI *fn_BluetoothGATTRegisterEvent)(
    HANDLE, BTH_LE_GATT_EVENT_TYPE, PVOID, PFNBLUETOOTH_GATT_EVENT_CALLBACK, PVOID,
    BLUETOOTH_GATT_EVENT_HANDLE*, DWORD);
typedef HRESULT (WINAPI *fn_BluetoothGATTUnregisterEvent)(
    BLUETOOTH_GATT_EVENT_HANDLE, DWORD);

// =====================================================================
// BLEGattImpl — GATT implementation (pimpl, friend of BLE)
// =====================================================================
struct BLEGattImpl {
    HMODULE hDll;

    fn_BluetoothGATTGetServices pGetServices;
    fn_BluetoothGATTGetCharacteristics pGetCharacteristics;
    fn_BluetoothGATTGetDescriptors pGetDescriptors;
    fn_BluetoothGATTSetDescriptorValue pSetDescriptorValue;
    fn_BluetoothGATTGetCharacteristicValue pGetCharValue;
    fn_BluetoothGATTSetCharacteristicValue pSetCharValue;
    fn_BluetoothGATTRegisterEvent pRegisterEvent;
    fn_BluetoothGATTUnregisterEvent pUnregisterEvent;

    HANDLE serviceHandle;
    HANDLE writeHandle;  // Osobny handle do operacji write (unika konfliktu z notify)
    std::wstring serviceDevicePath;  // Ścieżka do ponownego otwarcia handle'a
    BTH_LE_GATT_CHARACTERISTIC writeChar;
    BTH_LE_GATT_CHARACTERISTIC notifyChar;
    bool hasWriteChar;
    bool hasNotifyChar;
    bool ownsServiceHandle;  // false gdy serviceHandle == m_deviceHandle (fallback)
    BLUETOOTH_GATT_EVENT_HANDLE eventHandle;

    BLEGattImpl()
        : hDll(NULL), serviceHandle(INVALID_HANDLE_VALUE)
        , writeHandle(INVALID_HANDLE_VALUE)
        , hasWriteChar(false), hasNotifyChar(false), ownsServiceHandle(true), eventHandle(NULL)
        , pGetServices(NULL), pGetCharacteristics(NULL)
        , pGetDescriptors(NULL), pSetDescriptorValue(NULL)
        , pGetCharValue(NULL), pSetCharValue(NULL)
        , pRegisterEvent(NULL), pUnregisterEvent(NULL)
    {
        memset(&writeChar, 0, sizeof(writeChar));
        memset(&notifyChar, 0, sizeof(notifyChar));
    }

    bool loadDll() {
        hDll = LoadLibraryA("BluetoothApis.dll");
        if (!hDll) return false;

        pGetServices = (fn_BluetoothGATTGetServices)
            GetProcAddress(hDll, "BluetoothGATTGetServices");
        pGetCharacteristics = (fn_BluetoothGATTGetCharacteristics)
            GetProcAddress(hDll, "BluetoothGATTGetCharacteristics");
        pGetDescriptors = (fn_BluetoothGATTGetDescriptors)
            GetProcAddress(hDll, "BluetoothGATTGetDescriptors");
        pSetDescriptorValue = (fn_BluetoothGATTSetDescriptorValue)
            GetProcAddress(hDll, "BluetoothGATTSetDescriptorValue");
        pGetCharValue = (fn_BluetoothGATTGetCharacteristicValue)
            GetProcAddress(hDll, "BluetoothGATTGetCharacteristicValue");
        pSetCharValue = (fn_BluetoothGATTSetCharacteristicValue)
            GetProcAddress(hDll, "BluetoothGATTSetCharacteristicValue");
        pRegisterEvent = (fn_BluetoothGATTRegisterEvent)
            GetProcAddress(hDll, "BluetoothGATTRegisterEvent");
        pUnregisterEvent = (fn_BluetoothGATTUnregisterEvent)
            GetProcAddress(hDll, "BluetoothGATTUnregisterEvent");

        if (!pGetServices || !pGetCharacteristics || !pGetDescriptors ||
            !pSetDescriptorValue || !pSetCharValue || !pRegisterEvent || !pUnregisterEvent) {
            FreeLibrary(hDll);
            hDll = NULL;
            return false;
        }
        return true;
    }

    void cleanup() {
        if (eventHandle && pUnregisterEvent) {
            pUnregisterEvent(eventHandle, BLUETOOTH_GATT_FLAG_NONE);
            eventHandle = NULL;
        }
        if (writeHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(writeHandle);
            writeHandle = INVALID_HANDLE_VALUE;
        }
        if (serviceHandle != INVALID_HANDLE_VALUE && ownsServiceHandle) {
            CloseHandle(serviceHandle);
        }
        serviceHandle = INVALID_HANDLE_VALUE;
        ownsServiceHandle = true;
        serviceDevicePath.clear();
        hasWriteChar = false;
        hasNotifyChar = false;
        memset(&writeChar, 0, sizeof(writeChar));
        memset(&notifyChar, 0, sizeof(notifyChar));
    }

    ~BLEGattImpl() {
        cleanup();
        if (hDll) {
            FreeLibrary(hDll);
            hDll = NULL;
        }
    }

    // Parse UUID string to GUID
    static GUID parseUUID(const std::wstring& uuid) {
        GUID guid = {};
        unsigned long p1 = 0;
        unsigned int p2 = 0, p3 = 0, p4 = 0, p5 = 0;
        unsigned int p6 = 0, p7 = 0, p8 = 0, p9 = 0, p10 = 0, p11 = 0;
        swscanf(uuid.c_str(), L"%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10, &p11);
        guid.Data1 = p1;
        guid.Data2 = (unsigned short)p2;
        guid.Data3 = (unsigned short)p3;
        guid.Data4[0] = (unsigned char)p4;
        guid.Data4[1] = (unsigned char)p5;
        guid.Data4[2] = (unsigned char)p6;
        guid.Data4[3] = (unsigned char)p7;
        guid.Data4[4] = (unsigned char)p8;
        guid.Data4[5] = (unsigned char)p9;
        guid.Data4[6] = (unsigned char)p10;
        guid.Data4[7] = (unsigned char)p11;
        return guid;
    }

    // Compare BLE UUID with target GUID
    static bool matchesUUID(const BTH_LE_UUID& bleUuid, const GUID& targetGuid) {
        if (bleUuid.IsShortUuid) {
            GUID full = {};
            full.Data1 = bleUuid.Value.ShortUuid;
            full.Data2 = 0x0000;
            full.Data3 = 0x1000;
            full.Data4[0] = 0x80; full.Data4[1] = 0x00;
            full.Data4[2] = 0x00; full.Data4[3] = 0x80;
            full.Data4[4] = 0x5F; full.Data4[5] = 0x9B;
            full.Data4[6] = 0x34; full.Data4[7] = 0xFB;
            return memcmp(&full, &targetGuid, sizeof(GUID)) == 0;
        }
        return memcmp(&bleUuid.Value.LongUuid, &targetGuid, sizeof(GUID)) == 0;
    }

    // Extract device identifier from device path for matching service interfaces
    // Device path: \\?\BTHLE#Dev_AABBCCDDEEFF#7&1234&0&...
    // Service path: \\?\BTHLEDevice#{uuid}_Dev_AABBCCDDEEFF#7&1234&0#...
    // We extract from first 'dev_' through next '#' delimiter
    static std::wstring extractDeviceId(const std::wstring& devicePath) {
        std::wstring pathLower = devicePath;
        std::transform(pathLower.begin(), pathLower.end(), pathLower.begin(), ::towlower);
        size_t pos = pathLower.find(L"dev_");
        if (pos != std::wstring::npos) {
            // Szukaj tylko '#' jako delimiter — '&' jest częścią VID/PID identyfikatora
            size_t end = pathLower.find(L'#', pos + 4);
            if (end != std::wstring::npos) {
                return pathLower.substr(pos, end - pos);
            }
            return pathLower.substr(pos);
        }
        return L"";
    }

    // GATT notification callback (called from Windows thread pool)
    static VOID CALLBACK notifyCallback(
        BTH_LE_GATT_EVENT_TYPE EventType,
        PVOID EventOutParameter,
        PVOID Context
    ) {
        BLE* ble = (BLE*)Context;
        if (!ble || !ble->m_callbackHwnd) return;

        if (EventType == CharacteristicValueChangedEvent) {
            BLUETOOTH_GATT_VALUE_CHANGED_EVENT* event =
                (BLUETOOTH_GATT_VALUE_CHANGED_EVENT*)EventOutParameter;
            if (event && event->CharacteristicValue &&
                event->CharacteristicValue->DataSize > 0) {
                auto* data = new std::vector<uint8_t>(
                    event->CharacteristicValue->Data,
                    event->CharacteristicValue->Data + event->CharacteristicValue->DataSize
                );
                PostMessageW(ble->m_callbackHwnd, WM_BLE_DATA_RECEIVED, 0, (LPARAM)data);
            }
        }
    }
};

// =====================================================================
// Nazwa klasy ukrytego okna callback
// =====================================================================
static const wchar_t* BLE_CALLBACK_WND_CLASS = L"JQB_BLE_CallbackWnd";
static bool s_callbackClassRegistered = false;

// =====================================================================
// BLEDevice::toString()
// =====================================================================
std::string BLEDevice::toString() const {
    std::string nameStr(name.begin(), name.end());

    std::stringstream ss;
    ss << nameStr;
    if (rssi != 0) {
        ss << " (RSSI: " << rssi << " dBm)";
    }
    return ss.str();
}

// =====================================================================
// Konstruktor / Destruktor
// =====================================================================
BLE::BLE()
    : m_bleAvailable(false)
    , m_scanning(false)
    , m_connectionState(ConnectionState::DISCONNECTED)
    , m_callbackHwnd(NULL)
    , m_scanThread(NULL)
    , m_connectionThread(NULL)
    , m_stopScanThread(false)
    , m_stopConnectionThread(false)
    , m_stopEvent(NULL)
    , m_scanDurationSeconds(10)
    , m_deviceHandle(INVALID_HANDLE_VALUE)
    , m_gatt(NULL)
    , m_bthpropsDll(NULL)
    , pBluetoothFindFirstRadio(NULL)
    , pBluetoothFindRadioClose(NULL)
    , pBluetoothGetRadioInfo(NULL)
    , m_setupapiDll(NULL)
    , pSetupDiGetClassDevsW(NULL)
    , pSetupDiEnumDeviceInterfaces(NULL)
    , pSetupDiGetDeviceInterfaceDetailW(NULL)
    , pSetupDiGetDeviceRegistryPropertyW(NULL)
    , pSetupDiDestroyDeviceInfoList(NULL)
{
    m_stopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
}

BLE::~BLE() {
    // 1. Sygnalizuj wątkom zakończenie
    m_stopScanThread = true;
    m_stopConnectionThread = true;
    if (m_stopEvent) SetEvent(m_stopEvent);

    // 2. GATT cleanup (unregister events, close service handle)
    if (m_gatt) {
        m_gatt->cleanup();
    }

    // 3. Zamknij handle urządzenia
    if (m_deviceHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_deviceHandle);
        m_deviceHandle = INVALID_HANDLE_VALUE;
    }

    // 4. Czekaj na wątki z pompowaniem komunikatów
    signalAndJoinThread(m_connectionThread, m_stopConnectionThread, 2000);
    signalAndJoinThread(m_scanThread, m_stopScanThread, 3000);

    // 5. Zniszcz ukryte okno callback
    destroyCallbackWindow();

    // 6. Zwolnij zasoby
    if (m_stopEvent) {
        CloseHandle(m_stopEvent);
        m_stopEvent = NULL;
    }

    delete m_gatt;
    m_gatt = NULL;

    if (m_bthpropsDll) {
        FreeLibrary(m_bthpropsDll);
        m_bthpropsDll = NULL;
    }
    if (m_setupapiDll) {
        FreeLibrary(m_setupapiDll);
        m_setupapiDll = NULL;
    }
}

// =====================================================================
// Ukryte okno callback — marshaling wątki → UI
// =====================================================================
bool BLE::createCallbackWindow() {
    if (m_callbackHwnd) return true;

    // Rejestruj klasę okna (raz)
    if (!s_callbackClassRegistered) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = BLE::callbackWindowProc;
        wc.hInstance = _core.hInstance;
        wc.lpszClassName = BLE_CALLBACK_WND_CLASS;
        if (!RegisterClassW(&wc)) {
            // Może być już zarejestrowana — to ok
            if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
                return false;
            }
        }
        s_callbackClassRegistered = true;
    }

    // HWND_MESSAGE — okno message-only (nie widoczne, bez renderowania)
    m_callbackHwnd = CreateWindowExW(
        0, BLE_CALLBACK_WND_CLASS, L"", 0,
        0, 0, 0, 0,
        HWND_MESSAGE, NULL, _core.hInstance, NULL
    );

    if (!m_callbackHwnd) return false;

    // Zapisz wskaźnik do BLE w danych okna (GWLP_USERDATA)
    SetWindowLongPtrW(m_callbackHwnd, GWLP_USERDATA, (LONG_PTR)this);
    return true;
}

void BLE::destroyCallbackWindow() {
    if (m_callbackHwnd) {
        // Wyzeruj USERDATA — zabezpieczenie przed komunikatami w locie
        SetWindowLongPtrW(m_callbackHwnd, GWLP_USERDATA, 0);
        DestroyWindow(m_callbackHwnd);
        m_callbackHwnd = NULL;
    }
}

LRESULT CALLBACK BLE::callbackWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    BLE* self = (BLE*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    // Jeśli self == NULL → obiekt BLE już zniszczony, zwolnij pamięć
    if (!self) {
        // Zwolnij alokacje heap przekazane przez LPARAM
        switch (msg) {
            case WM_BLE_DEVICE_DISCOVERED:
                delete (BLEDevice*)lParam;
                break;
            case WM_BLE_DATA_RECEIVED:
                delete (std::vector<uint8_t>*)lParam;
                break;
            case WM_BLE_ERROR:
                delete (std::wstring*)lParam;
                break;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    switch (msg) {
        case WM_BLE_DEVICE_DISCOVERED: {
            BLEDevice* dev = (BLEDevice*)lParam;
            if (dev) {
                if (self->m_onDeviceDiscoveredCallback) {
                    self->m_onDeviceDiscoveredCallback(*dev);
                }
                delete dev;
            }
            return 0;
        }
        case WM_BLE_SCAN_COMPLETE: {
            if (self->m_onScanCompleteCallback) {
                self->m_onScanCompleteCallback();
            }
            return 0;
        }
        case WM_BLE_CONNECTED: {
            if (self->m_onConnectCallback) {
                self->m_onConnectCallback();
            }
            return 0;
        }
        case WM_BLE_DISCONNECTED: {
            if (self->m_onDisconnectCallback) {
                self->m_onDisconnectCallback();
            }
            return 0;
        }
        case WM_BLE_DATA_RECEIVED: {
            std::vector<uint8_t>* data = (std::vector<uint8_t>*)lParam;
            if (data) {
                if (self->m_onReceiveCallback) {
                    self->m_onReceiveCallback(*data);
                }
                delete data;
            }
            return 0;
        }
        case WM_BLE_ERROR: {
            std::wstring* errMsg = (std::wstring*)lParam;
            if (errMsg) {
                if (self->m_onErrorCallback) {
                    self->m_onErrorCallback(*errMsg);
                }
                delete errMsg;
            }
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// =====================================================================
// signalAndJoinThread — bezpieczne oczekiwanie z pompowaniem komunikatów
// =====================================================================
void BLE::signalAndJoinThread(HANDLE& hThread, std::atomic<bool>& stopFlag, DWORD timeoutMs) {
    if (hThread == NULL) return;

    stopFlag = true;
    if (m_stopEvent) SetEvent(m_stopEvent);

    DWORD startTick = GetTickCount();
    while (true) {
        DWORD elapsed = GetTickCount() - startTick;
        if (elapsed >= timeoutMs) {
            TerminateThread(hThread, 0);
            break;
        }
        DWORD remaining = timeoutMs - elapsed;
        DWORD waitMs = (remaining < 50) ? remaining : 50;

        DWORD result = MsgWaitForMultipleObjects(1, &hThread, FALSE, waitMs, QS_ALLINPUT);
        if (result == WAIT_OBJECT_0) {
            break; // Wątek zakończony
        } else if (result == WAIT_OBJECT_0 + 1) {
            MSG msg;
            while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
    }
    CloseHandle(hThread);
    hThread = NULL;
}

// =====================================================================
// init() — Inicjalizacja adaptera BLE
// =====================================================================
bool BLE::init() {
    // Utwórz ukryte okno callback
    if (!createCallbackWindow()) {
        m_bleAvailable = false;
        return false;
    }

    // --- Load bthprops.cpl dynamically ---
    m_bthpropsDll = LoadLibraryA("bthprops.cpl");
    if (!m_bthpropsDll) {
        m_bleAvailable = false;
        return false;
    }

    pBluetoothFindFirstRadio = (fn_BluetoothFindFirstRadio)
        GetProcAddress(m_bthpropsDll, "BluetoothFindFirstRadio");
    pBluetoothFindRadioClose = (fn_BluetoothFindRadioClose)
        GetProcAddress(m_bthpropsDll, "BluetoothFindRadioClose");
    pBluetoothGetRadioInfo = (fn_BluetoothGetRadioInfo)
        GetProcAddress(m_bthpropsDll, "BluetoothGetRadioInfo");

    if (!pBluetoothFindFirstRadio || !pBluetoothFindRadioClose || !pBluetoothGetRadioInfo) {
        FreeLibrary(m_bthpropsDll);
        m_bthpropsDll = NULL;
        m_bleAvailable = false;
        return false;
    }

    // --- Load setupapi.dll dynamically ---
    m_setupapiDll = LoadLibraryA("setupapi.dll");
    if (!m_setupapiDll) {
        FreeLibrary(m_bthpropsDll);
        m_bthpropsDll = NULL;
        m_bleAvailable = false;
        return false;
    }

    pSetupDiGetClassDevsW = (fn_SetupDiGetClassDevsW)
        GetProcAddress(m_setupapiDll, "SetupDiGetClassDevsW");
    pSetupDiEnumDeviceInterfaces = (fn_SetupDiEnumDeviceInterfaces)
        GetProcAddress(m_setupapiDll, "SetupDiEnumDeviceInterfaces");
    pSetupDiGetDeviceInterfaceDetailW = (fn_SetupDiGetDeviceInterfaceDetailW)
        GetProcAddress(m_setupapiDll, "SetupDiGetDeviceInterfaceDetailW");
    pSetupDiGetDeviceRegistryPropertyW = (fn_SetupDiGetDeviceRegistryPropertyW)
        GetProcAddress(m_setupapiDll, "SetupDiGetDeviceRegistryPropertyW");
    pSetupDiDestroyDeviceInfoList = (fn_SetupDiDestroyDeviceInfoList)
        GetProcAddress(m_setupapiDll, "SetupDiDestroyDeviceInfoList");

    if (!pSetupDiGetClassDevsW || !pSetupDiEnumDeviceInterfaces ||
        !pSetupDiGetDeviceInterfaceDetailW || !pSetupDiGetDeviceRegistryPropertyW ||
        !pSetupDiDestroyDeviceInfoList) {
        FreeLibrary(m_setupapiDll);
        m_setupapiDll = NULL;
        FreeLibrary(m_bthpropsDll);
        m_bthpropsDll = NULL;
        m_bleAvailable = false;
        return false;
    }

    // Sprawdzenie adaptera Bluetooth
    BLUETOOTH_FIND_RADIO_PARAMS_BLE btfrp = { sizeof(BLUETOOTH_FIND_RADIO_PARAMS_BLE) };
    HANDLE hRadio = NULL;
    HANDLE hFind = pBluetoothFindFirstRadio(&btfrp, &hRadio);

    if (hFind) {
        m_bleAvailable = true;
        CloseHandle(hRadio);
        pBluetoothFindRadioClose(hFind);

        // Load BluetoothApis.dll for GATT support
        m_gatt = new BLEGattImpl();
        if (!m_gatt->loadDll()) {
            delete m_gatt;
            m_gatt = NULL;
            // GATT niedostępne — skanowanie będzie działać, ale connect wymaga GATT
        }

        return true;
    }

    m_bleAvailable = false;
    return false;
}

// =====================================================================
// startScan() / stopScan()
// =====================================================================
bool BLE::startScan(int durationSeconds) {
    return startScan(BLEScanFilter(), durationSeconds);
}

bool BLE::startScan(const BLEScanFilter& filter, int durationSeconds) {
    if (!m_bleAvailable) {
        if (m_callbackHwnd) {
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(L"Bluetooth nie jest dostępny w systemie"));
        }
        return false;
    }

    if (m_scanning) {
        return true;
    }

    // Zatrzymaj poprzedni wątek skanowania
    if (m_scanThread != NULL) {
        m_stopScanThread = true;
        if (m_stopEvent) SetEvent(m_stopEvent);
        signalAndJoinThread(m_scanThread, m_stopScanThread, 3000);
    }

    // Reset stop event dla nowej operacji
    if (m_stopEvent) ResetEvent(m_stopEvent);

    m_stopScanThread = false;
    m_scanning = true;
    m_discoveredDevices.clear();
    m_availableDeviceStrings.clear();
    m_scanDurationSeconds = durationSeconds;
    m_scanFilter = filter;

    m_scanThread = CreateThread(NULL, 0, BLE::scanThreadWrapperStatic, this, 0, NULL);
    return (m_scanThread != NULL);
}

void BLE::stopScan() {
    if (m_scanning) {
        m_stopScanThread = true;
        if (m_stopEvent) SetEvent(m_stopEvent);
        m_scanning = false;
    }
}

// =====================================================================
// scanThreadFunction() — skanowanie przez SetupAPI
// =====================================================================
void BLE::scanThreadFunction() {
    GUID bleGuid = GUID_BTLE_DEVICE_INTERFACE;

    HDEVINFO deviceInfoSet = pSetupDiGetClassDevsW(
        &bleGuid, NULL, NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
    );

    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        if (m_callbackHwnd) {
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(L"Nie można pobrać listy urządzeń BLE"));
        }
        return;
    }

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    DWORD deviceIndex = 0;

    while (pSetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &bleGuid, deviceIndex, &deviceInterfaceData)) {
        if (m_stopScanThread) break;

        DWORD requiredSize = 0;
        pSetupDiGetDeviceInterfaceDetailW(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

        if (requiredSize > 0) {
            PSP_DEVICE_INTERFACE_DETAIL_DATA_W detailData =
                (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)malloc(requiredSize);

            if (detailData) {
                detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

                SP_DEVINFO_DATA devInfoData;
                devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

                if (pSetupDiGetDeviceInterfaceDetailW(deviceInfoSet, &deviceInterfaceData,
                    detailData, requiredSize, NULL, &devInfoData)) {

                    wchar_t friendlyName[256] = {0};
                    if (!pSetupDiGetDeviceRegistryPropertyW(deviceInfoSet, &devInfoData,
                        SPDRP_FRIENDLYNAME, NULL, (PBYTE)friendlyName, sizeof(friendlyName), NULL)) {
                        pSetupDiGetDeviceRegistryPropertyW(deviceInfoSet, &devInfoData,
                            SPDRP_DEVICEDESC, NULL, (PBYTE)friendlyName, sizeof(friendlyName), NULL);
                    }

                    std::wstring deviceName = friendlyName;
                    std::wstring devicePath = detailData->DevicePath;

                    // Zastosuj filtr skanowania
                    if (!matchesScanFilter(deviceName, devicePath)) {
                        free(detailData);
                        deviceIndex++;
                        continue;
                    }

                    BLEDevice device;
                    device.name = deviceName.empty() ? L"BLE Device" : deviceName;
                    device.address = devicePath;
                    device.rssi = 0;
                    device.isConnectable = true;

                    // Sprawdzenie duplikatu
                    bool alreadyExists = false;
                    for (const auto& d : m_discoveredDevices) {
                        if (d.address == device.address) {
                            alreadyExists = true;
                            break;
                        }
                    }

                    if (!alreadyExists) {
                        // Priorytetyzacja — pasujące urządzenia na początek listy
                        if (isPriorityDevice(deviceName, devicePath)) {
                            m_discoveredDevices.insert(m_discoveredDevices.begin(), device);
                        } else {
                            m_discoveredDevices.push_back(device);
                        }
                        updateAvailableDeviceStrings();

                        // PostMessage zamiast bezpośredniego callbacka!
                        if (m_callbackHwnd) {
                            PostMessageW(m_callbackHwnd, WM_BLE_DEVICE_DISCOVERED, 0,
                                (LPARAM)new BLEDevice(device));
                        }
                    }
                }
                free(detailData);
            }
        }
        deviceIndex++;
    }

    pSetupDiDestroyDeviceInfoList(deviceInfoSet);
}

void BLE::updateAvailableDeviceStrings() {
    m_availableDeviceStrings.clear();
    for (const auto& device : m_discoveredDevices) {
        m_availableDeviceStrings.push_back(device.toString());
    }
}

// =====================================================================
// connect() / disconnect()
// =====================================================================
bool BLE::connect(const std::wstring& deviceAddress) {
    if (!m_bleAvailable) {
        if (m_callbackHwnd) {
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(L"Bluetooth nie jest dostępny"));
        }
        return false;
    }

    // Rozłącz jeśli potrzeba
    if (m_connectionState == ConnectionState::CONNECTED ||
        m_connectionState == ConnectionState::CONNECTING) {
        disconnect();
    }

    m_selectedDeviceAddress = deviceAddress;
    m_connectionState = ConnectionState::CONNECTING;

    // Zatrzymaj poprzedni wątek połączenia
    if (m_connectionThread != NULL) {
        m_stopConnectionThread = true;
        if (m_stopEvent) SetEvent(m_stopEvent);
        signalAndJoinThread(m_connectionThread, m_stopConnectionThread, 3000);
    }

    // Reset eventów dla nowej operacji
    if (m_stopEvent) ResetEvent(m_stopEvent);
    m_stopConnectionThread = false;

    m_connectionThread = CreateThread(NULL, 0, BLE::connectionThreadWrapperStatic, this, 0, NULL);
    return (m_connectionThread != NULL);
}

void BLE::disconnect() {
    ConnectionState prevState = m_connectionState.load();
    if (prevState == ConnectionState::DISCONNECTED) {
        return;
    }

    // 1. Ustaw stan natychmiast
    m_connectionState = ConnectionState::DISCONNECTED;

    // 2. Sygnalizuj wątkom zakończenie
    m_stopConnectionThread = true;
    if (m_stopEvent) SetEvent(m_stopEvent);

    // 3. GATT cleanup (unregister events, close service handle)
    if (m_gatt) {
        m_gatt->cleanup();
    }

    // 4. Zamknij handle urządzenia
    if (m_deviceHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_deviceHandle);
        m_deviceHandle = INVALID_HANDLE_VALUE;
    }

    // 5. Czekaj na wątek połączenia
    signalAndJoinThread(m_connectionThread, m_stopConnectionThread, 3000);

    // 6. Powiadom UI
    if (m_callbackHwnd) {
        PostMessageW(m_callbackHwnd, WM_BLE_DISCONNECTED, 0, 0);
    }
}

// =====================================================================
// connectionThreadFunction() — otwieranie urządzenia BLE + GATT setup
// =====================================================================
void BLE::connectionThreadFunction() {
    if (m_stopConnectionThread) return;

    // 1. Otwórz uchwyt do urządzenia BLE
    m_deviceHandle = CreateFileW(
        m_selectedDeviceAddress.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (m_stopConnectionThread) {
        if (m_deviceHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_deviceHandle);
            m_deviceHandle = INVALID_HANDLE_VALUE;
        }
        m_connectionState = ConnectionState::DISCONNECTED;
        return;
    }

    if (m_deviceHandle == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        m_connectionState = ConnectionState::CONNECTION_ERROR;

        std::wstringstream ss;
        ss << L"Nie można otworzyć urządzenia BLE (błąd: " << error << L")";

        if (m_callbackHwnd) {
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(ss.str()));
        }
        return;
    }

    // 2. GATT setup (jeśli skonfigurowano serwis UUID)
    if (!m_serviceUUID.empty() && m_gatt && m_gatt->hDll) {
        if (!setupGatt()) {
            // GATT setup nie powiódł się — zamknij handle urządzenia
            if (m_deviceHandle != INVALID_HANDLE_VALUE) {
                CloseHandle(m_deviceHandle);
                m_deviceHandle = INVALID_HANDLE_VALUE;
            }
            m_connectionState = ConnectionState::CONNECTION_ERROR;
            return;
        }
    } else if (!m_serviceUUID.empty() && (!m_gatt || !m_gatt->hDll)) {
        // UUID skonfigurowane ale brak BluetoothApis.dll
        if (m_callbackHwnd) {
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(L"BluetoothApis.dll niedostępne — GATT nie będzie działać"));
        }
    }

    // 3. Połączono
    m_connectionState = ConnectionState::CONNECTED;

    if (m_callbackHwnd) {
        PostMessageW(m_callbackHwnd, WM_BLE_CONNECTED, 0, 0);
    }
}

// =====================================================================
// setupGatt() — konfiguracja serwisu, charakterystyk i powiadomień GATT
// =====================================================================
bool BLE::setupGatt() {
    GUID serviceGuid = BLEGattImpl::parseUUID(m_serviceUUID);

    // Pobierz listę serwisów GATT z urządzenia
    USHORT serviceCount = 0;
    HRESULT hr = m_gatt->pGetServices(
        m_deviceHandle, 0, NULL, &serviceCount, BLUETOOTH_GATT_FLAG_NONE);

    // Retry — urządzenie może potrzebować chwili po CreateFile
    for (int retry = 0; retry < 5 && serviceCount == 0 && !m_stopConnectionThread; retry++) {
        Sleep(500);
        hr = m_gatt->pGetServices(
            m_deviceHandle, 0, NULL, &serviceCount, BLUETOOTH_GATT_FLAG_NONE);
    }

    if (serviceCount == 0) {
        if (m_callbackHwnd) {
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(L"Nie znaleziono serwisów GATT na urządzeniu"));
        }
        return false;
    }

    std::vector<BTH_LE_GATT_SERVICE> services(serviceCount);
    hr = m_gatt->pGetServices(
        m_deviceHandle, serviceCount, services.data(), &serviceCount, BLUETOOTH_GATT_FLAG_NONE);
    if (FAILED(hr)) {
        if (m_callbackHwnd) {
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(L"Błąd odczytu serwisów GATT"));
        }
        return false;
    }

    // Znajdź serwis pasujący do UUID
    bool serviceFound = false;
    for (USHORT i = 0; i < serviceCount; i++) {
        if (BLEGattImpl::matchesUUID(services[i].ServiceUuid, serviceGuid)) {
            serviceFound = true;
            break;
        }
    }

    if (!serviceFound) {
        if (m_callbackHwnd) {
            std::wstring msg = L"Nie znaleziono serwisu GATT: " + m_serviceUUID;
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0, (LPARAM)new std::wstring(msg));
        }
        return false;
    }

    // Otwórz uchwyt do interfejsu serwisu (przez SetupDI z service UUID jako GUID)
    HDEVINFO devInfoSet = pSetupDiGetClassDevsW(
        &serviceGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    std::wstring deviceId = BLEGattImpl::extractDeviceId(m_selectedDeviceAddress);
    bool serviceHandleOpened = false;

    if (devInfoSet != INVALID_HANDLE_VALUE) {
        SP_DEVICE_INTERFACE_DATA ifData;
        ifData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        // Zbierz wszystkie ścieżki interfejsów serwisu
        std::vector<std::wstring> allServicePaths;
        for (DWORD i = 0; pSetupDiEnumDeviceInterfaces(devInfoSet, NULL, &serviceGuid, i, &ifData); i++) {
            DWORD requiredSize = 0;
            pSetupDiGetDeviceInterfaceDetailW(devInfoSet, &ifData, NULL, 0, &requiredSize, NULL);

            if (requiredSize > 0) {
                PSP_DEVICE_INTERFACE_DETAIL_DATA_W detailData =
                    (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)malloc(requiredSize);
                if (detailData) {
                    detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
                    if (pSetupDiGetDeviceInterfaceDetailW(devInfoSet, &ifData, detailData, requiredSize, NULL, NULL)) {
                        allServicePaths.push_back(detailData->DevicePath);
                    }
                    free(detailData);
                }
            }
        }

        pSetupDiDestroyDeviceInfoList(devInfoSet);

        // Próba 1: dopasuj po device ID
        if (!deviceId.empty()) {
            for (auto& servicePath : allServicePaths) {
                std::wstring servicePathLower = servicePath;
                std::transform(servicePathLower.begin(), servicePathLower.end(),
                               servicePathLower.begin(), ::towlower);

                if (servicePathLower.find(deviceId) != std::wstring::npos) {
                    m_gatt->serviceHandle = CreateFileW(
                        servicePath.c_str(),
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
                    if (m_gatt->serviceHandle != INVALID_HANDLE_VALUE) {
                        m_gatt->serviceDevicePath = servicePath;
                        serviceHandleOpened = true;
                        break;
                    }
                }
            }
        }

        // Próba 2: jeśli jest dokładnie 1 interfejs — użyj go bez dopasowywania
        if (!serviceHandleOpened && allServicePaths.size() == 1) {
            m_gatt->serviceHandle = CreateFileW(
                allServicePaths[0].c_str(),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
            if (m_gatt->serviceHandle != INVALID_HANDLE_VALUE) {
                m_gatt->serviceDevicePath = allServicePaths[0];
                serviceHandleOpened = true;
            }
        }

        // Próba 3: spróbuj każdy interfejs po kolei
        if (!serviceHandleOpened && allServicePaths.size() > 1) {
            for (auto& servicePath : allServicePaths) {
                m_gatt->serviceHandle = CreateFileW(
                    servicePath.c_str(),
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
                if (m_gatt->serviceHandle != INVALID_HANDLE_VALUE) {
                    // Weryfikuj — czy ten handle ma charakterystyki?
                    USHORT testCharCount = 0;
                    HRESULT testHr = m_gatt->pGetCharacteristics(
                        m_gatt->serviceHandle, NULL, 0, NULL, &testCharCount, BLUETOOTH_GATT_FLAG_NONE);
                    if (testCharCount > 0) {
                        m_gatt->serviceDevicePath = servicePath;
                        serviceHandleOpened = true;
                        break;
                    }
                    CloseHandle(m_gatt->serviceHandle);
                    m_gatt->serviceHandle = INVALID_HANDLE_VALUE;
                }
            }
        }

        // Diagnostyka jeśli nie udało się
        if (!serviceHandleOpened && m_callbackHwnd) {
            std::wstringstream ss;
            ss << L"SetupDI: " << allServicePaths.size() << L" interfejsów";
            if (!deviceId.empty()) ss << L", deviceId='" << deviceId << L"'";
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0, (LPARAM)new std::wstring(ss.str()));
        }
    } else {
        if (m_callbackHwnd) {
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(L"SetupDI: nie można enumerować interfejsów serwisu"));
        }
    }

    // Próba 4 (fallback): użyj device handle bezpośrednio jako service handle
    if (!serviceHandleOpened) {
        // Niektóre urządzenia pozwalają na operacje GATT bezpośrednio przez device handle
        USHORT testCharCount = 0;
        HRESULT testHr = m_gatt->pGetCharacteristics(
            m_deviceHandle, NULL, 0, NULL, &testCharCount, BLUETOOTH_GATT_FLAG_NONE);
        if (testCharCount > 0) {
            // Device handle działa — użyj go jako service handle (nie zamykamy go osobno)
            m_gatt->serviceHandle = m_deviceHandle;
            m_gatt->ownsServiceHandle = false;
            serviceHandleOpened = true;
        }
    }

    if (!serviceHandleOpened) {
        if (m_callbackHwnd) {
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(L"Nie można otworzyć uchwytu serwisu GATT"));
        }
        return false;
    }

    // Pobierz charakterystyki z serwisu
    USHORT charCount = 0;
    hr = m_gatt->pGetCharacteristics(
        m_gatt->serviceHandle, NULL, 0, NULL, &charCount, BLUETOOTH_GATT_FLAG_NONE);

    if (charCount == 0) {
        if (m_callbackHwnd) {
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(L"Nie znaleziono charakterystyk GATT"));
        }
        return false;
    }

    std::vector<BTH_LE_GATT_CHARACTERISTIC> chars(charCount);
    hr = m_gatt->pGetCharacteristics(
        m_gatt->serviceHandle, NULL, charCount, chars.data(), &charCount, BLUETOOTH_GATT_FLAG_NONE);
    if (FAILED(hr)) {
        if (m_callbackHwnd) {
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(L"Błąd odczytu charakterystyk GATT"));
        }
        return false;
    }

    // Znajdź charakterystykę notify
    if (!m_notifyCharUUID.empty()) {
        GUID notifyGuid = BLEGattImpl::parseUUID(m_notifyCharUUID);
        for (USHORT i = 0; i < charCount; i++) {
            if (BLEGattImpl::matchesUUID(chars[i].CharacteristicUuid, notifyGuid)) {
                m_gatt->notifyChar = chars[i];
                m_gatt->hasNotifyChar = true;
                break;
            }
        }
    } else {
        // Auto-detect: pierwsza notifiable charakterystyka
        for (USHORT i = 0; i < charCount; i++) {
            if (chars[i].IsNotifiable || chars[i].IsIndicatable) {
                m_gatt->notifyChar = chars[i];
                m_gatt->hasNotifyChar = true;
                break;
            }
        }
    }

    // Znajdź charakterystykę write
    if (!m_writeCharUUID.empty()) {
        GUID writeGuid = BLEGattImpl::parseUUID(m_writeCharUUID);
        for (USHORT i = 0; i < charCount; i++) {
            if (BLEGattImpl::matchesUUID(chars[i].CharacteristicUuid, writeGuid)) {
                m_gatt->writeChar = chars[i];
                m_gatt->hasWriteChar = true;
                break;
            }
        }
    } else {
        // Auto-detect: pierwsza writable charakterystyka
        for (USHORT i = 0; i < charCount; i++) {
            if (chars[i].IsWritable || chars[i].IsWritableWithoutResponse) {
                m_gatt->writeChar = chars[i];
                m_gatt->hasWriteChar = true;
                break;
            }
        }
    }

    // Otwórz osobny handle do operacji write (unika konfliktu z notify na tym samym handle)
    if (m_gatt->hasWriteChar && !m_gatt->serviceDevicePath.empty()) {
        m_gatt->writeHandle = CreateFileW(
            m_gatt->serviceDevicePath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
    }

    // Zarejestruj powiadomienia GATT
    if (m_gatt->hasNotifyChar) {
        // Włącz CCCD (Client Characteristic Configuration Descriptor)
        USHORT descCount = 0;
        hr = m_gatt->pGetDescriptors(
            m_gatt->serviceHandle, &m_gatt->notifyChar, 0, NULL, &descCount, BLUETOOTH_GATT_FLAG_NONE);

        if (descCount > 0) {
            std::vector<BTH_LE_GATT_DESCRIPTOR> descs(descCount);
            hr = m_gatt->pGetDescriptors(
                m_gatt->serviceHandle, &m_gatt->notifyChar, descCount,
                descs.data(), &descCount, BLUETOOTH_GATT_FLAG_NONE);

            if (SUCCEEDED(hr)) {
                for (USHORT i = 0; i < descCount; i++) {
                    if (descs[i].DescriptorType == ClientCharacteristicConfiguration) {
                        BTH_LE_GATT_DESCRIPTOR_VALUE descValue;
                        memset(&descValue, 0, sizeof(descValue));
                        descValue.DescriptorType = ClientCharacteristicConfiguration;
                        if (m_gatt->notifyChar.IsNotifiable) {
                            descValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = TRUE;
                        }
                        if (m_gatt->notifyChar.IsIndicatable) {
                            descValue.ClientCharacteristicConfiguration.IsSubscribeToIndication = TRUE;
                        }
                        m_gatt->pSetDescriptorValue(
                            m_gatt->serviceHandle, &descs[i], &descValue, BLUETOOTH_GATT_FLAG_NONE);
                        break;
                    }
                }
            }
        }

        // Zarejestruj callback na zmiany wartości
        size_t regSize = sizeof(BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION);
        BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION* reg =
            (BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION*)malloc(regSize);
        if (reg) {
            memset(reg, 0, regSize);
            reg->NumCharacteristics = 1;
            reg->Characteristics[0] = m_gatt->notifyChar;

            hr = m_gatt->pRegisterEvent(
                m_gatt->serviceHandle,
                CharacteristicValueChangedEvent,
                reg,
                BLEGattImpl::notifyCallback,
                this,
                &m_gatt->eventHandle,
                BLUETOOTH_GATT_FLAG_NONE
            );

            free(reg);

            if (FAILED(hr)) {
                if (m_callbackHwnd) {
                    std::wstringstream ss;
                    ss << L"Nie można zarejestrować powiadomień GATT (0x"
                       << std::hex << hr << L")";
                    PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                        (LPARAM)new std::wstring(ss.str()));
                }
                // Kontynuuj — połączenie nawiązane, ale brak powiadomień
            }
        }
    }

    return true;
}



// =====================================================================
// setDevice()
// =====================================================================
void BLE::setDevice(const std::wstring& deviceNameOrAddress) {
    for (const auto& device : m_discoveredDevices) {
        if (device.name == deviceNameOrAddress || device.address == deviceNameOrAddress) {
            m_selectedDeviceAddress = device.address;
            return;
        }
        std::string deviceStr = device.toString();
        std::string searchStr(deviceNameOrAddress.begin(), deviceNameOrAddress.end());
        if (deviceStr.find(searchStr) != std::string::npos) {
            m_selectedDeviceAddress = device.address;
            return;
        }
    }
    m_selectedDeviceAddress = deviceNameOrAddress;
}

// =====================================================================
// matchesScanFilter() — sprawdzenie czy urządzenie pasuje do filtra
// =====================================================================
bool BLE::matchesScanFilter(const std::wstring& name, const std::wstring& path) const {
    if (m_scanFilter.isEmpty()) return true;

    std::wstring nameLower = name;
    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::towlower);
    std::wstring pathLower = path;
    std::transform(pathLower.begin(), pathLower.end(), pathLower.begin(), ::towlower);

    if (!m_scanFilter.nameContains.empty()) {
        std::wstring filterLower = m_scanFilter.nameContains;
        std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::towlower);
        if (nameLower.find(filterLower) == std::wstring::npos) {
            return false;
        }
    }

    if (!m_scanFilter.pathContains.empty()) {
        std::wstring filterLower = m_scanFilter.pathContains;
        std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::towlower);
        if (pathLower.find(filterLower) == std::wstring::npos) {
            return false;
        }
    }

    return true;
}

// =====================================================================
// isPriorityDevice() — sprawdzenie czy urządzenie jest priorytetowe
// =====================================================================
bool BLE::isPriorityDevice(const std::wstring& name, const std::wstring& path) const {
    if (m_priorityFilters.empty()) return false;

    std::wstring nameLower = name;
    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::towlower);
    std::wstring pathLower = path;
    std::transform(pathLower.begin(), pathLower.end(), pathLower.begin(), ::towlower);

    for (const auto& filter : m_priorityFilters) {
        std::wstring filterLower = filter;
        std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::towlower);
        if (nameLower.find(filterLower) != std::wstring::npos ||
            pathLower.find(filterLower) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

// =====================================================================
// addPriorityFilter() / clearPriorityFilters()
// =====================================================================
void BLE::addPriorityFilter(const std::wstring& nameOrPathFragment) {
    m_priorityFilters.push_back(nameOrPathFragment);
}

void BLE::clearPriorityFilters() {
    m_priorityFilters.clear();
}

// =====================================================================
// send() / write()
// =====================================================================
bool BLE::send(const std::vector<uint8_t>& data) {
    return write(data);
}

bool BLE::write(const std::vector<uint8_t>& data) {
    if (m_connectionState != ConnectionState::CONNECTED) {
        if (m_callbackHwnd) {
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(L"write(): nie połączono"));
        }
        return false;
    }

    // GATT write — preferowane gdy dostępne
    if (m_gatt && m_gatt->hasWriteChar && m_gatt->serviceHandle != INVALID_HANDLE_VALUE) {
        // Użyj osobnego writeHandle jeśli dostępny (unika konfliktu z notify)
        HANDLE hWrite = (m_gatt->writeHandle != INVALID_HANDLE_VALUE)
            ? m_gatt->writeHandle : m_gatt->serviceHandle;

        size_t valueSize = sizeof(BTH_LE_GATT_CHARACTERISTIC_VALUE) + data.size();
        BTH_LE_GATT_CHARACTERISTIC_VALUE* charValue =
            (BTH_LE_GATT_CHARACTERISTIC_VALUE*)malloc(valueSize);
        if (!charValue) return false;

        memset(charValue, 0, valueSize);
        charValue->DataSize = (ULONG)data.size();
        memcpy(charValue->Data, data.data(), data.size());

        ULONG flags = BLUETOOTH_GATT_FLAG_NONE;
        if (m_gatt->writeChar.IsWritableWithoutResponse) {
            flags = BLUETOOTH_GATT_FLAG_WRITE_WITHOUT_RESPONSE;
        }

        HRESULT hr = m_gatt->pSetCharValue(
            hWrite, &m_gatt->writeChar, charValue, 0, flags);

        free(charValue);

        if (FAILED(hr) && m_callbackHwnd) {
            wchar_t errBuf[256];
            swprintf(errBuf, 256,
                L"GATT write FAILED: hr=0x%08lX, flags=0x%lX, writable=%d, writableNoResp=%d, separateHandle=%d",
                (unsigned long)hr, flags,
                (int)m_gatt->writeChar.IsWritable,
                (int)m_gatt->writeChar.IsWritableWithoutResponse,
                (m_gatt->writeHandle != INVALID_HANDLE_VALUE) ? 1 : 0);
            PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
                (LPARAM)new std::wstring(errBuf));
        }
        return SUCCEEDED(hr);
    }

    // Brak GATT write char — diagnostyka
    if (m_callbackHwnd) {
        wchar_t errBuf[256];
        swprintf(errBuf, 256,
            L"write(): brak GATT writeChar (gatt=%d, hasWrite=%d, svcHandle=%d)",
            m_gatt ? 1 : 0,
            m_gatt ? (int)m_gatt->hasWriteChar : -1,
            m_gatt ? (m_gatt->serviceHandle != INVALID_HANDLE_VALUE ? 1 : 0) : -1);
        PostMessageW(m_callbackHwnd, WM_BLE_ERROR, 0,
            (LPARAM)new std::wstring(errBuf));
    }

    // Fallback: raw WriteFile (dla urządzeń nie-GATT)
    if (m_deviceHandle == INVALID_HANDLE_VALUE) return false;

    DWORD bytesWritten = 0;
    OVERLAPPED overlapped = {0};
    overlapped.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!overlapped.hEvent) return false;

    bool success = false;

    if (WriteFile(m_deviceHandle, data.data(), (DWORD)data.size(), &bytesWritten, &overlapped)) {
        success = (bytesWritten == data.size());
    } else {
        DWORD error = GetLastError();
        if (error == ERROR_IO_PENDING) {
            HANDLE waitHandles[2] = { overlapped.hEvent, m_stopEvent };
            DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, 2000);
            if (waitResult == WAIT_OBJECT_0) {
                if (GetOverlappedResult(m_deviceHandle, &overlapped, &bytesWritten, FALSE)) {
                    success = (bytesWritten == data.size());
                }
            } else {
                CancelIo(m_deviceHandle);
                GetOverlappedResult(m_deviceHandle, &overlapped, &bytesWritten, TRUE);
            }
        }
    }

    CloseHandle(overlapped.hEvent);
    return success;
}

// =====================================================================
// Settery callbacków
// =====================================================================
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

// =====================================================================
// Statyczne wrappery wątków
// =====================================================================
DWORD WINAPI BLE::scanThreadWrapperStatic(LPVOID param) {
    BLE* self = (BLE*)param;
    self->scanThreadFunction();

    // Czekaj na koniec skanowania lub sygnał stop
    DWORD startTick = GetTickCount();
    while (!self->m_stopScanThread) {
        DWORD elapsed = GetTickCount() - startTick;
        if ((int)(elapsed / 1000) >= self->m_scanDurationSeconds) {
            break;
        }
        // Czekaj 100ms lub do sygnału stop
        if (self->m_stopEvent) {
            DWORD waitResult = WaitForSingleObject(self->m_stopEvent, 100);
            if (waitResult == WAIT_OBJECT_0) {
                break; // Stop event
            }
        } else {
            Sleep(100);
        }
    }

    self->m_scanning = false;

    // PostMessage zamiast bezpośredniego callbacka!
    if (self->m_callbackHwnd) {
        PostMessageW(self->m_callbackHwnd, WM_BLE_SCAN_COMPLETE, 0, 0);
    }
    return 0;
}

DWORD WINAPI BLE::connectionThreadWrapperStatic(LPVOID param) {
    ((BLE*)param)->connectionThreadFunction();
    return 0;
}

// =====================================================================
// UUID settery
// =====================================================================
void BLE::setServiceUUID(const std::wstring& uuid) {
    m_serviceUUID = uuid;
}

void BLE::setNotifyCharacteristicUUID(const std::wstring& uuid) {
    m_notifyCharUUID = uuid;
}

void BLE::setWriteCharacteristicUUID(const std::wstring& uuid) {
    m_writeCharUUID = uuid;
}
