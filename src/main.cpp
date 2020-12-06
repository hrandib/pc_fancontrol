#include "main.h"

using namespace std;
using ms = chrono::milliseconds;

void sigintHandler(int) {
    Controller::stop();
    std::cout << "SIGINT handler" << endl;
}

int main(int /* argc */, const char** /* argv[] */) {
    signal(SIGINT, sigintHandler);
    Hwmon hwmon{"it8665"};
    ConfigEntry config;
    config.AddSensor(hwmon.getSensor("temp1"))
            .AddPwm(hwmon.getPwm("pwm1", 90, 255))
            .SetTwoPointConfMode(45, 75)
            .SetPollConfig(5);
    auto sensor = config.getSensors()[0];
    sensor->open();
    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 1000'000; i; i--) {
        volatile auto val = sensor->get();
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    cout << "Execution time: " << duration << endl;
//    Controller controller{"First", config};
//    if(config.GetPwms()[0]->open() && config.getSensors()[0]->open()) {
//        std::cout << "Successfully opened\n";
//        controller.run();
//    }
//    else {
//        cout << "Error!" << endl;
//    }
}

