# Docker configuration
DOCKER_IMAGE := encrypted-verilator:latest
CONTAINER_ENGINE := $(shell which podman >/dev/null 2>&1 && echo "podman" || echo "docker")

# Docker run command template
DOCKER_RUN = $(CONTAINER_ENGINE) run --rm \
	-v $(PWD):/workspace \
	-w /workspace \
	$(DOCKER_IMAGE) \
	bash

CXX = g++
CXXFLAGS = -std=c++14 -Ilib -I$(shell verilator --getenv VERILATOR_ROOT)/include -I$(shell verilator --getenv VERILATOR_ROOT)/include/vltstd -DVL_PROTECTED -fPIC
LDFLAGS = /usr/share/verilator/lib64/libverilated.a /usr/share/verilator/lib64/libverilated_vcd_c.a -lpthread
SOURCES = sim_main.cpp mapper.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = doda_sim
# Compiler and submodule initialization
COMPILER = compiler
INIT_SUBMODULE = init_submodule

# Specify the path to the encrypted shared library
DODA_LIB = lib/DODA.so

# Build target using docker
all: $(INIT_SUBMODULE) $(COMPILER) $(TARGET)

run: $(TARGET)
	./$(TARGET)

# Build the target executable using Docker
$(TARGET): $(OBJECTS) $(DODA_LIB)
	$(DOCKER_RUN) -c "g++ -std=c++14 -fPIC -o $(TARGET) \
		-Ilib \
		-I/usr/local/include \
		-I/usr/local/share/verilator/include \
		$(OBJECTS) $(DODA_LIB) \
		-L/usr/local/lib \
		-lverilated \
		-lverilated_vcd_c \
		-lpthread"

# Compile object files using Docker
%.o: %.cpp
	$(DOCKER_RUN) -c "g++ -std=c++14 -fPIC -c \
		-Ilib \
		-I/usr/local/include \
		-I/usr/local/share/verilator/include \
		$< -o $@"

# Rule to ensure the doda.so library exists
$(DODA_LIB):
	@echo "Error: $(DODA_LIB) not found!"
	@echo "Make sure you've built the encrypted Verilator library first."
	@echo "Check that the .so file exists in the lib/ directory."
	@exit 1


# Also build the Docker image if needed
docker-build:
	$(CONTAINER_ENGINE) build -t $(DOCKER_IMAGE) ./docker


########################################################################################
# Building the compiler
COMPILER_DIR = doda_compiler

$(INIT_SUBMODULE):
	git submodule update --init --recursive

$(COMPILER): $(COMPILER_DIR)/Makefile
	cd $(COMPILER_DIR) && make

clean:
	rm -rf $(OBJECTS) $(TARGET) $(COMPILER_DIR)

.PHONY: all clean run docker-build init_submodule compiler
