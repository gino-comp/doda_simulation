# Root Makefile for DODA Simulation
# Usage: make simulate APP_SRC=your_file.cpp

# === Configuration ===
DOCKER_IMAGE := encrypted-verilator:latest
CONTAINER_ENGINE := $(shell which podman >/dev/null 2>&1 && echo "podman" || echo "docker")
EXTRA_INCLUDES ?=
EXTRA_MOUNT ?=

# Docker run command
DOCKER_RUN = $(CONTAINER_ENGINE) run --rm \
	-v $(PWD):/workspace \
	$(EXTRA_MOUNT) \
	-w /workspace \
	$(DOCKER_IMAGE) \
	bash -c

# Library paths
DODA_LIB = lib/DODA.so

# === Legacy simulator build (for backward compatibility) ===
SOURCES = sim_main.cpp mapper.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = doda_sim

all: $(COMPILER) $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJECTS) $(DODA_LIB)
	$(DOCKER_RUN) "g++ -std=c++14 -fPIC -o $(TARGET) \
		-Ilib \
		-I/usr/local/include \
		-I/usr/local/share/verilator/include \
		$(OBJECTS) $(DODA_LIB) \
		-L/usr/local/lib \
		-lverilated \
		-lverilated_vcd_c \
		-lpthread"

%.o: %.cpp
	$(DOCKER_RUN) "g++ -std=c++14 -fPIC -c \
		-Ilib \
		-I/usr/local/include \
		-I/usr/local/share/verilator/include \
		$< -o $@"

# === Parameterized simulation rule (for any input file) ===
build_sim: check_app_src $(DODA_LIB)
	$(eval DEST_DIR ?= $(dir $(APP_SRC)))
	@echo "→ Building simulation executable for $(APP_SRC)..."
	@echo "→ Output directory: $(DEST_DIR)"
	@mkdir -p $(DEST_DIR)
	$(DOCKER_RUN) "g++ -std=c++14 -fPIC -o /workspace/$(DEST_DIR)sim_app \
		$(EXTRA_INCLUDES) \
		-DDODA_SIMULATION_MODE \
		-Iinclude \
		-Ilib \
		-I/usr/local/include \
		-I/usr/local/share/verilator/include \
		/workspace/$(APP_SRC) /workspace/src/*.cpp \
		-L/workspace/lib \
		-L/usr/local/lib \
		-l:DODA.so \
		-lverilated \
		-lverilated_vcd_c \
		-lpthread \
		-Wl,-rpath,../lib"
	@echo "✓ Simulation executable built: $(DEST_DIR)sim_app"

# === Compiler setup ===
COMPILER_DIR = doda_compiler
COMPILER = compiler
INIT_SUBMODULE = init_submodule

$(INIT_SUBMODULE):
	git submodule update --init --remote

$(COMPILER): $(INIT_SUBMODULE) $(COMPILER_DIR)/Makefile
	cd $(COMPILER_DIR) && make

# === Utilities ===
$(DODA_LIB):
	@echo "Error: $(DODA_LIB) not found!"
	@echo "Make sure you've built the encrypted Verilator library first."
	@echo "Check that the .so file exists in the lib/ directory."
	@exit 1

docker-build:
	$(CONTAINER_ENGINE) build -t $(DOCKER_IMAGE) ./docker

clean:
	rm -rf $(OBJECTS) $(TARGET) $(COMPILER_DIR)

# === Validation rules ===
check_app_src:
	@if [ -z "$(APP_SRC)" ]; then echo "Error: Please specify APP_SRC=your_file.cpp"; exit 1; fi

.PHONY: all run simulate docker-build clean check_app_src $(COMPILER) $(INIT_SUBMODULE)