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
            if(it2->first.as<std::string>() == "type") {
                sensNode.type = it2->second.as<std::string>();
            }
            else if(it2->first.as<std::string>() == "bind") {
                sensNode.bind = it2->second.as<std::string>();
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
    parseHwmon();
    parseSensors();
}

void Config::parseHwmon()
{
    auto hwmonList = getNode("hwmon").as<std::vector<string>>();
    for (auto& hwmon : hwmonList) {
        std::cout << hwmon << ": ";
        hwmonMap_.emplace(hwmon, hwmon);
    }
    std::cout << "\n";
}

void Config::parseSensors()
{
    auto sensors = getNode("sensors");
    for(const auto& sensor : sensors) {
        SensorNode node;
        sensor >> node;
        cout << node.name << " " << node.type << " " << node.bind << endl;
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
