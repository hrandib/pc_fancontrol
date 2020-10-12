#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <vector>
#include <string_view>
#include <cstddef>
#include <string>
#include <algorithm>
#include <numeric>
#include <iomanip>

#include "sysfs/writer_impl.h"
#include "sysfs/reader_impl.h"
#include "hwmon/sensor_impl.h"
#include "hwmon/pwm_impl.h"
#include "hwmon/hwmon.h"
#include "config.h"
#include "controller.h"
#include "cstdio"

using namespace std;
using ms = chrono::milliseconds;

int main(int /* argc */, const char** /* argv[] */) {
    Hwmon hwmon{"it8665"};
    ConfigEntry config;
    config.AddSensor(hwmon.getSensor("temp1"))
            .AddPwm(hwmon.getPwm("pwm1", 90, 255))
            .SetTwoPointConfMode(45, 75)
            .SetPollConfig(5);
    Controller controller{"First", config};
    if(config.GetPwms()[0]->open() && config.getSensors()[0]->open()) {
        std::cout << "Successfully opened\n";
        controller.run();
    }
    else {
        cout << "Error!" << endl;
        this_thread::sleep_for(ms(5000));
    }
}
