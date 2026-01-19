#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <algorithm>
#include <doda/doda_mapper.hpp>

namespace doda_mapping_parser {

/**
 * Parse a Mapper_Node text format file and build a Mapper_DFG
 *
 * Expected format:
 *   Mapper_Node(id: <name> (pe_idx: <N>), op: <OP>, initial_output_used: <0|1>, initial_output: <val>, inputs: [
 *       type: <i1|i2|pred>, src_id: <name> (pe_index: <N>), const_value: <val>
 *       ...
 *   ])
 */
class MappingTxtParser {
public:
    static Mapper_DFG parse(const std::string& filepath) {
        Mapper_DFG dfg;
        std::ifstream file(filepath);

        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filepath);
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        file.close();

        // Regex to match Mapper_Node blocks
        // Mapper_Node(id: <id> (pe_idx: <pe>), op: <op>, initial_output_used: <used>, initial_output: <init>, inputs: [
        std::regex node_regex(
            R"(Mapper_Node\s*\(\s*id:\s*([^\s(]+)\s*\(pe_idx:\s*(\d+)\s*\)\s*,\s*op:\s*(\w+)\s*,\s*initial_output_used:\s*(\d+)\s*,\s*initial_output:\s*(-?\d+)\s*,\s*inputs:\s*\[([^\]]*)\]\s*\))",
            std::regex::multiline
        );

        // Regex to match input lines
        // type: <type>, src_id: <id> (pe_index: <pe>), const_value: <val>
        std::regex input_regex(
            R"(type:\s*(\w+)\s*,\s*src_id:\s*([^\s(]+)\s*\(pe_index:\s*(-?\d+)\s*\)\s*,\s*const_value:\s*(-?\d+))"
        );

        Mapper_Node::reset_node_counter();

        std::sregex_iterator node_it(content.begin(), content.end(), node_regex);
        std::sregex_iterator node_end;

        // First pass: collect all nodes with their pe_idx to build pe_idx mapping
        std::map<std::string, int> id_to_pe_idx;
        std::vector<std::tuple<std::string, int, Opcode, bool, int, std::string>> node_data;

        for (; node_it != node_end; ++node_it) {
            std::smatch match = *node_it;
            std::string node_id = match[1].str();
            int pe_idx = std::stoi(match[2].str());
            std::string op_str = match[3].str();
            bool initial_output_used = (match[4].str() == "1");
            int initial_output = std::stoi(match[5].str());
            std::string inputs_block = match[6].str();

            id_to_pe_idx[node_id] = pe_idx;
            // Convert opcode to lowercase for toOpcode compatibility
            std::transform(op_str.begin(), op_str.end(), op_str.begin(), ::tolower);
            node_data.emplace_back(node_id, pe_idx, toOpcode(op_str),
                                   initial_output_used, initial_output, inputs_block);
        }

        // Second pass: create nodes and add inputs
        for (const auto& [node_id, pe_idx, op, init_used, init_val, inputs_block] : node_data) {
            // Add node to DFG
            dfg.add_node(node_id, op, init_used, init_val);
            auto& node = dfg.get_node(node_id);

            // Manually set PE index to match the file
            node.set_pe_index(pe_idx);

            // Parse inputs
            std::sregex_iterator input_it(inputs_block.begin(), inputs_block.end(), input_regex);
            std::sregex_iterator input_end;

            for (; input_it != input_end; ++input_it) {
                std::smatch input_match = *input_it;
                std::string input_type = input_match[1].str();
                std::string src_id = input_match[2].str();
                int src_pe_index = std::stoi(input_match[3].str());
                int const_value = std::stoi(input_match[4].str());

                if (src_id == "const") {
                    // Constant input
                    node.add_input(input_type, const_value);
                } else {
                    // Reference to another node
                    node.add_input(input_type, src_id);
                    // Set the source PE index directly
                    auto& inputs = const_cast<std::vector<Input>&>(node.get_inputs());
                    inputs.back().set_src_pe_index(src_pe_index);
                }
            }

#ifdef DEBUG
            std::cout << "[MappingTxtParser] Parsed node: " << node_id
                      << " (pe_idx: " << pe_idx << ", op: " << op << ")" << std::endl;
#endif
        }

        // Third pass: resolve output relationships based on input references
        for (const auto& [node_id, node] : dfg.get_nodes()) {
            for (const auto& input : node.get_inputs()) {
                const std::string& src_id = input.get_id();
                if (src_id != "const" && dfg.has_node(src_id)) {
                    // Register this node as an output of the source node
                    auto& src_node = const_cast<Mapper_Node&>(dfg.get_node(src_id));
                    src_node.add_output(node_id);
                    // Set the destination PE index
                    auto& outputs = const_cast<std::vector<Output>&>(src_node.get_outputs());
                    outputs.back().set_dst_pe_index(node.get_pe_index());
                }
            }
        }

        std::cout << "[MappingTxtParser] Successfully parsed " << dfg.size()
                  << " nodes from " << filepath << std::endl;

        return dfg;
    }
};

} // namespace doda_mapping_parser
