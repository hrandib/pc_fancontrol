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

using namespace std;
using ms = chrono::milliseconds;


class Cpu
{
private:
    static constexpr string_view formatString = "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_cur_freq";
    static constexpr size_t pathStringLength = formatString.size() + 4;
    static const size_t coresCount;
    string buf_;
    vector<ifstream> freqs_;
    int getCoreClock(size_t coreNumber)
    {
        string result;
        auto& stream = freqs_[coreNumber];
        stream >> result;
        stream.seekg(0);
        return stoi(result);
    }
public:
    Cpu() : buf_(pathStringLength, '\0'), freqs_(coresCount)
    {
        try {
            for(size_t i{}; i < coresCount; ++i) {
                sprintf(&buf_[0], formatString.data(), i);
//                cout << buf_ << endl;
                freqs_[i] = ifstream(buf_);
            }
        }
        catch(exception e) {
            cout << e.what() << endl;
            exit(1);
        }
    }
    void print_clocks() {
        for(size_t i{}; i < coresCount; ++i) {
            cout << getCoreClock(i) << ' ';
        }
        cout << endl;
    }
    double convert_to_ghz(int clock) {
        return double(clock)/1000000;
    }
    void print_mean_minmax() {
        //static int stat_max{};
        vector<int> clockArr(coresCount);
        for(int i{}; i < coresCount; ++i) {
            clockArr[i] = getCoreClock(i);
        }
        auto minmax = minmax_element(begin(clockArr), end(clockArr));
        int mean = accumulate(begin(clockArr), end(clockArr), 0) / coresCount;
        //stat_max = stat_max < *minmax.second ? *minmax.second : stat_max;
        cout << fixed << setprecision(2) << convert_to_ghz(mean) << " "
                                        << convert_to_ghz(*minmax.first) << " "
                                        << convert_to_ghz(*minmax.second) << " "
                                        << /* '\t' << stat_max <<*/ '\n';
    }
};

const size_t Cpu::coresCount = thread::hardware_concurrency();

int main(int /* argc */, const char** /* argv[] */) {
    Cpu cpu;
//    while(true) {
        cpu.print_mean_minmax();
//        this_thread::sleep_for(ms(1000));
//    }
}
