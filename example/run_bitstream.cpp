// Standalone DODA simulator driver
// Usage: ./run_bitstream <bitstream.txt> [input_data.txt]
//
// Input data format (one value per line):
//   1
//   2
//   3
//   ...

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdint>
#include "doda_simulator.hpp"

std::vector<std::vector<std::string>> load_bitstream(const std::string& path) {
    std::vector<std::vector<std::string>> instructions;
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open bitstream file: " << path << std::endl;
        return instructions;
    }

    std::string line;
    std::vector<std::string> current_cluster;

    while (std::getline(file, line)) {
        if (line.empty()) {
            if (!current_cluster.empty()) {
                instructions.push_back(current_cluster);
                current_cluster.clear();
            }
        } else if (line[0] == '#') {
            // Skip comment lines
            continue;
        } else {
            current_cluster.push_back(line);
        }
    }

    if (!current_cluster.empty()) {
        instructions.push_back(current_cluster);
    }

    return instructions;
}

std::vector<uint32_t> load_input_data(const std::string& path) {
    std::vector<uint32_t> data;
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open input data file: " << path << std::endl;
        return data;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        data.push_back(static_cast<uint32_t>(std::stoul(line)));
    }

    return data;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <bitstream.txt> [input_data.txt]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "If no input_data.txt is provided, uses default test data [1, 2, 3, 4]" << std::endl;
        return 1;
    }

    std::string bitstream_path = argv[1];

    // Load bitstream
    std::cout << "Loading bitstream from: " << bitstream_path << std::endl;
    auto instructions = load_bitstream(bitstream_path);

    if (instructions.empty()) {
        std::cerr << "Error: No instructions loaded from bitstream file" << std::endl;
        return 1;
    }

    std::cout << "Loaded " << instructions.size() << " cluster(s)" << std::endl;
    for (size_t i = 0; i < instructions.size(); ++i) {
        std::cout << "  Cluster " << i << ": " << instructions[i].size() << " instructions" << std::endl;
    }

    // Load or generate input data
    std::vector<uint32_t> input_data;
    if (argc >= 3) {
        std::cout << "Loading input data from: " << argv[2] << std::endl;
        input_data = load_input_data(argv[2]);
    } else {
        std::cout << "Using default test data: [1, 2, 3, 4]" << std::endl;
        input_data = {1, 2, 3, 4};
    }

    std::cout << "Input data (" << input_data.size() << " elements): [";
    for (size_t i = 0; i < input_data.size(); ++i) {
        std::cout << input_data[i];
        if (i + 1 < input_data.size()) std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    // Initialize simulator
    std::cout << "\nInitializing DODA simulator..." << std::endl;
    DODASimulator simulator;
    simulator.initialize();

    // Program instructions
    std::cout << "Programming instructions..." << std::endl;
    simulator.programInstructions(instructions);

    // Load memory data
    std::vector<std::vector<int>> memory_data;
    std::vector<int> cluster0_data;
    for (uint32_t val : input_data) {
        cluster0_data.push_back(static_cast<int>(val));
    }
    memory_data.push_back(cluster0_data);

    std::cout << "Loading memory data..." << std::endl;
    simulator.loadMemoryData(memory_data);

    // Run simulation
    std::cout << "Starting execution..." << std::endl;
    simulator.startExecution();
    simulator.waitForCompletion();

    // Read results
    std::cout << "\nReading results..." << std::endl;
    auto result_memory = simulator.readMemory();

    if (!result_memory.empty()) {
        std::cout << "Output data (" << result_memory[0].size() << " elements): [";
        for (size_t i = 0; i < result_memory[0].size() && i < input_data.size(); ++i) {
            std::cout << result_memory[0][i];
            if (i + 1 < result_memory[0].size() && i + 1 < input_data.size()) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }

    std::cout << "\nSimulation complete." << std::endl;
    return 0;
}
