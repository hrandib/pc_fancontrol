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

PwmImpl::PwmImpl(const fs::path& pwmPath, int min, int max, Mode mode) :
  SysfsWriterImpl{pwmPath}, minPwm_{min}, maxPwm_{max}, mode_{mode}, previousRawValue_{UINT32_MAX}, isStopped_{}
{
    enablePath_ = modePath_ = pwmPath;
    enablePath_ += ENABLE_SUFFIX;
    modePath_ += MODE_SUFFIX;
}

PwmImpl::PwmImpl(const fs::path& pwmPath) : PwmImpl{pwmPath, 0, PWM_MAX_VAL, Mode::NoChange}
{ }

bool PwmImpl::setControl(Control control)
{
    SysfsWriterImpl writer{enablePath_};
    return writer.open() && writer.write(static_cast<uint32_t>(control));
}

bool PwmImpl::activateMode(Mode mode)
{
    SysfsWriterImpl writer{modePath_};
    return writer.open() && writer.write(static_cast<uint32_t>(mode));
}

double PwmImpl::selectMaxValue(double val, const string& sourceName)
{
    valueCache_[sourceName] = val;
    auto it = std::max_element(
      valueCache_.cbegin(), valueCache_.cend(), [](const auto& a, const auto& b) { return a.second < b.second; });
    return it->second;
}

bool PwmImpl::open()
{
    if(mode_ != Mode::NoChange) {
        // TODO: log set mode not supported
        activateMode(mode_);
    }
    return setControl(Control::Manual) && SysfsWriterImpl::open();
}

uint32_t PwmImpl::processFanStopCondition(int tempOffset)
{
    if(getFanStopHysteresis() == FANSTOP_DISABLE) {
        return static_cast<uint32_t>(minPwm_);
    }
    if(tempOffset >= 0) {
        isStopped_ = false;
    }
    else if(-tempOffset >= getFanStopHysteresis()) {
        isStopped_ = true;
    }
    return isStopped_ ? 0 : static_cast<uint32_t>(minPwm_);
}

bool PwmImpl::set(double val, int tempOffset, const string& sourceName)
{
    std::lock_guard lock{mutex_};
    val = selectMaxValue(val, sourceName);
    uint32_t rawValue = processFanStopCondition(tempOffset);
    if(val >= 0) {
        auto multiplier = (maxPwm_ - minPwm_) / 100.0;
        rawValue = static_cast<uint32_t>(minPwm_ + std::lround(multiplier * val));
    }
    if(rawValue != previousRawValue_) {
        if(rawValue == 0) {
            std::cout << sourceName << ": Fan stop" << std::endl;
        }
        else if(previousRawValue_ == 0) {
            std::cout << sourceName << ": Fan start" << std::endl;
        }
        previousRawValue_ = rawValue;
        return write(rawValue);
    }
    return true;
}

void PwmImpl::reset()
{
    valueCache_.clear();
}

PwmImpl::~PwmImpl()
{
    setControl(Control::Auto);
}
