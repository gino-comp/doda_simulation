#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include "VDODA.h"
#include "verilated.h"

// Helper function for hardware parameter calculations
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

// Hardware parameters for DODA processor
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
            opcode_width(5), inst_tab_size(256),
            data_mem_size_byte(1024)
    {
        num_data_mem_entries = data_mem_size_byte * 8 / data_width;
        src_pe_idx_width = log2Ceil(num_pe_per_cluster);
        src_cluster_idx_width = num_cluster; 
        src_idx_width = src_pe_idx_width + src_cluster_idx_width;

        assert((num_cluster & (num_cluster - 1)) == 0 && "num_cluster must be a power of 2");
        assert((num_pe_per_cluster & (num_pe_per_cluster - 1)) == 0 && "num_pe_per_cluster must be a power of 2");
    }
    // DON'T CHANGE THESE DEFAULT VALUES. THEY ARE MATCHED WITH THE RTL DESIGN.
};

class DODASimulator {
public:
    enum class Status {
        IDLE = 0,
        BEING_PROGRAMMED = 1,
        WAITING = 2,
        RUNNING = 3,
        DONE = 5
    };

    DODASimulator();
    ~DODASimulator();

    // Hardware initialization
    void initialize();
    void reset();
    
    // Clock management
    void cycle();
    void posedge();
    void negedge();
    
    // Status monitoring
    Status getStatus() const;
    bool isReady() const;
    bool isDone() const;
    
    // High-level programming interface
    void programInstructions(const std::vector<std::vector<std::string>>& binary_instructions);
    void loadMemoryData(const std::vector<std::vector<int>>& memory_data);
    
    // Execution control
    void startExecution();
    void waitForCompletion(int max_cycles = 1000);
    
    // Memory operations
    std::vector<std::vector<int>> readMemory();
    
    // Raw hardware access (for advanced users)
    VDODA* getHardware() { return doda_.get(); }

private:
    std::unique_ptr<VDODA> doda_;
    
    // Helper methods for signal management
    void setSignal(bool& signal, bool value);
    void pulseSignal(bool& signal);
    void pulseSignal(CData& signal);
    void clearProgrammingSignals();
    void clearMemorySignals();
    
    // Internal state management
    void waitForStatus(Status target_status);
    void sendInitSignal();
    void signalProgrammingDone();
    void signalMemoryLoadDone();
};