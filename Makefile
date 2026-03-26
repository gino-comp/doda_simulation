# Root Makefile for DODA Simulation
# Usage: make build_sim APP_SRC=your_file.cpp

# === Configuration ===
DOCKER_IMAGE := encrypted-verilator:latest
CONTAINER_ENGINE := $(shell which podman >/dev/null 2>&1 && echo "podman" || echo "docker")
EXTRA_INCLUDES ?=
EXTRA_MOUNT ?=
IN_CONTAINER ?= 0

ifeq ($(IN_CONTAINER),1)
RUN_IN_WORKSPACE = bash -lc
else
# Docker run command
DOCKER_RUN = $(CONTAINER_ENGINE) run --rm \
	-v $(PWD):/workspace \
	$(EXTRA_MOUNT) \
	-w /workspace \
	$(DOCKER_IMAGE) \
	bash -c
RUN_IN_WORKSPACE = $(DOCKER_RUN)
endif

# Compiler settings
CXX_STD := c++14
CXXFLAGS := -std=$(CXX_STD) -fPIC

## OpenClaw
OPENCLAW_IMAGE := openclaw-dev:latest
OPENCLAW_CONFIG_DIR := $(HOME)/.openclaw
OPENCLAW_GATEWAY_PORT := 18789 # Port to access the UI
OPENCLAW_UI_PORT := 18791
OPENCLAW_TUNNEL_PORT := 19099

OPENCLAW_RUN = $(CONTAINER_ENGINE) run --rm -it \
	--network host \
	-v $(PWD):/workspace \
	-v $(OPENCLAW_CONFIG_DIR):/root/.openclaw \
	-w /workspace \
	$(OPENCLAW_IMAGE) \
	bash -lc

OPENCLAW_SHELL = $(CONTAINER_ENGINE) run --rm -it \
	--network host \
	-v $(PWD):/workspace \
	-v $(OPENCLAW_CONFIG_DIR):/root/.openclaw \
	-w /workspace \
	$(OPENCLAW_IMAGE) \
	bash

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
	$(RUN_IN_WORKSPACE) "g++ $(CXXFLAGS) -o /workspace/$(DEST_DIR)sim_app \
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
	$(RUN_IN_WORKSPACE) "clang++-14 -std=c++17 -I/workspace/include \
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
	$(RUN_IN_WORKSPACE) "cd /workspace/$(OBJ_DIR) && \
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
	$(RUN_IN_WORKSPACE) "cd /workspace/$(OBJ_DIR) && \
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
	$(RUN_IN_WORKSPACE) "g++ $(CXXFLAGS) -o /workspace/$(DEST_DIR)app \
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


# === OpenClaw ===
openclaw-build:
	@mkdir -p $(OPENCLAW_CONFIG_DIR)
	$(CONTAINER_ENGINE) build -t $(OPENCLAW_IMAGE) -f ./docker/Dockerfile.openclaw ./docker

openclaw-shell:
	@mkdir -p $(OPENCLAW_CONFIG_DIR)
	$(OPENCLAW_SHELL)

openclaw-onboard:
	@mkdir -p $(OPENCLAW_CONFIG_DIR)
	$(OPENCLAW_RUN) "openclaw onboard"

openclaw-health:
	@mkdir -p $(OPENCLAW_CONFIG_DIR)
	$(OPENCLAW_RUN) "node --version && npm --version && openclaw --version"

openclaw-gateway:
	@mkdir -p $(OPENCLAW_CONFIG_DIR)
	$(CONTAINER_ENGINE) run --rm -it \
		--network host \
		-v $(PWD):/workspace \
		-v $(OPENCLAW_CONFIG_DIR):/root/.openclaw \
		-w /workspace \
		$(OPENCLAW_IMAGE) \
		openclaw gateway --port $(OPENCLAW_GATEWAY_PORT) --verbose

openclaw-gettoken:
	@mkdir -p $(OPENCLAW_CONFIG_DIR)
	$(OPENCLAW_RUN) "openclaw doctor --generate-gateway-token"

openclaw-ui-check:
	$(CONTAINER_ENGINE) exec -it $$( $(CONTAINER_ENGINE) ps -q --filter ancestor=$(OPENCLAW_IMAGE) | head -n 1 ) bash -lc "curl -I http://127.0.0.1:$(OPENCLAW_UI_PORT)/ || true"

.PHONY: all build_sim build_comp extract_lambdas generate_dfgs build_lambda_lib docker-build clean check_app_src \
	openclaw-build openclaw-shell openclaw-onboard openclaw-gateway openclaw-health openclaw-ui-check
