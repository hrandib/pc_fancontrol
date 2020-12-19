#include "main.h"

using namespace std;
using ms = chrono::milliseconds;

void sigintHandler(int) {
    Controller::stop();
    std::cout << "SIGINT handler" << endl;
}

int main(int /* argc */, const char** /* argv[] */) {
    signal(SIGINT, sigintHandler);
    Config config{"/etc/fancontrol.yaml"};
    config.run();
}

