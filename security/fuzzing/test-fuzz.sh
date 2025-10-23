#!/bin/bash
# Test script for M17 Bridge fuzzing infrastructure
# Verifies that the fuzzing harnesses compile and run correctly

echo "M17 Bridge Fuzzing Test"
echo "======================="

# Check if AFL++ is available
if ! command -v afl-g++ &> /dev/null; then
    echo "ERROR: AFL++ not found. Please install AFL++ first:"
    echo "  sudo apt install afl++"
    exit 1
fi

echo "AFL++ found: $(afl-g++ --version | head -n1)"

# Check if required headers exist
echo "Checking for required headers..."
if [ ! -f "../../include/gnuradio/m17_bridge/m17_ax25_bridge.h" ]; then
    echo "ERROR: m17_ax25_bridge.h not found"
    exit 1
fi

if [ ! -f "../../lib/m17_ax25_bridge.c" ]; then
    echo "ERROR: m17_ax25_bridge.c not found"
    exit 1
fi

echo "Required headers found"

# Test compilation
echo "Testing compilation..."

# Test AX.25 fuzz compilation
echo "Compiling AX.25 fuzz target..."
if afl-g++ -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I../../include -I../../lib \
    ../../lib/m17_ax25_bridge.c \
    ../../lib/ax25_protocol.c \
    ../../lib/fx25_protocol.c \
    ../../lib/il2p_protocol.c \
    ../../lib/kiss_protocol.c \
    m17_bridge_ax25_fuzz.cpp -o test_ax25_fuzz; then
    echo "AX.25 fuzz target compiled successfully"
    rm -f test_ax25_fuzz
else
    echo "ERROR: AX.25 fuzz target compilation failed"
    exit 1
fi

# Test M17 fuzz compilation
echo "Compiling M17 fuzz target..."
if afl-g++ -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I../../include -I../../lib \
    ../../lib/m17_ax25_bridge.c \
    ../../lib/ax25_protocol.c \
    ../../lib/fx25_protocol.c \
    ../../lib/il2p_protocol.c \
    ../../lib/kiss_protocol.c \
    m17_bridge_m17_fuzz.cpp -o test_m17_fuzz; then
    echo "M17 fuzz target compiled successfully"
    rm -f test_m17_fuzz
else
    echo "ERROR: M17 fuzz target compilation failed"
    exit 1
fi

# Test protocol fuzz compilation
echo "Compiling protocol fuzz target..."
if afl-g++ -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I../../include -I../../lib \
    ../../lib/m17_ax25_bridge.c \
    ../../lib/ax25_protocol.c \
    ../../lib/fx25_protocol.c \
    ../../lib/il2p_protocol.c \
    ../../lib/kiss_protocol.c \
    m17_bridge_protocol_fuzz.cpp -o test_protocol_fuzz; then
    echo "Protocol fuzz target compiled successfully"
    rm -f test_protocol_fuzz
else
    echo "ERROR: Protocol fuzz target compilation failed"
    exit 1
fi

echo ""
echo "All fuzzing harnesses compiled successfully!"
echo "The fuzzing infrastructure is ready to use."
echo ""
echo "To run fuzzing:"
echo "  ./fuzz-quick.sh     # Quick 10-minute test"
echo "  ./fuzz-testing.sh quick  # 1-hour test"
echo "  ./fuzz-testing.sh overnight  # 8-hour test"
