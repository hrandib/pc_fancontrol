#include "main.h"

constexpr std::string_view VERSION = "1.3.3";

using namespace std;
using ms = chrono::milliseconds;

void sigintHandler(int /*signum*/)
{
    Controller::stop();
    std::cout << "SIG handler finish" << endl;
}

int main(int /* argc */, const char** /* argv[] */)
{
    signal(SIGINT, sigintHandler);
    signal(SIGTERM, sigintHandler);
    std::cout << "Version: " << VERSION << (DEBUG_MODE ? "-debug\n" : "\n");
    Config config{"/etc/fancontrol.yaml"};
    config.run();
}
