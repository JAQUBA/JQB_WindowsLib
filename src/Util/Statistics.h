/**
 * Statistics.h — Śledzenie statystyk pomiarów (MIN / MAX / AVG / PEAK)
 *
 * Header-only. Nie korzysta z std::thread — bezpieczne dla MinGW.org.
 */

#ifndef STATISTICS_H
#define STATISTICS_H

#include <cmath>
#include <cfloat>

struct Statistics {
    double min       =  DBL_MAX;
    double max       = -DBL_MAX;
    double sum       = 0.0;
    double peak      = 0.0;
    int    count     = 0;
    bool   hasData   = false;

    /** Dodaj nową próbkę */
    void addSample(double value) {
        if (std::isinf(value) || std::isnan(value)) return;
        if (value < min) min = value;
        if (value > max) max = value;
        sum += value;
        ++count;
        double absVal = std::fabs(value);
        if (absVal > peak) peak = absVal;
        hasData = true;
    }

    /** Średnia arytmetyczna */
    double getAvg() const {
        return (count > 0) ? (sum / count) : 0.0;
    }

    /** Resetuj wszystko */
    void reset() {
        min     =  DBL_MAX;
        max     = -DBL_MAX;
        sum     = 0.0;
        peak    = 0.0;
        count   = 0;
        hasData = false;
    }
};

#endif // STATISTICS_H
