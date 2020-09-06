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
#include "cstdio"

using namespace std;
using ms = chrono::milliseconds;

int main(int /* argc */, const char** /* argv[] */) {
    Hwmon hwmon{"it8665"};
    cout << endl;
//    Sensor::ptr sensor = hwmon.getSensor("temp1");
    PwmImpl pwm = PwmImpl{hwmon.getHwmonPath()/"pwm2", 90, 255};
    if(pwm.open()) {
        std::cout << "Successfully opened\n";
        char c;
        uint_fast8_t val = 0;
        while(true) {
            c = static_cast<char>(getchar());
            switch(c) {
            case 'a':
                pwm.set(val, "console");
                if(val > 0) {
                    --val;
                }
                break;
            case 's':
                pwm.set(val, "console");
                if(val < 100) {
                    ++val;
                }
                break;
            default:
                goto x;
            }
            cout << val << endl;
        }
//        while(true) {
//            cout << "\r                                         \r";
//            cout << sensor->get();
//            cout.flush();
//            this_thread::sleep_for(ms(500));
//        }
    }
    else {
        cout << "Error!" << endl;
        this_thread::sleep_for(ms(5000));
    }
x: cout << "finish" << endl;
}
