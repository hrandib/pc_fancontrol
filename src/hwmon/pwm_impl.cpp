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

bool PwmImpl::setControl(Control control) {
    SysfsWriterImpl writer{enablePath_};
    return writer.open() && writer.write(static_cast<uint_fast8_t>(control));
}

bool PwmImpl::setMode(Mode mode) {
    SysfsWriterImpl writer{modePath_};
    return writer.open() && writer.write(static_cast<uint_fast8_t>(mode));
}

PwmImpl::PwmImpl(const fs::path& pwmPath,
                 uint_fast8_t min, uint_fast8_t max, Mode mode)
    : SysfsWriterImpl{pwmPath}, valueCache_{}, minPwm_{min}, maxPwm_{max}, mode_{mode}
{
    fs::path enablePath_ = modePath_ = pwmPath;
    enablePath_ += ENABLE_SUFFIX;
    modePath_ += MODE_SUFFIX;
}

bool PwmImpl::open()
{
    if(mode_ != Mode::NoChange) {
        //TODO: log set mode not supported
        setMode(mode_);
    }
    return setControl(Control::Manual) && SysfsWriterImpl::open();
}

bool PwmImpl::set(uint_fast8_t val, const string& sourceName)
{
    static const auto multiplier = (maxPwm_ - minPwm_)/100.0f;
    valueCache_[sourceName] = val;
    auto it = std::max_element(valueCache_.cbegin(), valueCache_.cend(),
                               [](const auto& a, const auto& b) {
                                       return std::max(a.second, b.second);});
    val = it->second;
    uint8_t rawValue{};
    if(val) {
        rawValue = static_cast<uint8_t>(minPwm_ + std::lround(multiplier * val));
    }
    return write(rawValue);
}

void PwmImpl::reset()
{
    valueCache_.clear();
}

PwmImpl::~PwmImpl()
{
    setControl(Control::Auto);
}
