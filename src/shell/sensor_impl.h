/*
 * Copyright (c) 2020 Dmytro Shestakov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef SHELLSENSOR_H
#define SHELLSENSOR_H

#include "interface/sensor.h"
#include <atomic>
#include <chrono>
#include <string_view>
#include <filesystem>


class ShellSensor : public Sensor
{
private:
    static inline constexpr std::chrono::seconds READ_PERIOD{1};

    std::string executablePath;
    std::chrono::time_point<std::chrono::system_clock> prevReadTime_{};
    std::atomic_int cachedVal_;
public:
    ShellSensor(const std::string& executablePath);

    bool open() override {
        return true;
    }

    int32_t get() override;

    bool exists() override {
        return true;
    }
private:
    static inline int exec(const char* cmd);
};

template<typename T>
inline Sensor::ptr make_sensor(const std::string&);

template<>
inline Sensor::ptr make_sensor<ShellSensor>(const std::string& path) {
    return std::make_shared<ShellSensor>(path);
}

#endif // SHELLSENSOR_H
