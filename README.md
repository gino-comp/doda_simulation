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

## Using OpenCLAW

OpenCLAW is available as a separate Docker environment for advanced workflow automation and optimization.

### Setup and Build

Build the OpenCLAW Docker image:

```bash
make openclaw-build
```

### Available OpenCLAW Commands

| Target | Description |
|--------|-------------|
| `make openclaw-shell` | Enter interactive OpenCLAW shell |
| `make openclaw-onboard` | Initialize OpenCLAW onboarding |
| `make openclaw-health` | Check OpenCLAW health status |
| `make openclaw-gateway` | Start OpenCLAW gateway server (port 18789) |
| `make openclaw-gettoken` | Generate gateway authentication token |
| `make openclaw-ui-check` | Check OpenCLAW UI status |

### Interactive Development

Enter the OpenCLAW shell for interactive development:

```bash
make openclaw-shell
```

Inside the shell, you can use OpenCLAW commands such as openclaw agents list

### Onboarding 

Start the setup process:
```bash
make openclaw-onboard
```
Follow the instructions accordingly to setup. Onetime setup unless docker is rebuild.

### Gateway Setup

To access the OpenCLAW UI:

1. Start the gateway:
   ```bash
   make openclaw-gateway
   ```

2. In another terminal, generate a token if you did not get the token while onboarding:
   ```bash
   make openclaw-gettoken
   ```

3. Open your local terminal
    ```bash
    ssh -N -L 19099:127.0.0.1:18890 user@host
    ```
4. Access the UI at `http://127.0.0.1:18789/` with you auth token.

### Environment Variables

OpenCLAW configuration is stored in:
- `$HOME/.openclaw` - OpenCLAW configuration directory
- Gateway port: 18789
- UI port: 18791



