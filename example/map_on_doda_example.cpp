#include <doda_runtime.hpp>
#include <iostream>
#include <vector>
//#include "include/doda_simulator.hpp"

int main() {
    std::vector<uint32_t> input{1, 0, 1, 0, 2, 0};
    std::vector<uint32_t> output(6);

    // map_on_doda accepts a lambda function that takes uint32_t and returns uint32_t.
    // First map_on_doda call
    map_on_doda([](uint32_t x) { return x>=1? uint32_t(1): uint32_t(0); }, input, output);

    std::cout << "Output after the first map_on_doda: ";
    for (auto x : output) std::cout << x << " ";
    std::cout << std::endl << std::endl;

    // Second map_on_doda call, to check if it handles multiple calls correctly
    map_on_doda([](uint32_t x) { return x*2; }, input, output);

    std::cout << "\nOutput after the second map_on_doda: ";
    for (auto x : output) std::cout << x << " ";
    std::cout << std::endl;
}