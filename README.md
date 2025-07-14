# DODA: Dynamically Orchestrated Dataflow Architecture

Welcome to the **Dynamically Orchestrated Dataflow Architecture (DODA)** simulation framework! This repository provides a complete development environment for experimenting with dataflow-based computing, featuring automatic compilation from C++ lambda functions to DODA bitstreams and cycle-accurate simulation.

## Project Structure

```
doda_simulation/
├── doda_compiler/          # DODA compiler (git submodule)
│   ├── dfg_gen/           # Data Flow Graph generation
│   ├── doda_plugin/       # LLVM plugin for lambda extraction
│   └── include/doda/      # Compiler headers
├── example/               # Example applications
│   └── map_on_doda_example.cpp
├── include/               # Simulation framework headers
│   ├── doda_runtime.hpp   # Main runtime interface
│   ├── doda_simulator.hpp # Verilator-based simulator
│   └── mapper.hpp         # Dataflow graph mapping
├── src/                   # Simulation framework source
├── lib/                   # Pre-compiled Verilator library
└── docker/                # Docker build environment
```

## What's Inside

- **DODA Compiler**: Automatically compiles C++ lambda functions to dataflow graphs and generates bitstreams
- **Runtime System**: High-level `map_on_doda()` API for seamless integration
- **Cycle-Accurate Simulator**: Verilator-based simulation of the DODA architecture
- **Example Applications**: Demonstrates vector operations using lambda functions

## Key Features

- **Lambda-to-Hardware**: Write C++ lambda functions that execute on DODA hardware
- **Automatic Compilation**: Seamless compilation from lambda to bitstream
- **Dual Mode Operation**: 
  - **Compilation Mode**: Generates dataflow graphs and bitstreams
  - **Simulation Mode**: Executes on cycle-accurate Verilator model
- **Docker-Based Build**: Consistent compilation environment across platforms

## Quick Start

### Prerequisites
- Docker or Podman
- Make
- Git with submodule support

### Setup
```bash
# Clone with submodules
git clone --recursive <repository-url>
cd doda_simulation

# Build Docker environment
make docker-build

# Setup compiler
make compiler
```

### Running Examples
```bash
# Navigate to example directory
cd example

# Compile and run on host CPU
make run

# Compile and simulate on DODA
make simulate
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

- `make compiler` - Setup DODA compiler
- `make build_sim APP_SRC=<file>` - Build simulation executable
- `make docker-build` - Build Docker environment
- `make clean` - Clean build artifacts

## Architecture Support

Currently supports x86_64 architecture.

## Development

For detailed development information, see:
- `doda_compiler/README.md` - Compiler documentation
- `example/` - Usage examples
- `include/` - API documentation 
