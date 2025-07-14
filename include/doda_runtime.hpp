#pragma once

#include <vector>
#include <cstdint>
#include <type_traits>
#include <iostream>
#include <cassert>
#include <dlfcn.h>
#include <unistd.h>  // for access()
#include <string>
#include <fstream>
#ifndef DODA_SIMULATION_MODE
#include <doda/dfg_parser.hpp>
#include <doda/doda_mapper.hpp>
#endif

#ifdef DODA_SIMULATION_MODE
#include "doda_simulator.hpp"
#endif

//#define VERBOSE  // Uncomment to enable verbose output

// The function pointer type for compiled lambdas
typedef uint32_t (*lambda_t)(uint32_t);

// Simple struct to keep run-time metadata
struct RuntimeMetadata { 
    bool vector_size_check;  // Did runtime size check pass?
    int size_bytes;          // Size of data type in bytes
};

// Function to add metadata to DFG file
inline void add_metadata(const std::string& dfg_path, const RuntimeMetadata& metadata) {
    try {
        // Read the existing DFG file
        std::ifstream inFile(dfg_path);
        std::string content((std::istreambuf_iterator<char>(inFile)),
                            std::istreambuf_iterator<char>());
        inFile.close();
        
        // Simple string-based modification to add/update metadata
        // This is a basic implementation - in production, use a proper JSON library
        
        // Find the closing brace of the JSON
        size_t lastBrace = content.rfind('}');
        if (lastBrace != std::string::npos) {
            // Check if metadata field exists
            size_t metadataPos = content.find("\"runtime_metadata\"");
            
            if (metadataPos != std::string::npos) {
                // Replace the existing metadata block
                size_t metadataBlockStart = content.find('{', metadataPos);
                size_t metadataBlockEnd = content.find('}', metadataBlockStart) + 1;
                
                // Create new metadata content
                std::string newMetadata = "\"runtime_metadata\": {\n    \"input_size_in_bytes\": " + 
                                         std::to_string(metadata.size_bytes) + ",\n    " +
                                         "\"vector_size_checked\": " + 
                                         (metadata.vector_size_check ? "true" : "false") + "\n  }";
                
                // Replace existing metadata with new content
                content.replace(metadataPos, metadataBlockEnd - metadataPos, newMetadata);
            } else {
                // Add metadata if it doesn't exist by appending to the last line with content
    
                // Find the last content line (ignoring whitespace lines at the end)
                size_t lastContentLine = content.find_last_not_of(" \t\n\r", lastBrace - 1);
                if (lastContentLine != std::string::npos) {
                    // Insert a comma right after the last content character
                    content.insert(lastContentLine + 1, ",");
        
                    // Then add the metadata on new lines
                    std::string newMetadata = "\n  \"runtime_metadata\": {\n    \"input_size_in_bytes\": " + 
                                            std::to_string(metadata.size_bytes) + ",\n    " +
                                            "\"vector_size_checked\": " + 
                                            (metadata.vector_size_check ? "true" : "false") + "\n  }";
        
                    content.insert(lastBrace, newMetadata);
                }
            }
            
            // Write the updated content back to the file
            std::ofstream outFile(dfg_path);
            outFile << content;
            outFile.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to update DFG with metadata: " << e.what() << std::endl;
    }
}

#ifndef DODA_SIMULATION_MODE
// Dynamically loads a pre-compiled lambda in a shared library by index and returns the function pointer
inline lambda_t load_lambda(int lambda_index, RuntimeMetadata& metadata) {
    // Parse DFG
    std::string dfg_path = "./obj/lambda_" + std::to_string(lambda_index) + "_dfg.json";
    auto dfg = parseDFG(dfg_path);

    // Load shared library
    const char* so_path = "./obj/liblambda.so";
    void* handle = dlopen(so_path, RTLD_LAZY);
    assert(handle && "Failed to load liblambda.so");

    // Get lambda function
    std::string symbol_name = "lambda_" + std::to_string(lambda_index);
    lambda_t f = (lambda_t)dlsym(handle, symbol_name.c_str());
    assert(f && ("Failed to find symbol '" + symbol_name).c_str());

    // Update the DFG with the run-time metadata
    add_metadata(dfg_path, metadata);

    // DODA mapper augments the DFG and generates the bitstream for DODA
    doda_mapper mapper(dfg_path);

    // Generate the bitstream
    std::string bitstream_path = "./obj/lambda_" + std::to_string(lambda_index) + "_bitstream.txt";

    std::vector<std::vector<std::string>> bitstream = mapper.generate_bitstream();
    std::ofstream out(bitstream_path);
    
    for (int cluster = 0; cluster < bitstream.size(); cluster++) {
        out << "#Cluster index: " << cluster << "\n";
        //#ifdef VERBOSE
        //std::cout << "#Cluster index: " << cluster << "\n";
        //#endif
        for (int pe = 0; pe < bitstream[cluster].size(); pe++) {
            out << bitstream[cluster][pe] << "\n";
            //#ifdef VERBOSE
            //std::cout << bitstream[cluster][pe] << "\n";
            //#endif
        }
        out << "\n";
    }
    out.close();

    return f;
}
#endif

#ifdef DODA_SIMULATION_MODE
// Function to execute on DODA hardware simulator
inline void execute_on_doda_simulator(int lambda_index, const std::vector<uint32_t>& input, std::vector<uint32_t>& output) {
    // Create simulator instance
    DODASimulator simulator;
    
    // Initialize the simulator
    simulator.initialize();
    
    // Read the bitstream file generated by load_lambda
    std::string bitstream_path = "./obj/lambda_" + std::to_string(lambda_index) + "_bitstream.txt";
    std::ifstream bitstream_file(bitstream_path);
    
    if (!bitstream_file.is_open()) {
        std::cerr << "[ERROR] Failed to open bitstream file: " << bitstream_path << std::endl;
        return;
    }
    
    // Parse the bitstream file to get instructions
    std::vector<std::vector<std::string>> instructions;
    std::string line;
    std::vector<std::string> current_cluster;
    
    while (std::getline(bitstream_file, line)) {
        if (line.empty()) {
            if (!current_cluster.empty()) {
                instructions.push_back(current_cluster);
                current_cluster.clear();
            }
        } else if (line.find("#Cluster index:") == 0) {
            // Skip cluster header lines
            continue;
        } else {
            current_cluster.push_back(line);
        }
    }
    
    // Add the last cluster if it exists
    if (!current_cluster.empty()) {
        instructions.push_back(current_cluster);
    }
    
    bitstream_file.close();
    
    // Program the DODA hardware with the bitstream
    simulator.programInstructions(instructions);
    
    // Prepare memory data from input vector
    std::vector<std::vector<int>> memory_data;
    std::vector<int> input_data;
    for (uint32_t val : input) {
        input_data.push_back(static_cast<int>(val));
    }
    memory_data.push_back(input_data);

    #ifdef VERBOSE
        std::cout << "memory_data:" << std::endl;
        for (size_t i = 0; i < memory_data.size(); ++i) {
            std::cout << "  Cluster " << i << ": [";
            for (size_t j = 0; j < memory_data[i].size(); ++j) {
                std::cout << memory_data[i][j];
                if (j + 1 < memory_data[i].size()) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
        }
    #endif

    
    // Load memory data into DODA
    simulator.loadMemoryData(memory_data);
    
    // Start execution
    simulator.startExecution();
    
    // Wait for completion
    simulator.waitForCompletion();
    
    // Read results from memory
    auto result_memory = simulator.readMemory();
    
    // Extract output data
    if (!result_memory.empty() && result_memory[0].size() >= output.size()) {
        for (size_t i = 0; i < output.size(); ++i) {
            output[i] = static_cast<uint32_t>(result_memory[0][i]);
        }
    }
}
#endif

// Only allow functions that take uint32_t and return uint32_t
static int g_lambda_counter = 0;        // global counter to keep track of lambda IDs

#ifndef DODA_SIMULATION_MODE
// CPU mode: generate bitstream and execute on CPU
template<typename Func>
typename std::enable_if<
    std::is_same<typename std::result_of<Func(uint32_t)>::type, uint32_t>::value
>::type
map_on_doda(Func /*f*/, const std::vector<uint32_t>& input, std::vector<uint32_t>& output) {
    // Check if the input and output vectors are of the same size
    RuntimeMetadata metadata;
    metadata.vector_size_check = (input.size() == output.size());
    // Later, we might have to support the case where input and output have different types.
    metadata.size_bytes = sizeof(input);
    
    // Size safety check
    if (!metadata.vector_size_check) {
        std::cerr << "[ERROR] Input vector size (" << input.size() 
                << ") is not equal to output vector size (" << output.size() 
                << ").\n";
        assert(metadata.vector_size_check && "Input and output vector sizes must match.");
    }
    
    // Instantiate the metadata struct and pass it to load_lambda
    lambda_t compiled_lambda = load_lambda(g_lambda_counter++, metadata);
    
    // Execute the lambda on each input element
    for (size_t i = 0; i < input.size(); ++i)
        output[i] = compiled_lambda(input[i]);
}
#else
// Simulation mode: read bitstream and execute on simulator
template<typename Func>
typename std::enable_if<
    std::is_same<typename std::result_of<Func(uint32_t)>::type, uint32_t>::value
>::type
map_on_doda(Func /*f*/, const std::vector<uint32_t>& input, std::vector<uint32_t>& output) {
    // Check if the input and output vectors are of the same size
    if (input.size() != output.size()) {
        std::cerr << "[ERROR] Input vector size (" << input.size() 
                << ") is not equal to output vector size (" << output.size() 
                << ").\n";
        assert(false && "Input and output vector sizes must match.");
    }
    
    // Execute on simulator using existing bitstream
    execute_on_doda_simulator(g_lambda_counter++, input, output);
}
#endif