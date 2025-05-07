#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <map>
#include <vector>

/**
 * @brief Klasa do zarządzania konfiguracją aplikacji
 * 
 * Pozwala na zapisywanie i odczytywanie ustawień konfiguracyjnych
 * z pliku w formacie klucz=wartość.
 */
class ConfigManager {
public:
    /**
     * @brief Konstruktor
     * 
     * @param configFilePath Ścieżka do pliku konfiguracyjnego
     */
    ConfigManager(const std::string& configFilePath = "config.ini");
    
    /**
     * @brief Destruktor
     */
    ~ConfigManager();
    
    /**
     * @brief Zapisuje wartość dla podanego klucza
     * 
     * @param key Klucz
     * @param value Wartość
     */
    void setValue(const std::string& key, const std::string& value);
    
    /**
     * @brief Odczytuje wartość dla podanego klucza
     * 
     * @param key Klucz
     * @param defaultValue Domyślna wartość, jeśli klucz nie istnieje
     * @return Wartość powiązana z kluczem lub wartość domyślna
     */
    std::string getValue(const std::string& key, const std::string& defaultValue = "") const;
    
    /**
     * @brief Sprawdza, czy klucz istnieje
     * 
     * @param key Klucz
     * @return true, jeśli klucz istnieje, false w przeciwnym razie
     */
    bool hasKey(const std::string& key) const;
    
    /**
     * @brief Usuwa klucz i powiązaną wartość
     * 
     * @param key Klucz
     */
    void removeKey(const std::string& key);
    
    /**
     * @brief Zapisuje konfigurację do pliku
     * 
     * @return true, jeśli zapis się powiódł, false w przeciwnym razie
     */
    bool saveToFile();
    
    /**
     * @brief Ładuje konfigurację z pliku
     * 
     * @return true, jeśli odczyt się powiódł, false w przeciwnym razie
     */
    bool loadFromFile();
    
    /**
     * @brief Ustawia ścieżkę do pliku konfiguracyjnego
     * 
     * @param configFilePath Nowa ścieżka do pliku konfiguracyjnego
     */
    void setConfigFilePath(const std::string& configFilePath);
    
    /**
     * @brief Pobiera listę wszystkich kluczy
     * 
     * @return Lista kluczy
     */
    std::vector<std::string> getKeys() const;

private:
    std::string m_configFilePath;
    std::map<std::string, std::string> m_configValues;
};

#endif // CONFIG_MANAGER_H