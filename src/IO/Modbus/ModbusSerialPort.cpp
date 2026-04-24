// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib
#include "ModbusSerialPort.h"
#include <algorithm>

namespace modbus {

std::vector<std::string> ModbusSerialPort::enumPorts() {
    std::vector<std::string> result;
    const DWORD bufSize = 64 * 1024;
    std::vector<char> buf(bufSize, 0);

    DWORD len = QueryDosDeviceA(NULL, buf.data(), bufSize);
    if (len == 0) return result;

    const char* p = buf.data();
    while (*p) {
        size_t l = strlen(p);
        if (l > 3 && (p[0] == 'C' || p[0] == 'c') &&
                     (p[1] == 'O' || p[1] == 'o') &&
                     (p[2] == 'M' || p[2] == 'm')) {
            bool numeric = true;
            for (size_t i = 3; i < l; ++i)
                if (p[i] < '0' || p[i] > '9') { numeric = false; break; }
            if (numeric) result.emplace_back(p, l);
        }
        p += l + 1;
    }

    std::sort(result.begin(), result.end(), [](const std::string& a, const std::string& b){
        return atoi(a.c_str() + 3) < atoi(b.c_str() + 3);
    });
    return result;
}

bool ModbusSerialPort::open(const SerialConfig& cfg, std::wstring* err) {
    close();

    std::string path = std::string("\\\\.\\") + cfg.port;
    m_handle = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                           0, NULL, OPEN_EXISTING, 0, NULL);
    if (m_handle == INVALID_HANDLE_VALUE) {
        if (err) {
            wchar_t buf[128];
            wsprintfW(buf, L"CreateFile failed (err=%lu)", GetLastError());
            *err = buf;
        }
        return false;
    }

    DCB dcb;
    ZeroMemory(&dcb, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(m_handle, &dcb)) {
        if (err) *err = L"GetCommState failed";
        close();
        return false;
    }

    dcb.BaudRate = cfg.baud;
    dcb.ByteSize = cfg.dataBits;
    dcb.fBinary  = TRUE;
    switch (cfg.parity) {
        case Parity::None: dcb.Parity = NOPARITY;   dcb.fParity = FALSE; break;
        case Parity::Odd:  dcb.Parity = ODDPARITY;  dcb.fParity = TRUE;  break;
        case Parity::Even: dcb.Parity = EVENPARITY; dcb.fParity = TRUE;  break;
    }
    dcb.StopBits = (cfg.stopBits == StopBits::Two) ? TWOSTOPBITS : ONESTOPBIT;

    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl  = DTR_CONTROL_ENABLE;
    dcb.fRtsControl  = RTS_CONTROL_ENABLE;
    dcb.fOutX = FALSE;
    dcb.fInX  = FALSE;
    dcb.fNull = FALSE;
    dcb.fAbortOnError = FALSE;

    if (!SetCommState(m_handle, &dcb)) {
        if (err) *err = L"SetCommState failed";
        close();
        return false;
    }

    m_readTimeoutMs = cfg.readTimeoutMs;
    setReadTimeout(m_readTimeoutMs);

    PurgeComm(m_handle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
    return true;
}

void ModbusSerialPort::close() {
    if (m_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

void ModbusSerialPort::setReadTimeout(DWORD ms) {
    m_readTimeoutMs = ms;
    if (m_handle == INVALID_HANDLE_VALUE) return;
    COMMTIMEOUTS to;
    ZeroMemory(&to, sizeof(to));
    to.ReadIntervalTimeout         = MAXDWORD;
    to.ReadTotalTimeoutMultiplier  = MAXDWORD;
    to.ReadTotalTimeoutConstant    = ms;
    to.WriteTotalTimeoutMultiplier = 10;
    to.WriteTotalTimeoutConstant   = 200;
    SetCommTimeouts(m_handle, &to);
}

void ModbusSerialPort::purge() {
    if (m_handle == INVALID_HANDLE_VALUE) return;
    PurgeComm(m_handle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
}

bool ModbusSerialPort::write(const uint8_t* data, size_t n) {
    if (m_handle == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    if (!WriteFile(m_handle, data, (DWORD)n, &written, NULL)) return false;
    return written == n;
}

int ModbusSerialPort::read(uint8_t* buf, size_t n) {
    if (m_handle == INVALID_HANDLE_VALUE) return -1;
    DWORD got = 0;
    if (!ReadFile(m_handle, buf, (DWORD)n, &got, NULL)) return -1;
    return (int)got;
}

bool ModbusSerialPort::readExact(uint8_t* buf, size_t n, DWORD totalTimeoutMs) {
    if (m_handle == INVALID_HANDLE_VALUE) return false;
    COMMTIMEOUTS to;
    ZeroMemory(&to, sizeof(to));
    to.ReadIntervalTimeout         = MAXDWORD;
    to.ReadTotalTimeoutMultiplier  = MAXDWORD;
    to.ReadTotalTimeoutConstant    = totalTimeoutMs;
    to.WriteTotalTimeoutMultiplier = 10;
    to.WriteTotalTimeoutConstant   = 200;
    SetCommTimeouts(m_handle, &to);

    size_t total = 0;
    DWORD start = GetTickCount();
    while (total < n) {
        DWORD got = 0;
        if (!ReadFile(m_handle, buf + total, (DWORD)(n - total), &got, NULL)) return false;
        total += got;
        if (total >= n) return true;
        DWORD elapsed = GetTickCount() - start;
        if (elapsed >= totalTimeoutMs) return false;
        to.ReadTotalTimeoutConstant = totalTimeoutMs - elapsed;
        SetCommTimeouts(m_handle, &to);
        if (got == 0) return false;
    }
    return true;
}

} // namespace modbus
