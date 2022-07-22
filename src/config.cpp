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

#include "config.h"
#include "shell/sensor_impl.h"

#include <iostream>

using std::cout, std::endl;
using PollConf = ConfigEntry::PollConf;
using SetMode = ConfigEntry::SetMode;
using ModeConf = ConfigEntry::ModeConf;
using StringVector = std::vector<std::string>;
using HwAttrs = Hwmon::Attributes;

[[maybe_unused]] static inline std::string to_string(StringVector& vec)
{
    std::string result = "\'";
    for(auto& el : vec) {
        result += el;
        result += " ";
    }
    *result.rbegin() = '\'';
    return result;
}

struct SensorNode
{
    std::string name;
    std::string type;
    std::string bind;
};

static void operator>>(const YAML::Node& node, SensorNode& sensNode)
{
    for(auto it = node.begin(); it != node.end(); ++it) {
        sensNode.name = it->first.as<std::string>();
        for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            auto key = it2->first.as<std::string>();
            if(key == "type") {
                sensNode.type = it2->second.as<std::string>();
            }
            else if(key == "bind") {
                sensNode.bind = it2->second.as<std::string>();
            }
        }
    }
}

struct PwmNode
{
    std::string name;
    std::string type;
    std::string bind;
    int minpwm{}, maxpwm{};
    Pwm::Mode mode{Pwm::Mode::NoChange};
    int fanStopHyst{FANSTOP_DISABLE};
};

static void operator>>(const YAML::Node& node, PwmNode& pwmNode)
{
    for(auto it = node.begin(); it != node.end(); ++it) {
        pwmNode.name = it->first.as<std::string>();
        for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            auto key = it2->first.as<std::string>();
            if(key == "type") {
                pwmNode.type = it2->second.as<std::string>();
            }
            else if(key == "bind") {
                pwmNode.bind = it2->second.as<std::string>();
            }
            else if(key == "mode") {
                auto rawMode = it2->second.as<std::string>();
                if(rawMode == "dc") {
                    pwmNode.mode = Pwm::Mode::Dc;
                }
                else if(rawMode == "pwm") {
                    pwmNode.mode = Pwm::Mode::Pwm;
                }
                else {
                    cout << "Incompatible PWM mode, no change: " + rawMode << "\n";
                }
            }
            else if(key == "minpwm") {
                pwmNode.minpwm = it2->second.as<int>();
            }
            else if(key == "maxpwm") {
                pwmNode.maxpwm = it2->second.as<int>();
            }
            else if(key == "fan_stop") {
                pwmNode.fanStopHyst = it2->second.as<bool>() ? FANSTOP_DEFAULT_HYSTERESIS : FANSTOP_DISABLE;
            }
            else if(key == "fan_stop_hysteresis") {
                pwmNode.fanStopHyst = static_cast<int>(it2->second.as<uint32_t>());
            }

            else {
                cout << "unknown attribute:" << key << "\n";
            }
        }
    }
}

struct ControllerNode
{
    std::string name;
    StringVector sensor;
    StringVector pwm;
    SetMode mode{};
    PollConf pollConfig{};
    ModeConf modeConfig{};
};

static inline int parseTime(const YAML::Node& poll)
{
    auto stringValue = poll.as<std::string>();
    int result{};
    try {
        result = std::stoi(stringValue);
    }
    catch(std::exception&) {
        throw std::invalid_argument("Error parsing poll time: " + stringValue);
    }
    if(stringValue.find("ms") == std::string::npos) {
        // used field units - secs, convert to ms
        result *= SYSFS_DEGREE_SCALE;
    }
    return result;
}

static inline PollConf parsePollConfig(const YAML::Node& poll)
{
    PollConf result;
    if(poll.IsScalar()) {
        result.timeMsecs = parseTime(poll);
    }
    else {
        for(auto it = poll.begin(); it != poll.end(); ++it) {
            auto key = it->first.as<std::string>();
            if(key == "time") {
                result.timeMsecs = parseTime(it->second);
            }
            else if(key == "ma_samples") {
                result.samplesCount = it->second.as<int>();
            }
        }
    }
    return result;
}

static inline StringVector parseSensorsPwms(const YAML::Node& node)
{
    StringVector result;
    if(node.IsMap()) {
        for(auto it = node.begin(); it != node.end(); ++it) {
            result.emplace_back(it->first.as<std::string>());
        }
    }
    else {
        result.emplace_back(node.as<std::string>());
    }
    return result;
}

static inline ModeConf parseModeConfig(SetMode mode, const YAML::Node& node)
{
    ModeConf result;
    switch(mode) {
        case ConfigEntry::SETMODE_TWO_POINT: {
            ConfigEntry::TwoPointConfMode confMode{};
            for(auto it = node.begin(); it != node.end(); ++it) {
                auto key = it->first.as<std::string>();
                if(key == "a") {
                    confMode.temp_a = it->second.as<int>();
                }
                else if(key == "b") {
                    confMode.temp_b = it->second.as<int>();
                }
                else {
                    throw std::invalid_argument("unknown attribute: " + key);
                }
            }
            result = confMode;
        } break;
        case ConfigEntry::SETMODE_MULTI_POINT: {
            ConfigEntry::MultiPointConfMode confMode;
            for(auto it = node.begin(); it != node.end(); ++it) {
                auto key = it->first.begin()->first.as<int>();
                auto val = it->first.begin()->second.as<int>();
                confMode.pointVec.emplace_back(key, val);
            }
            result = confMode;
        } break;
        case ConfigEntry::SETMODE_PI: {
            ConfigEntry::PiConfMode confMode;
            for(auto it = node.begin(); it != node.end(); ++it) {
                auto key = it->first.as<std::string>();
                if(key == "t") {
                    confMode.temp = it->second.as<double>();
                }
                else if(key == "kp") {
                    confMode.kp = it->second.as<double>();
                }
                else if(key == "ki") {
                    confMode.ki = it->second.as<double>();
                }
                else if(key == "max_i") {
                    int max_i = it->second.as<int>();
                    if(max_i > 0 && max_i <= MAX_PERCENT_VAL) {
                        confMode.max_i = max_i;
                    }
                }
                else {
                    throw std::invalid_argument("unknown attribute: " + key);
                }
            }
            result = confMode;
        } break;
    }
    return result;
}

static inline HwAttrs parseHwmonAttributes(const YAML::Node& node)
{
    using std::string;
    string keyPath{DEFAULT_HWMON_KEY_PATH};
    string nodeName;
    string bind;
    string alias;
    if(node.IsScalar()) {
        nodeName = node.as<std::string>();
    }
    else {
        nodeName = node.begin()->first.as<string>();
        const auto attrsNode = node.begin()->second;
        for(auto it = attrsNode.begin(); it != attrsNode.end(); ++it) {
            auto key = it->first.as<string>();
            if(key == "name") {
                alias = it->second.as<string>();
            }
            else if(key == "keyPath") {
                keyPath = it->second.as<string>();
            }
            else if(key == "bind") {
                bind = it->second.as<string>();
            }
            else {
                cout << "unknown attribute:" << key << "\n";
            }
        }
        if(nodeName.empty() || bind.empty()) {
            throw std::invalid_argument(R"(hwmon config entry is inconsistent, must contain 'bind' and 'name' fields)");
        }
    }
    return {.nodeName = nodeName, .alias = alias, .keyValue = bind, .keyPath = keyPath};
}

static void operator>>(const YAML::Node& node, ControllerNode& controllerNode)
{
    for(auto it = node.begin(); it != node.end(); ++it) {
        controllerNode.name = it->first.as<std::string>();
        for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            auto key = it2->first.as<std::string>();
            if(key == "sensor") {
                controllerNode.sensor = parseSensorsPwms(it2->second);
            }
            else if(key == "pwm") {
                controllerNode.pwm = parseSensorsPwms(it2->second);
            }
            else if(key == "mode") {
                auto rawMode = it2->second.as<std::string>();
                if(rawMode == "two_point") {
                    controllerNode.mode = ConfigEntry::SETMODE_TWO_POINT;
                }
                else if(rawMode == "multi_point") {
                    controllerNode.mode = ConfigEntry::SETMODE_MULTI_POINT;
                }
                else if(rawMode == "pi") {
                    controllerNode.mode = ConfigEntry::SETMODE_PI;
                }
                else {
                    cout << "Incompatible controller mode, the entry will be skipped: " + rawMode << "\n";
                    break;
                }
            }
            else if(key == "poll") {
                controllerNode.pollConfig = parsePollConfig(it2->second);
            }
            else if(key == "set") {
                controllerNode.modeConfig = parseModeConfig(controllerNode.mode, it2->second);
            }
            else {
                cout << "unknown attribute:" << key << "\n";
            }
        }
    }
}

Config::Config(const string& configPath)
{
    rootNode_ = YAML::LoadFile(configPath);
    if(!rootNode_) {
        throw std::invalid_argument("Config loading failed: " + configPath);
    }
    createHwmons();
    createSensors();
    createPwms();
    createControllers();
}

void Config::run()
{
    for(auto& c : controllers_) {
        c->run();
    }
}

void Config::createHwmons()
{
    cout << "=== HWMON ===\n";
    auto hwmonList = getNode("hwmon");
    for(const auto& hwmon : hwmonList) {
        auto attrs = parseHwmonAttributes(hwmon);
        cout << attrs.nodeName << (attrs ? " -> " + attrs.keyValue : "") << "\n";
        hwmonMap_.emplace((attrs ? attrs.alias : attrs.nodeName), attrs);
    }
}

void Config::createSensors()
{
    cout << "=== SENSORS ===\n";
    auto sensors = getNode("sensors");
    for(const auto& sensor : sensors) {
        SensorNode node;
        sensor >> node;
        cout << node.name << " " << node.type << " " << node.bind << "\n";
        if(node.type == "shell_cmd") {
            sensorMap_[node.name] = make_sensor<ShellSensor>(node.bind);
        }
        else if(hwmonMap_.contains(node.type)) {
            sensorMap_[node.name] = hwmonMap_.at(node.type).getSensor(node.bind);
            sensorMap_[node.name]->open();
        }
        else {
            throw std::invalid_argument("Wrong sensor type: " + node.type);
        }
    }
    cout << endl;
}

void Config::createPwms()
{
    cout << "=== PWMS ===\n";
    auto pwms = getNode("pwms");
    for(const auto& pwm : pwms) {
        PwmNode node;
        pwm >> node;
        cout << node.name << " " << node.type << " " << node.bind << " " << static_cast<uint32_t>(node.mode) << " "
             << node.minpwm << " " << node.maxpwm << "\n";
        if(hwmonMap_.contains(node.type)) {
            auto pwmObj = hwmonMap_.at(node.type).getPwm(node.bind);
            pwmObj->setMode(node.mode);
            pwmObj->setMin(node.minpwm);
            pwmObj->setMax(node.maxpwm);
            pwmObj->setFanStopHysteresis(node.fanStopHyst);
            pwmMap_[node.name] = pwmObj;
            pwmObj->open();
        }
        else {
            throw std::invalid_argument("Wrong pwm type: " + node.type);
        }
    }
    cout << endl;
}

void Config::createControllers()
{
    auto controllers = getNode("controllers");
    controllers_.reserve(controllers.size());
    for(const auto& controller : controllers) {
        ControllerNode node;
        controller >> node;
        ConfigEntry configEntry{};
        configEntry.setModeConfig(node.modeConfig).setPollConfig(node.pollConfig);
        for(auto& pwm : node.pwm) {
            if(pwmMap_.contains(pwm)) {
                configEntry.addPwm(pwmMap_[pwm]);
            }
            else {
                throw std::invalid_argument("Selected PWM is not defined: " + pwm);
            }
        }
        for(auto& sensor : node.sensor) {
            if(sensorMap_.contains(sensor)) {
                configEntry.addSensor(sensorMap_[sensor]);
            }
            else {
                throw std::invalid_argument("Selected Sensor is not defined: " + sensor);
            }
        }
        controllers_.push_back(std::make_shared<Controller>(node.name, configEntry));
    }
}

YAML::Node Config::getNode(const string& nodeName) const
{
    Node result;
    try {
        result = rootNode_[nodeName];
        if(!result.IsDefined()) {
            throw std::invalid_argument("Config node is not defined: " + nodeName);
        }
    }
    catch(const YAML::InvalidNode&) {
        throw std::invalid_argument("Config node not found: " + nodeName);
    }
    return result;
}
