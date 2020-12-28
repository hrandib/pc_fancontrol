#include "main.h"

constexpr std::string_view VERSION = "1.0.1";

using namespace std;
using ms = chrono::milliseconds;

void sigintHandler(int) {
    Controller::stop();
    std::cout << "SIG handler finish" << endl;
}

int main(int /* argc */, const char** /* argv[] */) {
    signal(SIGINT, sigintHandler);
    signal(SIGTERM, sigintHandler);
    std::cout << "Version: " << VERSION << "\n";
    Config config{"/etc/fancontrol.yaml"};
    config.run();
}

