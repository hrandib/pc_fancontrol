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

#ifndef ALGO_H
#define ALGO_H

#include "common/macro.h"
#include "configentry.h"

struct ControlAlgo
{
    virtual double getSetpoint(double) = 0;
    virtual int getNormalizedTemperature(double) = 0;
    virtual ~ControlAlgo();

    ControlAlgo() = default;
    DISABLE_OBJ_COPY(ControlAlgo)
};

class AlgoTwoPoint : public ControlAlgo
{
    double a_, b_, k_;
public:
    AlgoTwoPoint(int a, int b) :
      a_{static_cast<double>(a)}, b_{static_cast<double>(b)}, k_{double(MAX_PERCENT_VAL) / (b_ - a_)}
    { }
    double getSetpoint(double temp) final;
    int getNormalizedTemperature(double temp) final;
};

class AlgoMultiPoint : public ControlAlgo
{
    const ConfigEntry::PointVec points_;
public:
    explicit AlgoMultiPoint(ConfigEntry::PointVec& vec) : points_{std::move(vec)}
    { }
    double getSetpoint(double temp) final;
    int getNormalizedTemperature(double temp) final;
};

class AlgoPI : public ControlAlgo
{
    const double t_, kp_, ki_;
    const int max_i_;
    double integralErr_{};
public:
    AlgoPI(double t, double kp, double ki, int max_i) : t_{t}, kp_{kp}, ki_{ki}, max_i_{max_i}
    { }
    double getSetpoint(double temp) final;
    int getNormalizedTemperature(double temp) final;
};

#endif // ALGO_H
