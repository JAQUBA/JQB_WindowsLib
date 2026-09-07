// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

#ifndef TCP_H
#define TCP_H

// Must be defined/included before <windows.h> (pulled in by Core.h) to avoid
// winsock.h / winsock2.h redefinition clashes. Include this header first in any
// translation unit that also uses other JQB_WindowsLib modules.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include "Core.h"
#include <string>
#include <vector>
#include <functional>

class TCP {
public:
    TCP();
    ~TCP();

    bool connect(const std::string& host, unsigned short port);
    void disconnect();
    bool isConnected() const { return m_connected; }

    bool write(const std::vector<uint8_t>& data);
    bool send(const std::vector<uint8_t>& data);  // Alias for write()

    // Funkcje obsługi zdarzeń
    void onConnect(std::function<void()> callback);
    void onDisconnect(std::function<void()> callback);
    void onReceive(std::function<void(const std::vector<uint8_t>&)> callback);
    void onError(std::function<void()> callback);  // Callback błędu połączenia

    // Sprawdza czy połączenie zostało utracone (do wywoływania w loop)
    bool isConnectionLost() const { return m_connectionLost; }
    void clearConnectionLost() { m_connectionLost = false; }

private:
    SOCKET m_socket;
    bool m_connected;
    std::string m_host;
    unsigned short m_port;

    // Callbacki
    std::function<void()> m_onConnectCallback;
    std::function<void()> m_onDisconnectCallback;
    std::function<void(const std::vector<uint8_t>&)> m_onReceiveCallback;
    std::function<void()> m_onErrorCallback;

    // Wątek odczytu danych (Windows API zamiast std::thread)
    HANDLE m_readThread;
    volatile bool m_stopReadThread;
    volatile bool m_connectionLost;
    void readThreadFunction();
    static DWORD WINAPI readThreadWrapper(LPVOID param);
    void stopReadThread();  // Bezpieczne zatrzymanie wątku (odblokowuje recv() przez shutdown())

    // WSAStartup/WSACleanup — zliczane referencyjnie (może istnieć wiele instancji TCP)
    static int s_wsaRefCount;
    static bool acquireWinsock();
    static void releaseWinsock();
};

#endif // TCP_H
