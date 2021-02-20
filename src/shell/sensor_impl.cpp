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

#include "shell/sensor_impl.h"
#include <array>
#include <iostream>

ShellSensor::ShellSensor(const std::string& path) : executablePath{path}
{ }

int32_t ShellSensor::get()
{
    auto now = std::chrono::system_clock::now();
    if (now > READ_PERIOD + prevReadTime_) {
        try {
            cachedVal_ = exec(executablePath.c_str());
            prevReadTime_ = now;
        }  catch (std::exception& e) {
            std::cerr << e.what() << "\n";
        }
    }
    return cachedVal_;
}

int ShellSensor::exec(const char* cmd) {
    std::array<char, 128> buffer{};
    std::string rawResult;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        rawResult += buffer.data();
    }
    int result{};
    try {
       result = std::stoi(rawResult);
    }  catch (std::exception&) {
        throw std::invalid_argument("Parsing shell command output failed: " + rawResult);
    }
    return result;
}
