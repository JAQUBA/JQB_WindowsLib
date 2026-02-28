/*
 * HID.cpp — USB HID communication (Windows)
 *
 * Part of JQB_WindowsLib.
 */

#include "HID.h"
#include <setupapi.h>
#include <cstring>

/* ------------------------------------------------------------------ */
/*  HIDDevice helpers                                                  */
/* ------------------------------------------------------------------ */

std::string HIDDevice::toString() const {
    char buf[128];
    snprintf(buf, sizeof(buf), "HID %04X:%04X  UsagePage=0x%04X Usage=0x%04X",
             vendorId, productId, usagePage, usage);
    return std::string(buf);
}

/* ------------------------------------------------------------------ */
/*  Constructor / Destructor                                           */
/* ------------------------------------------------------------------ */

HID::HID()
    : m_handle(INVALID_HANDLE_VALUE),
      m_hidDll(NULL),
      m_open(false),
      m_initialized(false),
      m_vid(0),
      m_pid(0),
      m_usagePage(0),
      m_usage(0),
      m_featureReportSize(7),
      pGetHidGuid(NULL),
      pGetAttributes(NULL),
      pGetFeature(NULL),
      pSetFeature(NULL),
      pGetPreparsedData(NULL),
      pFreePreparsedData(NULL),
      pGetCaps(NULL)
{
}

HID::~HID() {
    close();
    if (m_hidDll) {
        FreeLibrary(m_hidDll);
        m_hidDll = NULL;
    }
}

/* ------------------------------------------------------------------ */
/*  init — load hid.dll                                                */
/* ------------------------------------------------------------------ */

bool HID::init() {
    if (m_initialized) return true;

    m_hidDll = LoadLibraryA("hid.dll");
    if (!m_hidDll) {
        if (m_onErrorCallback) m_onErrorCallback("Failed to load hid.dll");
        return false;
    }

    pGetHidGuid        = (fn_HidD_GetHidGuid)       GetProcAddress(m_hidDll, "HidD_GetHidGuid");
    pGetAttributes     = (fn_HidD_GetAttributes)     GetProcAddress(m_hidDll, "HidD_GetAttributes");
    pGetFeature        = (fn_HidD_GetFeature)        GetProcAddress(m_hidDll, "HidD_GetFeature");
    pSetFeature        = (fn_HidD_SetFeature)        GetProcAddress(m_hidDll, "HidD_SetFeature");
    pGetPreparsedData  = (fn_HidD_GetPreparsedData)  GetProcAddress(m_hidDll, "HidD_GetPreparsedData");
    pFreePreparsedData = (fn_HidD_FreePreparsedData) GetProcAddress(m_hidDll, "HidD_FreePreparsedData");
    pGetCaps           = (fn_HidP_GetCaps)           GetProcAddress(m_hidDll, "HidP_GetCaps");

    if (!pGetHidGuid || !pGetAttributes || !pGetFeature || !pSetFeature ||
        !pGetPreparsedData || !pFreePreparsedData || !pGetCaps) {
        FreeLibrary(m_hidDll);
        m_hidDll = NULL;
        if (m_onErrorCallback) m_onErrorCallback("Failed to resolve hid.dll functions");
        return false;
    }

    m_initialized = true;
    return true;
}

/* ------------------------------------------------------------------ */
/*  Configuration setters                                              */
/* ------------------------------------------------------------------ */

void HID::setVidPid(uint16_t vid, uint16_t pid) {
    m_vid = vid;
    m_pid = pid;
}

void HID::setUsage(uint16_t usagePage, uint16_t usage) {
    m_usagePage = usagePage;
    m_usage     = usage;
}

void HID::setFeatureReportSize(size_t dataBytes) {
    m_featureReportSize = dataBytes;
}

/* ------------------------------------------------------------------ */
/*  Callback setters                                                   */
/* ------------------------------------------------------------------ */

void HID::onConnect(std::function<void()> callback) {
    m_onConnectCallback = callback;
}

void HID::onDisconnect(std::function<void()> callback) {
    m_onDisconnectCallback = callback;
}

void HID::onError(std::function<void(const std::string&)> callback) {
    m_onErrorCallback = callback;
}

/* ------------------------------------------------------------------ */
/*  matchDevice — check VID/PID and (optionally) UsagePage/Usage       */
/* ------------------------------------------------------------------ */

bool HID::matchDevice(HANDLE h, std::string* outPath) {
    /* Check VID / PID */
    HID_ATTRIBUTES attrs;
    attrs.Size = sizeof(HID_ATTRIBUTES);
    if (!pGetAttributes(h, &attrs)) return false;

    if (m_vid != 0 && attrs.VendorID  != m_vid)  return false;
    if (m_pid != 0 && attrs.ProductID != m_pid) return false;

    /* Check Usage Page / Usage if specified */
    if (m_usagePage != 0 || m_usage != 0) {
        void* preparsed = NULL;
        if (!pGetPreparsedData(h, &preparsed)) return false;

        HID_CAPS caps;
        memset(&caps, 0, sizeof(caps));
        LONG status = pGetCaps(preparsed, &caps);
        pFreePreparsedData(preparsed);

        /* HIDP_STATUS_SUCCESS = 0x00110000 */
        if (status != 0x00110000L) return false;

        if (m_usagePage != 0 && caps.UsagePage != (USHORT)m_usagePage) return false;
        if (m_usage     != 0 && caps.Usage     != (USHORT)m_usage)     return false;
    }

    return true;
}

/* ------------------------------------------------------------------ */
/*  findAndOpen — enumerate and open first matching device              */
/* ------------------------------------------------------------------ */

bool HID::findAndOpen() {
    close();

    if (!m_initialized) {
        if (m_onErrorCallback) m_onErrorCallback("HID not initialized — call init() first");
        return false;
    }

    GUID hidGuid;
    pGetHidGuid(&hidGuid);

    HDEVINFO devInfo = SetupDiGetClassDevsA(
        &hidGuid, NULL, NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (devInfo == INVALID_HANDLE_VALUE) {
        if (m_onErrorCallback) m_onErrorCallback("SetupDiGetClassDevs failed");
        return false;
    }

    SP_DEVICE_INTERFACE_DATA ifaceData;
    ifaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for (DWORD idx = 0;
         SetupDiEnumDeviceInterfaces(devInfo, NULL, &hidGuid, idx, &ifaceData);
         idx++)
    {
        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetailA(
            devInfo, &ifaceData, NULL, 0, &requiredSize, NULL);

        if (requiredSize == 0) continue;

        SP_DEVICE_INTERFACE_DETAIL_DATA_A* detail =
            (SP_DEVICE_INTERFACE_DETAIL_DATA_A*)malloc(requiredSize);
        if (!detail) continue;
        detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

        if (!SetupDiGetDeviceInterfaceDetailA(
                devInfo, &ifaceData, detail, requiredSize, NULL, NULL)) {
            free(detail);
            continue;
        }

        HANDLE h = CreateFileA(
            detail->DevicePath,
            0,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        std::string devPath = detail->DevicePath;
        free(detail);

        if (h == INVALID_HANDLE_VALUE) continue;

        if (matchDevice(h)) {
            m_handle = h;
            m_open   = true;
            SetupDiDestroyDeviceInfoList(devInfo);
            if (m_onConnectCallback) m_onConnectCallback();
            return true;
        }

        CloseHandle(h);
    }

    SetupDiDestroyDeviceInfoList(devInfo);
    return false;
}

/* ------------------------------------------------------------------ */
/*  scan — enumerate all matching devices without opening              */
/* ------------------------------------------------------------------ */

std::vector<HIDDevice> HID::scan() {
    std::vector<HIDDevice> result;

    if (!m_initialized) return result;

    GUID hidGuid;
    pGetHidGuid(&hidGuid);

    HDEVINFO devInfo = SetupDiGetClassDevsA(
        &hidGuid, NULL, NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (devInfo == INVALID_HANDLE_VALUE) return result;

    SP_DEVICE_INTERFACE_DATA ifaceData;
    ifaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for (DWORD idx = 0;
         SetupDiEnumDeviceInterfaces(devInfo, NULL, &hidGuid, idx, &ifaceData);
         idx++)
    {
        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetailA(
            devInfo, &ifaceData, NULL, 0, &requiredSize, NULL);

        if (requiredSize == 0) continue;

        SP_DEVICE_INTERFACE_DETAIL_DATA_A* detail =
            (SP_DEVICE_INTERFACE_DETAIL_DATA_A*)malloc(requiredSize);
        if (!detail) continue;
        detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

        if (!SetupDiGetDeviceInterfaceDetailA(
                devInfo, &ifaceData, detail, requiredSize, NULL, NULL)) {
            free(detail);
            continue;
        }

        HANDLE h = CreateFileA(
            detail->DevicePath,
            0,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        std::string devPath = detail->DevicePath;
        free(detail);

        if (h == INVALID_HANDLE_VALUE) continue;

        /* Get attributes */
        HID_ATTRIBUTES attrs;
        attrs.Size = sizeof(HID_ATTRIBUTES);
        if (!pGetAttributes(h, &attrs)) {
            CloseHandle(h);
            continue;
        }

        /* Filter by VID/PID if set */
        if (m_vid != 0 && attrs.VendorID  != m_vid)  { CloseHandle(h); continue; }
        if (m_pid != 0 && attrs.ProductID != m_pid) { CloseHandle(h); continue; }

        /* Get caps */
        void* preparsed = NULL;
        if (!pGetPreparsedData(h, &preparsed)) {
            CloseHandle(h);
            continue;
        }

        HID_CAPS caps;
        memset(&caps, 0, sizeof(caps));
        LONG status = pGetCaps(preparsed, &caps);
        pFreePreparsedData(preparsed);
        CloseHandle(h);

        if (status != 0x00110000L) continue;

        /* Filter by Usage if set */
        if (m_usagePage != 0 && caps.UsagePage != (USHORT)m_usagePage) continue;
        if (m_usage     != 0 && caps.Usage     != (USHORT)m_usage)     continue;

        HIDDevice dev;
        dev.vendorId      = attrs.VendorID;
        dev.productId     = attrs.ProductID;
        dev.versionNumber = attrs.VersionNumber;
        dev.usagePage     = caps.UsagePage;
        dev.usage         = caps.Usage;
        dev.path          = devPath;
        result.push_back(dev);
    }

    SetupDiDestroyDeviceInfoList(devInfo);
    return result;
}

/* ------------------------------------------------------------------ */
/*  open — open a specific device by path                              */
/* ------------------------------------------------------------------ */

bool HID::open(const std::string& devicePath) {
    close();

    if (!m_initialized) {
        if (m_onErrorCallback) m_onErrorCallback("HID not initialized — call init() first");
        return false;
    }

    HANDLE h = CreateFileA(
        devicePath.c_str(),
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (h == INVALID_HANDLE_VALUE) {
        if (m_onErrorCallback) m_onErrorCallback("Failed to open device: " + devicePath);
        return false;
    }

    m_handle = h;
    m_open   = true;
    if (m_onConnectCallback) m_onConnectCallback();
    return true;
}

/* ------------------------------------------------------------------ */
/*  close                                                              */
/* ------------------------------------------------------------------ */

void HID::close() {
    if (m_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
    if (m_open) {
        m_open = false;
        if (m_onDisconnectCallback) m_onDisconnectCallback();
    }
}

/* ------------------------------------------------------------------ */
/*  Feature Reports                                                    */
/* ------------------------------------------------------------------ */

bool HID::getFeatureReport(uint8_t reportId, uint8_t* data, size_t dataLen) {
    if (!m_open || !pGetFeature) {
        if (m_onErrorCallback) m_onErrorCallback("getFeatureReport: device not open");
        return false;
    }

    size_t bufSize = 1 + m_featureReportSize;
    uint8_t* buf = (uint8_t*)alloca(bufSize);
    memset(buf, 0, bufSize);
    buf[0] = reportId;

    if (!pGetFeature(m_handle, buf, (ULONG)bufSize)) {
        if (m_onErrorCallback) m_onErrorCallback("HidD_GetFeature failed");
        return false;
    }

    if (dataLen > m_featureReportSize) dataLen = m_featureReportSize;
    memcpy(data, buf + 1, dataLen);
    return true;
}

bool HID::setFeatureReport(uint8_t reportId, const uint8_t* data, size_t dataLen) {
    if (!m_open || !pSetFeature) {
        if (m_onErrorCallback) m_onErrorCallback("setFeatureReport: device not open");
        return false;
    }

    size_t bufSize = 1 + m_featureReportSize;
    uint8_t* buf = (uint8_t*)alloca(bufSize);
    memset(buf, 0, bufSize);
    buf[0] = reportId;

    if (dataLen > m_featureReportSize) dataLen = m_featureReportSize;
    memcpy(buf + 1, data, dataLen);

    if (!pSetFeature(m_handle, buf, (ULONG)bufSize)) {
        if (m_onErrorCallback) m_onErrorCallback("HidD_SetFeature failed");
        return false;
    }

    return true;
}
