# Root Makefile for DODA Simulation
# Usage: make build_sim APP_SRC=your_file.cpp

# === Configuration ===
DOCKER_IMAGE := encrypted-verilator:latest
CONTAINER_ENGINE := $(shell which podman >/dev/null 2>&1 && echo "podman" || echo "docker")
EXTRA_INCLUDES ?=
EXTRA_MOUNT ?=

# Compiler settings
CXX_STD := c++14
CXXFLAGS := -std=$(CXX_STD) -fPIC

# Docker run command
DOCKER_RUN = $(CONTAINER_ENGINE) run --rm \
	-v $(PWD):/workspace \
	$(EXTRA_MOUNT) \
	-w /workspace \
	$(DOCKER_IMAGE) \
	bash -c

# Paths
DODA_LIB = lib/DODA.so
COMPILER_DIR = doda_compiler

# Include paths
INCLUDES = -Iinclude -Ilib -I/usr/local/include -I/usr/local/share/verilator/include

# Library paths and flags
LDFLAGS = -L/workspace/lib -L/usr/local/lib
LIBS = -l:DODA.so -lverilated -lverilated_vcd_c -lpthread

# === Default target ===
all: compiler

# === Simulation build ===
build_sim: check_app_src $(DODA_LIB)
	$(eval DEST_DIR ?= $(dir $(APP_SRC)))
	@echo "→ Building simulation executable for $(APP_SRC)..."
	@echo "→ Output directory: $(DEST_DIR)"
	@mkdir -p $(DEST_DIR)
	$(DOCKER_RUN) "g++ $(CXXFLAGS) -o /workspace/$(DEST_DIR)sim_app \
		$(EXTRA_INCLUDES) \
		-DDODA_SIMULATION_MODE \
		$(INCLUDES) \
		/workspace/$(APP_SRC) /workspace/src/*.cpp \
		$(LDFLAGS) \
		$(LIBS) \
		-Wl,-rpath,../lib"
	@echo "✓ Simulation executable built: $(DEST_DIR)sim_app"

# === Compiler setup ===
init_submodule:
	git submodule update --init --remote

compiler: init_submodule $(COMPILER_DIR)/Makefile
	cd $(COMPILER_DIR) && make

# === Library dependency ===
$(DODA_LIB):
	@echo "Error: $(DODA_LIB) not found!"
	@echo "Make sure you've built the encrypted Verilator library first."
	@echo "Check that the .so file exists in the lib/ directory."
	@exit 1

# === Utilities ===
docker-build:
	$(CONTAINER_ENGINE) build -t $(DOCKER_IMAGE) ./docker

clean:
	rm -rf $(COMPILER_DIR)
	find . -name "*.o" -delete
	find . -name "sim_app" -delete

# === Validation ===
check_app_src:
	@if [ -z "$(APP_SRC)" ]; then echo "Error: Please specify APP_SRC=your_file.cpp"; exit 1; fi

.PHONY: all build_sim compiler init_submodule docker-build clean check_app_src