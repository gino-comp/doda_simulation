#pragma once

/**
 * DODA Compiler Libraries - Unified Header
 * 
 * This header provides access to all DODA compiler functionality:
 * - Core data structures and utilities
 * - DFG generation from LLVM IR
 * - Mapping and bitstream generation
 * - Lambda extraction (via Clang plugin)
 * 
 * Usage:
 *   #include <doda.h>
 *   
 * Link with: -ldoda_compiler -ldoda_dfg -ldoda_mapper -ldoda_core
 */

// Core functionality
#include <doda/dfg_parser.hpp>
#include <doda/doda_mapper_utils.hpp>
#include <doda/doda_mapper.hpp>
#include <doda/mapping_txt_parser.hpp>

// Library version info
#define DODA_VERSION_MAJOR 1
#define DODA_VERSION_MINOR 0
#define DODA_VERSION_PATCH 0

namespace doda {

/**
 * Get library version string
 */
inline const char* getVersion() {
    return "1.0.0";
}

/**
 * Simple wrapper functions for common operations
 */

/**
 * Parse DFG from JSON file
 * @param filename Path to DFG JSON file
 * @return Parsed DFG structure
 */
DFG loadDFG(const std::string& filename);

/**
 * Create mapper and generate bitstream from DFG file
 * @param dfg_file Path to DFG JSON file
 * @return 2D vector of bitstream strings [cluster][pe]
 */
std::vector<std::vector<std::string>> generateBitstream(const std::string& dfg_file);

/**
 * End-to-end compilation from DFG JSON to bitstream files
 * @param dfg_file Input DFG JSON file
 * @param output_dir Output directory for bitstream files
 * @return true if successful
 */
bool compileDFGToBitstream(const std::string& dfg_file, const std::string& output_dir);

/**
 * Parse a Mapper_Node text file and return a Mapper_DFG
 * @param txt_file Path to Mapper_Node text file (e.g., DFG_CONV_Mapping.txt)
 * @return Parsed Mapper_DFG structure
 */
Mapper_DFG parseMappingTxt(const std::string& txt_file);

/**
 * Generate bitstream from a Mapper_Node text file
 * @param txt_file Path to Mapper_Node text file
 * @return 2D vector of bitstream strings [cluster][pe]
 */
std::vector<std::vector<std::string>> generateBitstreamFromTxt(const std::string& txt_file);

/**
 * End-to-end compilation from Mapper_Node text file to bitstream files
 * @param txt_file Input Mapper_Node text file
 * @param output_dir Output directory for bitstream files
 * @return true if successful
 */
bool compileTxtToBitstream(const std::string& txt_file, const std::string& output_dir);

} // namespace doda
