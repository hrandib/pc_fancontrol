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

struct SensorNode
{
    std::string name;
    std::string type;
    std::string bind;
};

static void operator >>(const YAML::Node& node, SensorNode& sensNode) {
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

static void operator >>(const YAML::Node& node, PwmNode& pwmNode) {
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
    using StringVector = std::vector<std::string>;

    std::string name;
    StringVector sensor;
    StringVector pwm;
    ConfigEntry::SetMode mode;
    ConfigEntry::PollConf pollConfig;
    ConfigEntry::ModeConf modeConfig;
};

static void operator >>(const YAML::Node& node, ControllerNode& controlllerNode) {
    for(auto it = node.begin(); it != node.end(); ++it) {
        controlllerNode.name = it->first.as<std::string>();
        for(auto it2 = it->second.begin(); it2 != it->second.end(); ++ it2) {
            auto key = it2->first.as<std::string>();
            if(key == "sensor") {
                //TODO: array support
                controlllerNode.sensor.push_back(it2->second.as<std::string>());
            }
            else if(key == "pwm") {
                //TODO: array support
                controlllerNode.pwm.push_back(it2->second.as<std::string>());
            }
            else if(key == "mode") {
                auto rawMode = it2->second.as<std::string>();
                if(rawMode == "two_point") {
                    controlllerNode.mode = ConfigEntry::SETMODE_TWO_POINT;
                }
                else if(rawMode == "multi_point") {
                    controlllerNode.mode = ConfigEntry::SETMODE_MULTI_POINT;
                }
                else if(rawMode == "pi") {
                    controlllerNode.mode = ConfigEntry::SETMODE_PI;
                }
                else {
                    cout << "Incompatible controller mode, entry will be skipped: "
                            + rawMode << "\n";
                    break;
                }
            }
            else if(key == "poll") {
                //TODO: moving average support
                auto stringValue = it2->second.as<std::string>();
                auto intValue = it2->second.as<int>();
                if (stringValue.find("ms") == std::string::npos) {
                    //used field units - secs, convert to ms
                    intValue *= 1000;
                }
                controlllerNode.pollConfig.timeMsecs = intValue;
            }
            else if(key == "set") {
                //controlllerNode.pwm.push_back(it2->second.as<std::string>());
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
             << node.sensor[0] << " "
             << node.pwm[0] << " "
             << static_cast<uint32_t>(node.mode) << " "
             //<< node.pollConfig.mode << " "
             << node.pollConfig.timeMsecs << "\n";
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
