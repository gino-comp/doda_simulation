# Minimal example Makefile for doda_simulation/example
ROOT_DIR = $(PWD)/..

# Docker configuration for running apps
IN_CONTAINER ?= 0

ifeq ($(IN_CONTAINER),1)
RUN_IN_EXAMPLE = bash -lc
else
CONTAINER_ENGINE := $(shell which podman >/dev/null 2>&1 && echo "podman" || echo "docker")
DOCKER_RUN = $(CONTAINER_ENGINE) run --rm -it \
	-v $(PWD)/..:/workspace \
	-w /workspace/example \
	encrypted-verilator:latest \
	bash -c
RUN_IN_EXAMPLE = $(DOCKER_RUN)
endif

all: build

# Build everything in one go - only run if source changed
app: map_on_doda_example.cpp 
	@echo "→ Building complete lambda pipeline..."
	@mkdir -p obj
	cd .. && make build_comp APP_SRC=example/map_on_doda_example.cpp DEST_DIR=example/ IN_CONTAINER=$(IN_CONTAINER)

# Legacy targets for compatibility
obj/liblambda.so: app
	@# Library is built as part of app target

# Convenience targets
process: app
build: app

# Run the application in container
run: app
	@echo "- Running application in container..."
	$(RUN_IN_EXAMPLE) "LD_LIBRARY_PATH=/workspace/lib:$$LD_LIBRARY_PATH ./app"

# Run simulation using root Makefile
simulate: obj/liblambda.so
	@echo "→ Running simulation using bitstream..."
	cd .. && make build_sim APP_SRC=example/map_on_doda_example.cpp IN_CONTAINER=$(IN_CONTAINER)
	@echo "→ Executing simulation in container..."
	$(RUN_IN_EXAMPLE) "LD_LIBRARY_PATH=/workspace/lib:$$LD_LIBRARY_PATH ./sim_app"

# Convert Mapper_Node txt file to bitstream using libdoda_compiler
# Usage: make txt-to-bitstream [INPUT_DFG_TXT=<file.txt>]
INPUT_DFG_TXT ?= DFG_CONV_Mapping.txt
txt-to-bitstream:
	@echo "→ Building txt_to_bitstream..."
	@mkdir -p obj
	$(RUN_IN_EXAMPLE) "g++ -std=c++17 -I/workspace/include txt_to_bitstream.cpp -L/workspace/lib -ldoda_compiler -o obj/txt_to_bitstream"
	@echo "→ Converting $(INPUT_DFG_TXT) to bitstream..."
	$(RUN_IN_EXAMPLE) "LD_LIBRARY_PATH=/workspace/lib ./obj/txt_to_bitstream $(INPUT_DFG_TXT) obj/"

# Build standalone bitstream runner
obj/run_bitstream: run_bitstream.cpp
	@echo "→ Building run_bitstream..."
	@mkdir -p obj
	cd .. && make build_sim APP_SRC=example/run_bitstream.cpp DEST_DIR=example/obj/ IN_CONTAINER=$(IN_CONTAINER)
	@mv obj/sim_app obj/run_bitstream

# Run simulation directly with a bitstream file
# Usage: make run-bitstream BITSTREAM=<file.txt> [INPUT_DATA=<data.txt>]
BITSTREAM ?= obj/DFG_CONV_Mapping.txt
INPUT_DATA ?= input_data_mem.txt
run-bitstream: obj/run_bitstream
	@echo "→ Running simulation with bitstream: $(BITSTREAM)"
	$(RUN_IN_EXAMPLE) "LD_LIBRARY_PATH=/workspace/lib:$$LD_LIBRARY_PATH ./obj/run_bitstream $(BITSTREAM) $(INPUT_DATA)"

# Combined: convert txt to bitstream and run simulation
# Usage: make simulate-txt [INPUT_DFG_TXT=<file.txt>] [INPUT_DATA=<data.txt>]
simulate-txt: txt-to-bitstream obj/run_bitstream
	@echo "→ Running simulation with generated bitstream..."
	$(eval BITSTREAM_FILE := obj/$(basename $(notdir $(INPUT_DFG_TXT)))_bitstream.txt)
	$(RUN_IN_EXAMPLE) "LD_LIBRARY_PATH=/workspace/lib:$$LD_LIBRARY_PATH ./obj/run_bitstream $(BITSTREAM_FILE) $(INPUT_DATA)"

clean:
	rm -rf obj/ app sim_app

.PHONY: all process build run visualize simulate clean txt-to-bitstream run-bitstream simulate-txt
