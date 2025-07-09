#pragma once

#include <vector>
#include <string>
#include <memory>
#include "VDODA.h"
#include "verilated.h"
#include "mapper.hpp"

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