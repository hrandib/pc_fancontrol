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

#ifndef CONFIG_H
#define CONFIG_H

#include "configentry.h"
#include "yaml-cpp/yaml.h"
#include "hwmon/hwmon.h"
#include "controller.h"

#include <filesystem>
#include <optional>

using OptionalNode = std::optional<YAML::Node>;

class Config
{
public:
    using path = std::filesystem::path;
    using string = std::string;
    using HwmonMap = std::map<std::string, Hwmon>;
    using SensorMap = std::map<std::string, Sensor::ptr>;
    using PwmMap = std::map<std::string, Pwm::ptr>;
    using Node = YAML::Node;

    Config(const std::string& configPath);
private:
    Node rootNode_;
    HwmonMap hwmonMap_;
    SensorMap sensorMap_;
    PwmMap pwmMap_;
    std::vector<Controller> controllers_;

    void createHwmons();
    void createSensors();
    void createPwms();
    void createControllers();
    Node getNode(const string& nodeName) const;
};

#endif // CONFIG_H
