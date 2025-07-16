# Minimal example Makefile for doda_simulation/example
ROOT_DIR = $(PWD)/..

# Docker configuration for running apps
CONTAINER_ENGINE := $(shell which podman >/dev/null 2>&1 && echo "podman" || echo "docker")
DOCKER_RUN = $(CONTAINER_ENGINE) run --rm \
	-v $(PWD)/..:/workspace \
	-w /workspace/example \
	encrypted-verilator:latest \
	bash -c

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

# Run the application in container
run: app
	@echo "- Running application in container..."
	$(DOCKER_RUN) "LD_LIBRARY_PATH=/workspace/lib:$$LD_LIBRARY_PATH ./app"

# Run simulation using root Makefile
simulate: obj/liblambda.so
	@echo "→ Running simulation using bitstream..."
	cd .. && make build_sim APP_SRC=example/map_on_doda_example.cpp
	@echo "→ Executing simulation in container..."
	$(DOCKER_RUN) "LD_LIBRARY_PATH=/workspace/lib:$$LD_LIBRARY_PATH ./sim_app"

clean:
	rm -rf obj/ app sim_app

.PHONY: all process build run visualize simulate clean
