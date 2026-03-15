// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

/**
 * DataLogger.cpp — Implementacja generycznego rejestratora CSV
 */

#include "DataLogger.h"
#include <cstring>

DataLogger::DataLogger(const std::string& filenamePrefix)
    : m_filenamePrefix(filenamePrefix) {}

DataLogger::~DataLogger() {
    stopRecording();
}

std::string DataLogger::generateFilename() const {
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char buf[128];
    snprintf(buf, sizeof(buf), "%s_%04d-%02d-%02d_%02d-%02d-%02d.csv",
             m_filenamePrefix.c_str(),
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    return std::string(buf);
}

bool DataLogger::startRecording(const std::vector<std::string>& headers,
                                const std::string& filename)
{
    if (m_recording) stopRecording();

    m_filename = filename.empty() ? generateFilename() : filename;
    m_file = fopen(m_filename.c_str(), "w");
    if (!m_file) return false;

    m_recording   = true;
    m_sampleCount = 0;
    m_startTime   = time(nullptr);

    // Zapisz nagłówki
    for (size_t i = 0; i < headers.size(); ++i) {
        if (i > 0) fprintf(m_file, ",");
        fprintf(m_file, "%s", headers[i].c_str());
    }
    fprintf(m_file, "\n");
    fflush(m_file);

    return true;
}

void DataLogger::stopRecording() {
    if (m_file) {
        fflush(m_file);
        fclose(m_file);
        m_file = nullptr;
    }
    m_recording = false;
}

void DataLogger::addRow(const std::vector<std::string>& columns) {
    if (!m_recording || !m_file) return;

    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) fprintf(m_file, ",");
        fprintf(m_file, "%s", columns[i].c_str());
    }
    fprintf(m_file, "\n");
    fflush(m_file);
    ++m_sampleCount;
}

int DataLogger::getElapsedSeconds() const {
    if (!m_recording) return 0;
    return static_cast<int>(difftime(time(nullptr), m_startTime));
}
