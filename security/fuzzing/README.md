# M17 Bridge Fuzzing Infrastructure

This directory contains comprehensive fuzzing harnesses for the M17 Bridge module, adapted from the gr-m17 fuzzing infrastructure.

## Overview

The fuzzing framework tests the M17 Bridge module's protocol conversion capabilities (without encryption features), including:

- **AX.25 Frame Processing**: Tests AX.25 frame parsing and validation
- **M17 Frame Processing**: Tests M17 frame parsing and validation  
- **Protocol Detection**: Tests automatic protocol detection and conversion
- **Bridge Operations**: Tests the core M17-AX.25 bridge functionality

## Fuzzing Harnesses

### 1. AX.25 Fuzzing (`m17_bridge_ax25_fuzz.cpp`)
- Tests AX.25 frame validation and parsing
- Validates AX.25 to M17 conversion
- Tests various AX.25 frame types (I, S, U frames)
- Tests APRS frame processing

### 2. M17 Fuzzing (`m17_bridge_m17_fuzz.cpp`)
- Tests M17 frame validation and parsing
- Validates M17 to AX.25 conversion
- Tests various M17 frame types (LSF, Stream, Packet, BERT)
- Tests M17 sync word detection

### 3. Protocol Detection Fuzzing (`m17_bridge_protocol_fuzz.cpp`)
- Tests automatic protocol detection
- Tests protocol switching and conversion
- Tests FX.25 and IL2P protocol handling
- Tests mixed protocol scenarios

## Usage

### Quick Test (10 minutes)
```bash
./fuzz-quick.sh
```

### Full Fuzzing Sessions
```bash
# Quick (1 hour)
./fuzz-testing.sh quick

# Overnight (8 hours)
./fuzz-testing.sh overnight

# Thorough (24 hours)
./fuzz-testing.sh thorough

# Continuous (until stopped)
./fuzz-testing.sh continuous
```

## Prerequisites

### Required Software
- **AFL++**: Advanced fuzzing framework
  ```bash
  sudo apt install afl++
  ```

### Required Headers
The fuzzing harnesses require the M17 bridge implementation headers:
- `m17_ax25_bridge.h`
- `ax25_protocol.h`
- `fx25_protocol.h`
- `il2p_protocol.h`
- `kiss_protocol.h`

## Test Corpus

The fuzzing framework includes comprehensive test corpora:

### AX.25 Corpus
- Basic UI frames
- APRS position reports
- I-frames with data
- Various frame types and sizes

### M17 Corpus
- LSF (Link Setup Frame) frames
- Packet frames with data
- Stream frames
- BERT (Bit Error Rate Test) frames

### FX.25 Corpus
- Frames with correlation tags
- Reed-Solomon encoded data
- Various RS parity configurations

### IL2P Corpus
- Modern protocol frames
- Header structures
- Payload data

## Output

Fuzzing results are saved in timestamped directories:
```
security/fuzzing/reports/YYYYMMDD_HHMMSS/
├── output/
│   ├── ax25/          # AX.25 fuzzing results
│   ├── m17/           # M17 fuzzing results
│   └── protocol/      # Protocol detection results
├── ax25_report.txt    # AX.25 fuzzing summary
├── m17_report.txt     # M17 fuzzing summary
└── protocol_report.txt # Protocol detection summary
```

## Analysis

### Crash Analysis
Crashes are saved in the `crashes/` directories:
```bash
# View crash files
ls output/ax25/default/crashes/
ls output/m17/default/crashes/
ls output/protocol/default/crashes/
```

### Reproducing Crashes
```bash
# Reproduce a specific crash
./m17_bridge_ax25_fuzz < output/ax25/default/crashes/id:000000,sig:11,src:000000,op:flip1,pos:0
```

### Performance Analysis
Check the `fuzzer_stats` files for performance metrics:
```bash
cat output/ax25/default/fuzzer_stats
```

## Security Features

The fuzzing harnesses include comprehensive security testing for protocol conversion:

- **Address Sanitizer**: Detects memory errors in protocol processing
- **Undefined Behavior Sanitizer**: Detects undefined behavior in frame parsing
- **Stack Protection**: Prevents stack overflow attacks during conversion
- **Buffer Overflow Detection**: Detects buffer overruns in frame handling

**Note**: This module does not include encryption features - it focuses purely on protocol conversion between M17 and AX.25.

## Integration with CI/CD

The fuzzing framework can be integrated into continuous integration:

```bash
# Run in CI environment
./fuzz-testing.sh ultra-quick
if [ $? -eq 0 ]; then
    echo "Fuzzing passed"
else
    echo "Fuzzing found issues"
    exit 1
fi
```

## Troubleshooting

### Common Issues

1. **AFL++ not found**: Install AFL++ package
2. **Compilation errors**: Check include paths and dependencies
3. **Permission denied**: Ensure scripts are executable
4. **Timeout issues**: Adjust timeout values in scripts

### Debug Mode

For debugging fuzzing issues, compile with debug flags:
```bash
afl-g++ -g -O0 -DDEBUG -fsanitize=address,undefined \
    -I../../include -I../../lib \
    ../../lib/m17_ax25_bridge.c \
    m17_bridge_ax25_fuzz.cpp -o m17_bridge_ax25_fuzz
```

## Contributing

When adding new fuzzing harnesses:

1. Follow the existing naming convention
2. Include comprehensive test corpora
3. Add meaningful branch conditions
4. Update the main fuzzing script
5. Document new features

## References

- [AFL++ Documentation](https://github.com/AFLplusplus/AFLplusplus)
- [GNU Radio Fuzzing Guidelines](https://wiki.gnuradio.org/index.php/Fuzzing)
- [M17 Protocol Specification](https://spec.m17project.org/)
- [AX.25 Protocol Specification](https://www.tapr.org/pdf/AX25.2.2.pdf)
