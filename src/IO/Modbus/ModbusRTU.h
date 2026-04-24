// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib
//
// Synchronous Modbus RTU master built on ModbusSerialPort.
// Supports FC01/02/03/04/05/06/15/16 with exception code handling and CRC check.
#ifndef JQB_MODBUS_RTU_H
#define JQB_MODBUS_RTU_H

#include "ModbusSerialPort.h"
#include <vector>
#include <cstdint>
#include <string>

namespace modbus {

enum class Status {
    Ok,
    NotConnected,
    WriteFailed,
    Timeout,
    CrcError,
    InvalidResponse,
    ExceptionCode,
};

struct Result {
    Status status = Status::Ok;
    uint8_t exceptionCode = 0;
    std::wstring message;
    bool ok() const { return status == Status::Ok; }
};

namespace FC {
    constexpr uint8_t ReadCoils              = 0x01;
    constexpr uint8_t ReadDiscreteInputs     = 0x02;
    constexpr uint8_t ReadHoldingRegisters   = 0x03;
    constexpr uint8_t ReadInputRegisters     = 0x04;
    constexpr uint8_t WriteSingleCoil        = 0x05;
    constexpr uint8_t WriteSingleRegister    = 0x06;
    constexpr uint8_t WriteMultipleCoils     = 0x0F;
    constexpr uint8_t WriteMultipleRegisters = 0x10;
}

class RtuMaster {
public:
    explicit RtuMaster(ModbusSerialPort& port) : m_port(port) {}

    void  setTimeout(DWORD ms) { m_timeoutMs = ms; }
    DWORD getTimeout() const   { return m_timeoutMs; }

    Result readHoldingRegisters(uint8_t slave, uint16_t addr, uint16_t count, std::vector<uint16_t>& out);
    Result readInputRegisters  (uint8_t slave, uint16_t addr, uint16_t count, std::vector<uint16_t>& out);
    Result readCoils           (uint8_t slave, uint16_t addr, uint16_t count, std::vector<bool>& out);
    Result readDiscreteInputs  (uint8_t slave, uint16_t addr, uint16_t count, std::vector<bool>& out);

    Result writeSingleRegister  (uint8_t slave, uint16_t addr, uint16_t value);
    Result writeSingleCoil      (uint8_t slave, uint16_t addr, bool value);
    Result writeMultipleRegisters(uint8_t slave, uint16_t addr, const std::vector<uint16_t>& values);
    Result writeMultipleCoils   (uint8_t slave, uint16_t addr, const std::vector<bool>& values);

    const std::vector<uint8_t>& lastTx() const { return m_lastTx; }
    const std::vector<uint8_t>& lastRx() const { return m_lastRx; }

private:
    Result transact(uint8_t slave, uint8_t fc,
                    const std::vector<uint8_t>& payload,
                    std::vector<uint8_t>& responsePayload,
                    size_t expectedRespLen);
    static void appendU16BE(std::vector<uint8_t>& v, uint16_t x);

    ModbusSerialPort& m_port;
    DWORD m_timeoutMs = 1000;
    std::vector<uint8_t> m_lastTx;
    std::vector<uint8_t> m_lastRx;
};

// Format hex bytes (e.g. "01 03 00 00 00 0A C5 CD") for logging.
std::wstring toHex(const std::vector<uint8_t>& v);

} // namespace modbus

#endif // JQB_MODBUS_RTU_H
