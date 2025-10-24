# M17 Bridge - Protocol Conversion between M17 and AX.25

A GNU Radio module for bridging between M17 digital radio and AX.25 packet radio protocols.

## Features

### Protocol Support

- **M17 Digital Radio**: Complete M17 protocol support with audio encoding and data packets
- **AX.25 Packet Radio**: Full AX.25 support for I, S, and U frame types with KISS TNC interface
- **APRS Integration**: Position reporting and messaging support
- **FX.25 FEC**: Forward Error Correction for noisy channels
- **IL2P Protocol**: Modern replacement for AX.25 with data whitening for error correction optimization

### Bridge Capabilities

- **M17 ↔ AX.25 Conversion**: Seamless protocol translation
- **Callsign Mapping**: Automatic address translation between protocols
- **Mode Switching**: Dynamic protocol selection
- **Data Format Conversion**: Automatic payload adaptation

### Error Correction Features

- **FX.25 FEC**: Forward Error Correction for noisy channels
- **IL2P Protocol**: Modern replacement for AX.25 with data whitening for error correction optimization with improved reliability
- **Frame Validation**: Automatic frame integrity checking
- **Retry Mechanisms**: Automatic retransmission for failed frames

## Installation

### Prerequisites

- GNU Radio 3.10 or later
- CMake 3.16 or later
- C++17 compatible compiler
- Python 3.6 or later (for Python bindings)

### Installing Dependencies

#### Ubuntu/Debian

```bash
# Core dependencies
sudo apt update
sudo apt install -y \
    cmake \
    build-essential \
    pkg-config \
    libvolk2-dev \
    libgnuradio-dev \
    gnuradio-dev

# Python bindings (optional)
sudo apt install -y \
    python3-dev \
    python3-pybind11 \
    python3-numpy

# Testing framework (optional)
sudo apt install -y \
    libgtest-dev \
    libgmock-dev

# Fuzzing framework (optional)
sudo apt install -y \
    afl++
```

#### Fedora/RHEL/CentOS

```bash
# Core dependencies
sudo dnf install -y \
    cmake \
    gcc-c++ \
    pkgconfig \
    volk-devel \
    gnuradio-devel

# Python bindings (optional)
sudo dnf install -y \
    python3-devel \
    pybind11-devel \
    python3-numpy

# Testing framework (optional)
sudo dnf install -y \
    gtest-devel \
    gmock-devel

# Fuzzing framework (optional)
sudo dnf install -y \
    afl++
```

### Building from Source

```bash
git clone https://github.com/your-username/gr-m17-bridge.git
cd gr-m17-bridge
mkdir build
cd build
cmake ..
make -j$(nproc)
sudo make install
sudo ldconfig
```

### Configuration Options

```bash
cmake -DENABLE_PYTHON=ON \
      -DENABLE_GRC=ON \
      -DENABLE_EXAMPLES=ON \
      -DENABLE_DOXYGEN=ON \
      -DENABLE_TESTING=ON \
      ..
```

## Usage

### Import this package with:

#### Python
```python
import gnuradio.m17_bridge as m17_bridge
```

#### C++
```cpp
#include <gnuradio/m17_bridge/m17_to_ax25.h>
#include <gnuradio/m17_bridge/ax25_to_m17.h>
#include <gnuradio/m17_bridge/protocol_converter.h>
#include <gnuradio/m17_bridge/callsign_mapper.h>
```

### GNU Radio Companion

The module provides several blocks in the "M17 Bridge" category:

- **M17 to AX.25**: Convert M17 frames to AX.25 frames
- **AX.25 to M17**: Convert AX.25 frames to M17 frames
- **Protocol Converter**: Bidirectional conversion with advanced features
- **Callsign Mapper**: Automatic callsign translation

### Python API

```python
import gnuradio.m17_bridge as m17_bridge

# Create M17 to AX.25 converter
converter = m17_bridge.m17_to_ax25(
    callsign="N0CALL",
    destination="APRS",
    enable_fec=True
)

# Create protocol converter with FX.25 FEC
bridge = m17_bridge.protocol_converter(
    m17_callsign="N0CALL",
    m17_destination="APRS",
    ax25_callsign="N0CALL", 
    ax25_destination="APRS",
    enable_fx25=True,
    enable_il2p=False
)
```

### C++ API

```cpp
#include <gnuradio/m17_bridge/m17_to_ax25.h>
#include <gnuradio/m17_bridge/protocol_converter.h>

// Create M17 to AX.25 converter
auto converter = m17_bridge::m17_to_ax25::make(
    "N0CALL", "APRS", true);

// Create protocol converter
auto bridge = m17_bridge::protocol_converter::make(
    "N0CALL", "APRS", "N0CALL", "APRS",
    true, false);
```

## Examples

### Basic M17 to AX.25 Bridge

```bash
gnuradio-companion examples/m17_ax25_bridge.grc
```

### FX.25 FEC Bridge

```bash
gnuradio-companion examples/m17_fx25_bridge.grc
```

### IL2P Protocol Bridge

```bash
gnuradio-companion examples/m17_il2p_bridge.grc
```

## Configuration

### Callsign Mapping

The callsign mapper allows automatic translation between M17 and AX.25 callsigns:

```python
mapper = m17_bridge.callsign_mapper()

# Add manual mappings
mapper.add_mapping("N0CALL", "N0CALL")
mapper.add_mapping("W1AW", "W1AW")

# Enable auto-mapping for unknown callsigns
mapper.set_auto_mapping_enabled(True)
```

### Protocol Converter Settings

```python
converter = m17_bridge.protocol_converter(
    m17_callsign="N0CALL",
    m17_destination="APRS", 
    ax25_callsign="N0CALL",
    ax25_destination="APRS",
    enable_fx25=True,      # Enable FX.25 FEC
    enable_il2p=False     # Disable IL2P protocol
)

# Set conversion mode
converter.set_conversion_mode(m17_bridge.protocol_converter.CONVERSION_AUTO)
```

## Legal Disclaimer

**IMPORTANT: This module is for protocol conversion only and does not include encryption features.**

This module provides pure protocol conversion between M17 and AX.25 without any cryptographic capabilities. It is designed for amateur radio use and protocol interoperability.

**User Responsibility:**
- Users are entirely responsible for compliance with local laws and regulations
- Check your local regulations before using amateur radio equipment
- Ensure proper licensing for amateur radio operation
- Follow all applicable amateur radio regulations

**Regulatory Considerations:**
- Amateur radio regulations vary significantly by country
- Ensure proper licensing and operation within amateur radio bands
- Commercial use may be subject to additional regulations
- Follow all applicable frequency and power regulations

**Recommendation:** Always consult with your local amateur radio regulatory authority before operating amateur radio equipment.

## Security Testing

### Fuzzing Results

Comprehensive security testing has been performed using LibFuzzer with structure-aware protocol generation:

- **Duration**: 8-hour comprehensive fuzzing session
- **Coverage**: 527 edges discovered with plateau confirmation
- **Total Tests**: 1,294,453,414 test cases executed
- **Protocols Tested**: AX.25, KISS, FX.25, IL2P, M17
- **Security Status**: No vulnerabilities found - code demonstrates exceptional robustness

**[View Detailed Fuzzing Results](security/fuzzing/fuzzing-results.md)**

The M17 Bridge module has been thoroughly tested and shows excellent security posture with robust error handling and input validation across all protocol implementations.

## Contributing

This project is part of the M17 Foundation. For contribution guidelines and development information, see the project documentation.

## License

GPL-3.0-or-later

## Acknowledgments

- M17 Project for the original M17 protocol implementation
- GNU Radio community for the excellent framework
- Amateur radio community for testing and feedback
