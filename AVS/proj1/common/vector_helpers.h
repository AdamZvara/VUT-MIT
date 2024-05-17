/**
 * @file    vector_helpers.h
 *
 * @authors Filip Vaverka <ivaverka@fit.vutbr.cz>
 *          Vojtech Mrazek <mrazek@fit.vutbr.cz>
 *
 * @brief   Helper types for calculating duration
 *
 * @date    06 November 2020, 11:07
 **/

#ifndef VECTOR_HELPERS_H
#define VECTOR_HELPERS_H

#include <chrono>


#define VECTOR_HELPERS_H

typedef std::chrono::steady_clock PerfClock_t;


template<typename T>
std::chrono::milliseconds PerfClockDurationMs(const T &dur) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(dur);
}

#endif // VECTOR_HELPERS_H
