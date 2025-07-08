# Minimal example Makefile for doda_simulation/example
COMPILER_SETUP = compiler_setup
ROOT_DIR = ..
COMPILER_DIR = $(ROOT_DIR)/doda_compiler

all: process simulate

# Ensure the doda_compiler submodule is initialized and compiled
$(COMPILER_SETUP):
	@if [ ! -d "$(COMPILER_DIR)" ]; then \
		cd $(ROOT_DIR) && make compiler; \
	fi

process:
	@mkdir -p obj
	cd ../doda_compiler && make process APP_SRC=example/map_on_doda_example.cpp DEST_DIR=example/ EXTRA_MOUNT="-v $(PWD)/../include:/workspace/simulation_include -v $(PWD):/workspace/example" EXTRA_INCLUDES="-I/workspace/simulation_include"

build: process
	cd ../doda_compiler && make build-app APP_SRC=example/map_on_doda_example.cpp DEST_DIR=example/ EXTRA_MOUNT="-v $(PWD)/../include:/workspace/simulation_include -v $(PWD):/workspace/example" EXTRA_INCLUDES="-I/workspace/simulation_include"

# Visualize DFGs (optional)
visualize:
	@echo "- Visualizing DFGs..."
	cd ../doda_compiler && make visualize APP_SRC=example/map_on_doda_example.cpp	

# Run the application
run: build
	@echo "- Running application..."
	./app

clean:
	rm -rf obj/ app

.PHONY: all process simulate clean