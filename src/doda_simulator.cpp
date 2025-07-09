#include "doda_simulator.hpp"
#include <iostream>
#include <cassert>

DODASimulator::DODASimulator() : doda_(std::make_unique<VDODA>()) {
    // Initialize Verilator
    const char* dummy_argv[] = {nullptr};
    Verilated::commandArgs(0, dummy_argv);
}

DODASimulator::~DODASimulator() {
    // Cleanup handled by unique_ptr
}

void DODASimulator::initialize() {
    reset();
}

void DODASimulator::reset() {
    // Reset sequence
    doda_->clock = 1;
    doda_->reset = 1;
    doda_->eval();
    doda_->clock = 0;
    doda_->eval();
    doda_->clock = 1;
    doda_->reset = 0;
    doda_->eval();
    doda_->clock = 0;
    doda_->eval();
}

void DODASimulator::cycle() {
    posedge();
    negedge();
}

void DODASimulator::posedge() {
    doda_->clock = 1;
    doda_->eval();
}

void DODASimulator::negedge() {
    doda_->clock = 0;
    doda_->eval();
}

DODASimulator::Status DODASimulator::getStatus() const {
    return static_cast<Status>(doda_->io_status);
}

bool DODASimulator::isReady() const {
    return getStatus() == Status::WAITING;
}

bool DODASimulator::isDone() const {
    return getStatus() == Status::DONE;
}

void DODASimulator::programInstructions(const std::vector<std::vector<std::string>>& binary_instructions) {
    // Send init signal to enter programming mode
    sendInitSignal();
    
    // Wait for BEING_PROGRAMMED status
    waitForStatus(Status::BEING_PROGRAMMED);
    
    General_Params g;
    
    // Program instructions for each cluster
    for (int inst_tab_idx = 0; inst_tab_idx < g.inst_tab_size; inst_tab_idx++) {
        posedge();
        
        // Enable programming for all clusters
        doda_->io_v_inst_prog_in_0_valid = 1;
        doda_->io_v_inst_prog_in_1_valid = 1;
        doda_->io_v_inst_prog_in_2_valid = 1;
        doda_->io_v_inst_prog_in_3_valid = 1;
        
        // Program each cluster
        for (int cluster = 0; cluster < 4; cluster++) {
            const std::string& bitstr = binary_instructions[cluster][inst_tab_idx];
            
            // Split 128-bit instruction into 4 32-bit chunks
            for (int i = 0; i < 4; i++) {
                std::string slice = bitstr.substr(128 - (i + 1) * 32, 32);
                uint32_t value = std::stoul(slice, nullptr, 2);
                
                switch (cluster) {
                    case 0: doda_->io_v_inst_prog_in_0_bits[i] = value; break;
                    case 1: doda_->io_v_inst_prog_in_1_bits[i] = value; break;
                    case 2: doda_->io_v_inst_prog_in_2_bits[i] = value; break;
                    case 3: doda_->io_v_inst_prog_in_3_bits[i] = value; break;
                }
            }
        }
        
        doda_->eval();
        negedge();
    }
    
    // Clear programming signals
    clearProgrammingSignals();
    
    // Signal programming completion
    signalProgrammingDone();
}

void DODASimulator::loadMemoryData(const std::vector<std::vector<int>>& memory_data) {
    General_Params g;
    
    // Load data into scratchpad memories
    int cnt = 0;
    while (cnt < g.num_data_mem_entries && 
           doda_->io_v_t_axi_read_in_0_ready && 
           doda_->io_v_t_axi_read_in_1_ready && 
           doda_->io_v_t_axi_read_in_2_ready && 
           doda_->io_v_t_axi_read_in_3_ready) {
        
        posedge();
        
        doda_->io_v_t_axi_read_in_0_valid = 1;
        doda_->io_v_t_axi_read_in_0_bits = memory_data[0][cnt];
        doda_->io_v_t_axi_read_in_1_valid = 1;
        doda_->io_v_t_axi_read_in_1_bits = memory_data[1][cnt];
        doda_->io_v_t_axi_read_in_2_valid = 1;
        doda_->io_v_t_axi_read_in_2_bits = memory_data[2][cnt];
        doda_->io_v_t_axi_read_in_3_valid = 1;
        doda_->io_v_t_axi_read_in_3_bits = memory_data[3][cnt];
        
        doda_->eval();
        negedge();
        cnt++;
    }
    
    // Clear memory signals
    clearMemorySignals();
    
    // Signal memory loading completion
    signalMemoryLoadDone();
}

void DODASimulator::startExecution() {
    // Send init signal to start execution
    sendInitSignal();
}

void DODASimulator::waitForCompletion(int max_cycles) {
    int cycle = 0;
    while (getStatus() < Status::DONE && cycle < max_cycles) {
        this->cycle();
        cycle++;
    }
    
    if (getStatus() == Status::DONE) {
        std::cout << "DODASimulator: Execution completed successfully." << std::endl;
    } else {
        std::cout << "DODASimulator: Execution timeout after " << max_cycles << " cycles." << std::endl;
    }
}

std::vector<std::vector<int>> DODASimulator::readMemory() {
    General_Params g;
    std::vector<std::vector<int>> memory_out(4);
    
    int mem_out_cnt = 0;
    while (mem_out_cnt < g.num_data_mem_entries) {
        posedge();
        
        doda_->io_v_t_axi_write_out_0_ready = 1;
        doda_->io_v_t_axi_write_out_1_ready = 1;
        doda_->io_v_t_axi_write_out_2_ready = 1;
        doda_->io_v_t_axi_write_out_3_ready = 1;
        
        doda_->eval();
        negedge();
        
        memory_out[0].push_back(doda_->io_v_t_axi_write_out_0_valid ? doda_->io_v_t_axi_write_out_0_bits : -1);
        memory_out[1].push_back(doda_->io_v_t_axi_write_out_1_valid ? doda_->io_v_t_axi_write_out_1_bits : -1);
        memory_out[2].push_back(doda_->io_v_t_axi_write_out_2_valid ? doda_->io_v_t_axi_write_out_2_bits : -1);
        memory_out[3].push_back(doda_->io_v_t_axi_write_out_3_valid ? doda_->io_v_t_axi_write_out_3_bits : -1);
        
        mem_out_cnt++;
    }
    
    return memory_out;
}

// Helper methods
void DODASimulator::setSignal(bool& signal, bool value) {
    signal = value;
}

void DODASimulator::pulseSignal(bool& signal) {
    posedge();
    signal = 1;
    doda_->eval();
    negedge();
    posedge();
    signal = 0;
    doda_->eval();
    negedge();
}

void DODASimulator::pulseSignal(CData& signal) {
    posedge();
    signal = 1;
    doda_->eval();
    negedge();
    posedge();
    signal = 0;
    doda_->eval();
    negedge();
}

void DODASimulator::clearProgrammingSignals() {
    posedge();
    doda_->io_v_inst_prog_in_0_valid = 0;
    doda_->io_v_inst_prog_in_1_valid = 0;
    doda_->io_v_inst_prog_in_2_valid = 0;
    doda_->io_v_inst_prog_in_3_valid = 0;
    doda_->eval();
    negedge();
}

void DODASimulator::clearMemorySignals() {
    posedge();
    doda_->io_v_t_axi_read_in_0_valid = 0;
    doda_->io_v_t_axi_read_in_1_valid = 0;
    doda_->io_v_t_axi_read_in_2_valid = 0;
    doda_->io_v_t_axi_read_in_3_valid = 0;
    doda_->eval();
    negedge();
}

void DODASimulator::waitForStatus(Status target_status) {
    while (getStatus() != target_status) {
        cycle();
    }
}

void DODASimulator::sendInitSignal() {
    pulseSignal(doda_->io_init);
}

void DODASimulator::signalProgrammingDone() {
    posedge();
    doda_->io_v_in_prog_read_done_0 = 1;
    doda_->io_v_in_prog_read_done_1 = 1;
    doda_->io_v_in_prog_read_done_2 = 1;
    doda_->io_v_in_prog_read_done_3 = 1;
    doda_->eval();
    negedge();
    
    posedge();
    doda_->io_v_in_prog_read_done_0 = 0;
    doda_->io_v_in_prog_read_done_1 = 0;
    doda_->io_v_in_prog_read_done_2 = 0;
    doda_->io_v_in_prog_read_done_3 = 0;
    doda_->eval();
    negedge();
}

void DODASimulator::signalMemoryLoadDone() {
    posedge();
    doda_->io_v_in_spm_read_done_0 = 1;
    doda_->io_v_in_spm_read_done_1 = 1;
    doda_->io_v_in_spm_read_done_2 = 1;
    doda_->io_v_in_spm_read_done_3 = 1;
    doda_->eval();
    negedge();
    
    posedge();
    doda_->io_v_in_spm_read_done_0 = 0;
    doda_->io_v_in_spm_read_done_1 = 0;
    doda_->io_v_in_spm_read_done_2 = 0;
    doda_->io_v_in_spm_read_done_3 = 0;
    doda_->eval();
    negedge();
    
    // Additional cycle for state transition
    cycle();
}