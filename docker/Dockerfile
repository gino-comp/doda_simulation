# Use fully qualified Ubuntu image for Podman compatibility
FROM docker.io/ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Update package lists and install dependencies
RUN apt-get update && apt-get install -y \
    git \
    build-essential \
    autoconf \
    automake \
    autotools-dev \
    curl \
    perl \
    python3 \
    python3-pip \
    make \
    g++ \
    flex \
    bison \
    ccache \
    cmake \
    universal-ctags \
    pkg-config \
    libgoogle-perftools-dev \
    numactl \
    perl-doc \
    help2man \
    && rm -rf /var/lib/apt/lists/*

# Create a non-root user for building
RUN useradd -m -s /bin/bash builder && \
    mkdir -p /home/builder/build && \
    chown -R builder:builder /home/builder

# Switch to non-root user for building
USER builder
WORKDIR /home/builder/build

# Clone and build Verilator as non-root user
RUN git clone https://github.com/verilator/verilator.git && \
    cd verilator && \
    git checkout v5.020 && \
    autoconf && \
    ./configure --prefix=/usr/local && \
    make -j2

# Switch back to root for installation
USER root
WORKDIR /home/builder/build/verilator

# Install Verilator properly
RUN make install

# Set working directory for user code
WORKDIR /workspace

# Create directory for encrypted libraries
RUN mkdir -p /workspace/encrypted

# Set environment variables properly - EXPLICITLY UNSET VERILATOR_ROOT!
ENV PATH=/usr/local/bin:$PATH
ENV LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
# Make sure VERILATOR_ROOT is not set at all

# Verify the installation and copy missing files if needed
RUN echo "Checking Verilator installation..." && \
    which verilator && \
    ls -la /usr/local/bin/verilator*

# Build the required libraries
WORKDIR /home/builder/build/verilator/include
RUN echo "Building essential Verilator libraries..." && \
    g++ -fPIC -c verilated.cpp && \
    g++ -fPIC -c verilated_threads.cpp && \
    ar rcs /usr/local/lib/libverilated.a verilated.o verilated_threads.o && \
    echo "Compiling verilated_vcd_c.cpp..." && \
    g++ -fPIC -c verilated_vcd_c.cpp && \
    ar rcs /usr/local/lib/libverilated_vcd_c.a verilated_vcd_c.o

# Create shared libraries
RUN echo "Creating shared libraries..." && \
    g++ -shared -fPIC -o /usr/local/lib/libverilated.so verilated.o verilated_threads.o && \
    g++ -shared -fPIC -o /usr/local/lib/libverilated_vcd_c.so verilated_vcd_c.o

# Ensure include files are accessible
RUN mkdir -p /usr/local/share/verilator/include && \
    cp /home/builder/build/verilator/include/*.h /usr/local/share/verilator/include/ && \
    cp /home/builder/build/verilator/include/*.sv /usr/local/share/verilator/include/ 2>/dev/null || true

# Run ldconfig to refresh library cache
RUN ldconfig

# Clean up build directory
RUN rm -rf /home/builder/build/verilator

# Set working directory for user code
WORKDIR /workspace

# Verify installation
RUN echo "Verifying Verilator installation..." && \
    verilator --version && \
    ls -la /usr/local/lib/libverilated* && \
    echo "Installation complete!"

# Default command
CMD ["/bin/bash"]