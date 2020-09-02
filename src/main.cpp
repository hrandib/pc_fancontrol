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


using namespace std;
using ms = chrono::milliseconds;

int main(int /* argc */, const char** /* argv[] */) {
    Sensor::ptr sensor = make_sensor<SensorImpl>("/sys/class/hwmon/hwmon2/temp1");
    if (sensor->open()) {
        while(true) {
            cout << "\r    \r";
            cout << sensor->get();
            cout.flush();
            this_thread::sleep_for(ms(500));
        }
    }
    else {
        cout << "Error!" << endl;
        this_thread::sleep_for(ms(5000));
    }
}
