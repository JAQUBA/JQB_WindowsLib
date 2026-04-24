// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib
//
// ModbusSerialPort — synchronous Win32 COM port wrapper.
//
// Independent of the higher-level JQB Serial class — provides full DCB control
// (parity, stop bits, data bits) and synchronous read with hard timeout, with
// no UI dialogs on errors. Designed for blocking master protocols
// (Modbus RTU, custom request/response).
#ifndef JQB_MODBUS_SERIAL_PORT_H
#define JQB_MODBUS_SERIAL_PORT_H

#include "Core.h"
#include <string>
#include <vector>
#include <cstdint>

namespace modbus {

enum class Parity   { None, Odd, Even };
enum class StopBits { One, Two };

struct SerialConfig {
    std::string port = "COM1";       // "COM1".."COM999"
    DWORD baud       = 9600;
    uint8_t dataBits = 8;            // 7 or 8
    Parity parity    = Parity::None;
    StopBits stopBits= StopBits::One;
    DWORD readTimeoutMs = 1000;      // total per-read timeout
};

class ModbusSerialPort {
public:
    ModbusSerialPort() = default;
    ~ModbusSerialPort() { close(); }

    // Enumerate currently available "COMx" ports via QueryDosDevice.
    static std::vector<std::string> enumPorts();

    bool open(const SerialConfig& cfg, std::wstring* err = nullptr);
    void close();
    bool isOpen() const { return m_handle != INVALID_HANDLE_VALUE; }

    void setReadTimeout(DWORD ms);
    void purge();

    bool write(const uint8_t* data, size_t n);
    int  read (uint8_t* buf, size_t n);                            // 0=timeout, -1=error
    bool readExact(uint8_t* buf, size_t n, DWORD totalTimeoutMs);

private:
    HANDLE m_handle = INVALID_HANDLE_VALUE;
    DWORD  m_readTimeoutMs = 1000;
};

} // namespace modbus

#endif // JQB_MODBUS_SERIAL_PORT_H
