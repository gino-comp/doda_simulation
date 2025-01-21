# DODA: Dynamically Orchestrated Dataflow Architecture (Protected Verilator Library)

Welcome to the protected Verilator library for the **Dynamically Orchestrated Dataflow Architecture (DODA)**! This repository contains an example dataflow graph that demonstrates a map-reduce operation, showcasing the power and flexibility of DODA. Alongside the Verilated library, a C++ testbench is included to facilitate simulation and experimentation.

## What's Inside
- **Protected Verilator Library**: A secure, pre-compiled Verilator library for simulating the DODA architecture.
- **Example Dataflow Graph**: A simple graph that first performs a **map** function and subsequently applies a **reduce** function.
- **C++ Testbench**: A complete C++ testbench to simulate the map-reduce graph on DODA.

## Experiment and Explore
Feel free to tweak the input dataflow graph to experience the versatility of DODA. Imagine the possibilities:
- **Accelerate Chip Design**: Rapidly prototype and iterate on new chip designs.
- **Offload CPU Workloads**: Shift computationally intensive tasks to DODA, relieving the CPU.
- **Runtime Reconfiguration**: Dynamically reconfigure DODA at runtime, adapting to new tasks on the fly.

With DODA, the limits of conventional architecture no longer constrain you. Explore new frontiers in computing!

## How to Compile and Run
### Compilation:
```bash
make
```
### Execution
```bash
make run
```
Note) certain Verilator libraries are required to run the simulation successfully. These include:
libverilated.a and libverilated_vcd_c.a


### How to build required Verilator libraries
If you encounter missing `libverilated.a` errors, follow these steps to manually compile it:

1. Clone the Verilator source code:
   ```bash
   git clone https://github.com/verilator/verilator.git
   ```
2. Checkout to version v5.020:
   ```bash
   cd verilator
   git checkout v5.020
   ```

3. Build Verilator
   ``` bash
   autoconf
   ./configure
   make
   ```
4. Compile the required files:
   ```bash
   cd include
   g++ -c verilated.cpp verilated_dpi.cpp verilated_threads.cpp
   mkdir -p ../lib64
   ar rcs ../lib64/libverilated.a verilated.o verilated_dpi.o verilated_threads.o
   g++ -c verilated_vcd_c.cpp
   ar rcs ../lib64/libverilated_vcd_c.a verilated_vcd_c.o
   ```
5. Install the compiled libraries:
   ```bash
   sudo cp ../lib64/libverilated.a /usr/share/verilator/lib64/
   sudo cp ../lib64/libverilated_vcd_c.a /usr/share/verilator/lib64/
   ```

For a detailed explanation regarding Verilator installation, please refer to https://verilator.org/guide/latest/install.html.