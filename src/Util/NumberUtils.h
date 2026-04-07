#pragma once

#include <string>

namespace NumberUtils {

// Parse a double from a string, accepting both ',' and '.' as the decimal separator.
// Trims leading/trailing whitespace.  Throws std::invalid_argument on empty or malformed input.
double parseDouble(const std::string& s);

// Non-throwing version.  Fills `out` and returns true on success; leaves `out` unchanged on failure.
bool tryParseDouble(const std::string& s, double& out);

// Format a double with a fixed number of decimal places using locale-neutral '.' separator.
std::string formatDouble(double v, int decimals = 3);

} // namespace NumberUtils
