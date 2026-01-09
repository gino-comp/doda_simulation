# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the DODA (Dynamically Orchestrated Dataflow Architecture) simulation framework, which enables compilation of C++ lambda functions to dataflow graphs and provides cycle-accurate simulation of DODA hardware.

## Core Architecture

The system operates in two modes controlled by the `DODA_SIMULATION_MODE` compile-time flag:

1. **Compilation Mode** (`DODA_SIMULATION_MODE` not defined): Compiles lambda functions to DODA bitstreams and executes on host CPU
2. **Simulation Mode** (`DODA_SIMULATION_MODE` defined): Executes compiled bitstreams on the Verilator-based DODA simulator

### Key Components

- **doda_runtime.hpp**: Main API providing `map_on_doda()` function that accepts C++ lambdas with signature `uint32_t -> uint32_t`
- **DODASimulator**: Verilator-based cycle-accurate simulator of DODA hardware architecture
- **Mapper**: Converts dataflow operations to hardware instructions and handles PE (Processing Element) mapping
- **DODA Compiler**: Shared library that generates dataflow graphs from C++ lambdas

### Hardware Architecture

- 4 clusters with 32 processing elements (PEs) each
- 32-bit data width, 128-bit instruction width
- 256-entry instruction table per PE
- 1024-byte data memory per cluster

## Development Commands

### Build System
```bash
# Build Docker environment (required first step)
make docker-build

# Build simulation executable
make build_sim APP_SRC=<path_to_cpp_file>

# Build compilation executable  
make build_comp APP_SRC=<path_to_cpp_file>

```

### Example Development Workflow
```bash
cd example/

# Generate DFGs and build shared library
make build

# Build and run on host CPU
make app
make run

# Build and run on DODA simulator
make simulate

# Clean build artifacts
make clean
```

### Docker-Based Build
All compilation happens inside Docker containers using either `docker` or `podman`. The build system automatically detects which container engine is available.

### Library Dependencies
The system uses pre-compiled shared libraries in `lib/`:
- `DODA.so`: Verilator-based simulation library
- `libdoda_compiler.so`: DODA compiler shared library

## Key Files and Locations

- `include/doda_runtime.hpp`: Main runtime API - start here for understanding the system
- `example/map_on_doda_example.cpp`: Simple example showing dual lambda usage
- `src/doda_simulator.cpp`: Simulator implementation
- `src/mapper.cpp`: Hardware mapping logic
- `lib/`: Contains pre-compiled Verilator libraries (`DODA.so`, `libdoda_compiler.so`)

## Development Notes

- Lambda functions must have signature `uint32_t -> uint32_t`
- Input and output vectors must have matching sizes
- Global lambda counter (`g_lambda_counter`) tracks multiple `map_on_doda()` calls
- Bitstream files are generated in `./obj/lambda_X_bitstream.txt`
- DFG metadata is stored in `./obj/lambda_X_dfg.json`
- Build artifacts are placed in `obj/` subdirectories

## Architecture-Specific Details

The system requires x86_64 architecture and depends on Verilator for simulation. The DODA hardware parameters are fixed and must match the RTL design:
- 4 clusters (power of 2)
- 32 PEs per cluster (power of 2)  
- 32-bit data width
- 1024-byte data memory per cluster