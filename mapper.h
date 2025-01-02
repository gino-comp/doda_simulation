#ifndef MAPPER_H
#define MAPPER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <cassert>

enum class Opcode {
    NIL,
    ADD, SUB, MUL, LS, RS,
    AND, OR, XOR, SELECT, CMP, CLT, CLTE,
    LOAD,
    STORE, JUMP
};

class General_Params {
public:
    int prog_mem_width;
    int data_width;
    int num_pe_per_cluster;
    int opcode_width;
    int src_idx_width;
    int inst_tab_size;
    int data_mem_size_byte;
    int num_data_mem_entries;

    // DON'T CHANGE THESE DEFAULT VALUES. THEY ARE MATCHED WITH THE RTL DESIGN.
    General_Params()
        : prog_mem_width(128), data_width(32), num_pe_per_cluster(16),
            opcode_width(4), src_idx_width(8), inst_tab_size(32),
            data_mem_size_byte(512) {
                num_data_mem_entries = data_mem_size_byte * 8 / data_width;
        }
    // DON'T CHANGE THESE DEFAULT VALUES. THEY ARE MATCHED WITH THE RTL DESIGN.
};

struct PE_OP {
    int idx;
    bool i1_used;
    bool i1_const_used;
    long i1_src_or_const;
    bool i2_used;
    bool i2_const_used;
    long i2_src_or_const;
    bool p_used;
    long p_src;
    bool init_out_used;
    long init_out;
    Opcode opcode;
    General_Params g;

    // Default constructor
    PE_OP() : idx(0), i1_used(false), i1_const_used(false), i1_src_or_const(0),
              i2_used(false), i2_const_used(false), i2_src_or_const(0), p_used(false),
              p_src(0), init_out_used(false), init_out(0), opcode(Opcode::NIL), g() {}

    // Parameterized constructor
    PE_OP(int idx, General_Params g)
        : idx(idx), i1_used(false), i1_const_used(false), i1_src_or_const(0),
          i2_used(false), i2_const_used(false), i2_src_or_const(0), p_used(false),
          p_src(0), init_out_used(false), init_out(0), opcode(Opcode::NIL), g(g) {}

    std::string toString() const {
        std::ostringstream oss;
        oss << "(idx: " << idx
            << ", opcode: " << static_cast<int>(opcode)
            << ", i1_used: " << i1_used
            << ", i1_const_used: " << i1_const_used
            << ", i1_src_or_const: " << i1_src_or_const
            << ", i2_used: " << i2_used
            << ", i2_const_used: " << i2_const_used
            << ", i2_src_or_const: " << i2_src_or_const
            << ", p_used: " << p_used
            << ", p_src: " << p_src
            << ", init_out_used: " << init_out_used
            << ", init_out: " << init_out << ")";
        return oss.str();
    }
};

class Mapper {
public:
    Mapper(const std::string& code_filename, General_Params g);

    std::vector<PE_OP> parsed();
    std::string binaryFormat(const PE_OP& pe_op);
    std::string hexFormat(const PE_OP& pe_op);

private:
    std::vector<std::string> code_lines;
    std::map<std::string, Opcode> map_opcode;
    std::vector<PE_OP> init_ret_vec;
    General_Params g;

    PE_OP parse_op(const std::vector<std::string>& args, PE_OP op);

    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string toBinStr(int len, long dec);
};

#endif // MAPPER_H