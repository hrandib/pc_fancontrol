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

#include "configentry.h"

struct ControlAlgo {
    virtual double getSetpoint(double) = 0;
    virtual int getNormalizedTemperature(double) = 0;
    virtual ~ControlAlgo();
};

class AlgoTwoPoint : public ControlAlgo {
    double a_, b_;
public:
    AlgoTwoPoint(int a, int b) : a_{static_cast<double>(a)}, b_{static_cast<double>(b)}
    {  }
    double getSetpoint(double temp) final;
    int getNormalizedTemperature(double) final;
};

class AlgoMultiPoint : public ControlAlgo {
    const ConfigEntry::PointVec points_;
public:
    AlgoMultiPoint(ConfigEntry::PointVec& vec) : points_{std::move(vec)}
    {  }
    double getSetpoint(double temp) final;
    int getNormalizedTemperature(double) final;
};

class AlgoPI : public ControlAlgo {
    const double t_, kp_, ki_;
    double integralErr_;
public:
    AlgoPI(double t, double kp, double ki) : t_{t}, kp_{kp}, ki_{ki}, integralErr_{}
    {  }
    double getSetpoint(double temp) final;
    int getNormalizedTemperature(double) final;
};


#endif // ALGO_H
