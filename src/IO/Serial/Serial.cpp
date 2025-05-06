#include "Serial.h"
#include "../../Util/StringUtils.h"
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>

Serial::Serial() : m_serialHandle(INVALID_HANDLE_VALUE), m_connected(false), 
                   m_onConnectCallback(nullptr), m_onDisconnectCallback(nullptr),
                   m_onReceiveCallback(nullptr), m_stopReadThread(false) {
}

Serial::~Serial() {
    disconnect();
}

bool Serial::init() {
    updateComPorts();
    return true;
}

bool Serial::connect() {
    // Sprawdź, czy wybrano port
    if (m_portName.empty()) {
        MessageBoxW(NULL, L"Nie wybrano portu COM! Wybierz port z listy przed próbą połączenia.", L"Błąd połączenia", MB_ICONERROR);
        return false;
    }

    // Zamknij port, jeśli jest już otwarty
    if (m_serialHandle != INVALID_HANDLE_VALUE) {
        disconnect();
    }

    // Otwórz port COM
    std::string portString = std::string("\\\\.\\") + m_portName;
    m_serialHandle = CreateFileA(
        portString.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (m_serialHandle == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        std::wstring errorMsg = L"Nie udało się otworzyć portu " + StringUtils::utf8ToWide(m_portName) + L"!\n";
        
        // Dodanie szczegółowego opisu błędu
        if (error == ERROR_FILE_NOT_FOUND) {
            errorMsg += L"Port nie istnieje lub jest niedostępny.";
        } else if (error == ERROR_ACCESS_DENIED) {
            errorMsg += L"Dostęp zabroniony. Port może być już używany przez inną aplikację.";
        } else {
            errorMsg += L"Kod błędu: " + std::to_wstring(error);
        }
        
        MessageBoxW(NULL, errorMsg.c_str(), L"Błąd połączenia", MB_ICONERROR);
        return false;
    }

    // Konfiguracja parametrów portu
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(m_serialHandle, &dcbSerialParams)) {
        DWORD error = GetLastError();
        MessageBoxW(NULL, (L"Błąd podczas pobierania stanu portu COM! Kod błędu: " + std::to_wstring(error)).c_str(), L"Błąd połączenia", MB_ICONERROR);
        CloseHandle(m_serialHandle);
        m_serialHandle = INVALID_HANDLE_VALUE;
        return false;
    }

    // Konfiguracja stałych port COM dla urządzenia OWON OW18B
    dcbSerialParams.BaudRate = CBR_9600;  // 9600 bps
    dcbSerialParams.ByteSize = 8;         // 8 bitów danych
    dcbSerialParams.StopBits = ONESTOPBIT;// 1 bit stopu
    dcbSerialParams.Parity = NOPARITY;    // Brak parzystości
    
    // Dodatkowe ustawienia dla lepszej stabilności
    dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;  // Sygnał DTR włączony
    dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE;  // Sygnał RTS włączony

    if (!SetCommState(m_serialHandle, &dcbSerialParams)) {
        DWORD error = GetLastError();
        MessageBoxW(NULL, (L"Błąd podczas konfiguracji portu COM! Kod błędu: " + std::to_wstring(error)).c_str(), L"Błąd połączenia", MB_ICONERROR);
        CloseHandle(m_serialHandle);
        m_serialHandle = INVALID_HANDLE_VALUE;
        return false;
    }

    // Konfiguracja timeoutów
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(m_serialHandle, &timeouts)) {
        DWORD error = GetLastError();
        MessageBoxW(NULL, (L"Błąd podczas konfiguracji timeoutów! Kod błędu: " + std::to_wstring(error)).c_str(), L"Błąd połączenia", MB_ICONERROR);
        CloseHandle(m_serialHandle);
        m_serialHandle = INVALID_HANDLE_VALUE;
        return false;
    }

    // Czyszczenie buforów po otwarciu portu
    PurgeComm(m_serialHandle, PURGE_TXCLEAR | PURGE_RXCLEAR);

    m_connected = true;
    
    // Uruchom wątek odczytu danych
    m_stopReadThread = false;
    m_readThread = std::thread(&Serial::readThreadFunction, this);
    
    // Wywołanie callbacku onConnect, jeśli został zdefiniowany
    if (m_onConnectCallback) {
        m_onConnectCallback();
    }
    
    return true;
}

void Serial::disconnect() {
    // Zatrzymaj wątek odczytu, jeśli działa
    if (m_readThread.joinable()) {
        m_stopReadThread = true;
        m_readThread.join();
    }
    
    if (m_serialHandle != INVALID_HANDLE_VALUE) {
        // Wywołanie callbacku onDisconnect przed zamknięciem portu, jeśli został zdefiniowany
        if (m_connected && m_onDisconnectCallback) {
            m_onDisconnectCallback();
        }
        
        CloseHandle(m_serialHandle);
        m_serialHandle = INVALID_HANDLE_VALUE;
    }
    m_connected = false;
}

void Serial::setPort(const char* portName) {
    m_portName = portName;
}

void Serial::updateComPorts() {
    m_availablePorts.clear();

    // Użyj setupapi do znalezienia dostępnych portów COM
    HDEVINFO hDevInfo = SetupDiGetClassDevsA(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        return;
    }

    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    // Iteracja po wszystkich dostępnych portach
    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); ++i) {
        char friendlyName[256] = { 0 };
        DWORD propertyType = 0;
        DWORD requiredSize = 0;

        // Pobierz przyjazną nazwę urządzenia
        SetupDiGetDeviceRegistryPropertyA(
            hDevInfo, &devInfoData, SPDRP_FRIENDLYNAME,
            &propertyType, (BYTE*)friendlyName, sizeof(friendlyName), &requiredSize);

        // Sprawdź, czy to jest port COM
        if (strstr(friendlyName, "COM") != NULL) {
            // Wyodrębnij tylko numer portu COM
            std::string comPort = StringUtils::extractComPort(friendlyName);
            if (!comPort.empty()) {
                // Dodaj port do listy
                m_availablePorts.push_back(comPort);
            }
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
}

bool Serial::write(const std::vector<uint8_t>& data) {
    if (m_serialHandle == INVALID_HANDLE_VALUE || !m_connected) {
        return false;
    }

    DWORD bytesWritten = 0;
    BOOL result = WriteFile(m_serialHandle, data.data(), data.size(), &bytesWritten, NULL);
    
    return (result && bytesWritten == data.size());
}

bool Serial::read(std::vector<uint8_t>& data, size_t bytesToRead) {
    if (m_serialHandle == INVALID_HANDLE_VALUE || !m_connected) {
        return false;
    }

    data.resize(bytesToRead);
    DWORD bytesRead = 0;
    BOOL result = ReadFile(m_serialHandle, data.data(), bytesToRead, &bytesRead, NULL);
    
    if (result && bytesRead > 0) {
        data.resize(bytesRead);
        return true;
    }
    
    data.clear();
    return false;
}

// Implementacja nowych metod

void Serial::onConnect(std::function<void()> callback) {
    m_onConnectCallback = callback;
}

void Serial::onDisconnect(std::function<void()> callback) {
    m_onDisconnectCallback = callback;
}

void Serial::onReceive(std::function<void(const std::vector<uint8_t>&)> callback) {
    m_onReceiveCallback = callback;
}

bool Serial::send(const std::vector<uint8_t>& data) {
    // Wrapper dla metody write, może być w przyszłości rozbudowany
    return write(data);
}

// Funkcja wątku odczytu danych
void Serial::readThreadFunction() {
    const size_t bufferSize = 256;
    std::vector<uint8_t> buffer;
    int consecutiveErrors = 0;
    const int maxConsecutiveErrors = 10;
    
    while (!m_stopReadThread) {
        try {
            // Sprawdź, czy port jest otwarty
            if (m_serialHandle != INVALID_HANDLE_VALUE && m_connected) {
                // Sprawdź dostępne dane przed odczytem
                DWORD errors;
                COMSTAT stat;
                if (ClearCommError(m_serialHandle, &errors, &stat) && stat.cbInQue > 0) {
                    // Ograniczenie rozmiaru bufora do dostępnych danych
                    size_t bytesToRead = std::min(bufferSize, static_cast<size_t>(stat.cbInQue));
                    
                    // Spróbuj odczytać dane
                    if (read(buffer, bytesToRead) && !buffer.empty()) {
                        // Jeśli zarejestrowano callback, wywołaj go z odczytanymi danymi
                        if (m_onReceiveCallback) {
                            m_onReceiveCallback(buffer);
                        }
                        consecutiveErrors = 0; // Zresetuj licznik błędów po udanym odczycie
                    }
                } else if (errors > 0) {
                    // Obsługa błędów komunikacyjnych
                    consecutiveErrors++;
                    
                    if (consecutiveErrors >= maxConsecutiveErrors) {
                        // Po zbyt wielu błędach, zamknij i ponownie otwórz port
                        disconnect();
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        connect();
                        consecutiveErrors = 0;
                    }
                }
            }
        }
        catch (const std::exception&) {
            // Obsługa wyjątków
            consecutiveErrors++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Krótkie opóźnienie, aby nie obciążać procesora
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}