# Makefile

CXX = g++
CXXFLAGS = -std=c++14 -Iobj_dir -I$(shell verilator --getenv VERILATOR_ROOT)/include -I$(shell verilator --getenv VERILATOR_ROOT)/include/vltstd -DVL_PROTECTED -fPIC
LDFLAGS = /usr/share/verilator/lib64/libverilated.a /usr/share/verilator/lib64/libverilated_vcd_c.a -lpthread
SOURCES = sim_main.cpp mapper.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = doda_sim

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJECTS) obj_dir/doda.so
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
.PHONY: all clean
