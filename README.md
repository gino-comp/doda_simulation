# DODA Simulation

Cycle-accurate simulator for **Dynamically Orchestrated Dataflow Architecture (DODA)**.

## Quick Start

```bash
# Build Docker environment
make docker-build

cd example
```

## Two Ways to Run

### 1. C++ Lambda Pipeline

Write C++ lambdas that compile to DODA bitstreams:

```cpp
#include <doda_runtime.hpp>

std::vector<uint32_t> input{1, 2, 3, 4};
std::vector<uint32_t> output(4);

map_on_doda([](uint32_t x) { return x * 2; }, input, output);
```

```bash
make run       # Generate bitstream and run on CPU (required first)
make simulate  # Run on DODA simulator
```

### 2. Custom Dataflow Graph

Write your own dataflow graph in text format and simulate directly:

```bash
# Convert DFG txt to bitstream and simulate (no prior setup needed)
make simulate-txt

# Or with custom inputs:
make simulate-txt INPUT_DFG_TXT=my_graph.txt INPUT_DATA=my_data.txt
```

See `example/DFG_CONV_Mapping.txt` for graph format and `example/input_data_mem.txt` for input data format.

## Build Targets

### Root Makefile

| Target | Description |
|--------|-------------|
| `make docker-build` | Build Docker environment |
| `make clean` | Clean build artifacts |

### example/Makefile

| Target | Description |
|--------|-------------|
| `make run` | Generate bitstream and run on CPU |
| `make simulate` | Run on DODA simulator (requires `make run` first) |
| `make simulate-txt` | Convert custom DFG and simulate |
| `make clean` | Clean example build artifacts |
