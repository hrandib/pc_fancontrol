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
    virtual int getSetpoint(int) = 0;
};

class AlgoTwoPoint : public ControlAlgo {
    int32_t a, b;
public:
    AlgoTwoPoint(int32_t a, int32_t b) : a{a}, b{b}
    {  }
    int getSetpoint(int temp) override
    {
        static const double k = 100.0/(b - a);
        int norm_temp = temp - a;
        if(temp >= b) {
            return 100;
        }
        else if(norm_temp > 0) {
            return static_cast<int>(norm_temp * k);
        }
        else {
            return 1;
        }
    }
};

class MovingAverageBuf
{
private:
  std::vector<int> buf_;
  size_t index;
public:
  MovingAverageBuf(size_t bufSize) : buf_(bufSize), index{}
  {  }
  void operator=(int val)
  {
    buf_[index] = val;
    index = (index + 1) % buf_.size();
  }
  operator int()
  {
    return std::accumulate(buf_.cbegin(), buf_.cend(), static_cast<int>(0))
            / static_cast<int>(buf_.size());
  }
};

class Controller
{
    using string = std::string;
    using ms = std::chrono::milliseconds;
    using s = std::chrono::seconds;

    static std::atomic_bool breakExecution_;

    string name_;
    const ConfigEntry config_;
    std::unique_ptr<ControlAlgo> algo_;
    std::thread processingThread_;
    MovingAverageBuf samples_;
    void handle() {
        while(!breakExecution_) {
            int temp = getHighestTemp();
            samples_ = temp;
            std::cout << "Peak deg: " << temp << " ma deg: " << samples_ << " ";
            int normalizedValue = algo_->getSetpoint(samples_);
            std::cout << normalizedValue << " % pwm\n";
            setAllPwms(normalizedValue);
            std::this_thread::sleep_for(ms(config_.getPollConfig().timeMsecs));
        }
    }

    int32_t getHighestTemp() {
        auto sensors = config_.getSensors();
        auto highest = std::max_element(sensors.cbegin(), sensors.cend(),
                                        [](auto& sensor_a, auto& sensor_b) {
            return sensor_a->get() > sensor_b->get();
        });
        return (*highest)->get();
    }

    void setAllPwms(int value) {
        for(auto& pwm : config_.getPwms()) {
            pwm->set(value, name_);
        }
    }

public:
    Controller(string name, const ConfigEntry& conf) : name_{name}, config_{std::move(conf)},
        samples_(static_cast<size_t>(conf.getPollConfig().samplesCount))
    {
        switch(conf.getMode()) {
        case ConfigEntry::SETMODE_MULTI_POINT:
        case ConfigEntry::SETMODE_PI:
        case ConfigEntry::SETMODE_TWO_POINT:
            ConfigEntry::TwoPointConfMode mode
                    = std::get<ConfigEntry::SETMODE_TWO_POINT>(conf.getModeConfig());
            algo_ = std::make_unique<AlgoTwoPoint>(mode.temp_a, mode.temp_b);
            break;
        }
    }

    Controller(Controller&&) = default;

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
