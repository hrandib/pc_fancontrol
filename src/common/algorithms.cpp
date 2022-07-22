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

#include "common/algorithms.h"
#include "common/constants.h"
#include <cmath>

ControlAlgo::~ControlAlgo() = default;

double AlgoTwoPoint::getSetpoint(double temp)
{
    double result{};
    double norm_temp = temp - a_;
    if(temp >= b_) {
        result = MAX_PERCENT_VAL;
    }
    else if(norm_temp < 0) {
        result = -1;
    }
    else {
        static const auto k = double(MAX_PERCENT_VAL) / (b_ - a_);
        result = norm_temp * k;
    }
    return result;
}

int AlgoTwoPoint::getNormalizedTemperature(double temp)
{
    return static_cast<int>(lround(temp - a_));
}

double AlgoMultiPoint::getSetpoint(double temp)
{
    double result{};
    const auto* begin = points_.data();
    for(size_t i = 1; i < points_.size(); ++i) {
        if(begin->first > temp) {
            result = -1;
            break;
        }
        const auto* end = &points_[i];
        if(end->first > temp) {
            auto degDiff = double(end->first - begin->first);
            auto pwmDiff = double(end->second - begin->second);
            auto multiplier = pwmDiff / degDiff;
            result = begin->second + ((temp - begin->first) * multiplier);
            break;
        }
        if(end == &*points_.rbegin()) {
            result = end->second;
        }
        begin = end;
    }
    return result;
}

int AlgoMultiPoint::getNormalizedTemperature(double temp)
{
    return static_cast<int>(lround(temp - points_[0].first));
}

double AlgoPI::getSetpoint(double temp)
{
    double pe = kp_ * (temp - t_);
    integralErr_ += ki_ * (temp - t_);
    if(integralErr_ > max_i_) {
        integralErr_ = max_i_;
    }
    else if(integralErr_ < 0) {
        integralErr_ = 0;
    }
    double result = pe + integralErr_;
    if(result > MAX_PERCENT_VAL) {
        result = MAX_PERCENT_VAL;
    }
    return result;
}

int AlgoPI::getNormalizedTemperature(double temp)
{
    return static_cast<int>(lround(temp - t_));
}
