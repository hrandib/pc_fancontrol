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

#ifndef CONFIGENTRY_H
#define CONFIGENTRY_H

#include "interface/pwm.h"
#include "interface/sensor.h"
#include <vector>
#include <variant>

class ConfigEntry
{
public:
    enum SetMode {
        SETMODE_PI,
        SETMODE_TWO_POINT,
        SETMODE_MULTI_POINT
    };

    struct PiConfMode {
        uint32_t temp, p, i;
    };

    struct TwoPointConfMode {
        uint32_t temp_a, temp_b;
    };

    struct MultiPointConfMode {
        //Temperature -> Pwm entry table
        std::vector<std::pair<uint32_t, uint32_t>> pointVec;
    };

    struct PollConf {
        enum PollMode {
            PollSimple,
            PollMovingAverage
        };
        PollMode mode;
        uint32_t timeSecs, samplesCount;
    };
private:
    using Sensors = std::vector<Sensor::ptr>;
    using Pwms = std::vector<Pwm::ptr>;
    using ModeConf = std::variant<PiConfMode, TwoPointConfMode, MultiPointConfMode>;

    Sensors sensors_;
    Pwms pwms_;
    ModeConf modeConf_;
    PollConf pollConf_;
public:

    ConfigEntry& SetPiConfMode(uint32_t temp, uint32_t p, uint32_t i)
    {
        modeConf_ = PiConfMode{temp, p, i};
        return *this;
    }

    ConfigEntry& SetTwoPointConfMode(uint32_t temp_a, uint32_t temp_b)
    {
        modeConf_ = TwoPointConfMode{temp_a, temp_b};
        return *this;
    }

    ConfigEntry& SetMultiPointConfMode()
    {
        modeConf_ = MultiPointConfMode{};
        return *this;
    }

    ConfigEntry& AddPoint(uint32_t temp, uint32_t pwm_percent) {
        if(modeConf_.index() == SETMODE_MULTI_POINT) {
            std::get<SETMODE_MULTI_POINT>(modeConf_)
                    .pointVec
                    .emplace_back(temp, pwm_percent);
        }
        return *this;
    }

    ConfigEntry& AddSensor(Sensor::ptr sensor)
    {
        sensors_.push_back(sensor);
        return *this;
    }

    ConfigEntry& AddPwm(Pwm::ptr pwm)
    {
        pwms_.push_back(pwm);
        return *this;
    }

    ConfigEntry& SetPollConfig(PollConf::PollMode mode, uint32_t pollTime, uint32_t samplesCount)
    {
        pollConf_.mode = mode;
        pollConf_.timeSecs = pollTime;
        pollConf_.samplesCount = samplesCount;
        return *this;
    }

    ConfigEntry& SetPollConfig(uint32_t pollTime)
    {
        pollConf_.mode = PollConf::PollSimple;
        pollConf_.timeSecs = pollTime;
        return *this;
    }

    SetMode GetMode() const
    {
        return static_cast<SetMode>(modeConf_.index());
    }

    const PollConf& GetPollConfig() const
    {
        return pollConf_;
    }

    const ModeConf& GetModeConfig() const
    {
        return modeConf_;
    }

    const Pwms& GetPwms() const
    {
        return pwms_;
    }

    const Sensors& getSensors() const
    {
       return sensors_;
    }
};

#endif // CONFIGENTRY_H
