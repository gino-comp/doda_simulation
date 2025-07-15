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

# Include paths
INCLUDES = -Iinclude -Ilib -I/usr/local/include -I/usr/local/share/verilator/include

# Library paths and flags
LDFLAGS = -L/workspace/lib -L/usr/local/lib
LIBS = -l:DODA.so -lverilated -lverilated_vcd_c -lpthread -ldoda_c_api

# === Default target ===
all: 

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
		-Wl,-rpath,/workspace/lib"
	@echo "✓ Simulation executable built: $(DEST_DIR)sim_app"

# === Lambda extraction ===
extract_lambdas: check_app_src
	$(eval DEST_DIR ?= $(dir $(APP_SRC)))
	$(eval OBJ_DIR := $(DEST_DIR)obj)
	@echo "→ Extracting lambdas from $(APP_SRC)..."
	@mkdir -p $(OBJ_DIR)
	$(DOCKER_RUN) "clang++-14 -std=c++17 -I/workspace/include \
		-Xclang -load -Xclang /workspace/lib/libExtractLambdaPlugin.so \
		-Xclang -plugin -Xclang doda-plugin \
		-fsyntax-only /workspace/$(APP_SRC) && \
		mv lambda_*.cpp lambda_manifest.txt /workspace/$(OBJ_DIR)/ 2>/dev/null || true"
	@echo "✓ Lambda extraction completed"

# === DFG generation ===
generate_dfgs: extract_lambdas
	$(eval DEST_DIR ?= $(dir $(APP_SRC)))
	$(eval OBJ_DIR := $(DEST_DIR)obj)
	@echo "→ Generating DFGs..."
	$(DOCKER_RUN) "cd /workspace/$(OBJ_DIR) && \
		for lambda in lambda_*.cpp; do \
			if [ -f \$$lambda ]; then \
				base=\$${lambda%.cpp}; \
				clang++-14 -S -emit-llvm -std=c++17 -I/usr/include/c++/11 -I/usr/include/x86_64-linux-gnu/c++/11 \$$lambda -o \$${base}.ll; \
				/workspace/lib/dfg_gen \$${base}.ll \$$base > \$${base}_dfg.json; \
			fi; \
		done"
	@echo "✓ DFG generation completed"

# === Lambda library build ===
build_lambda_lib: generate_dfgs
	$(eval DEST_DIR ?= $(dir $(APP_SRC)))
	$(eval OBJ_DIR := $(DEST_DIR)obj)
	@echo "→ Building lambda shared library..."
	$(DOCKER_RUN) "cd /workspace/$(OBJ_DIR) && \
		if ls lambda_*.cpp 1> /dev/null 2>&1; then \
			g++ -shared -fPIC -o liblambda.so lambda_*.cpp; \
		fi"
	@echo "✓ Lambda library built"

# === Complete build process ===
build_comp: build_lambda_lib check_app_src $(DODA_LIB)
	$(eval DEST_DIR ?= $(dir $(APP_SRC)))
	@echo "→ Building compilation executable for $(APP_SRC)..."
	@echo "→ Output directory: $(DEST_DIR)"
	@mkdir -p $(DEST_DIR)
	$(DOCKER_RUN) "g++ $(CXXFLAGS) -o /workspace/$(DEST_DIR)app \
		$(EXTRA_INCLUDES) \
		$(INCLUDES) \
		/workspace/$(APP_SRC) /workspace/src/*.cpp \
		$(LDFLAGS) \
		$(LIBS) \
		-Wl,-rpath,/workspace/lib"
	@echo "✓ Compilation executable built: $(DEST_DIR)app"


# === Library dependency ===
$(DODA_LIB):
	@echo "Error: $(DODA_LIB) not found!"
	@echo "Make sure you've built the encrypted Verilator library first."
	@echo "Check that the .so file exists in the lib/ directory."
	@exit 1

# === Utilities ===
docker-build:
	$(CONTAINER_ENGINE) build --cpuset-cpus=4 -t $(DOCKER_IMAGE) ./docker

clean:
	find . -name "*.o" -delete
	find . -name "sim_app" -delete
	cd ./example && make clean

# === Validation ===
check_app_src:
	@if [ -z "$(APP_SRC)" ]; then echo "Error: Please specify APP_SRC=your_file.cpp"; exit 1; fi

.PHONY: all build_sim build_comp extract_lambdas generate_dfgs build_lambda_lib docker-build clean check_app_src