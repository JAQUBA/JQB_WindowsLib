// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib
#include "ModbusRTU.h"
#include "ModbusCrc16.h"

namespace modbus {

void RtuMaster::appendU16BE(std::vector<uint8_t>& v, uint16_t x) {
    v.push_back(static_cast<uint8_t>((x >> 8) & 0xFF));
    v.push_back(static_cast<uint8_t>(x & 0xFF));
}

std::wstring toHex(const std::vector<uint8_t>& v) {
    std::wstring s;
    s.reserve(v.size() * 3);
    wchar_t buf[8];
    for (size_t i = 0; i < v.size(); ++i) {
        wsprintfW(buf, L"%02X", v[i]);
        s += buf;
        if (i + 1 < v.size()) s += L' ';
    }
    return s;
}

Result RtuMaster::transact(uint8_t slave, uint8_t fc,
                           const std::vector<uint8_t>& payload,
                           std::vector<uint8_t>& responsePayload,
                           size_t expectedRespLen) {
    Result r;
    if (!m_port.isOpen()) {
        r.status = Status::NotConnected;
        r.message = L"Port nie otwarty";
        return r;
    }

    std::vector<uint8_t> tx;
    tx.reserve(2 + payload.size() + 2);
    tx.push_back(slave);
    tx.push_back(fc);
    tx.insert(tx.end(), payload.begin(), payload.end());
    uint16_t crc = crc16(tx.data(), tx.size());
    tx.push_back(static_cast<uint8_t>(crc & 0xFF));
    tx.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
    m_lastTx = tx;
    m_lastRx.clear();

    m_port.purge();
    if (!m_port.write(tx.data(), tx.size())) {
        r.status = Status::WriteFailed;
        r.message = L"Blad zapisu do portu";
        return r;
    }

    uint8_t hdr[2];
    if (!m_port.readExact(hdr, 2, m_timeoutMs)) {
        r.status = Status::Timeout;
        r.message = L"Timeout (brak odpowiedzi)";
        return r;
    }
    m_lastRx.push_back(hdr[0]);
    m_lastRx.push_back(hdr[1]);

    if (hdr[0] != slave) {
        r.status = Status::InvalidResponse;
        r.message = L"Nieprawidlowy adres slave w odpowiedzi";
        return r;
    }

    if (hdr[1] == (fc | 0x80)) {
        uint8_t excAndCrc[3];
        if (!m_port.readExact(excAndCrc, 3, m_timeoutMs)) {
            r.status = Status::Timeout;
            r.message = L"Timeout w odpowiedzi exception";
            return r;
        }
        m_lastRx.insert(m_lastRx.end(), excAndCrc, excAndCrc + 3);
        uint16_t expCrc = crc16(m_lastRx.data(), 3);
        uint16_t gotCrc = static_cast<uint16_t>(excAndCrc[1]) |
                          (static_cast<uint16_t>(excAndCrc[2]) << 8);
        if (expCrc != gotCrc) {
            r.status = Status::CrcError;
            r.message = L"CRC nieprawidlowe (exception)";
            return r;
        }
        r.status = Status::ExceptionCode;
        r.exceptionCode = excAndCrc[0];
        wchar_t buf[64];
        wsprintfW(buf, L"Modbus exception 0x%02X", excAndCrc[0]);
        r.message = buf;
        return r;
    }

    if (hdr[1] != fc) {
        r.status = Status::InvalidResponse;
        r.message = L"Nieprawidlowy kod funkcji w odpowiedzi";
        return r;
    }

    size_t bodyLen = expectedRespLen;
    if (bodyLen == 0) {
        uint8_t bc;
        if (!m_port.readExact(&bc, 1, m_timeoutMs)) {
            r.status = Status::Timeout;
            r.message = L"Timeout przy odczycie byte count";
            return r;
        }
        m_lastRx.push_back(bc);
        bodyLen = bc;
        responsePayload.assign(1, bc);
        std::vector<uint8_t> tail(bodyLen);
        if (bodyLen > 0) {
            if (!m_port.readExact(tail.data(), bodyLen, m_timeoutMs)) {
                r.status = Status::Timeout;
                r.message = L"Timeout przy odczycie danych";
                return r;
            }
            m_lastRx.insert(m_lastRx.end(), tail.begin(), tail.end());
            responsePayload.insert(responsePayload.end(), tail.begin(), tail.end());
        }
    } else {
        std::vector<uint8_t> tail(bodyLen);
        if (!m_port.readExact(tail.data(), bodyLen, m_timeoutMs)) {
            r.status = Status::Timeout;
            r.message = L"Timeout przy odczycie odpowiedzi";
            return r;
        }
        m_lastRx.insert(m_lastRx.end(), tail.begin(), tail.end());
        responsePayload = tail;
    }

    uint8_t crcBuf[2];
    if (!m_port.readExact(crcBuf, 2, m_timeoutMs)) {
        r.status = Status::Timeout;
        r.message = L"Timeout przy odczycie CRC";
        return r;
    }
    m_lastRx.insert(m_lastRx.end(), crcBuf, crcBuf + 2);

    uint16_t expCrc = crc16(m_lastRx.data(), m_lastRx.size() - 2);
    uint16_t gotCrc = static_cast<uint16_t>(crcBuf[0]) |
                      (static_cast<uint16_t>(crcBuf[1]) << 8);
    if (expCrc != gotCrc) {
        r.status = Status::CrcError;
        r.message = L"CRC nieprawidlowe";
        return r;
    }
    return r;
}

static void unpackRegisters(const std::vector<uint8_t>& payload, std::vector<uint16_t>& out) {
    out.clear();
    if (payload.empty()) return;
    uint8_t bc = payload[0];
    size_t n = bc / 2;
    out.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        size_t off = 1 + i * 2;
        if (off + 1 >= payload.size()) break;
        uint16_t v = (static_cast<uint16_t>(payload[off]) << 8) | payload[off + 1];
        out.push_back(v);
    }
}

static void unpackBits(const std::vector<uint8_t>& payload, uint16_t count, std::vector<bool>& out) {
    out.clear();
    if (payload.empty()) return;
    out.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        size_t byteIdx = 1 + (i / 8);
        if (byteIdx >= payload.size()) { out.push_back(false); continue; }
        uint8_t mask = static_cast<uint8_t>(1u << (i % 8));
        out.push_back((payload[byteIdx] & mask) != 0);
    }
}

Result RtuMaster::readHoldingRegisters(uint8_t slave, uint16_t addr, uint16_t count, std::vector<uint16_t>& out) {
    std::vector<uint8_t> req; appendU16BE(req, addr); appendU16BE(req, count);
    std::vector<uint8_t> resp;
    Result r = transact(slave, FC::ReadHoldingRegisters, req, resp, 0);
    if (r.ok()) unpackRegisters(resp, out);
    return r;
}
Result RtuMaster::readInputRegisters(uint8_t slave, uint16_t addr, uint16_t count, std::vector<uint16_t>& out) {
    std::vector<uint8_t> req; appendU16BE(req, addr); appendU16BE(req, count);
    std::vector<uint8_t> resp;
    Result r = transact(slave, FC::ReadInputRegisters, req, resp, 0);
    if (r.ok()) unpackRegisters(resp, out);
    return r;
}
Result RtuMaster::readCoils(uint8_t slave, uint16_t addr, uint16_t count, std::vector<bool>& out) {
    std::vector<uint8_t> req; appendU16BE(req, addr); appendU16BE(req, count);
    std::vector<uint8_t> resp;
    Result r = transact(slave, FC::ReadCoils, req, resp, 0);
    if (r.ok()) unpackBits(resp, count, out);
    return r;
}
Result RtuMaster::readDiscreteInputs(uint8_t slave, uint16_t addr, uint16_t count, std::vector<bool>& out) {
    std::vector<uint8_t> req; appendU16BE(req, addr); appendU16BE(req, count);
    std::vector<uint8_t> resp;
    Result r = transact(slave, FC::ReadDiscreteInputs, req, resp, 0);
    if (r.ok()) unpackBits(resp, count, out);
    return r;
}

Result RtuMaster::writeSingleRegister(uint8_t slave, uint16_t addr, uint16_t value) {
    std::vector<uint8_t> req; appendU16BE(req, addr); appendU16BE(req, value);
    std::vector<uint8_t> resp;
    return transact(slave, FC::WriteSingleRegister, req, resp, 4);
}
Result RtuMaster::writeSingleCoil(uint8_t slave, uint16_t addr, bool value) {
    std::vector<uint8_t> req; appendU16BE(req, addr); appendU16BE(req, value ? 0xFF00 : 0x0000);
    std::vector<uint8_t> resp;
    return transact(slave, FC::WriteSingleCoil, req, resp, 4);
}
Result RtuMaster::writeMultipleRegisters(uint8_t slave, uint16_t addr,
                                         const std::vector<uint16_t>& values) {
    std::vector<uint8_t> req;
    appendU16BE(req, addr); appendU16BE(req, static_cast<uint16_t>(values.size()));
    req.push_back(static_cast<uint8_t>(values.size() * 2));
    for (uint16_t v : values) appendU16BE(req, v);
    std::vector<uint8_t> resp;
    return transact(slave, FC::WriteMultipleRegisters, req, resp, 4);
}
Result RtuMaster::writeMultipleCoils(uint8_t slave, uint16_t addr, const std::vector<bool>& values) {
    std::vector<uint8_t> req;
    appendU16BE(req, addr); appendU16BE(req, static_cast<uint16_t>(values.size()));
    uint8_t bc = static_cast<uint8_t>((values.size() + 7) / 8);
    req.push_back(bc);
    std::vector<uint8_t> bytes(bc, 0);
    for (size_t i = 0; i < values.size(); ++i)
        if (values[i]) bytes[i / 8] |= static_cast<uint8_t>(1u << (i % 8));
    req.insert(req.end(), bytes.begin(), bytes.end());
    std::vector<uint8_t> resp;
    return transact(slave, FC::WriteMultipleCoils, req, resp, 4);
}

} // namespace modbus
