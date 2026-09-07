// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

#include "TCP.h"
#include "../../Util/StringUtils.h"

int TCP::s_wsaRefCount = 0;

TCP::TCP() : m_socket(INVALID_SOCKET), m_connected(false), m_port(0),
             m_onConnectCallback(nullptr), m_onDisconnectCallback(nullptr),
             m_onReceiveCallback(nullptr), m_onErrorCallback(nullptr),
             m_readThread(NULL), m_stopReadThread(false), m_connectionLost(false) {
}

TCP::~TCP() {
    disconnect();
}

bool TCP::acquireWinsock() {
    if (s_wsaRefCount > 0) {
        s_wsaRefCount++;
        return true;
    }

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        MessageBoxW(NULL, (L"WSAStartup nie powiodło się! Kod błędu: " + jqb_compat::to_wstring(result)).c_str(),
                    L"Błąd połączenia TCP", MB_ICONERROR);
        return false;
    }

    s_wsaRefCount = 1;
    return true;
}

void TCP::releaseWinsock() {
    if (s_wsaRefCount <= 0) return;
    s_wsaRefCount--;
    if (s_wsaRefCount == 0) {
        WSACleanup();
    }
}

bool TCP::connect(const std::string& host, unsigned short port) {
    // Zamknij połączenie, jeśli jest już otwarte
    if (m_socket != INVALID_SOCKET) {
        disconnect();
    }

    if (!acquireWinsock()) {
        return false;
    }

    char portStr[8];
    _snprintf(portStr, sizeof(portStr), "%u", port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo* addrResult = NULL;
    int gaiResult = getaddrinfo(host.c_str(), portStr, &hints, &addrResult);
    if (gaiResult != 0) {
        MessageBoxW(NULL, (L"Nie udało się rozwiązać adresu " + StringUtils::utf8ToWide(host) +
                            L"! Kod błędu: " + jqb_compat::to_wstring(gaiResult)).c_str(),
                    L"Błąd połączenia TCP", MB_ICONERROR);
        releaseWinsock();
        return false;
    }

    SOCKET sock = INVALID_SOCKET;
    for (struct addrinfo* addr = addrResult; addr != NULL; addr = addr->ai_next) {
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock == INVALID_SOCKET) continue;

        if (::connect(sock, addr->ai_addr, (int)addr->ai_addrlen) == 0) {
            break;  // Połączono
        }

        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    freeaddrinfo(addrResult);

    if (sock == INVALID_SOCKET) {
        int error = WSAGetLastError();
        MessageBoxW(NULL, (L"Nie udało się połączyć z " + StringUtils::utf8ToWide(host) + L":" +
                            jqb_compat::to_wstring((unsigned long)port) +
                            L"! Kod błędu: " + jqb_compat::to_wstring(error)).c_str(),
                    L"Błąd połączenia TCP", MB_ICONERROR);
        releaseWinsock();
        return false;
    }

    // Wyłącz algorytm Nagle'a — mniejsze opóźnienia przy przezroczystym mostkowaniu
    BOOL noDelay = TRUE;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&noDelay, sizeof(noDelay));

    m_socket = sock;
    m_host = host;
    m_port = port;
    m_connected = true;
    m_connectionLost = false;

    // Uruchom wątek odczytu danych
    m_stopReadThread = false;
    m_readThread = CreateThread(NULL, 0, TCP::readThreadWrapper, this, 0, NULL);

    if (m_onConnectCallback) {
        m_onConnectCallback();
    }

    return true;
}

void TCP::stopReadThread() {
    if (m_readThread != NULL) {
        m_stopReadThread = true;

        // Odblokuj recv() w wątku odczytu
        if (m_socket != INVALID_SOCKET) {
            shutdown(m_socket, SD_BOTH);
        }

        DWORD waitResult = WaitForSingleObject(m_readThread, 2000);
        if (waitResult == WAIT_TIMEOUT) {
            OutputDebugStringA("TCP::stopReadThread - Timeout, terminating thread\n");
            TerminateThread(m_readThread, 0);
        }

        CloseHandle(m_readThread);
        m_readThread = NULL;
    }
}

void TCP::disconnect() {
    stopReadThread();

    if (m_socket != INVALID_SOCKET) {
        if (m_connected && m_onDisconnectCallback) {
            m_onDisconnectCallback();
        }

        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        releaseWinsock();
    }

    m_connected = false;
    m_connectionLost = false;
}

bool TCP::write(const std::vector<uint8_t>& data) {
    if (m_socket == INVALID_SOCKET || !m_connected) {
        return false;
    }

    size_t totalSent = 0;
    while (totalSent < data.size()) {
        int sent = ::send(m_socket, (const char*)data.data() + totalSent,
                           (int)(data.size() - totalSent), 0);
        if (sent == SOCKET_ERROR) {
            DWORD error = WSAGetLastError();
            char buf[128];
            snprintf(buf, sizeof(buf), "TCP::write - send failed, error=%lu\n", error);
            OutputDebugStringA(buf);
            return false;
        }
        totalSent += (size_t)sent;
    }

    return true;
}

bool TCP::send(const std::vector<uint8_t>& data) {
    // Wrapper dla metody write, może być w przyszłości rozbudowany
    return write(data);
}

void TCP::onConnect(std::function<void()> callback) {
    m_onConnectCallback = callback;
}

void TCP::onDisconnect(std::function<void()> callback) {
    m_onDisconnectCallback = callback;
}

void TCP::onReceive(std::function<void(const std::vector<uint8_t>&)> callback) {
    m_onReceiveCallback = callback;
}

void TCP::onError(std::function<void()> callback) {
    m_onErrorCallback = callback;
}

// Funkcja wątku odczytu danych
void TCP::readThreadFunction() {
    const size_t bufferSize = 4096;
    std::vector<uint8_t> buffer(bufferSize);

    while (!m_stopReadThread) {
        int bytesRead = recv(m_socket, (char*)buffer.data(), (int)bufferSize, 0);

        if (bytesRead > 0) {
            if (m_onReceiveCallback) {
                std::vector<uint8_t> received(buffer.begin(), buffer.begin() + bytesRead);
                m_onReceiveCallback(received);
            }
        } else if (bytesRead == 0) {
            // Serwer zamknął połączenie
            if (!m_stopReadThread) {
                OutputDebugStringA("TCP: Connection closed by server\n");
                m_connectionLost = true;
                m_connected = false;
                if (m_onErrorCallback) {
                    m_onErrorCallback();
                }
            }
            break;
        } else {
            // recv() zwrócił błąd — jeśli to celowe rozłączenie (shutdown), po prostu wyjdź
            if (m_stopReadThread) {
                break;
            }

            DWORD error = WSAGetLastError();
            char buf[128];
            snprintf(buf, sizeof(buf), "TCP: recv error=%lu\n", error);
            OutputDebugStringA(buf);

            m_connectionLost = true;
            m_connected = false;
            if (m_onErrorCallback) {
                m_onErrorCallback();
            }
            break;
        }
    }
}

DWORD WINAPI TCP::readThreadWrapper(LPVOID param) {
    ((TCP*)param)->readThreadFunction();
    return 0;
}
