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
#ifndef PWM_H
#define PWM_H

#include "common/constants.h"
#include "common/macro.h"
#include <memory>
#include <utility>

struct Pwm
{
    Pwm() = default;

    using ptr = std::shared_ptr<Pwm>;
    using string = std::string;

    enum class Mode {
        Dc,
        Pwm,
        NoChange,
    };

    virtual bool open() = 0;
    virtual bool set(double val, int tempOffset, const string& sourceName) = 0;
    virtual void setMin(int val) = 0;
    virtual void setMax(int val) = 0;
    virtual void setMode(Mode mode) = 0;
    virtual void reset() = 0;
    virtual bool exists() = 0;

    // FANSTOP_DISABLE constant is used to disable automatic fan stop feature
    void setFanStopHysteresis(int hysteresis)
    {
        fanStopHyst_ = hysteresis;
    }

    int getFanStopHysteresis() const
    {
        return fanStopHyst_;
    }

    virtual ~Pwm();

    DISABLE_OBJ_COPY(Pwm)
private:
    int fanStopHyst_{FANSTOP_DISABLE};
};

#endif // PWM_H
