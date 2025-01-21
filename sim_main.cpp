#include <iostream>
#include "VDODA.h"
#include "verilated.h"
#include "mapper.h"
#include <bitset>

#define DEBUG

int main() {
    General_Params g;               // Default parameters defined in mapper.h. This should not be changed.
    Mapper mapper("fully_connected.txt", g);
    auto parsed_ops = mapper.parsed();
    assert(parsed_ops.size() == g.num_cluster * g.inst_tab_size);

    std::cout << "Instruction set" << std::endl;
    auto v_split_pe_ops = mapper.split_to_clusters(parsed_ops);
    auto v_pe_w_dst_oh = mapper.updated_with_dst_oh(v_split_pe_ops);
    std::vector<std::vector<std::string>> v_bin_pe_ops;

    for (const auto& v_pe_ops : v_pe_w_dst_oh) {
        std::vector<std::string> bin_pe_ops;
        for (const auto& v_pe_ops_per_cluster : v_pe_ops) {
            for (const auto& pe_op : v_pe_ops_per_cluster) {
                //#ifdef DEBUG
                //std::cout << "DEBUG) pe_op: " << pe_op.toString() << std::endl;
                //#endif
                bin_pe_ops.push_back(mapper.binaryFormat(pe_op));
            }
        }
        v_bin_pe_ops.push_back(bin_pe_ops);
    }


    const char* dummy_argv[] = {nullptr};  // Avoid ambiguity
    Verilated::commandArgs(0, dummy_argv);  // Pass an empty argument list


    VDODA* doda = new VDODA;
    
    doda->clock = 1;
    doda->reset = 1;
    doda->eval();
    doda->clock = 0;
    doda->eval();
    doda->clock = 1;
    doda->reset = 0;
    doda->eval();
    doda->clock = 0;
    doda->eval();

    // At the beginning, the status should be 0 (IDLE)
    std::cout << "SIM_MAIN) Status: " << static_cast<int>(doda->io_status) << std::endl;
    // Sending an init signal
    doda->clock = 1;
    doda->io_init = 1;
    doda->eval();
    doda->clock = 0;
    doda->eval();

    doda->clock = 1;
    doda->io_init = 0;
    doda->eval();
    doda->clock = 0;
    doda->eval();

    // The status should be 1 (BEING_PROGRAMMED)
    std::cout << "SIM_MAIN) Status: " << static_cast<int>(doda->io_status) << std::endl;
    
    // START PROGRAMMING (Send instructions)
    //std::cout << sizeof(doda->io_inst_prog_in_bits) << std::endl;       32 bybtes
    //std::cout <<  sizeof(doda->io_inst_prog_in_bits[0]) << std::endl;    4 bytes

    // There are 4 PE clusters of 32 PEs each, equipped with instruction tables of 256 entries.
    // Programming the 4 PE clusters in parallel.
    for (int inst_tab_idx=0; inst_tab_idx<g.inst_tab_size; inst_tab_idx++) {
        doda->clock = 1;
        doda->io_v_inst_prog_in_0_valid = 1;
        doda->io_v_inst_prog_in_1_valid = 1;
        doda->io_v_inst_prog_in_2_valid = 1;
        doda->io_v_inst_prog_in_3_valid = 1;
        
        std::string bitstr0 = v_bin_pe_ops[0][inst_tab_idx];
        std::string bitstr1 = v_bin_pe_ops[1][inst_tab_idx];
        std::string bitstr2 = v_bin_pe_ops[2][inst_tab_idx];
        std::string bitstr3 = v_bin_pe_ops[3][inst_tab_idx];

        for (int i = 0; i < 4; ++i) {
            std::string slice0 = bitstr0.substr(128 - (i + 1) * 32, 32); // Ensure correct order
            std::string slice1 = bitstr1.substr(128 - (i + 1) * 32, 32); // Ensure correct order
            std::string slice2 = bitstr2.substr(128 - (i + 1) * 32, 32); // Ensure correct order
            std::string slice3 = bitstr3.substr(128 - (i + 1) * 32, 32); // Ensure correct order
            doda->io_v_inst_prog_in_0_bits[i] = 0;    // by default
            doda->io_v_inst_prog_in_1_bits[i] = 0;    // by default
            doda->io_v_inst_prog_in_2_bits[i] = 0;    // by default
            doda->io_v_inst_prog_in_3_bits[i] = 0;    // by default
            doda->io_v_inst_prog_in_0_bits[i] = std::stoul(slice0, nullptr, 2);
            doda->io_v_inst_prog_in_1_bits[i] = std::stoul(slice1, nullptr, 2);
            doda->io_v_inst_prog_in_2_bits[i] = std::stoul(slice2, nullptr, 2);
            doda->io_v_inst_prog_in_3_bits[i] = std::stoul(slice3, nullptr, 2);
        }
        doda->eval();
        doda->clock = 0;
        doda->eval();
    }
    
    doda->clock = 1;
    doda->io_v_inst_prog_in_0_valid = 0;
    doda->io_v_inst_prog_in_1_valid = 0;
    doda->io_v_inst_prog_in_2_valid = 0;
    doda->io_v_inst_prog_in_3_valid = 0;
    doda->eval();
    doda->clock = 0;
    doda->eval();

    doda->clock = 1;
    doda->io_v_in_prog_read_done_0 = 1;
    doda->io_v_in_prog_read_done_1 = 1;
    doda->io_v_in_prog_read_done_2 = 1;
    doda->io_v_in_prog_read_done_3 = 1;
    doda->eval();
    doda->clock = 0;
    doda->eval();
    doda->clock = 1;
    doda->io_v_in_prog_read_done_0 = 0;
    doda->io_v_in_prog_read_done_1 = 0;
    doda->io_v_in_prog_read_done_2 = 0;
    doda->io_v_in_prog_read_done_3 = 0;
    doda->eval();
    doda->clock = 0;
    doda->eval();
    
    // Sending memory data to the scratchpad memories of the PE clusters.

    // Filling the data memory
    // mem[0][i] = i
    // mem[1][i] = i + 32
    // mem[2][i] = i + 64
    // mem[3][i] = i + 96
    int cnt = 0;
    while (cnt < g.num_data_mem_entries && doda->io_v_t_axi_read_in_0_ready && doda->io_v_t_axi_read_in_1_ready && doda->io_v_t_axi_read_in_2_ready && doda->io_v_t_axi_read_in_3_ready) {
        doda->clock = 1;
        doda->io_v_t_axi_read_in_0_valid = 1;
        doda->io_v_t_axi_read_in_0_bits = cnt;
        doda->io_v_t_axi_read_in_1_valid = 1;
        doda->io_v_t_axi_read_in_1_bits = cnt+32;
        doda->io_v_t_axi_read_in_2_valid = 1;
        doda->io_v_t_axi_read_in_2_bits = cnt+64;
        doda->io_v_t_axi_read_in_3_valid = 1;
        doda->io_v_t_axi_read_in_3_bits = cnt+96;
        doda->eval();
        doda->clock = 0;
        doda->eval();
        cnt++;
    }
    doda->clock = 1;
    doda->io_v_t_axi_read_in_0_valid = 0;
    doda->io_v_t_axi_read_in_1_valid = 0;
    doda->io_v_t_axi_read_in_2_valid = 0;
    doda->io_v_t_axi_read_in_3_valid = 0;
    doda->eval();
    doda->clock = 0;
    doda->eval();
    // Stop filling the data memory

    // Informing DODA that the data transfer is done for a single cycle.
    doda->clock = 1;
    doda->io_v_in_spm_read_done_0 = 1;
    doda->io_v_in_spm_read_done_1 = 1;
    doda->io_v_in_spm_read_done_2 = 1;
    doda->io_v_in_spm_read_done_3 = 1;
    doda->eval();
    doda->clock = 0;
    doda->eval();
    doda->clock = 1;
    doda->io_v_in_spm_read_done_0 = 0;
    doda->io_v_in_spm_read_done_1 = 0;
    doda->io_v_in_spm_read_done_2 = 0;
    doda->io_v_in_spm_read_done_3 = 0;
    doda->eval();
    doda->clock = 0;
    doda->eval();
    doda->clock = 1;
    doda->eval();
    doda->clock = 0;
    doda->eval();
    // STAUTS should be 2 (WAITING)
    std::cout << static_cast<int>(doda->io_out_read_done) << std::endl;
    std::cout << "SIM_MAIN) Status: " << static_cast<int>(doda->io_status) << std::endl;

    // Sending an init signal to start execution.
    doda->clock = 1;
    doda->io_init = 1;
    doda->eval();
    doda->clock = 0;
    doda->eval();
    doda->clock = 1;
    doda->io_init = 0;
    doda->eval();
    doda->clock = 0;
    doda->eval();
    // STAUTS should be 3 (RUNNING)

    // Run until the execution is done.
    int cycle = 0;
    int cycle_limit = 1000;
    while (static_cast<int>(doda->io_status) < 5 && cycle < cycle_limit) {
        doda->clock = 1;
        doda->eval();
        doda->clock = 0;
        doda->eval();
        cycle++;
    }

    // If the execution is done successfully, the status should be 5 (DONE)
    if (static_cast<int>(doda->io_status) == 5) {
        std::cout << "SIM_MAIN) Execution is done successfully." << std::endl;
    }

    std::cout << "SIM_MAIN) Reading the scratchpad memory of doda." << std::endl;
    int mem_out_cnt = 0;
    std::vector<std::vector<int>> mem_out(4);
    while (mem_out_cnt < g.num_data_mem_entries) {
        doda->clock = 1;
        doda->io_v_t_axi_write_out_0_ready = 1;
        doda->io_v_t_axi_write_out_1_ready = 1;
        doda->io_v_t_axi_write_out_2_ready = 1;
        doda->io_v_t_axi_write_out_3_ready = 1;
        doda->eval();
        doda->clock = 0;
        doda->eval();
        mem_out[0].push_back(doda->io_v_t_axi_write_out_0_valid ? doda->io_v_t_axi_write_out_0_bits : -1);
        mem_out[1].push_back(doda->io_v_t_axi_write_out_1_valid ? doda->io_v_t_axi_write_out_1_bits : -1);
        mem_out[2].push_back(doda->io_v_t_axi_write_out_2_valid ? doda->io_v_t_axi_write_out_2_bits : -1);
        mem_out[3].push_back(doda->io_v_t_axi_write_out_3_valid ? doda->io_v_t_axi_write_out_3_bits : -1);
        mem_out_cnt++;
    }

    // Print the scratchpad memory
    for (int cluster_idx=0; cluster_idx<g.num_cluster; cluster_idx++) {
        std::cout << "Scratch Memory of Cluster " << cluster_idx << std::endl;
        for (int i=0; i<g.num_data_mem_entries; i++) {
            std::cout << mem_out[cluster_idx][i] << " ";
        }
        std::cout << std::endl;
    }


    delete doda;

    return 0;
}
