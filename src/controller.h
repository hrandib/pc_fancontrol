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
#include "common/algorithms.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <numeric>
#include <cmath>

class MovingAverageBuf
{
private:
    static constexpr int INIT_DEGREE = 45;
    std::vector<int> buf_;
    size_t index;
public:
    explicit MovingAverageBuf(size_t bufSize) : buf_(bufSize, INIT_DEGREE), index{}
    {  }
    MovingAverageBuf& operator=(int val)
    {
        buf_[index++] = val;
        if(buf_.size() == index) {
            index = 0;
        }
        return *this;
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
    int previousDegreeValue_;

    void handle();

    int32_t getHighestTemp();

    void setAllPwms(double value);

public:
    Controller(const string& name, ConfigEntry& conf);

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
