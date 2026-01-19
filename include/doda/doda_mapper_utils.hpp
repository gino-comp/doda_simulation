#pragma once

#include <string>
#include <bitset>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <doda/dfg_parser.hpp>

namespace doda_mapper_utils {

/**
 * Bitstream encoding constants
 */
struct BitstreamConstants {
    static constexpr int DATA_WIDTH = 32;
    static constexpr int PROG_MEM_WIDTH = 128;
    static constexpr int NUM_CLUSTER = 4;
    static constexpr int PES_PER_CLUSTER = 32;
    static constexpr int OPCODE_WIDTH = 5;
    static constexpr int SRC_PE_IDX_WIDTH = 5; // log2(32 PEs per cluster)
    static constexpr int SRC_IDX_WIDTH = SRC_PE_IDX_WIDTH + NUM_CLUSTER; // 4 bits for cluster OH
};

/**
 * Convert integer value to binary string of specified width
 */
inline std::string to_binary_string(long value, int width) {
    if (width <= 0) return "";
    if (width > 64) width = 64; // Prevent overflow
    
    std::string bin = std::bitset<64>(value).to_string();
    return bin.substr(bin.size() - width);
}

/**
 * Calculate ceiling of log2
 */
inline int log2_ceil(int x) {
    if (x <= 0) return 0;
    int result = 0;
    x--;
    while (x > 0) {
        x >>= 1;
        result++;
    }
    return result;
}


/**
 * Map from DFG JSON operation strings to Opcode enum
 */
inline Opcode map_string_to_opcode(const std::string& op_str) {
    if (op_str == "nil") return Opcode::NIL;
    if (op_str == "add") return Opcode::ADD;
    if (op_str == "sub") return Opcode::SUB;
    if (op_str == "mul") return Opcode::MUL;
    if (op_str == "shl") return Opcode::LS;
    if (op_str == "lshr" || op_str == "ashr") return Opcode::RS;
    if (op_str == "and") return Opcode::AND;
    if (op_str == "or") return Opcode::OR;
    if (op_str == "xor") return Opcode::XOR;
    if (op_str == "select") return Opcode::SELECT;
    if (op_str == "icmp_eq") return Opcode::CMP;
    if (op_str == "icmp_ne") return Opcode::CNE;
    if (op_str == "icmp_slt" || op_str == "icmp_ult") return Opcode::CLT;
    if (op_str == "icmp_sle" || op_str == "icmp_ule") return Opcode::CLTE;
    if (op_str == "icmp_sgt" || op_str == "icmp_ugt") return Opcode::CGT;
    if (op_str == "icmp_sge" || op_str == "icmp_uge") return Opcode::CGTE;
    
    return Opcode::UNSUPPORTED;
}



/**
 * Load and parse a JSON file
 * @param file_path Path to the JSON file
 * @return Parsed JSON object
 * @throws std::runtime_error if file cannot be opened
 * @throws nlohmann::json::parse_error if JSON parsing fails
 */
inline nlohmann::json load_and_parse_json(const std::string& file_path) {
    std::ifstream input_file(file_path);
    if (!input_file) {
        std::cerr << "[load_and_parse_json] Error: Could not open " << file_path << std::endl;
        throw std::runtime_error("Failed to open JSON file: " + file_path);
    }

    nlohmann::json result;
    try {
        input_file >> result;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[load_and_parse_json] Error: Failed to parse JSON: " << e.what() << std::endl;
        throw;
    }

    return result;
}

} // namespace doda_mapper_utils