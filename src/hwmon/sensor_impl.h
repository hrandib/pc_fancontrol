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
#ifndef SENSORIMPL_H
#define SENSORIMPL_H

#include "interface/sensor.h"
#include "sysfs/reader_impl.h"

class SensorImpl : SysfsReaderImpl, public Sensor
{
private:
    static inline constexpr sv PATH_SUFFIX = "_input";
public:
    SensorImpl(const fs::path& path);

    bool open() override;

    int32_t get() override;

    bool exists() override {
        return fs::exists(getFilePath());
    }
};

template<typename T>
inline Sensor::ptr make_sensor(const fs::path&);

template<>
inline Sensor::ptr make_sensor<SensorImpl>(const fs::path& path) {
    return std::make_shared<SensorImpl>(path);
}

#endif // SENSORIMPL_H
