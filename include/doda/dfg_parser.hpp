#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <nlohmann/json.hpp>

//#define DEBUG

enum class Opcode {
    NIL,  // For unused processing elements
    ADD, SUB, MUL, LS, RS,
    AND, OR, XOR, SELECT,
    CMP, CNE, CLT, CLTE, CGT, CGTE,
    LOAD,
    STORE, JUMP,
    UNSUPPORTED  // For error handling
};
    
inline std::ostream& operator<<(std::ostream& os, const Opcode& opcode) {
    switch (opcode) {
        case Opcode::NIL: return os << "NIL";
        case Opcode::ADD: return os << "ADD";
        case Opcode::SUB: return os << "SUB";
        case Opcode::MUL: return os << "MUL";
        case Opcode::LS: return os << "LS";
        case Opcode::RS: return os << "RS";
        case Opcode::AND: return os << "AND";
        case Opcode::OR: return os << "OR";
        case Opcode::XOR: return os << "XOR";
        case Opcode::SELECT: return os << "SELECT";
        case Opcode::CMP: return os << "CMP";
        case Opcode::CNE: return os << "CNE";
        case Opcode::CLT: return os << "CLT";
        case Opcode::CLTE: return os << "CLTE";
        case Opcode::CGT: return os << "CGT";
        case Opcode::CGTE: return os << "CGTE";
        case Opcode::LOAD: return os << "LOAD";
        case Opcode::STORE: return os << "STORE";
        case Opcode::JUMP: return os << "JUMP";
        case Opcode::UNSUPPORTED: return os << "UNSUPPORTED";
        default: return os << "UNSUPPORTED";
    }
}

Opcode toOpcode(const std::string& op) {
    static const std::unordered_map<std::string, Opcode> opmap = {
        {"nil", Opcode::NIL},
        {"add", Opcode::ADD},
        {"sub", Opcode::SUB},
        {"mul", Opcode::MUL},
        {"shl", Opcode::LS},
        {"lshr", Opcode::RS},
        {"ashr", Opcode::RS},
        {"and", Opcode::AND},
        {"or", Opcode::OR},
        {"xor", Opcode::XOR},
        {"icmp_eq", Opcode::CMP},
        {"icmp_ne", Opcode::CNE},
        {"icmp_slt", Opcode::CLT},      // All comparisons are considered unsigned
        {"icmp_sle", Opcode::CLTE},
        {"icmp_sgt", Opcode::CGT},
        {"icmp_sge", Opcode::CGTE},
        {"icmp_ult", Opcode::CLT},
        {"icmp_ule", Opcode::CLTE},
        {"icmp_ugt", Opcode::CGT},
        {"icmp_uge", Opcode::CGTE},
        {"cmp", Opcode::CMP},
        {"cne", Opcode::CNE},
        {"clt", Opcode::CLT},
        {"clte", Opcode::CLTE},
        {"cgt", Opcode::CGT},
        {"cgte", Opcode::CGTE},
        {"select", Opcode::SELECT},
        {"load", Opcode::LOAD},
        {"store", Opcode::STORE},
        {"jump", Opcode::JUMP}
    };
    
    auto it = opmap.find(op);
    return (it != opmap.end()) ? it->second : Opcode::UNSUPPORTED;
}

struct InputSpec {
    std::string type;  // "i1", "i2", "pred"
    std::string id;    // node id or "const"
    int value;         // for constants
    bool is_constant;
    
    InputSpec() : value(-1), is_constant(false) {}
};

inline std::ostream& operator<<(std::ostream& os, const InputSpec& input) {
    if (input.is_constant) {
        os << input.type << ":" << input.value;
    } else {
        os << input.type << ":" << input.id;
    }
    return os;
}

struct Node {
    std::string id;
    std::string op;
    std::vector<InputSpec> inputs;  // Changed from vector<string>
};

struct DFG {
    std::vector<Node> nodes;
    std::vector<std::string> inputs;
    std::string output;
};

inline DFG parseDFG(const std::string& filename) {
    DFG result_dfg;
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "[Error] Could not open " << filename << std::endl;
        return result_dfg;
    }

    nlohmann::json dfg;
    in >> dfg;

    for (const auto& item : dfg["nodes"]) {
        Node n;
        n.id = item["id"];
        n.op = item["op"];
        for (const auto& input : item["inputs"]) {
            InputSpec spec;
            spec.type = input["type"];
            
            if (input.contains("id")) {
                spec.id = input["id"];
                spec.is_constant = false;
            } else if (input.contains("value")) {
                spec.id = "const";
                spec.value = input["value"];
                spec.is_constant = true;
            }
            
            n.inputs.push_back(spec);
        }
        
        Opcode opcode = toOpcode(n.op);
        if (opcode == Opcode::UNSUPPORTED) {
            std::cerr << "[DFG Error] Unsupported operation: " << n.op << " in node " << n.id << std::endl;
            exit(1);
        }
        result_dfg.nodes.push_back(n);
    }

    for (const auto& i : dfg["inputs"])
        result_dfg.inputs.push_back(i);

    if (dfg.contains("output")) {
        if (dfg["output"].is_string()) {
            result_dfg.output = dfg["output"];
        } else if (dfg["output"].is_object() && dfg["output"].contains("id")) {
            result_dfg.output = dfg["output"]["id"];
        } else {
            std::cerr << "[DFG Error] Invalid format for output field" << std::endl;
            exit(1);
        }
    }

    // Debugging output of the runtime dfg parser.
    #ifdef DEBUG
    std::cout << "[DFG] Inputs: ";
    for (const auto& input : result_dfg.inputs)
        std::cout << input << " ";
    std::cout << std::endl; 
    std::cout << "[DFG] Output: " << result_dfg.output << std::endl;
    std::cout << "[DFG] Parsed " << result_dfg.nodes.size() << " nodes: " << std::endl;
    for (const auto& node: result_dfg.nodes) {
        std::cout << "\t(" << node.id << " " << node.op << " [";
        for (const auto& input : node.inputs)
            std::cout << input << " ";
        std::cout << "])" << std::endl;
    }
    std::cout << std::endl;
    #endif
    return result_dfg;
}