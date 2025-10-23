#!/bin/bash
# M17 Bridge Fuzzing Framework
# Comprehensive fuzz testing for M17 Bridge module

set -e

echo "M17 BRIDGE FUZZING FRAMEWORK"
echo "============================"

# Parse command line arguments
FUZZ_MODE=${1:-quick}  # ultra-quick, quick, overnight, thorough, continuous

case $FUZZ_MODE in
    ultra-quick)
        TIMEOUT=600         # 10 minutes
        echo "Mode: Ultra-Quick (10 minutes)"
        ;;
    quick)
        TIMEOUT=3600        # 1 hour
        echo "Mode: Quick (1 hour)"
        ;;
    6hour)
        TIMEOUT=21600       # 6 hours
        echo "Mode: 6-Hour (6 hours)"
        ;;
    overnight)
        TIMEOUT=28800       # 8 hours
        echo "Mode: Overnight (8 hours)"
        ;;
    thorough)
        TIMEOUT=86400       # 24 hours
        echo "Mode: Thorough (24 hours)"
        ;;
    weekend)
        TIMEOUT=259200      # 72 hours
        echo "Mode: Weekend (72 hours)"
        ;;
    continuous)
        TIMEOUT=0           # No timeout
        echo "Mode: Continuous (runs until stopped)"
        ;;
    *)
        echo "Usage: $0 {ultra-quick|quick|6hour|overnight|thorough|weekend|continuous}"
        echo "  ultra-quick: 10 minutes"
        echo "  quick: 1 hour"
        echo "  6hour: 6 hours"
        echo "  overnight: 8 hours"
        echo "  thorough: 24 hours"
        echo "  weekend: 72 hours"
        echo "  continuous: runs until stopped"
        exit 1
        ;;
esac

# Create fuzzing directory
FUZZ_DIR="security/fuzzing/reports/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$FUZZ_DIR"
cd "$FUZZ_DIR"

echo "Setting up AFL++ fuzzing..."
echo "Working directory: $FUZZ_DIR"

# Check if AFL++ is available
if ! command -v afl-g++ &> /dev/null; then
    echo "ERROR: AFL++ not found. Installing AFL++..."
    echo "Please install AFL++ first:"
    echo "  sudo apt install afl++"
    echo "  or build from source: https://github.com/AFLplusplus/AFLplusplus"
    exit 1
fi

# Compile with security features enabled
COMPILE_FLAGS="-g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra"

echo "Creating fuzz targets..."

# Create corpus directory
mkdir -p corpus

# Create test corpus for AX.25
echo "Creating AX.25 test corpus..."
mkdir -p corpus/ax25

# Basic AX.25 frames
cat > corpus/ax25/basic_ui_frame << 'EOF'
7E 82 A0 A4 A6 40 40 E0 96 8E 96 8E 40 40 03 F0 48 65 6C 6C 6F 7E
EOF

cat > corpus/ax25/aprs_position << 'EOF'
7E 82 A0 A4 A6 40 40 E0 96 8E 96 8E 40 40 03 F0 21 33 35 2E 35 30 4E 2F 30 37 38 2E 33 30 57 2D 4D 31 37 20 42 72 69 64 67 65 7E
EOF

cat > corpus/ax25/i_frame << 'EOF'
7E 82 A0 A4 A6 40 40 E0 96 8E 96 8E 40 40 00 00 48 65 6C 6C 6F 20 57 6F 72 6C 64 7E
EOF

# Create test corpus for M17
echo "Creating M17 test corpus..."
mkdir -p corpus/m17

# M17 LSF frame
cat > corpus/m17/lsf_frame << 'EOF'
5D 5F 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
EOF

# M17 packet frame
cat > corpus/m17/packet_frame << 'EOF'
5D 5F 02 48 65 6C 6C 6F 20 57 6F 72 6C 64 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
EOF

# M17 stream frame
cat > corpus/m17/stream_frame << 'EOF'
5D 5F 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00
EOF

# Create test corpus for FX.25
echo "Creating FX.25 test corpus..."
mkdir -p corpus/fx25

# FX.25 frame with correlation tag
cat > corpus/fx25/basic_frame << 'EOF'
B7 4D B7 DF 8A 53 2F 3E 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 7E 82 A0 A4 A6 40 40 E0 96 8E 96 8E 40 40 03 F0 48 65 6C 6C 6F 7E
EOF

# Create test corpus for IL2P
echo "Creating IL2P test corpus..."
mkdir -p corpus/il2p

# IL2P frame
cat > corpus/il2p/basic_frame << 'EOF'
55 F1 5E 48 00 00 00 00 00 00 00 00 00 00 00 48 65 6C 6C 6F 20 57 6F 72 6C 64
EOF

# Convert hex files to binary
echo "Converting corpus to binary format..."
for dir in corpus/*; do
    for file in "$dir"/*; do
        if [ -f "$file" ]; then
            xxd -r -p "$file" > "${file}.bin" 2>/dev/null || true
        fi
    done
done

# Compile fuzz targets
echo "Compiling fuzz targets..."

# AX.25 fuzz target
echo "Compiling AX.25 fuzz target..."
afl-g++ $COMPILE_FLAGS -o m17_bridge_ax25_fuzz \
    -I../../include \
    -I../../lib \
    ../../lib/m17_ax25_bridge.c \
    ../../lib/ax25_protocol.c \
    ../../lib/fx25_protocol.c \
    ../../lib/il2p_protocol.c \
    ../../lib/kiss_protocol.c \
    m17_bridge_ax25_fuzz.cpp

# M17 fuzz target
echo "Compiling M17 fuzz target..."
afl-g++ $COMPILE_FLAGS -o m17_bridge_m17_fuzz \
    -I../../include \
    -I../../lib \
    ../../lib/m17_ax25_bridge.c \
    ../../lib/ax25_protocol.c \
    ../../lib/fx25_protocol.c \
    ../../lib/il2p_protocol.c \
    ../../lib/kiss_protocol.c \
    m17_bridge_m17_fuzz.cpp

# Protocol detection fuzz target
echo "Compiling protocol detection fuzz target..."
afl-g++ $COMPILE_FLAGS -o m17_bridge_protocol_fuzz \
    -I../../include \
    -I../../lib \
    ../../lib/m17_ax25_bridge.c \
    ../../lib/ax25_protocol.c \
    ../../lib/fx25_protocol.c \
    ../../lib/il2p_protocol.c \
    ../../lib/kiss_protocol.c \
    m17_bridge_protocol_fuzz.cpp

# Set up AFL++ environment
export AFL_SKIP_CPUFREQ=1
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1

# Create output directories
mkdir -p output/ax25
mkdir -p output/m17
mkdir -p output/protocol

# Run fuzzing
echo "Starting AFL++ fuzzing..."

# AX.25 fuzzing
echo "Running AX.25 fuzzing..."
if [ $TIMEOUT -gt 0 ]; then
    timeout $TIMEOUT afl-fuzz -i corpus/ax25 -o output/ax25 -t 1000+ ./m17_bridge_ax25_fuzz @@ &
    AX25_PID=$!
else
    afl-fuzz -i corpus/ax25 -o output/ax25 -t 1000+ ./m17_bridge_ax25_fuzz @@ &
    AX25_PID=$!
fi

# M17 fuzzing
echo "Running M17 fuzzing..."
if [ $TIMEOUT -gt 0 ]; then
    timeout $TIMEOUT afl-fuzz -i corpus/m17 -o output/m17 -t 1000+ ./m17_bridge_m17_fuzz @@ &
    M17_PID=$!
else
    afl-fuzz -i corpus/m17 -o output/m17 -t 1000+ ./m17_bridge_m17_fuzz @@ &
    M17_PID=$!
fi

# Protocol detection fuzzing
echo "Running protocol detection fuzzing..."
if [ $TIMEOUT -gt 0 ]; then
    timeout $TIMEOUT afl-fuzz -i corpus -o output/protocol -t 1000+ ./m17_bridge_protocol_fuzz @@ &
    PROTOCOL_PID=$!
else
    afl-fuzz -i corpus -o output/protocol -t 1000+ ./m17_bridge_protocol_fuzz @@ &
    PROTOCOL_PID=$!
fi

# Wait for fuzzing to complete
echo "Fuzzing in progress..."
echo "AX.25 fuzzer PID: $AX25_PID"
echo "M17 fuzzer PID: $M17_PID"
echo "Protocol fuzzer PID: $PROTOCOL_PID"

if [ $TIMEOUT -gt 0 ]; then
    echo "Fuzzing will run for $TIMEOUT seconds"
    sleep $TIMEOUT
    echo "Timeout reached, stopping fuzzing..."
    
    # Kill fuzzing processes
    kill $AX25_PID 2>/dev/null || true
    kill $M17_PID 2>/dev/null || true
    kill $PROTOCOL_PID 2>/dev/null || true
else
    echo "Fuzzing will run continuously until stopped (Ctrl+C)"
    wait
fi

# Generate reports
echo "Generating fuzzing reports..."

# AX.25 report
if [ -d "output/ax25/default" ]; then
    echo "AX.25 Fuzzing Results:" > ax25_report.txt
    echo "=====================" >> ax25_report.txt
    echo "Crashes found: $(find output/ax25/default/crashes -name "id:*" 2>/dev/null | wc -l)" >> ax25_report.txt
    echo "Hangs found: $(find output/ax25/default/hangs -name "id:*" 2>/dev/null | wc -l)" >> ax25_report.txt
    echo "Total executions: $(grep 'execs_done' output/ax25/default/fuzzer_stats 2>/dev/null | cut -d: -f2 | tr -d ' ' || echo 'N/A')" >> ax25_report.txt
fi

# M17 report
if [ -d "output/m17/default" ]; then
    echo "M17 Fuzzing Results:" > m17_report.txt
    echo "===================" >> m17_report.txt
    echo "Crashes found: $(find output/m17/default/crashes -name "id:*" 2>/dev/null | wc -l)" >> m17_report.txt
    echo "Hangs found: $(find output/m17/default/hangs -name "id:*" 2>/dev/null | wc -l)" >> m17_report.txt
    echo "Total executions: $(grep 'execs_done' output/m17/default/fuzzer_stats 2>/dev/null | cut -d: -f2 | tr -d ' ' || echo 'N/A')" >> m17_report.txt
fi

# Protocol report
if [ -d "output/protocol/default" ]; then
    echo "Protocol Detection Fuzzing Results:" > protocol_report.txt
    echo "==================================" >> protocol_report.txt
    echo "Crashes found: $(find output/protocol/default/crashes -name "id:*" 2>/dev/null | wc -l)" >> protocol_report.txt
    echo "Hangs found: $(find output/protocol/default/hangs -name "id:*" 2>/dev/null | wc -l)" >> protocol_report.txt
    echo "Total executions: $(grep 'execs_done' output/protocol/default/fuzzer_stats 2>/dev/null | cut -d: -f2 | tr -d ' ' || echo 'N/A')" >> protocol_report.txt
fi

echo "Fuzzing completed!"
echo "Results saved in: $FUZZ_DIR"
echo ""
echo "To view results:"
echo "  - AX.25: cat $FUZZ_DIR/ax25_report.txt"
echo "  - M17: cat $FUZZ_DIR/m17_report.txt"
echo "  - Protocol: cat $FUZZ_DIR/protocol_report.txt"
echo ""
echo "To analyze crashes:"
echo "  - AX.25: ls $FUZZ_DIR/output/ax25/default/crashes/"
echo "  - M17: ls $FUZZ_DIR/output/m17/default/crashes/"
echo "  - Protocol: ls $FUZZ_DIR/output/protocol/default/crashes/"
