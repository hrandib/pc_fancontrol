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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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
#include <variant>
#include <vector>

class ConfigEntry
{
public:
    enum SetMode {
        SETMODE_TWO_POINT,
        SETMODE_MULTI_POINT,
        SETMODE_PI,
    };

    using PointVec = std::vector<std::pair<int, int>>;

    struct PiConfMode
    {
        static constexpr int DEFAULT_MAX_I = 50;

        double temp, kp, ki;
        int max_i = DEFAULT_MAX_I;
    };

    struct TwoPointConfMode
    {
        int temp_a, temp_b;
    };

    struct MultiPointConfMode
    {
        // Temperature -> Pwm entry table
        PointVec pointVec;
    };

    using ModeConf = std::variant<TwoPointConfMode, MultiPointConfMode, PiConfMode>;

    struct PollConf
    {
        static constexpr int DEFAULT_POLL_MSECS = 1000;
        static constexpr int DEFAULT_SAMPLES_AMOUNT = 1;

        int timeMsecs{DEFAULT_POLL_MSECS};
        int samplesCount{DEFAULT_SAMPLES_AMOUNT};
    };
private:
    using Sensors = std::vector<Sensor::ptr>;
    using Pwms = std::vector<Pwm::ptr>;

    Sensors sensors_;
    Pwms pwms_;
    ModeConf modeConf_;
    PollConf pollConf_;
public:
    ConfigEntry& setModeConfig(ModeConf& modeConfig)
    {
        modeConf_ = modeConfig;
        return *this;
    }

    ConfigEntry& setPiConfMode(double temp, double p, double i, int max_i)
    {
        modeConf_ = PiConfMode{temp, p, i, max_i};
        return *this;
    }

    ConfigEntry& setTwoPointConfMode(int temp_a, int temp_b)
    {
        modeConf_ = TwoPointConfMode{temp_a, temp_b};
        return *this;
    }

    ConfigEntry& setMultiPointConfMode()
    {
        modeConf_ = MultiPointConfMode{};
        return *this;
    }

    ConfigEntry& addPoint(int temp, int pwm_percent)
    {
        if(auto* mode = std::get_if<MultiPointConfMode>(&modeConf_)) {
            mode->pointVec.emplace_back(temp, pwm_percent);
        }
        return *this;
    }

    ConfigEntry& addSensor(Sensor::ptr& sensor)
    {
        sensors_.push_back(sensor);
        return *this;
    }

    ConfigEntry& addPwm(Pwm::ptr& pwm)
    {
        pwms_.push_back(pwm);
        return *this;
    }

    ConfigEntry& setPollConfig(int pollTime, int samplesCount)
    {
        pollConf_.timeMsecs = pollTime;
        pollConf_.samplesCount = samplesCount;
        return *this;
    }

    ConfigEntry& setPollConfig(PollConf conf)
    {
        pollConf_ = conf;
        return *this;
    }

    ConfigEntry& setPollConfig(int pollTime)
    {
        pollConf_.timeMsecs = pollTime;
        return *this;
    }

    SetMode getMode() const
    {
        return static_cast<SetMode>(modeConf_.index());
    }

    const PollConf& getPollConfig() const
    {
        return pollConf_;
    }

    const ModeConf& getModeConfig() const
    {
        return modeConf_;
    }

    const Pwms& getPwms() const
    {
        return pwms_;
    }

    const Sensors& getSensors() const
    {
        return sensors_;
    }
};

#endif // CONFIGENTRY_H
