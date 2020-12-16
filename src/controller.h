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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "interface/pwm.h"
#include "interface/sensor.h"
#include "configentry.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <numeric>

struct ControlAlgo {
    virtual uint32_t getSetpoint(int32_t) = 0;
};

class AlgoTwoPoint : public ControlAlgo {
    int32_t a, b;
public:
    AlgoTwoPoint(int32_t a, int32_t b) : a{a}, b{b}
    {  }
    uint32_t getSetpoint(int32_t temp) override
    {
        static const double k = 100.0/(b - a);
        int32_t norm_temp = temp - a;
        if(temp >= b) {
            return 100;
        }
        else if(norm_temp > 0) {
            return static_cast<uint32_t>(norm_temp * k);
        }
        else {
            return 1;
        }
    }
};

class Controller
{
    using string = std::string;
    using ms = std::chrono::milliseconds;
    using s = std::chrono::seconds;

    static std::atomic_bool breakExecution_;

    string name_;
    const ConfigEntry& config_;
    std::unique_ptr<ControlAlgo> algo_;
    std::thread processingThread_;

    void handle() {
        while(!breakExecution_) {
            int32_t temp = getHighestTemp();
            std::cout << temp << " deg ";
            uint32_t normalizedValue = algo_->getSetpoint(temp);
            std::cout << normalizedValue << " pwm%\n";
            setAllPwms(normalizedValue);
            std::this_thread::sleep_for(s(config_.GetPollConfig().timeMsecs));
        }
    }

    int32_t getHighestTemp() {
        auto sensors = config_.getSensors();
        auto highest = std::max_element(sensors.cbegin(), sensors.cend(),
                                        [](auto& sensor_a, auto& sensor_b) {
            return sensor_a > sensor_b;
        });
        return (*highest)->get();
    }

    void setAllPwms(uint32_t value) {
        for(auto& pwm : config_.GetPwms()) {
            pwm->set(static_cast<uint_fast8_t>(value), name_);
        }
    }

public:
    Controller(string name, const ConfigEntry& conf) : name_{name}, config_{conf}
    {
        switch(conf.GetMode()) {
        case ConfigEntry::SETMODE_MULTI_POINT:
        case ConfigEntry::SETMODE_PI:
        case ConfigEntry::SETMODE_TWO_POINT:
            ConfigEntry::TwoPointConfMode mode
                    = std::get<ConfigEntry::SETMODE_TWO_POINT>(conf.GetModeConfig());
            algo_ = std::make_unique<AlgoTwoPoint>(mode.temp_a, mode.temp_b);
            break;
        }
    }

    void run() {
        processingThread_ = std::thread{&Controller::handle, this};
    }

    ~Controller() {
        processingThread_.join();
        std::cout << "Controller " << name_ << " finished\n";
    }

    static void stop() {
        breakExecution_ = true;
    }
};

#endif // CONTROLLER_H
