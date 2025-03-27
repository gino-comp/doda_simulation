#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <bitset>
#include <cassert>
#include <map>
#include <algorithm>
#include "mapper.h"

#include "mapper.h"
//#define DEBUG

Mapper::Mapper(const std::string& code_filename, General_Params g)
    : code_filename(code_filename), g(g) {
    std::ifstream file(code_filename);
    std::string line;
    while (std::getline(file, line)) {
        //#ifdef DEBUG
        //std::cout << "Line: " << line << std::endl;
        //#endif
        code_lines.push_back(line);
    }

    std::vector<std::string> opcodes = { "NIL", "ADD", "SUB", "MUL", "LS", "RS", "AND", "OR", "XOR", "SELECT", "CMP", "CLT", "CLTE", "LOAD", "STORE", "JUMP" };
    for (size_t i = 0; i < opcodes.size(); ++i) {
        map_opcode[opcodes[i]] = static_cast<Opcode>(static_cast<int>(Opcode::NIL) + i);
    }

    init_ret_vec.resize(g.num_cluster * g.inst_tab_size);
    for (int i = 0; i < g.num_cluster * g.inst_tab_size; ++i) {
        init_ret_vec[i] = PE_OP(i % (g.num_cluster * g.num_pe_per_cluster), g);
    }
}

std::vector<PE_OP> Mapper::parsed() {
    std::vector<PE_OP> ops = init_ret_vec;
    int vec_idx = 0;

    for (const auto& line : code_lines) {
        std::string head_wo_comments = line.substr(0, line.find("//"));
        if (head_wo_comments.empty()) continue;

        std::vector<std::string> tokens = split(head_wo_comments, ',');
        PE_OP op = parse_op(tokens, PE_OP(-1, g));
        #ifdef DEBUG
        std::cout << "DEBUG) Parsed: " << op.toString() << std::endl;
        #endif


        //assert(op.idx >= 0 && op.idx < g.inst_tab_size);
        assert(op.idx >= 0 && op.idx < g.num_cluster * g.num_pe_per_cluster);
        ops[vec_idx++] = op;
    }

    return ops;
}

PE_OP Mapper::parse_op(const std::vector<std::string>& args, PE_OP op) {
    for (const auto& arg : args) {
        auto key_val = split(arg, ':');

        // Trimming whitespaces
        std::string keyword = key_val[0];
        keyword.erase(std::remove_if(keyword.begin(), keyword.end(), ::isspace), keyword.end());
        std::string value = key_val[1];
        value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());

        if (keyword == "IDX") op.idx = std::stoi(value);
        else if (keyword == "OPCODE") op.opcode = map_opcode[value];
        else if (keyword == "I1_USED") op.i1_used = std::stoi(value) == 1;
        else if (keyword == "I1_CONST_USED") op.i1_const_used = std::stoi(value) == 1;
        else if (keyword == "I1_SRC_OR_CONST") op.i1_src_or_const = std::stol(value);
        else if (keyword == "I2_USED") op.i2_used = std::stoi(value) == 1;
        else if (keyword == "I2_CONST_USED") op.i2_const_used = std::stoi(value) == 1;
        else if (keyword == "I2_SRC_OR_CONST") op.i2_src_or_const = std::stol(value);
        else if (keyword == "P_USED") op.p_used = std::stoi(value) == 1;
        else if (keyword == "P_SRC") op.p_src = std::stol(value);
        else if (keyword == "INIT_OUT_USED") op.init_out_used = std::stoi(value) == 1;
        else if (keyword == "INIT_OUT") op.init_out = std::stol(value);

        #ifdef DEBUG
        #endif
    }
    #ifdef DEBUG
    std::cout << "DEBUG) op: " << op.toString() << std::endl; 
    #endif
    return op;
}

std::vector<std::string> Mapper::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string Mapper::toBinStr(int len, long dec) {
    std::string bin = "";
    for (int i = 0; i < len; i++) {
        bin = ((dec & 1) ? '1' : '0') + bin;
        dec >>= 1;
    }
    return bin.substr(bin.size() - len);
}

std::vector<std::vector<std::vector<PE_OP>>> Mapper::split_to_clusters(const std::vector<PE_OP>& ops) {
    std::map<int, std::vector<PE_OP>> m_grouped;
    for (const auto& op : ops) {
        int cluster_idx = op.idx / g.num_pe_per_cluster;
        m_grouped[cluster_idx].push_back(op);
    }

    std::vector<std::vector<PE_OP>> v_grouped(g.num_cluster);
    for (int i = 0; i < g.num_cluster; ++i) {
        v_grouped[i] = m_grouped[i];
    }

    std::vector<std::vector<std::vector<PE_OP>>> result(g.num_cluster);
    for (int i = 0; i < g.num_cluster; ++i) {
        result[i] = split_to_phases(v_grouped[i]);
    }

    return result;
}

std::vector<std::vector<PE_OP>> Mapper::split_to_phases(const std::vector<PE_OP>& v_ops) {
    std::vector<std::vector<PE_OP>> phases;
    auto it = v_ops.begin();
    while (it != v_ops.end()) {
        auto next_it = std::adjacent_find(it, v_ops.end(), [](const PE_OP& a, const PE_OP& b) {
            return a.idx > b.idx;
        });
        if (next_it == v_ops.end()) {
            phases.push_back(std::vector<PE_OP>(it, v_ops.end()));
            break;
        } else {
            phases.push_back(std::vector<PE_OP>(it, next_it + 1));
            it = next_it + 1;
        }
    }
    return phases;
}

std::vector<std::vector<std::vector<PE_OP>>> Mapper::updated_with_dst_oh(const std::vector<std::vector<std::vector<PE_OP>>>& v_split_ops) {
    std::vector<std::vector<std::vector<PE_OP>>> v_ops_transposed(v_split_ops[0].size(), std::vector<std::vector<PE_OP>>(v_split_ops.size()));
    // Transposing from
    // cluster -> phase -> op
    // to
    // phase -> cluster -> op
    for (size_t i = 0; i < v_split_ops.size(); ++i) {
        for (size_t j = 0; j < v_split_ops[i].size(); ++j) {
            v_ops_transposed[j][i] = v_split_ops[i][j];
        }
    }

    for (size_t phase_idx = 0; phase_idx < v_ops_transposed.size(); ++phase_idx) {
        auto& v_cluster_ops = v_ops_transposed[phase_idx];
        auto s_deps = find_dependencies(v_cluster_ops);

        for (size_t cluster_idx = 0; cluster_idx < v_cluster_ops.size(); ++cluster_idx) {
            auto& v_ops = v_cluster_ops[cluster_idx];
            for (auto& op : v_ops) {
                auto deps = filter_deps(s_deps, op.idx, cluster_idx);
                //#ifdef DEBUG
                //std::cout << "DEBUG) op idx: " << op.idx << ", deps.size: " << deps.size() << std::endl;
                //#endif
                std::string dst_oh_rev(g.num_cluster, '0');
                for (const auto& dep : deps) {
                    dst_oh_rev[dep.second.first] = '1';
                }
                op.dst_oh = std::string(dst_oh_rev.rbegin(), dst_oh_rev.rend());
            }
        }
    }

    std::vector<std::vector<std::vector<PE_OP>>> ret_v_ops(v_ops_transposed[0].size(), std::vector<std::vector<PE_OP>>(v_ops_transposed.size()));
    for (size_t i = 0; i < v_ops_transposed.size(); ++i) {
        for (size_t j = 0; j < v_ops_transposed[i].size(); ++j) {
            ret_v_ops[j][i] = v_ops_transposed[i][j];
        }
    }

    return ret_v_ops;
}

std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> Mapper::find_dependencies(const std::vector<std::vector<PE_OP>>& v_ops) {
    std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> deps;
    for (size_t cluster_idx = 0; cluster_idx < v_ops.size(); ++cluster_idx) {
        const auto& ops = v_ops[cluster_idx];
        for (size_t op_idx = 0; op_idx < ops.size(); ++op_idx) {
            const auto& op = ops[op_idx];
            if (op.i1_used) {
                int cluster_idx_i1 = op.i1_src_or_const / g.num_pe_per_cluster;
                int op_idx_i1 = op.i1_src_or_const;
                deps.insert({{cluster_idx_i1, op_idx_i1}, {cluster_idx, op_idx}});
            }
            if (op.i2_used) {
                int cluster_idx_i2 = op.i2_src_or_const / g.num_pe_per_cluster;
                int op_idx_i2 = op.i2_src_or_const;
                deps.insert({{cluster_idx_i2, op_idx_i2}, {cluster_idx, op_idx}});
            }
        }
    }
    return deps;
}

std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> Mapper::filter_deps(const std::set<std::pair<std::pair<int, int>, std::pair<int, int>>>& deps, int op_idx, int cluster_idx) {
    std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> filtered_deps;
    for (const auto& dep : deps) {
        int src_cluster_idx = dep.first.first;
        int src_op_idx = dep.first.second;
        int dst_cluster_idx = dep.second.first;
        int dst_op_idx = dep.second.second;

        if (src_op_idx == op_idx && src_cluster_idx != dst_cluster_idx) {
            filtered_deps.insert(dep);
        }
    }
    return filtered_deps;
}

std::string Mapper::binaryFormat(const PE_OP& pe_op) {
    int datawidth = g.data_width;
    std::string s_pe_idx = toBinStr(log2Ceil(g.num_cluster) + g.src_pe_idx_width, pe_op.idx);
    std::string s_i1_used = pe_op.i1_used ? "1" : "0";
    std::string s_i1_const_used = pe_op.i1_const_used ? "1" : "0";
    std::string s_i1_src_or_const = toBinStr(datawidth, pe_op.i1_src_or_const);
    std::string s_i2_used = pe_op.i2_used ? "1" : "0";
    std::string s_i2_const_used = pe_op.i2_const_used ? "1" : "0";
    std::string s_i2_src_or_const = toBinStr(datawidth, pe_op.i2_src_or_const);
    std::string s_p_used = pe_op.p_used ? "1" : "0";
    std::string s_p_src = toBinStr(g.src_idx_width, pe_op.p_src);
    std::string init_out_used = pe_op.init_out_used ? "1" : "0";
    std::string init_out = toBinStr(datawidth, pe_op.init_out);
    std::string s_opcode = toBinStr(g.opcode_width, static_cast<int>(pe_op.opcode));
    std::string s_dst_oh = pe_op.dst_oh;
    std::string s_margin = toBinStr(g.prog_mem_width - (log2Ceil(g.num_cluster) + g.src_pe_idx_width + 3 * g.data_width + g.src_idx_width + g.opcode_width + g.num_cluster + 6), 0);

    std::string ret = s_margin + s_dst_oh + s_opcode + init_out + init_out_used + s_p_src + s_p_used + s_i2_src_or_const + s_i2_const_used + s_i2_used + s_i1_src_or_const + s_i1_const_used + s_i1_used + s_pe_idx;

    assert(ret.size() == g.prog_mem_width);
    return ret;
}

std::string Mapper::hexFormat(const PE_OP& pe_op) {
    std::string bin_str = binaryFormat(pe_op);
    std::stringstream ss;
    for (size_t i = 0; i < bin_str.size(); i += 4) {
        std::string nibble = bin_str.substr(i, 4);
        ss << std::hex << std::stoi(nibble, nullptr, 2);
    }
    return ss.str();
}