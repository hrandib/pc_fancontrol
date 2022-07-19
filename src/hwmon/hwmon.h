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
#ifndef HWMON_H
#define HWMON_H

#include <filesystem>
#include <optional>
#include <string_view>

#include "common/constants.h"
#include "hwmon/pwm_impl.h"
#include "interface/sensor.h"

namespace fs = std::filesystem;

class Hwmon
{
private:
    using sv = std::string_view;
    using optionalPath = std::optional<fs::path>;

    static constexpr std::chrono::seconds INIT_POLL_SECS{2};
    static constexpr size_t INIT_POLL_CYCLES{3};

    fs::path hwmonPath_;
    std::map<std::string, Sensor::ptr> sensorCache_{};
    std::map<std::string, Pwm::ptr> pwmCache_{};

    static optionalPath getHwmonPathByName(sv hwmonName);
    static optionalPath tryHwmonInit(sv hwmonName);
public:
    Hwmon() : hwmonPath_{}
    { }
    Hwmon(sv hwmonName);
    bool setName(sv hwmonName);
    Sensor::ptr getSensor(sv sensorName);
    Pwm::ptr getPwm(sv pwmName);
    const fs::path& getHwmonPath();
};

#endif // HWMON_H
