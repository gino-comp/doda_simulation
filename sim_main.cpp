#include <iostream>
#include "VPE_Cluster.h"
#include "verilated.h"
#include "mapper.h"
#include <bitset>

int main() {
    General_Params g;               // Default parameters defined in mapper.h. This should not be changed.
    Mapper mapper("map_reduce.txt", g);
    auto parsed_ops = mapper.parsed();
    assert(parsed_ops.size() == g.inst_tab_size);

    std::cout << "Instruction set" << std::endl;
    for (const auto& op : parsed_ops) {
        std::cout << op.toString() << std::endl;
        //std::cout << "Binary format: " << mapper.binaryFormat(op) << std::endl;
        assert(mapper.binaryFormat(op).size() == g.prog_mem_width);
    }


    const char* dummy_argv[] = {nullptr};  // Avoid ambiguity
    Verilated::commandArgs(0, dummy_argv);  // Pass an empty argument list

    /*
    Vadder* adder = new Vadder;

    adder->a = 42;
    adder->b = 27;
    adder->eval();

    std::cout << "Sum: " << (int)adder->sum << std::endl;

    delete adder;
    */

    VPE_Cluster* doda = new VPE_Cluster;
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

    for (const auto& op : parsed_ops) {
        doda->clock = 1;
        doda->io_inst_prog_in_valid = 1;
        std::bitset<128> bits(mapper.binaryFormat(op));
        std::string bitstr = bits.to_string();
        for (int i = 0; i < 4; ++i) {
            std::string slice = bitstr.substr(128 - (i + 1) * 32, 32); // Ensure correct order
            doda->io_inst_prog_in_bits[i] = 0;    // padding
            doda->io_inst_prog_in_bits[i] = std::stoul(slice, nullptr, 2);
        }
        doda->eval();
        doda->clock = 0;
        doda->eval();
    }
    
    doda->clock = 1;
    doda->io_inst_prog_in_valid = 0;
    doda->eval();
    doda->clock = 0;
    doda->eval();
    // Stop sending instructions.

    // Filling the data memory
    // mem[i] = i
    int cnt = 0;
    while (cnt < g.num_data_mem_entries) {
        doda->clock = 1;
        doda->io_t_axi_read_in_valid = 1;
        doda->io_t_axi_read_in_bits = cnt;
        doda->eval();
        doda->clock = 0;
        doda->eval();
        if (doda->io_t_axi_read_in_ready) {
            cnt++;
        }
    }
    doda->clock = 1;
    doda->io_t_axi_read_in_valid = 0;
    doda->eval();
    doda->clock = 1;
    doda->eval();
    // Stop filling the data memory

    // Informing DODA that the data transfer is done for a single cycle.
    doda->clock = 1;
    doda->io_in_prog_read_done = 1;
    doda->io_in_spm_read_done = 1;
    doda->eval();
    doda->clock = 0;
    doda->eval();
    doda->clock = 1;
    doda->eval();
    doda->clock = 0;
    doda->eval();
    doda->clock = 1;
    doda->io_in_prog_read_done = 0;
    doda->io_in_spm_read_done = 0;
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
    while (mem_out_cnt < g.num_data_mem_entries) {
        doda->clock = 1;
        doda->io_t_axi_write_out_ready = 1;
        doda->eval();
        doda->clock = 0;
        doda->eval();
        if (doda->io_t_axi_write_out_valid) {
            std::cout << "SIM_MAIN) idx:" << mem_out_cnt << ", Data: " << doda->io_t_axi_write_out_bits << std::endl;
            mem_out_cnt++;
        }
    }

    delete doda;

    return 0;
}
