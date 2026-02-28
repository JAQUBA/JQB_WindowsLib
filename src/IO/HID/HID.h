/*
 * HID.h — USB HID communication (Windows)
 *
 * Dynamically loads hid.dll at runtime so no import library is needed.
 * Uses SetupDI to enumerate HID devices by VID/PID and Usage Page,
 * then communicates via HidD_GetFeature / HidD_SetFeature.
 *
 * Part of JQB_WindowsLib.
 */

#ifndef HID_H
#define HID_H

#include "Core.h"
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

/* Forward declaration for setupapi types (loaded dynamically) */
#ifndef _SETUPAPI_H_
typedef PVOID HDEVINFO;
#endif

/* Manually defined HID structs (avoids dependency on hidsdi.h / ddk) */
typedef struct {
    ULONG  Size;
    USHORT VendorID;
    USHORT ProductID;
    USHORT VersionNumber;
} HID_ATTRIBUTES;

typedef struct {
    USHORT Usage;
    USHORT UsagePage;
    USHORT InputReportByteLength;
    USHORT OutputReportByteLength;
    USHORT FeatureReportByteLength;
    USHORT Reserved[17];
    USHORT NumberLinkCollectionNodes;
    USHORT NumberInputButtonCaps;
    USHORT NumberInputValueCaps;
    USHORT NumberInputDataIndices;
    USHORT NumberOutputButtonCaps;
    USHORT NumberOutputValueCaps;
    USHORT NumberOutputDataIndices;
    USHORT NumberFeatureButtonCaps;
    USHORT NumberFeatureValueCaps;
    USHORT NumberFeatureDataIndices;
} HID_CAPS;

/**
 * HID device information returned by scan().
 */
struct HIDDevice {
    uint16_t    vendorId;
    uint16_t    productId;
    uint16_t    versionNumber;
    uint16_t    usagePage;
    uint16_t    usage;
    std::string path;           // Device interface path (for internal use)

    std::string toString() const;
};

/**
 * USB HID communication class.
 *
 * Usage:
 *   HID hid;
 *   hid.init();
 *   hid.setVidPid(0x1209, 0xC55D);
 *   hid.setUsage(0xFF00, 0x01);
 *   hid.setFeatureReportSize(7);      // data bytes (without Report ID)
 *   if (hid.findAndOpen()) { ... }
 */
class HID {
public:
    HID();
    ~HID();

    /**
     * Load hid.dll function pointers. Must be called once before
     * any other method.
     * @return true on success.
     */
    bool init();

    /* ---- Configuration (call before findAndOpen) ---- */

    /** Set target VID and PID. */
    void setVidPid(uint16_t vid, uint16_t pid);

    /** Set target Usage Page and Usage to match. */
    void setUsage(uint16_t usagePage, uint16_t usage);

    /**
     * Set the Feature Report data size in bytes (without the Report ID byte).
     * Default: 7 bytes.
     */
    void setFeatureReportSize(size_t dataBytes);

    /* ---- Connection ---- */

    /**
     * Enumerate HID devices and find the first one matching VID/PID and
     * Usage Page/Usage. Opens the device with shared access.
     * @return true if device was found and opened.
     */
    bool findAndOpen();

    /**
     * Scan for all matching HID devices without opening them.
     * @return vector of found devices.
     */
    std::vector<HIDDevice> scan();

    /**
     * Open a specific device by its interface path.
     * @param devicePath  Device path from HIDDevice::path.
     * @return true on success.
     */
    bool open(const std::string& devicePath);

    /** Close the device handle. */
    void close();

    /** Is the device currently open? */
    bool isOpen() const { return m_open; }

    /* ---- Communication ---- */

    /**
     * Read a Feature Report from the device.
     * @param reportId  HID Report ID.
     * @param data      Output buffer (at least dataLen bytes).
     * @param dataLen   Number of data bytes to read.
     * @return true on success.
     */
    bool getFeatureReport(uint8_t reportId, uint8_t* data, size_t dataLen);

    /**
     * Write a Feature Report to the device.
     * @param reportId  HID Report ID.
     * @param data      Data to send.
     * @param dataLen   Number of data bytes.
     * @return true on success.
     */
    bool setFeatureReport(uint8_t reportId, const uint8_t* data, size_t dataLen);

    /* ---- Getters ---- */

    uint16_t getVendorId()  const { return m_vid; }
    uint16_t getProductId() const { return m_pid; }

    /* ---- Callbacks ---- */

    void onConnect(std::function<void()> callback);
    void onDisconnect(std::function<void()> callback);
    void onError(std::function<void(const std::string&)> callback);

private:
    HANDLE  m_handle;
    HMODULE m_hidDll;
    bool    m_open;
    bool    m_initialized;

    /* Target device parameters */
    uint16_t m_vid;
    uint16_t m_pid;
    uint16_t m_usagePage;
    uint16_t m_usage;
    size_t   m_featureReportSize;   // data bytes (without Report ID)

    /* Callbacks */
    std::function<void()>                   m_onConnectCallback;
    std::function<void()>                   m_onDisconnectCallback;
    std::function<void(const std::string&)> m_onErrorCallback;

    /* hid.dll function pointers */
    typedef void    (WINAPI *fn_HidD_GetHidGuid)(GUID*);
    typedef BOOLEAN (WINAPI *fn_HidD_GetAttributes)(HANDLE, HID_ATTRIBUTES*);
    typedef BOOLEAN (WINAPI *fn_HidD_GetFeature)(HANDLE, PVOID, ULONG);
    typedef BOOLEAN (WINAPI *fn_HidD_SetFeature)(HANDLE, PVOID, ULONG);
    typedef BOOLEAN (WINAPI *fn_HidD_GetPreparsedData)(HANDLE, void**);
    typedef BOOLEAN (WINAPI *fn_HidD_FreePreparsedData)(void*);
    typedef LONG    (WINAPI *fn_HidP_GetCaps)(void*, HID_CAPS*);

    fn_HidD_GetHidGuid          pGetHidGuid;
    fn_HidD_GetAttributes       pGetAttributes;
    fn_HidD_GetFeature          pGetFeature;
    fn_HidD_SetFeature          pSetFeature;
    fn_HidD_GetPreparsedData    pGetPreparsedData;
    fn_HidD_FreePreparsedData   pFreePreparsedData;
    fn_HidP_GetCaps             pGetCaps;

    /* setupapi.dll function pointers */
    HMODULE m_setupapiDll;

    typedef HDEVINFO (WINAPI *fn_SetupDiGetClassDevsA)(const GUID*, PCSTR, HWND, DWORD);
    typedef BOOL     (WINAPI *fn_SetupDiEnumDeviceInterfaces)(HDEVINFO, void*, const GUID*, DWORD, void*);
    typedef BOOL     (WINAPI *fn_SetupDiGetDeviceInterfaceDetailA)(HDEVINFO, void*, void*, DWORD, DWORD*, void*);
    typedef BOOL     (WINAPI *fn_SetupDiDestroyDeviceInfoList)(HDEVINFO);

    fn_SetupDiGetClassDevsA               pSetupDiGetClassDevsA;
    fn_SetupDiEnumDeviceInterfaces        pSetupDiEnumDeviceInterfaces;
    fn_SetupDiGetDeviceInterfaceDetailA   pSetupDiGetDeviceInterfaceDetailA;
    fn_SetupDiDestroyDeviceInfoList       pSetupDiDestroyDeviceInfoList;

    /* Internal helpers */
    bool matchDevice(HANDLE h, std::string* outPath = nullptr);
};

#endif /* HID_H */
