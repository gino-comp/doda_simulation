# DODA: Dynamically Orchestrated Dataflow Architecture

Welcome to the **Dynamically Orchestrated Dataflow Architecture (DODA)** simulation framework! This repository provides a complete development environment for experimenting with dataflow-based computing, featuring automatic compilation from C++ lambda functions to DODA bitstreams and cycle-accurate simulation.

## 🎉 Integration Status: COMPLETE

✅ **Full End-to-End Pipeline Working**
- Lambda extraction from C++ source
- DFG (Dataflow Graph) generation from LLVM IR  
- Real bitstream generation with actual hardware instructions
- Cycle-accurate Verilator-based hardware simulation
- Both compilation and simulation modes produce identical results

## Project Structure

```
doda_simulation/
├── lib/                     # Shared libraries (FIXED with real mapper)
│   ├── DODA.so              # Verilator simulation library
│   ├── libdoda_c_api.so     # DODA C API (now with real bitstream generation)
│   ├── libdoda_mapper.so    # Real mapper implementation
│   ├── libdoda_core.so      # Core utilities
│   └── libExtractLambdaPlugin.so # Lambda extraction plugin
├── example/                 # Example applications
│   ├── map_on_doda_example.cpp  # Main test application
│   └── obj/                 # Generated files (DFGs, bitstreams, libraries)
├── include/                 # Simulation framework headers
│   ├── doda_runtime.hpp     # Main runtime interface (FIXED bitstream parsing)
│   ├── doda_simulator.hpp   # Verilator-based simulator
│   └── doda_compiler_api.h  # C API header
├── src/                     # Simulation framework source
│   ├── doda_simulator.cpp   # Simulator implementation
│   └── mapper.cpp           # Mapper utilities
└── docker/                  # Docker build environment
```

## What's Inside

- **DODA Compiler**: Automatically compiles C++ lambda functions to dataflow graphs and generates **real hardware bitstreams** (FIXED)
- **Runtime System**: High-level `map_on_doda()` API with dual-mode execution
- **Cycle-Accurate Simulator**: Verilator-based simulation that executes actual bitstreams
- **Complete Integration**: End-to-end pipeline from C++ lambda to hardware simulation

## Key Features

- **Lambda-to-Hardware**: Write C++ lambda functions that execute on DODA hardware
- **Real Bitstream Generation**: DODA compiler now generates actual hardware instructions (not dummy zeros)
- **Dual Mode Operation**: 
  - **Compilation Mode** (`make run`): Generates DFGs, bitstreams, and executes on CPU
  - **Simulation Mode** (`make simulate`): Executes on cycle-accurate Verilator DODA model
- **Verified Results**: Both modes produce identical, correct outputs
- **Docker-Based Build**: Consistent LLVM/Clang environment across platforms

## Quick Start

### Prerequisites
- Docker or Podman
- Make

### Setup
```bash
# Clone repository
git clone <repository-url>
cd doda_simulation

# Build Docker environment
make docker-build
```

### Running Examples
```bash
# Navigate to example directory
cd example

# Compile and run on host CPU (generates real bitstreams)
make run
# Expected output:
# Output after the first map_on_doda: 1 0 1 0 1 0 
# Output after the second map_on_doda: 2 0 2 0 4 0

# Compile and simulate on DODA hardware (cycle-accurate)
make simulate
# Expected output: Same as compilation mode - proves integration works!
```

## Usage

The DODA runtime provides a simple API for executing lambda functions:

```cpp
#include <doda_runtime.hpp>

std::vector<uint32_t> input{1, 2, 3, 4, 5};
std::vector<uint32_t> output(5);

// Execute lambda on DODA
map_on_doda([](uint32_t x) { 
    return x >= 1 ? uint32_t(1) : uint32_t(0); 
}, input, output);
```

## Build Targets

- `make run` - Build and run compilation mode (CPU execution + bitstream generation)
- `make simulate` - Build and run simulation mode (Verilator hardware simulation)
- `make build_comp APP_SRC=<file>` - Build compilation executable for custom source
- `make build_sim APP_SRC=<file>` - Build simulation executable for custom source
- `make docker-build` - Build Docker environment
- `make clean` - Clean build artifacts

## Recent Fixes (Integration Complete)

### 🔧 **Critical Issue Resolved**: 
The DODA compiler's C API was using a dummy implementation that generated all-zero bitstreams instead of real hardware instructions.

### ✅ **Fix Applied**:
- Updated `doda_compiler/CMakeLists.txt` to use proper template (`doda_c_api.cpp.in` instead of `doda_c_api_standalone_complete.cpp.in`)
- Now uses real `doda_mapper` to generate actual hardware bitstreams
- Updated simulator libraries with fixed implementation

### 🎯 **Result**:
Both compilation and simulation modes now produce identical, correct results, proving the complete integration pipeline works end-to-end.

## Architecture Support

Currently supports x86_64 architecture.

## Development

For detailed development information, see:
- `include/doda_compiler_api.h` - Compiler API documentation
- `example/` - Usage examples
- `include/` - Runtime API documentation 
