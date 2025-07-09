#ifndef MAPPER_H
#define MAPPER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cassert>

static int log2Ceil(int x) {
    assert(x > 0);
    int result = 0;
    x--;
    while (x > 0) {
        x >>= 1;
        result++;
    }
    return result;
}

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
    int num_cluster;
    int opcode_width;
    int inst_tab_size;
    int data_mem_size_byte;
    int num_data_mem_entries;
    int src_pe_idx_width;
    int src_cluster_idx_width;
    int src_idx_width;

    // DON'T CHANGE THESE DEFAULT VALUES. THEY ARE MATCHED WITH THE RTL DESIGN.
    General_Params()
        : prog_mem_width(128), data_width(32), num_pe_per_cluster(32), num_cluster(4),
            opcode_width(4), inst_tab_size(256),
            data_mem_size_byte(1024)
    {
        num_data_mem_entries = data_mem_size_byte * 8 / data_width;
        src_pe_idx_width = log2Ceil(num_pe_per_cluster);
        src_cluster_idx_width = num_cluster; // As it is represented in OH format
        src_idx_width = src_pe_idx_width + src_cluster_idx_width;

        assert((num_cluster & (num_cluster - 1)) == 0 && "num_cluster must be a power of 2");
        assert((num_pe_per_cluster & (num_pe_per_cluster - 1)) == 0 && "num_pe_per_cluster must be a power of 2");
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
    std::string dst_oh;
    General_Params g;

    // Default constructor
    PE_OP() : idx(0), i1_used(false), i1_const_used(false), i1_src_or_const(0),
              i2_used(false), i2_const_used(false), i2_src_or_const(0), p_used(false),
              p_src(0), init_out_used(false), init_out(0), opcode(Opcode::NIL), dst_oh(""), g() {}

    // Parameterized constructor
    PE_OP(int idx, General_Params g)
        : idx(idx), i1_used(false), i1_const_used(false), i1_src_or_const(0),
          i2_used(false), i2_const_used(false), i2_src_or_const(0), p_used(false),
          p_src(0), init_out_used(false), init_out(0), opcode(Opcode::NIL), dst_oh(""), g(g) {}

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
            << ", init_out: " << init_out ;
            if (!dst_oh.empty()) {
                oss <<  ", dst_oh: " + dst_oh;
            }
            oss << ")";
        return oss.str();
    }
};

class Mapper {
public:
    Mapper(const std::string& code_filename, General_Params g);

    std::vector<PE_OP> parsed();
    std::string binaryFormat(const PE_OP& pe_op);
    std::string hexFormat(const PE_OP& pe_op);
    std::vector<std::vector<std::vector<PE_OP>>> split_to_clusters(const std::vector<PE_OP>& ops);
    std::vector<std::vector<std::vector<PE_OP>>> updated_with_dst_oh(const std::vector<std::vector<std::vector<PE_OP>>>& v_split_ops);

private:
    std::string code_filename;
    std::vector<std::string> code_lines;
    std::map<std::string, Opcode> map_opcode;
    std::vector<PE_OP> init_ret_vec;
    General_Params g;

    PE_OP parse_op(const std::vector<std::string>& args, PE_OP op);

    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string toBinStr(int len, long dec);
    std::vector<std::vector<PE_OP>> split_to_phases(const std::vector<PE_OP>& v_ops);
    std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> find_dependencies(const std::vector<std::vector<PE_OP>>& v_ops);
    std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> filter_deps(const std::set<std::pair<std::pair<int, int>, std::pair<int, int>>>& deps, int op_idx, int cluster_idx);
};

#endif // MAPPER_H