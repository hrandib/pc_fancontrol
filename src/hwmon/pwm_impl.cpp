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

#include "pwm_impl.h"
#include <cmath>
#include <iostream>

PwmImpl::PwmImpl(const fs::path& pwmPath,
                 int min, int max, Mode mode)
    : SysfsWriterImpl{pwmPath}, valueCache_{}, minPwm_{min}, maxPwm_{max}, mode_{mode}
{
    enablePath_ = modePath_ = pwmPath;
    enablePath_ += ENABLE_SUFFIX;
    modePath_ += MODE_SUFFIX;
}

PwmImpl::PwmImpl(const fs::path &pwmPath) : PwmImpl{pwmPath, 0, 255, Mode::NoChange}
{ }

bool PwmImpl::setControl(Control control)
{
    SysfsWriterImpl writer{enablePath_};
    bool openSuccess = writer.open();
    bool writeSuccess = writer.write(static_cast<uint_fast8_t>(control));
    return openSuccess && writeSuccess;
}

bool PwmImpl::activateMode(Mode mode)
{
    SysfsWriterImpl writer{modePath_};
    return writer.open() && writer.write(static_cast<uint_fast8_t>(mode));
}

int PwmImpl::selectMaxValue(int val, const string& sourceName)
{
    valueCache_[sourceName] = val;
    auto it = std::max_element(valueCache_.cbegin(), valueCache_.cend(),
                               [](const auto& a, const auto& b) {
                                       return std::max(a.second, b.second);});
    return it->second;
}

bool PwmImpl::open()
{
    if(mode_ != Mode::NoChange) {
        //TODO: log set mode not supported
        activateMode(mode_);
    }
    return setControl(Control::Manual) && SysfsWriterImpl::open();
}

bool PwmImpl::set(int val, const string& sourceName)
{
    static const auto multiplier = (maxPwm_ - minPwm_)/100.0;
    val = selectMaxValue(val, sourceName);
    int rawValue{};
    if(val) {
        rawValue = static_cast<int>(minPwm_ + std::lround(multiplier * val));
        std::cout << rawValue << std::endl;
    }
    return write(static_cast<uint32_t>(rawValue));
}

void PwmImpl::reset()
{
    valueCache_.clear();
}

PwmImpl::~PwmImpl()
{
    setControl(Control::Auto);
}
