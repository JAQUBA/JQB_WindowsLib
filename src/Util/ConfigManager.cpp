#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <Windows.h>
#include <ShlObj.h>

ConfigManager::ConfigManager(const std::string& configFilePath)
    : m_configFilePath(configFilePath) {
    // Automatycznie próbuj wczytać konfigurację przy tworzeniu obiektu
    loadFromFile();
}

ConfigManager::~ConfigManager() {
    // Zapisz konfigurację przy niszczeniu obiektu
    saveToFile();
}

void ConfigManager::setValue(const std::string& key, const std::string& value) {
    // Usuń białe znaki z kluczy
    std::string trimmedKey = key;
    trimmedKey.erase(0, trimmedKey.find_first_not_of(" \t\n\r\f\v"));
    trimmedKey.erase(trimmedKey.find_last_not_of(" \t\n\r\f\v") + 1);
    
    // Nie dopuszczamy pustych kluczy
    if (trimmedKey.empty()) {
        return;
    }
    
    m_configValues[trimmedKey] = value;
}

std::string ConfigManager::getValue(const std::string& key, const std::string& defaultValue) const {
    auto it = m_configValues.find(key);
    if (it != m_configValues.end()) {
        return it->second;
    }
    return defaultValue;
}

bool ConfigManager::hasKey(const std::string& key) const {
    return m_configValues.find(key) != m_configValues.end();
}

void ConfigManager::removeKey(const std::string& key) {
    m_configValues.erase(key);
}

bool ConfigManager::saveToFile() {
    std::ofstream file(m_configFilePath);
    if (!file.is_open()) {
        return false;
    }
    
    // Zapisujemy każdą parę klucz-wartość w osobnej linii
    for (const auto& pair : m_configValues) {
        file << pair.first << "=" << pair.second << std::endl;
    }
    
    file.close();
    return true;
}

bool ConfigManager::loadFromFile() {
    std::ifstream file(m_configFilePath);
    if (!file.is_open()) {
        return false;
    }
    
    m_configValues.clear();
    std::string line;
    
    // Wczytujemy każdą linię i dzielimy ją na klucz i wartość
    while (std::getline(file, line)) {
        // Pomijamy puste linie i komentarze
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Szukamy znaku '='
        size_t equalsPos = line.find('=');
        if (equalsPos != std::string::npos) {
            std::string key = line.substr(0, equalsPos);
            std::string value = line.substr(equalsPos + 1);
            
            // Usuwamy białe znaki z klucza i wartości
            key.erase(0, key.find_first_not_of(" \t\n\r\f\v"));
            key.erase(key.find_last_not_of(" \t\n\r\f\v") + 1);
            
            value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
            value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);
            
            // Jeśli klucz nie jest pusty, dodajemy parę do mapy
            if (!key.empty()) {
                m_configValues[key] = value;
            }
        }
    }
    
    file.close();
    return true;
}

void ConfigManager::setConfigFilePath(const std::string& configFilePath) {
    m_configFilePath = configFilePath;
}

std::vector<std::string> ConfigManager::getKeys() const {
    std::vector<std::string> keys;
    keys.reserve(m_configValues.size());
    
    for (const auto& pair : m_configValues) {
        keys.push_back(pair.first);
    }
    
    return keys;
}