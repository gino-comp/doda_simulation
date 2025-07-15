# Minimal example Makefile for doda_simulation/example
ROOT_DIR = $(PWD)/..

all: build

# Build everything in one go - only run if source changed
app: map_on_doda_example.cpp 
	@echo "→ Building complete lambda pipeline..."
	@mkdir -p obj
	cd .. && make build_comp APP_SRC=example/map_on_doda_example.cpp DEST_DIR=example/

# Legacy targets for compatibility
obj/liblambda.so: app
	@# Library is built as part of app target

# Convenience targets
process: app
build: app

# Run the application
run: app
	@echo "- Running application..."
	LD_LIBRARY_PATH=../lib:$$LD_LIBRARY_PATH ./app

# Run simulation using root Makefile
simulate: obj/liblambda.so
	@echo "→ Running simulation using bitstream..."
	cd .. && make build_sim APP_SRC=example/map_on_doda_example.cpp
	@echo "→ Executing simulation..."
	LD_LIBRARY_PATH=../lib:$$LD_LIBRARY_PATH ./sim_app

clean:
	rm -rf obj/ app sim_app

.PHONY: all process build run visualize simulate clean
