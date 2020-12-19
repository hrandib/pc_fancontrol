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
#include <cmath>

struct ControlAlgo {
    virtual double getSetpoint(double) = 0;
};

class AlgoTwoPoint : public ControlAlgo {
    double a, b;
public:
    AlgoTwoPoint(int a, int b) : a{static_cast<double>(a)}, b{static_cast<double>(b)}
    {  }
    double getSetpoint(double temp) override
    {
        static const double k = 100.0/(b - a);
        double norm_temp = temp - a;
        if(temp >= b) {
            return 100;
        } else if(norm_temp <= 0) {
            return 0;
        }
        else {
            return norm_temp * k;
        }
    }
};

class AlgoMultiPoint : public ControlAlgo {
    const ConfigEntry::PointVec points_;
public:
    AlgoMultiPoint(ConfigEntry::PointVec& vec) : points_{std::move(vec)}
    {  }
    double getSetpoint(double temp) override
    {
        double result{};
        auto* begin = &points_[0];
        for(size_t i = 1; i < points_.size(); ++i) {
            if(begin->first >= temp) {
                result = begin->second;
                std::cout << "1: " << result << std::endl;
                break;
            }
            auto* end = &points_[i];
            if(end->first >= temp) {
                auto degDiff = double(end->first - begin->first);
                auto pwmDiff = double(end->second - begin->second);
                auto multiplier = pwmDiff/degDiff;
                result = begin->second + ((temp - begin->first) * multiplier);
                std::cout << "\nbegin_deg: " << begin->first << " end_deg: " << end->first << std::endl;
                std::cout << "begin_pwm: " << begin->second << " end_pwm: " << end->second << std::endl;
                std::cout << "2: " << result << " " << multiplier << std::endl;
                break;
            }
            else if(end == &*points_.rbegin()) {
                result = end->second;
                std::cout << "3: " << result << std::endl;
            }
            begin = end;
        }
        return result;
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
  operator double()
  {
    return std::accumulate(buf_.cbegin(), buf_.cend(), static_cast<int>(0))
            / static_cast<double>(buf_.size());
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
            auto setpoint = algo_->getSetpoint(samples_);
            std::cout << setpoint << " % pwm\n";
            setAllPwms(setpoint);
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

    void setAllPwms(double value) {
        for(auto& pwm : config_.getPwms()) {
            pwm->set(value, name_);
        }
    }

public:
    Controller(string name, const ConfigEntry& conf) : name_{name}, config_{std::move(conf)},
        samples_(static_cast<size_t>(conf.getPollConfig().samplesCount))
    {
        switch(conf.getMode()) {
        case ConfigEntry::SETMODE_TWO_POINT: {
            ConfigEntry::TwoPointConfMode mode
                    = std::get<ConfigEntry::SETMODE_TWO_POINT>(config_.getModeConfig());
            algo_ = std::make_unique<AlgoTwoPoint>(mode.temp_a, mode.temp_b);
        }
            break;
        case ConfigEntry::SETMODE_MULTI_POINT: {
            ConfigEntry::MultiPointConfMode mode
                    = std::get<ConfigEntry::SETMODE_MULTI_POINT>(config_.getModeConfig());
            algo_ = std::make_unique<AlgoMultiPoint>(mode.pointVec);
        }
            break;
        case ConfigEntry::SETMODE_PI:
            break;
        }
    }

    Controller(Controller&&) = default;

    void run() {
        processingThread_ = std::thread{&Controller::handle, this};
    }

    ~Controller() {
        processingThread_.join();
        std::cout << "\'" << name_<< "\' controller finished\n";
    }

    static void stop() {
        breakExecution_ = true;
    }
};

#endif // CONTROLLER_H
