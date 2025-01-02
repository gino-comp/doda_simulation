#include "mapper.h"
#include <bitset>

Mapper::Mapper(const std::string& code_filename, General_Params g)
    : g(g) {
    std::ifstream file(code_filename);
    std::string line;
    while (std::getline(file, line)) {
        code_lines.push_back(line);
    }
    init_ret_vec.resize(g.num_pe_per_cluster);
    for (int i = 0; i < g.num_pe_per_cluster; ++i) {
        init_ret_vec[i] = PE_OP(i, g);
    }
    // Initialize map_opcode with Opcode values
    map_opcode["NIL"] = Opcode::NIL;
    map_opcode["ADD"] = Opcode::ADD;
    map_opcode["SUB"] = Opcode::SUB;
    map_opcode["MUL"] = Opcode::MUL;
    map_opcode["LS"] = Opcode::LS;
    map_opcode["RS"] = Opcode::RS;
    map_opcode["AND"] = Opcode::AND;
    map_opcode["OR"] = Opcode::OR;
    map_opcode["XOR"] = Opcode::XOR;
    map_opcode["SELECT"] = Opcode::SELECT;
    map_opcode["CMP"] = Opcode::CMP;
    map_opcode["CLT"] = Opcode::CLT;
    map_opcode["CLTE"] = Opcode::CLTE;
    map_opcode["LOAD"] = Opcode::LOAD;
    map_opcode["STORE"] = Opcode::STORE;
    map_opcode["JUMP"] = Opcode::JUMP;
}

std::vector<PE_OP> Mapper::parsed() {
    std::vector<PE_OP> result;
    for (auto line : code_lines) {
        auto comment_pos = line.find("//");
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        if (line.empty()) {
            continue;
        }
        //std::cout << "DEBUG) parsing line: " << line << std::endl;

        std::istringstream iss(line);
        std::vector<std::string> args;
        std::string arg;
        while (iss >> arg) {
            if (arg.back() == ',') {
                arg.pop_back();
            }
            args.push_back(arg);
        }
        result.push_back(parse_op(args, PE_OP(0, g)));
    }
    return result;
}

PE_OP Mapper::parse_op(const std::vector<std::string>& args, PE_OP op) {
    for (const auto& arg : args) {
        //std::cout << "DEBUG) parsing arg: " << arg << std::endl;
        assert(arg.find("//") == std::string::npos);        // Check if comment is present
        auto key_val = split(arg, ':');
        auto keyword = key_val[0];
        auto value = key_val[1];

        if (keyword == "IDX") {
            op.idx = std::stoi(value);
        } else if (keyword == "OPCODE") {
            op.opcode = map_opcode[value];
        } else if (keyword == "I1_USED") {
            op.i1_used = std::stoi(value) == 1;
        } else if (keyword == "I1_CONST_USED") {
            op.i1_const_used = std::stoi(value) == 1;
        } else if (keyword == "I1_SRC_OR_CONST") {
            op.i1_src_or_const = std::stol(value);
        } else if (keyword == "I2_USED") {
            op.i2_used = std::stoi(value) == 1;
        } else if (keyword == "I2_CONST_USED") {
            op.i2_const_used = std::stoi(value) == 1;
        } else if (keyword == "I2_SRC_OR_CONST") {
            op.i2_src_or_const = std::stol(value);
        } else if (keyword == "P_USED") {
            op.p_used = std::stoi(value) == 1;
        } else if (keyword == "P_SRC") {
            op.p_src = std::stol(value);
        } else if (keyword == "INIT_OUT_USED") {
            op.init_out_used = std::stoi(value) == 1;
        } else if (keyword == "INIT_OUT") {
            op.init_out = std::stol(value);
        } else {
            std::cerr << "ERROR) Unknown keyword: " << keyword << std::endl;
            assert(false);
        }
    }
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
    std::string bin = std::bitset<64>(dec).to_string();
    return bin.substr(bin.size() - len);
}

std::string Mapper::binaryFormat(const PE_OP& pe_op) {
    int datawidth = g.data_width;
    std::string s_pe_idx = toBinStr(g.src_idx_width, pe_op.idx);
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
    std::string s_margin(g.prog_mem_width - (g.opcode_width + datawidth * 3 + g.src_idx_width * 2 + 6), '0');

    std::string ret = s_margin + s_opcode + init_out + init_out_used + s_p_src + s_p_used + s_i2_src_or_const + s_i2_const_used + s_i2_used + s_i1_src_or_const + s_i1_const_used + s_i1_used + s_pe_idx;

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