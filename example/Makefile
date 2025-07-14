# Minimal example Makefile for doda_simulation/example
ROOT_DIR = $(PWD)/..
COMPILER_DIR = $(ROOT_DIR)/doda_compiler

all: build

$(COMPILER_DIR)/Makefile:
	@echo "Compiler not set up. Please run 'make compiler' at the root directory to setup the compiler first."
	@exit 1

# processing the input cpp file to generate the DFGs
obj/lambda_manifest.txt: map_on_doda_example.cpp $(COMPILER_DIR)/Makefile
	@echo "- Processing DFG for the input cpp file. This will generate the DFGs."
	@mkdir -p obj
	@rm -f app
	cd ../doda_compiler && make process APP_SRC=example/map_on_doda_example.cpp DEST_DIR=example/ EXTRA_MOUNT="-v $(PWD)/../include:/workspace/simulation_include -v $(PWD):/workspace/example" EXTRA_INCLUDES="-I/workspace/simulation_include -DDODA_COMPILATION_MODE"
process: obj/lambda_manifest.txt

# building the lambda functions to a shared library
obj/liblambda.so: obj/lambda_manifest.txt
	cd ../doda_compiler && make build-app APP_SRC=example/map_on_doda_example.cpp DEST_DIR=example/ EXTRA_MOUNT="-v $(PWD)/../include:/workspace/simulation_include -v $(PWD):/workspace/example" EXTRA_INCLUDES="-I/workspace/simulation_include -DDODA_COMPILATION_MODE"
build: obj/liblambda.so

# executable
app: obj/liblambda.so

# Run the application
run: app
	@echo "- Running application..."
	./app

# Run simulation using root Makefile
simulate: obj/liblambda.so
	@echo "→ Running simulation using bitstream..."
	cd .. && make build_sim APP_SRC=example/map_on_doda_example.cpp
	@echo "→ Executing simulation..."
	LD_LIBRARY_PATH=../lib:$$LD_LIBRARY_PATH ./sim_app

clean:
	rm -rf obj/ app sim_app

.PHONY: all process build run visualize simulate clean
