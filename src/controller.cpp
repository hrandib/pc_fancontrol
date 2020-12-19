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

#include "controller.h"

std::atomic_bool Controller::breakExecution_;

void Controller::handle() {
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

int32_t Controller::getHighestTemp() {
    auto sensors = config_.getSensors();
    auto highest = std::max_element(sensors.cbegin(), sensors.cend(),
                                    [](auto& sensor_a, auto& sensor_b) {
        return sensor_a->get() < sensor_b->get();
    });
    return (*highest)->get();
}

void Controller::setAllPwms(double value) {
    for(auto& pwm : config_.getPwms()) {
        pwm->set(value, name_);
    }
}

Controller::Controller(Controller::string name, const ConfigEntry& conf) : name_{name}, config_{std::move(conf)},
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
    case ConfigEntry::SETMODE_PI: {
        ConfigEntry::PiConfMode mode
                = std::get<ConfigEntry::SETMODE_PI>(config_.getModeConfig());
        algo_ = std::make_unique<AlgoPI>(mode.temp, mode.kp, mode.ki);
    }
        break;
    }
}
