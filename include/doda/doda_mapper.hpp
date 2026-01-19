#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <doda/dfg_parser.hpp>
#include <doda/doda_mapper_utils.hpp>

#define DEBUG

// Forward declaration
class Mapper_DFG;

// Input class - represents an input to a node
class Input {
private:
    std::string src_id;          // ID of the source node or "const" for constants
    std::string input_type;      // i1, i2, or pred
    int const_value;             // Constant value if str_id is "const"
    int src_pe_index;            // PE index of the source node

public:
    Input()
     :src_id(""), input_type(""), const_value(-1), src_pe_index(-1) {}
    
    Input(const std::string& type, const std::string& id)
        : src_id(id), input_type(type), const_value(-1), src_pe_index(-1) {}
    
    Input(const std::string& type, int const_val)
        : src_id("const"), input_type(type), const_value(const_val), src_pe_index(-1) {}

    // Getters
    const std::string& get_type() const { return input_type; }
    const std::string& get_id() const { return src_id; }
    int get_const_value() const { return const_value; }
    int get_src_pe_index() const { return src_pe_index; }

    // Setters
    void set_src_pe_index(int pe_index) { src_pe_index = pe_index; }

    friend std::ostream& operator<<(std::ostream& os, const Input& input);
};

class Output {
    private:
        std::string dst_id;      // ID of the destination node
        int dst_pe_index;        // PE index of the destination node

    public:
        Output(const std::string& id)
            : dst_id(id), dst_pe_index(-1) {}

        // Getters
        const std::string& get_id() const { return dst_id; }
        int get_dst_pe_index() const { return dst_pe_index; }

        // Setters
        void set_dst_pe_index(int pe_index) { dst_pe_index = pe_index; }
        friend std::ostream& operator<<(std::ostream& os, Output& output);
};

// Mapper_Node class - represents a single node in the dataflow graph
class Mapper_Node {
private:
    std::string id;
    Opcode op;
    std::vector<Input> inputs;
    std::vector<Output> outputs;   //  output IDs in strings

    bool initial_output_used = false;   // Flag to indicate if initial output is used
    int initial_output = 0;             // Initial output value, if used

    inline static int node_counter = 0; // Inline static counter for unique node IDs
    int pe_idx;

public:
    Mapper_Node() : id(""), op(Opcode::NIL), pe_idx(node_counter++) {}
    Mapper_Node(const std::string& node_id, Opcode operation, bool initial_output_used = false, int initial_output = -1) 
        : id(node_id), op(operation), initial_output_used(initial_output_used), initial_output(initial_output),
            pe_idx(node_counter++) {}

    // Input management
    void add_input(const std::string& type, const std::string& id) {    // source id and type (i1/i2/pred)
        inputs.emplace_back(type, id);
    }
    
    void add_input(const std::string& type, int const_val) {     // i1/i2/pred with constant value
        inputs.emplace_back(type, const_val);
    }

    void add_output(const std::string& output_id) {
        outputs.emplace_back(output_id);
    }

    // Getters
    const std::string& get_id() const { return id; }
    Opcode get_opcode() const { return op; }
    const std::vector<Input>& get_inputs() const { return inputs; }
    const std::vector<Output>& get_outputs() const { return outputs; }
    int get_pe_index() const { return pe_idx; }
    bool is_initial_output_used() const { return initial_output_used; }
    int get_initial_output() const { return initial_output; }

    static void reset_node_counter() { node_counter = 0; } // Reset counter before constructing a new DFG.

    // Setter for PE index (used by parsers that read pre-mapped DFGs)
    void set_pe_index(int idx) { pe_idx = idx; }

    friend std::ostream& operator<<(std::ostream& os, const Mapper_Node& node);
    friend class Mapper_DFG;  // Allow Mapper_DFG to access private members
};

// Mapper_DFG class - represents the entire dataflow graph
class Mapper_DFG {
private:
    std::map<std::string, Mapper_Node> m_nodes;

public:
    // Node management
    void add_node(const std::string& id, Opcode op, 
                  bool initial_output_used = false, int initial_output = -1) {
        if (m_nodes.find(id) != m_nodes.end()) {
            std::cerr << "[Mapper_DFG] Warning: Node with id " << id 
                      << " already exists. Overwriting." << std::endl;
        }
        //m_nodes[id] = Mapper_Node(id, op, initial_output_used, initial_output);
        m_nodes.emplace(id, Mapper_Node(id, op, initial_output_used, initial_output));
    }

    // Node access
    Mapper_Node& get_node(const std::string& id) {
        auto it = m_nodes.find(id);
        if (it == m_nodes.end()) {
            throw std::runtime_error("Node with id '" + id + "' not found");
        }
        return it->second;
    }

    const Mapper_Node& get_node(const std::string& id) const {
        auto it = m_nodes.find(id);
        if (it == m_nodes.end()) {
            throw std::runtime_error("Node with id '" + id + "' not found");
        }
        return it->second;
    }

    bool has_node(const std::string& id) const {
        return m_nodes.find(id) != m_nodes.end();
    }

    size_t size() const { return m_nodes.size(); }

    // Iterator access for external functions
    const std::map<std::string, Mapper_Node>& get_nodes() const { return m_nodes; }

    friend std::ostream& operator<<(std::ostream& os, const Mapper_DFG& dfg);
    friend class doda_mapper;  // Allow doda_mapper to access private members
};

// Main mapper class - orchestrates the mapping process
class doda_mapper {
private:
    // Configuration
    std::string input_dfg_path;
    int input_size_byte;
    int input_size_element;
    
    // Data structures
    nlohmann::json dfg_json;
    Mapper_DFG dfg;

    // Helper methods for graph construction
    void add_counter_node();
    void add_loop_condition_nodes();
    void add_load_node(const std::string& input_name);
    void add_store_node(const std::string& output_name);
    void add_terminal_node();
    
    // Initialization helpers
    void extract_vector_size();
    void construct_graph();
    void resolve_input_pe_indices();        // Trace input nodes and add their PE indices

public:
    explicit doda_mapper(const std::string& dfg_path);
    
    // Getters
    const Mapper_DFG& get_dfg() const { return dfg; }
    int get_input_size_bytes() const { return input_size_byte; }
    int get_input_size_elements() const { return input_size_element; }
    
    // Utility
    void print_debug_info() const;

    // Convert JSON nodes to DFG nodes
    static void convert_json_to_dfg(const nlohmann::json& json, Mapper_DFG& target_dfg);

    // Generate bitstream for DODA
    static std::string node_to_bitstream(const Mapper_Node& node);
    static std::vector<std::vector<std::string>> generate_bitstream(const Mapper_DFG& target_dfg);
};

// Implementation of stream operators
inline std::ostream& operator<<(std::ostream& os, const Input& input) {
    os << "\t\ttype: " << input.input_type
        << ", src_id: " << input.src_id 
        << " (pe_index: " << input.src_pe_index
        << "), const_value: " << input.const_value;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Mapper_Node& node) {
    os << "\tMapper_Node(id: " << node.id << " (pe_idx: " << node.pe_idx
                                << "), op: " << node.op 
                                << ", initial_output_used: " << node.initial_output_used
                                << ", initial_output: " << node.initial_output
                                << ", inputs: [\n";
    for (size_t i = 0; i < node.inputs.size(); ++i) {
        os << "\t" << node.inputs[i] << "\n";
    }
    os << "\t])";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Mapper_DFG& dfg) {
    os << "Mapper_DFG with " << dfg.m_nodes.size() << " nodes:\n";

    // Collect nodes and sort by pe_idx in ascending order
    std::vector<const Mapper_Node*> sorted_nodes;
    sorted_nodes.reserve(dfg.m_nodes.size());
    for (const auto& [id, node] : dfg.m_nodes) {
        sorted_nodes.push_back(&node);
    }
    std::sort(sorted_nodes.begin(), sorted_nodes.end(),
              [](const Mapper_Node* a, const Mapper_Node* b) {
                  return a->get_pe_index() < b->get_pe_index();
              });

    for (const Mapper_Node* node : sorted_nodes) {
        os << *node << "\n";
    }
    return os;
}



// Constructor
doda_mapper::doda_mapper(const std::string& dfg_path)
    : input_dfg_path(dfg_path), input_size_byte(0), input_size_element(0) {

    Mapper_Node::reset_node_counter(); // Reset node counter for a fresh start
    dfg_json = doda_mapper_utils::load_and_parse_json(input_dfg_path);

    extract_vector_size();
    construct_graph();
    resolve_input_pe_indices();

#ifdef DEBUG
    print_debug_info();
#endif
}

void doda_mapper::extract_vector_size() {
    if (dfg_json.contains("runtime_metadata") && 
        dfg_json["runtime_metadata"].contains("input_size_in_bytes")) {
        input_size_byte = dfg_json["runtime_metadata"]["input_size_in_bytes"];
        input_size_element = input_size_byte / sizeof(uint32_t);
        
#ifdef DEBUG
        std::cout << "[doda_mapper] Vector size: " << input_size_element 
                  << " (" << input_size_byte << " bytes)" << std::endl;
#endif
    } else {
        std::cerr << "[doda_mapper] Warning: No runtime metadata found. "
                  << "Vector size will need to be set later." << std::endl;
    }
}

void doda_mapper::construct_graph() {
    // Add basic infrastructure nodes
    add_counter_node();
    add_loop_condition_nodes();
    // Add output node
    if (dfg_json.contains("output") && dfg_json["output"].contains("id")) {
        std::string output_name = dfg_json["output"]["id"].get<std::string>();
        add_store_node(output_name);
    } else {
        throw std::runtime_error("Missing or invalid output specification in JSON");
    }

    // Add termination node
    add_terminal_node();

    // Add the input node
    if (dfg_json.contains("inputs") && dfg_json["inputs"].is_array()) {
        auto inputs_array = dfg_json["inputs"];
        if (inputs_array.size() == 1) {
            std::string input_name = inputs_array[0].get<std::string>();
            add_load_node(input_name);
        } else {
            std::cerr << "[doda_mapper] Error: Input array size is not 1." << std::endl;
            throw std::runtime_error("Invalid input array size");
        }
    } else {
        std::cerr << "[doda_mapper] Error: No valid 'inputs' array found in JSON." << std::endl;
        throw std::runtime_error("Missing or invalid 'inputs' array in JSON");
    }

    // Convert JSON nodes to DFG nodes and update the DFG
    convert_json_to_dfg(dfg_json, dfg);

    // Register output relationship: output_node -> store_output
    // (must be done after convert_json_to_dfg creates the output node)
    if (dfg_json.contains("output") && dfg_json["output"].contains("id")) {
        std::string output_name = dfg_json["output"]["id"].get<std::string>();
        auto& output_node = dfg.get_node(output_name);
        output_node.add_output("store_output");
    }
}

void doda_mapper::convert_json_to_dfg(const nlohmann::json& json, Mapper_DFG& target_dfg) {
    if (json.contains("nodes") && json["nodes"].is_array()) {
        std::cout << "[convert_json_to_dfg] Adding nodes from JSON..." << std::endl;
        for (const auto& json_node : json["nodes"]) {
            if (json_node.contains("id") && json_node.contains("op")) {
                std::string node_id = json_node["id"].get<std::string>();
                std::string opcode_str = json_node["op"].get<std::string>();

                // Convert string opcode to enum
                Opcode op = toOpcode(opcode_str);

                // Add the node to the DFG
                target_dfg.add_node(node_id, op);
                auto& node = target_dfg.get_node(node_id);

                std::cout << "[convert_json_to_dfg] Adding node: " << node_id
                   << " with opcode: " << opcode_str << std::endl;

                // Add inputs if they exist
                if (json_node.contains("inputs") && json_node["inputs"].is_array()) {
                    for (const auto& input : json_node["inputs"]) {
                        std::string input_type = input["type"].get<std::string>();
                        if (input.contains("type") && input.contains("id")) {       // input from another node
                            std::string input_id = input["id"].get<std::string>();
                            node.add_input(input_type, input_id);
                            auto& input_node = target_dfg.get_node(input_id);       // Register this node as an output of the input node
                            input_node.add_output(node_id);                   
                        } else if (input.contains("value")) {                       // constant input
                            node.add_input(input_type, input["value"].get<int>());
                        } else {
                            std::cerr << "[convert_json_to_dfg] Warning: Invalid input format for node "
                                      << node_id << std::endl;
                        }
                    }
                }

#ifdef DEBUG
                std::cout << "[convert_json_to_dfg] Added node from JSON: " << node_id
                          << " (opcode: " << opcode_str << ")" << std::endl;
#endif
            }
        }
    } else {
        std::cerr << "[convert_json_to_dfg] Warning: No 'nodes' array found in JSON." << std::endl;
    }
}

void doda_mapper::add_counter_node() {
    dfg.add_node("counter", Opcode::ADD, true, 0);     // Initial output is 0
    auto& counter_node = dfg.get_node("counter");

    counter_node.add_input("i1", "counter");           // Self-reference from the initial output 0
    counter_node.add_output("counter");                // Self-reference output
    counter_node.add_input("i2", 1);                   // Constant increment of 1
}

void doda_mapper::add_loop_condition_nodes() {
    auto& counter_node = dfg.get_node("counter");

    // Continue condition: counter < input_size_element
    dfg.add_node("continue_condition", Opcode::CLT);
    auto& continue_node = dfg.get_node("continue_condition");
    continue_node.add_input("i1", "counter");
    counter_node.add_output("continue_condition");
    continue_node.add_input("i2", input_size_element);

    // Terminal condition: counter >= input_size_element
    dfg.add_node("terminal_condition", Opcode::CGTE);
    auto& terminal_cond_node = dfg.get_node("terminal_condition");
    terminal_cond_node.add_input("i1", "counter");
    counter_node.add_output("terminal_condition");
    terminal_cond_node.add_input("i2", input_size_element);
}

void doda_mapper::add_load_node(const std::string& input_name) {
    dfg.add_node(input_name, Opcode::LOAD);
    auto& load_node = dfg.get_node(input_name);
    auto& counter_node = dfg.get_node("counter");
    auto& continue_cond = dfg.get_node("continue_condition");

    load_node.add_input("i1", "counter");              // Index from counter
    counter_node.add_output(input_name);
    load_node.add_input("pred", "continue_condition"); // Predicated on continue condition
    continue_cond.add_output(input_name);
}

void doda_mapper::add_store_node(const std::string& output_name) {
    dfg.add_node("store_output", Opcode::STORE);
    auto& store_node = dfg.get_node("store_output");
    auto& counter_node = dfg.get_node("counter");
    auto& continue_cond = dfg.get_node("continue_condition");

    store_node.add_input("i1", "counter");              // Index from counter
    counter_node.add_output("store_output");
    store_node.add_input("i2", output_name);            // Data to store (output registered in convert_json_to_dfg)
    store_node.add_input("pred", "continue_condition"); // Predicated on continue condition
    continue_cond.add_output("store_output");
}

void doda_mapper::add_terminal_node() {
    dfg.add_node("terminal", Opcode::JUMP);
    auto& terminal_node = dfg.get_node("terminal");
    auto& store_node = dfg.get_node("store_output");
    auto& terminal_cond = dfg.get_node("terminal_condition");

    terminal_node.add_input("i1", 100);                    // Jump target (artificial)
    terminal_node.add_input("i2", "store_output");         // Dependency on store completion
    store_node.add_output("terminal");
    terminal_node.add_input("pred", "terminal_condition"); // Predicated on terminal condition
    terminal_cond.add_output("terminal");
}

void doda_mapper::resolve_input_pe_indices() {
    // Step 1: Build a mapping from node ID to PE index
    std::map<std::string, int> node_id_to_pe_index;
    
    // map all node IDs to their PE indices
    for (const auto& [node_id, node] : dfg.m_nodes) {
        node_id_to_pe_index[node_id] = node.get_pe_index();
    }
    
    // Step 2: Go through all nodes and resolve their input dependencies
    for (auto& [node_id, node] : dfg.m_nodes) {
        auto& inputs = const_cast<std::vector<Input>&>(node.get_inputs());
        
        for (auto& input : inputs) {
            const std::string& input_id = input.get_id();
            
            // Skip constants (they don't need PE index resolution)
            if (input_id == "const") {
                continue;
            }
            
            // Look up the PE index for this input ID
            auto it = node_id_to_pe_index.find(input_id);
            if (it != node_id_to_pe_index.end()) {
                // Found the dependency - update the input with PE index
                input.set_src_pe_index(it->second);
            } else {
                // This might be a function argument or external input
                std::cerr << "[resolve_input_pe_indices] Warning: Could not resolve input '" 
                          << input_id << "' for node '" << node_id << "'" << std::endl;
            }
        }

        // Also resolve output PE indices, to help generate output OH-keys
        for (auto& output : const_cast<std::vector<Output>&>(node.get_outputs())) {
            const std::string& output_id = output.get_id();
            auto it = node_id_to_pe_index.find(output_id);
            if (it != node_id_to_pe_index.end()) {
                output.set_dst_pe_index(it->second);
            } else {
                std::cerr << "[resolve_input_pe_indices] Warning: Could not resolve output '"
                          << output_id << "' for node '" << node_id << "'" << std::endl;
            }
        }
    }
    
#ifdef DEBUG
    std::cout << "[resolve_input_pe_indices] PE index resolution complete" << std::endl;
#endif
}

std::string doda_mapper::node_to_bitstream(const Mapper_Node& node) {
    using namespace doda_mapper_utils;
    
    // PE index (0 - NUM_CLUSTER*PES_PER_CLUSTER-1)
    int pe_idx = node.get_pe_index();
    std::string bin_pe_idx = to_binary_string(pe_idx, BitstreamConstants::SRC_PE_IDX_WIDTH + log2_ceil(BitstreamConstants::NUM_CLUSTER)); // 5 bits for PE index + 2 bits for cluster

    // Inputs
    bool i1_used                    = false;
    bool i1_const_used              = false;
    int  i1_src_or_const            = 0; // Source PE index or constant value for i1

    bool i2_used                    = false;
    bool i2_const_used              = false;
    int  i2_src_or_const            = 0; // Source PE index or constant value for i1

    bool pred_used                  = false;
    int  pred_src                   = 0;   // Source PE index for predicate

    for (const auto& input : node.get_inputs()) {
        if (input.get_type() == "i1") {
            i1_used = true;
            if (input.get_id() == "const") {
                i1_const_used = true;
                i1_src_or_const = input.get_const_value();
            } else {
                i1_src_or_const = input.get_src_pe_index();
            }
        } else if (input.get_type() == "i2") {
            i2_used = true;
            if (input.get_id() == "const") {
                i2_const_used = true;
                i2_src_or_const = input.get_const_value();
            } else {
                i2_src_or_const = input.get_src_pe_index();
            }
        } else if (input.get_type() == "pred") {
            pred_used = true;
            pred_src = input.get_src_pe_index();
        }
    }

    std::string bin_i1_used         = i1_used ? "1" : "0"; // 0 or 1
    std::string bin_i1_const_used   = i1_const_used ? "1" : "0"; // 0 or 1
    std::string bin_i1_src_or_const = to_binary_string(i1_src_or_const, BitstreamConstants::DATA_WIDTH); // Binary representation of i1 source or constant
    std::string bin_i2_used         = i2_used ? "1" : "0"; // 0 or 1
    std::string bin_i2_const_used   = i2_const_used ? "1" : "0"; // 0 or 1
    std::string bin_i2_src_or_const = to_binary_string(i2_src_or_const, BitstreamConstants::DATA_WIDTH); // Binary representation of i1 source or constant
    std::string bin_pred_used       = pred_used ? "1" : "0"; // 0 or 1
    std::string bin_pred_src        = to_binary_string(pred_src, BitstreamConstants::SRC_IDX_WIDTH); // Binary representation of predicate source

    // Initial output
    std::string bin_initial_output_used = node.is_initial_output_used() ? "1" : "0"; // 0 or 1
    std::string bin_initial_output      = to_binary_string(node.get_initial_output(), BitstreamConstants::DATA_WIDTH); // Binary representation of initial outputk

    // Opcode
    Opcode op                       = node.get_opcode();
    std::string bin_opcode          = to_binary_string(static_cast<int>(op), BitstreamConstants::OPCODE_WIDTH);

    // DST PE Cluster OH-key: compute which clusters consume this node's output
    int dst_oh = 0;
    for (const auto& output : node.get_outputs()) {
        int dst_pe_idx = output.get_dst_pe_index();
        int dst_cluster = dst_pe_idx / BitstreamConstants::PES_PER_CLUSTER;
        int this_cluster = pe_idx / BitstreamConstants::PES_PER_CLUSTER;
        if (dst_cluster == this_cluster) {
            continue; // Skip same cluster
        } else {
            dst_oh |= (1 << dst_cluster);  // Set the bit for this cluster
        }
    }
    std::string bin_dst_oh = to_binary_string(dst_oh, BitstreamConstants::NUM_CLUSTER);


    int used_bit_size = BitstreamConstants::SRC_PE_IDX_WIDTH + 2 +
                        BitstreamConstants::SRC_IDX_WIDTH +  // node's PE index, pred src
                        BitstreamConstants::DATA_WIDTH * 3 + // i1, i2, init output
                        BitstreamConstants::OPCODE_WIDTH + 
                        BitstreamConstants::NUM_CLUSTER +    // cluster OH-key 
                        6; // 6 bits for i1_used, i1_const_used, i2_used, i2_const_used, pred_used, initial_output_used

    if (used_bit_size > BitstreamConstants::PROG_MEM_WIDTH) {
        throw std::runtime_error("Bitstream size exceeds program memory width");
    }

    std::string margin = std::string(BitstreamConstants::PROG_MEM_WIDTH - used_bit_size, '0'); // Padding to fill the program memory width

    return margin + bin_dst_oh + bin_opcode + 
            bin_initial_output + bin_initial_output_used +
            bin_pred_src + bin_pred_used +
            bin_i2_src_or_const + bin_i2_const_used + bin_i2_used +
            bin_i1_src_or_const + bin_i1_const_used + bin_i1_used +
            bin_pe_idx;
}

std::vector<std::vector<std::string>> doda_mapper::generate_bitstream(const Mapper_DFG& target_dfg) {
    // DODA simulator expects: [cluster][instruction_index] -> binary_string
    const int num_clusters = doda_mapper_utils::BitstreamConstants::NUM_CLUSTER;
    const int num_pe_per_cluster = doda_mapper_utils::BitstreamConstants::PES_PER_CLUSTER;
    const int prog_mem_width = doda_mapper_utils::BitstreamConstants::PROG_MEM_WIDTH;
    const int src_idx_width = doda_mapper_utils::BitstreamConstants::SRC_IDX_WIDTH;

    std::vector<std::vector<std::string>> bitstream(num_clusters, std::vector<std::string>());
    // Initialize with index only bitstream
    for (int cluster = 0; cluster < num_clusters; cluster++) {
        for (int pe = 0; pe < num_pe_per_cluster; pe++) {
            // Each PE in a cluster has a bitstream of width PROG_MEM_WIDTH
            int idx = cluster * num_pe_per_cluster + pe;
            std::string str_pe_idx = doda_mapper_utils::to_binary_string(idx, src_idx_width);
            std::string initial_bitstream = std::string(prog_mem_width - src_idx_width, '0') + str_pe_idx;
            bitstream[cluster].push_back(initial_bitstream);
        }
    }

    // Collect and sort nodes by pe_idx
    std::vector<const Mapper_Node*> sorted_nodes;
    sorted_nodes.reserve(target_dfg.get_nodes().size());
    for (const auto& [id, node] : target_dfg.get_nodes()) {
        sorted_nodes.push_back(&node);
    }
    std::sort(sorted_nodes.begin(), sorted_nodes.end(),
              [](const Mapper_Node* a, const Mapper_Node* b) {
                  return a->get_pe_index() < b->get_pe_index();
              });

    // Generate bitstream for each node
    for (const Mapper_Node* node_ptr : sorted_nodes) {
        const Mapper_Node& node = *node_ptr;
        int node_idx = node.get_pe_index();
        int cluster_idx = node_idx / num_pe_per_cluster;
        int pe_idx = node_idx % num_pe_per_cluster;

        if (cluster_idx < num_clusters && pe_idx < num_pe_per_cluster) {
            std::string bin_instruction = node_to_bitstream(node);
            bitstream[cluster_idx][pe_idx] = bin_instruction;

#ifdef DEBUG
            std::cout << "[generate_bitstream] " << node <<
                      " -> binary: " << bin_instruction << std::endl;
#endif
        } else {
            std::cerr << "[generate_bitstream] Error: Node " << node.get_id()
                      << " has invalid cluster or PE index ("
                      << cluster_idx << ", " << pe_idx << ")" << std::endl;
            throw std::runtime_error("Invalid cluster or PE index for node");
        }
    }
    return bitstream;
}

void doda_mapper::print_debug_info() const {
    std::cout << "[doda_mapper] Final DFG structure:" << std::endl;
    std::cout << dfg << std::endl;
}