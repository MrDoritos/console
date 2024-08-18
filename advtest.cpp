#include "advancedConsole.h"
#include <unistd.h>

int main() {
    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
    adv::setFrametime(100.0f);
    while (!HASKEY(console::readKeyAsync(), 'q')) {
        long elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
        std::string str = "Elapsed time: " + std::to_string(elapsed) + "us";
        adv::write(0,0,str.c_str());
        usleep(1);
    }
}