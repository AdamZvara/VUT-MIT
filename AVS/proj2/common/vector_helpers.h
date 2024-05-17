/**
 * @file    vector_helpers.h
 *
 * @authors Filip Vaverka <ivaverka@fit.vutbr.cz>
 *          Vojtech Mrazek <mrazek@fit.vutbr.cz>
 *
 * @brief   Helper types such as vector in 3D space, etc.
 *
 * @date    06 November 2020, 11:07
 **/

#ifndef VECTOR_HELPERS_H
#define VECTOR_HELPERS_H

#include <chrono>

/**
 * @class Vec3_t
 * @brief Simple generic implementation of point (vector) in 3D space.
 */
template<typename T>
struct Vec3_t {
    Vec3_t() : x(0), y(0), z(0) {}
    Vec3_t(T xyz) : x(xyz), y(xyz), z(xyz) {}
    Vec3_t(T x, T y, T z) : x(x), y(y), z(z) {}

    T x, y, z;
};

typedef std::chrono::steady_clock PerfClock_t;

template<typename T>
std::chrono::milliseconds PerfClockDurationMs(const T &dur) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(dur);
}

#endif // VECTOR_HELPERS_H
