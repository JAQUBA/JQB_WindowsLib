/**
 * DataLogger.h — Generyczny rejestrator danych do pliku CSV
 *
 * Niezależny od formatu danych — przyjmuje kolumny jako vector<string>.
 * Automatycznie generuje nazwy plików z timestampem.
 */

#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <string>
#include <vector>
#include <cstdio>
#include <ctime>

class DataLogger {
public:
    /**
     * @param filenamePrefix Prefiks nazwy auto-generowanego pliku (np. "owon")
     */
    DataLogger(const std::string& filenamePrefix = "log");
    ~DataLogger();

    /**
     * Rozpoczyna nagrywanie.
     * @param headers Nagłówki kolumn CSV
     * @param filename Ścieżka pliku CSV (pusty → auto-generowana)
     * @return true jeśli plik otwarto poprawnie
     */
    bool startRecording(const std::vector<std::string>& headers,
                        const std::string& filename = "");

    /** Zatrzymuje nagrywanie i zamyka plik */
    void stopRecording();

    /** Dodaje jeden wiersz danych (kolumny muszą odpowiadać nagłówkom) */
    void addRow(const std::vector<std::string>& columns);

    bool isRecording()    const { return m_recording; }
    int  getSampleCount() const { return m_sampleCount; }
    const std::string& getFilename() const { return m_filename; }

    /** Czas nagrywania (sekundy) */
    int getElapsedSeconds() const;

private:
    std::string m_filenamePrefix;
    FILE*       m_file        = nullptr;
    bool        m_recording   = false;
    int         m_sampleCount = 0;
    std::string m_filename;
    time_t      m_startTime   = 0;

    std::string generateFilename() const;
};

#endif // DATA_LOGGER_H
