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
        SETMODE_TWO_POINT,
        SETMODE_MULTI_POINT,
        SETMODE_PI
    };

    using PointVec = std::vector<std::pair<int, int>>;

    struct PiConfMode {
        int temp, p, i;
    };

    struct TwoPointConfMode {
        int temp_a, temp_b;
    };

    struct MultiPointConfMode {
        //Temperature -> Pwm entry table
        PointVec pointVec;
    };

    using ModeConf = std::variant<TwoPointConfMode, MultiPointConfMode, PiConfMode>;

    struct PollConf {
        enum PollMode {
            PollSimple,
            PollMovingAverage
        };
        PollMode mode;
        int timeMsecs, samplesCount;
    };
private:
    using Sensors = std::vector<Sensor::ptr>;
    using Pwms = std::vector<Pwm::ptr>;

    Sensors sensors_;
    Pwms pwms_;
    ModeConf modeConf_;
    PollConf pollConf_;
    bool autoOff_;
public:

    ConfigEntry& setModeConfig(ModeConf& modeConfig)
    {
        modeConf_ = modeConfig;
        return *this;
    }

    ConfigEntry& setPiConfMode(int temp, int p, int i)
    {
        modeConf_ = PiConfMode{temp, p, i};
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

    ConfigEntry& addPoint(int temp, int pwm_percent) {
        if(modeConf_.index() == SETMODE_MULTI_POINT) {
            std::get<SETMODE_MULTI_POINT>(modeConf_)
                    .pointVec
                    .emplace_back(temp, pwm_percent);
        }
        return *this;
    }

    ConfigEntry& addSensor(Sensor::ptr sensor)
    {
        sensors_.push_back(sensor);
        return *this;
    }

    ConfigEntry& addPwm(Pwm::ptr pwm)
    {
        pwms_.push_back(pwm);
        return *this;
    }

    ConfigEntry& setPollConfig(PollConf::PollMode mode, int pollTime, int samplesCount)
    {
        pollConf_.mode = mode;
        pollConf_.timeMsecs = pollTime;
        pollConf_.samplesCount = samplesCount;
        return *this;
    }

    ConfigEntry& setPollConfig(const PollConf& conf)
    {
        pollConf_ = conf;
        return *this;
    }

    ConfigEntry& setPollConfig(int pollTime)
    {
        pollConf_.mode = PollConf::PollSimple;
        pollConf_.timeMsecs = pollTime;
        return *this;
    }

    ConfigEntry& setAutoOff(bool autoOff) {
        autoOff_ = autoOff;
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

    bool getAutoOff()
    {
        return autoOff_;
    }
};

#endif // CONFIGENTRY_H
