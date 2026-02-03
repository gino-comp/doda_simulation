// Standalone DODA simulator driver with interactive memory updates
// Usage: ./run_bitstream <bitstream.txt>
//
// Commands:
//   run                  - Execute simulation with current memory
//   set <c> <i> <v>      - Set memory[cluster][index] = value
//   show                 - Display current memory contents
//   reset                - Reset simulator
//   quit                 - Exit program

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdint>
#include "doda_simulator.hpp"

// Static initial memory data - 2D vector [cluster][index]
// Modify these values as needed for your test cases
static std::vector<std::vector<int>> g_memory_data = {
    {1, 2, 3, 4, 5, 6, 7, 8},  // Cluster 0
    {0, 0, 0, 0, 0, 0, 0, 0},  // Cluster 1
    {0, 0, 0, 0, 0, 0, 0, 0},  // Cluster 2
    {0, 0, 0, 0, 0, 0, 0, 0}   // Cluster 3
};

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

void print_memory(const std::vector<std::vector<int>>& mem) {
    for (size_t c = 0; c < mem.size(); ++c) {
        std::cout << "Cluster " << c << ": [";
        for (size_t i = 0; i < mem[c].size(); ++i) {
            std::cout << mem[c][i];
            if (i + 1 < mem[c].size()) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
}

void print_help() {
    std::cout << "\nCommands:\n"
              << "  run                  - Execute simulation with current memory\n"
              << "  set <c> <i> <v>      - Set memory[cluster][index] = value\n"
              << "  show                 - Display current memory contents\n"
              << "  reset                - Reset simulator\n"
              << "  help                 - Show this help\n"
              << "  quit                 - Exit program\n" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <bitstream.txt>" << std::endl;
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

    // Initialize simulator
    std::cout << "\nInitializing DODA simulator..." << std::endl;
    DODASimulator simulator;
    simulator.initialize();

    // Program instructions
    std::cout << "Programming instructions..." << std::endl;
    simulator.programInstructions(instructions);

    std::cout << "\nStatic initial memory:" << std::endl;
    print_memory(g_memory_data);
    print_help();

    // Interactive loop
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "quit" || cmd == "q" || cmd == "exit") {
            break;
        } else if (cmd == "help" || cmd == "h" || cmd == "?") {
            print_help();
        } else if (cmd == "show" || cmd == "s") {
            print_memory(g_memory_data);
        } else if (cmd == "set") {
            int cluster, index, value;
            if (iss >> cluster >> index >> value) {
                if (cluster >= 0 && cluster < static_cast<int>(g_memory_data.size()) &&
                    index >= 0 && index < static_cast<int>(g_memory_data[cluster].size())) {
                    g_memory_data[cluster][index] = value;
                    std::cout << "Set memory[" << cluster << "][" << index << "] = " << value << std::endl;
                } else {
                    std::cerr << "Error: Index out of range" << std::endl;
                }
            } else {
                std::cerr << "Usage: set <cluster> <index> <value>" << std::endl;
            }
        } else if (cmd == "reset") {
            simulator.reset();
            simulator.initialize();
            simulator.programInstructions(instructions);
            std::cout << "Simulator reset and reprogrammed." << std::endl;
        } else if (cmd == "run" || cmd == "r") {
            std::cout << "Loading memory..." << std::endl;
            simulator.loadMemoryData(g_memory_data);

            std::cout << "Starting execution..." << std::endl;
            simulator.startExecution();
            simulator.waitForCompletion();

            // Read results and copy to g_memory_data for next iteration
            auto result_memory = simulator.readMemory();
            g_memory_data = result_memory;
            std::cout << "\nOutput:" << std::endl;
            print_memory(g_memory_data);
            print_help();
        } else if (!cmd.empty()) {
            std::cerr << "Unknown command: " << cmd << ". Type 'help' for commands." << std::endl;
        }
    }

    std::cout << "Goodbye." << std::endl;
    return 0;
}
