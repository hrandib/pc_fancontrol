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
#include <iostream>

using std::cout, std::endl;
using PollConf = ConfigEntry::PollConf;
using SetMode = ConfigEntry::SetMode;
using ModeConf = ConfigEntry::ModeConf;
using StringVector = std::vector<std::string>;

static inline std::string to_string(StringVector& vec) {
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

static void operator >>(const YAML::Node& node, SensorNode& sensNode)
{
    for(auto it = node.begin(); it != node.end(); ++it) {
        sensNode.name = it->first.as<std::string>();
        for(auto it2 = it->second.begin(); it2 != it->second.end(); ++ it2) {
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
    int minpwm, maxpwm;
    Pwm::Mode mode{Pwm::Mode::NoChange};
};

static void operator >>(const YAML::Node& node, PwmNode& pwmNode)
{
    for(auto it = node.begin(); it != node.end(); ++it) {
        pwmNode.name = it->first.as<std::string>();
        for(auto it2 = it->second.begin(); it2 != it->second.end(); ++ it2) {
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
    SetMode mode;
    PollConf pollConfig;
    ModeConf modeConfig;
    bool autoOff;
};

static inline int parseTime(const YAML::Node& poll)
{
    auto stringValue = poll.as<std::string>();
    int result;
    try {
        result = std::stoi(stringValue);
    }
    catch(std::exception&) {
        throw std::invalid_argument("Error parsing poll time: " + stringValue);
    }
    if (stringValue.find("ms") == std::string::npos) {
        //used field units - secs, convert to ms
        result *= 1000;
    }
    return result;
}

static inline PollConf parsePollConfig(const YAML::Node& poll)
{
    PollConf result;
    result.samplesCount = 1;
    if(poll.IsScalar()) {
        result.mode = PollConf::PollSimple;
        result.timeMsecs = parseTime(poll);
    }
    else {
        result.samplesCount = 1;
        for(auto it = poll.begin(); it != poll.end(); ++it) {
            auto key = it->first.as<std::string>();
            if(key == "time") {
                result.timeMsecs = parseTime(it->second);
            }
            else if(key == "ma_samples") {
                result.samplesCount = it->second.as<int>();
                result.mode = PollConf::PollMovingAverage;
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
        result.push_back(node.as<std::string>());
    }
    return result;
}

static inline ModeConf parseModeConfig(SetMode mode, const YAML::Node& node)
{
    ModeConf result;
    switch(mode) {
    case ConfigEntry::SETMODE_TWO_POINT: {
        ConfigEntry::TwoPointConfMode confMode;
        for(auto it = node.begin(); it != node.end(); ++it) {
            auto key = it->first.as<std::string>();
            if(key == "a") {
                confMode.temp_a = it->second.as<int>();
            }
            else if(key == "b") {
                confMode.temp_b = it->second.as<int>();
            }
            else {
                throw std::invalid_argument("unrecognized attribute: " + key);
            }
        }
        result = confMode;
    }
        break;
    case ConfigEntry::SETMODE_MULTI_POINT: {
        ConfigEntry::MultiPointConfMode confMode;
        for(auto it = node.begin(); it != node.end(); ++it) {
            auto key = it->first.begin()->first.as<int>();
            auto val = it->first.begin()->second.as<int>();
            confMode.pointVec.emplace_back(key, val);
        }
        result = confMode;
    }
        break;
    case ConfigEntry::SETMODE_PI: {
        ConfigEntry::PiConfMode confMode;
        for(auto it = node.begin(); it != node.end(); ++it) {
            auto key = it->first.as<std::string>();
            if(key == "temp") {
                confMode.temp = it->second.as<int>();
            }
            else if(key == "p") {
                confMode.p = it->second.as<int>();
            }
            else if(key == "i") {
                confMode.i = it->second.as<int>();
            }
            else {
                throw std::invalid_argument("unrecognized attribute: " + key);
            }
        }
        result = confMode;
    }
        break;

    }
    return result;
}

static void operator >>(const YAML::Node& node, ControllerNode& controllerNode) {

    controllerNode.autoOff = false;
    controllerNode.pollConfig.mode = PollConf::PollSimple;
    controllerNode.pollConfig.samplesCount = 1;
    controllerNode.pollConfig.timeMsecs = 1000;

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
                    cout << "Incompatible controller mode, entry will be skipped: "
                            + rawMode << "\n";
                    break;
                }
            }
            else if(key == "poll") {
                controllerNode.pollConfig = parsePollConfig(it2->second);
            }
            else if(key == "set") {
                controllerNode.modeConfig = parseModeConfig(controllerNode.mode, it2->second);
            }
            else if(key == "fan_stop") {
                controllerNode.autoOff = it2->second.as<bool>();
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

void Config::createHwmons()
{
    auto hwmonList = getNode("hwmon").as<std::vector<string>>();
    for (auto& hwmon : hwmonList) {
        std::cout << hwmon << ": ";
        hwmonMap_.emplace(hwmon, hwmon);
    }
    std::cout << "\n";
}

void Config::createSensors()
{
    auto sensors = getNode("sensors");
    for(const auto& sensor : sensors) {
        SensorNode node;
        sensor >> node;
        cout << node.name << " " << node.type << " " << node.bind << "\n";
        if (node.type == "shell_cmd") {
            //TODO: implement
        }
        else if (hwmonMap_.contains(node.type)){
            sensorMap_[node.name] = hwmonMap_[node.type].getSensor(node.bind);
        }
        else {
            throw std::invalid_argument("Wrong sensor type: " + node.type);
        }
    }
    cout << endl;
}

void Config::createPwms()
{
    auto pwms = getNode("pwms");
    for(const auto& pwm : pwms) {
        PwmNode node;
        pwm >> node;
        cout << node.name << " "
             << node.type << " "
             << node.bind << " "
             << static_cast<uint32_t>(node.mode) << " "
             << node.minpwm << " "
             << node.maxpwm << "\n";
        if (hwmonMap_.contains(node.type)) {
            auto pwmObj = hwmonMap_[node.type].getPwm(node.bind);
            pwmObj->setMode(node.mode);
            pwmObj->setMin(node.minpwm);
            pwmObj->setMax(node.maxpwm);
            pwmMap_[node.name] = pwmObj;
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
    for(const auto& controller : controllers) {
        ControllerNode node;
        controller >> node;
        cout << node.name << " "
             << to_string(node.sensor) << " "
             << to_string(node.pwm) << " "
             << static_cast<uint32_t>(node.mode) << " "
             << node.pollConfig.mode << " "
             << node.pollConfig.samplesCount << " "
             << node.pollConfig.timeMsecs << "\n";
        switch(node.mode) {
        case SetMode::SETMODE_TWO_POINT:
        {
            ConfigEntry::TwoPointConfMode conf =
                    std::get<SetMode::SETMODE_TWO_POINT>(node.modeConfig);
            cout << conf.temp_a << " " << conf.temp_b << endl;
        }
            break;
        case SetMode::SETMODE_MULTI_POINT:
        {
            ConfigEntry::MultiPointConfMode conf =
                    std::get<SetMode::SETMODE_MULTI_POINT>(node.modeConfig);
            for(auto& point : conf.pointVec) {
                cout << point.first << "<>" << point.second << " ";
            }
            cout << endl;
        }
            break;
        case ConfigEntry::SETMODE_PI:
        {
            ConfigEntry::PiConfMode conf = std::get<SetMode::SETMODE_PI>(node.modeConfig);
            cout << conf.temp << "," << conf.p << "," << conf.i << endl;
        }
            break;
        }
    }
    cout << endl;
}

YAML::Node Config::getNode(const string& nodeName) const
{
    Node result;
    try {
        result = rootNode_[nodeName];
        if(!result.IsDefined()) {
            throw std::invalid_argument("Config node is not defined: " + nodeName);
        }
    } catch(const YAML::InvalidNode&) {
        throw std::invalid_argument("Config node not found: " + nodeName);
    }
    return result;
}
