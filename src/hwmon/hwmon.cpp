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
#include "hwmon/hwmon.h"
#include "hwmon/sensor_impl.h"
#include "hwmon/pwm_impl.h"
#include "sysfs/reader_impl.h"
#include <iostream>

using std::cout, std::endl;

Hwmon::optionalPath Hwmon::getHwmonPathByName(sv hwmonName)
{
    optionalPath result = {};
    for (const auto& entry : fs::directory_iterator(HWMON_ROOT)) {
        if (!entry.is_directory()) {
            continue;
        }
        auto file = SysfsReaderImpl(entry.path()/"name");
        cout << entry.path() << ": ";
        if (file.open() && (file.read() == hwmonName)) {
            result = entry.path();
            break;
        }
        if (file) {
            cout << file.read() << endl;
        }
    }
    return result;
}

Hwmon::Hwmon(sv hwmonName)
{
    auto result = getHwmonPathByName(hwmonName);
    if(result) {
        hwmonPath_ = result.value();
    } else {
        throw std::invalid_argument(hwmonName.data());
    }
}

bool Hwmon::setName(Hwmon::sv hwmonName)
{
    auto result = getHwmonPathByName(hwmonName);
    return result.has_value();
}

Sensor::ptr Hwmon::getSensor(Hwmon::sv sensorName)
{
    Sensor::ptr result = make_sensor<SensorImpl>(getHwmonPath()/sensorName);
    if(!result->exists()) {
        result = nullptr;
    }
    return result;
}

Pwm::ptr Hwmon::getPwm(sv pwmName, uint_fast8_t min, uint_fast8_t max,
                       Pwm::Mode mode)
{
//    std::unique_ptr<PwmImpl> result = std::make_unique<PwmImpl>(getHwmonPath()/pwmName, 90, 255);
//    cout << result->getFilePath() << endl;
    Pwm::ptr result = make_pwm<PwmImpl>(getHwmonPath()/pwmName, min, max, mode);
    if(!result->exists()) {
        result = nullptr;
    }
    return result;
}

const fs::path& Hwmon::getHwmonPath()
{
    return hwmonPath_;
}
