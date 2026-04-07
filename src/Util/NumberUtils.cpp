// ============================================================================
// NumberUtils.cpp — Locale-tolerant number parsing helpers
// ============================================================================
#include "NumberUtils.h"

#include <algorithm>
#include <cstdio>
#include <stdexcept>

namespace NumberUtils {

static std::string prepareSrc(const std::string& s) {
    // Trim whitespace
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return {};
    size_t end = s.find_last_not_of(" \t\r\n");
    std::string tmp = s.substr(start, end - start + 1);
    // Accept comma as decimal separator
    std::replace(tmp.begin(), tmp.end(), ',', '.');
    return tmp;
}

double parseDouble(const std::string& s) {
    const std::string tmp = prepareSrc(s);
    if (tmp.empty())
        throw std::invalid_argument("NumberUtils::parseDouble: empty input");
    // stod throws std::invalid_argument / std::out_of_range on failure
    return std::stod(tmp);
}

bool tryParseDouble(const std::string& s, double& out) {
    const std::string tmp = prepareSrc(s);
    if (tmp.empty()) return false;
    try {
        out = std::stod(tmp);
        return true;
    } catch (...) {
        return false;
    }
}

std::string formatDouble(double v, int decimals) {
    char fmt[16];
    std::snprintf(fmt, sizeof(fmt), "%%.%df", decimals);
    char buf[64];
    std::snprintf(buf, sizeof(buf), fmt, v);
    return buf;
}

} // namespace NumberUtils
