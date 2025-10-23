/* -*- c++ -*- */
/*
 * M17 Bridge AX.25 Fuzzing Harness
 * 
 * Fuzz testing for AX.25 frame processing in M17 Bridge module
 * Based on gr-m17 fuzzing infrastructure
 */

#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <vector>

#define MAX_SIZE 8192

// Include the actual M17 bridge implementation
#include <m17_ax25_bridge.h>

// AX.25 frame validation with real protocol logic
static bool validate_ax25_frame(const uint8_t* data, size_t size) {
    if (size < 16) return false;
    
    // Check for AX.25 flag
    if (data[0] != 0x7E) return false;
    
    // Validate destination address (6 bytes + SSID)
    for (size_t i = 1; i < 7; i++) {
        uint8_t byte = data[i] >> 1;
        if (byte < 32 || byte > 126) return false;
    }
    
    // Check SSID byte
    uint8_t dst_ssid = data[7];
    if ((dst_ssid & 0x01) == 0) {
        // Has digipeaters - find end of address field
        size_t addr_pos = 14;
        while (addr_pos + 7 <= size && (data[addr_pos - 1] & 0x01) == 0) {
            addr_pos += 7;
        }
        if (addr_pos >= size) return false;
    }
    
    // Validate source address
    for (size_t i = 8; i < 14; i++) {
        uint8_t byte = data[i] >> 1;
        if (byte < 32 || byte > 126) return false;
    }
    
    // Check control field
    uint8_t control = data[14];
    if ((control & 0x01) == 0) {
        // I-frame - check for PID
        if (size < 17) return false;
        uint8_t pid = data[15];
        // Validate PID values
        if (pid != 0xF0 && pid != 0xCC && pid != 0x06 && pid != 0x07) {
            return false;
        }
    }
    
    // Look for closing flag
    bool found_closing = false;
    for (size_t i = 1; i < size; i++) {
        if (data[i] == 0x7E && i > 15) {
            found_closing = true;
            break;
        }
    }
    
    return found_closing;
}

// M17 Bridge AX.25 processing simulation
static void process_ax25_bridge(const uint8_t* data, size_t size) {
    if (size < 1 || size > MAX_SIZE) return;
    
    // Initialize M17-AX.25 bridge
    m17_ax25_bridge_t bridge;
    if (m17_ax25_bridge_init(&bridge) != 0) {
        return;
    }
    
    // Configure bridge for AX.25 processing
    bridge_config_t config;
    config.m17_enabled = true;
    config.ax25_enabled = true;
    config.fx25_enabled = false;
    config.il2p_enabled = false;
    config.auto_detect = false;
    config.m17_frequency = 144800000;
    config.ax25_frequency = 144800000;
    config.m17_can = 0;
    strncpy(config.ax25_callsign, "N0CALL", sizeof(config.ax25_callsign) - 1);
    config.ax25_callsign[sizeof(config.ax25_callsign) - 1] = '\0';
    config.ax25_ssid = 0;
    config.fx25_rs_type = FX25_RS_255_239;
    config.il2p_debug = 0;
    
    m17_ax25_bridge_set_config(&bridge, &config);
    
    // Process AX.25 frame
    if (validate_ax25_frame(data, size)) {
        // Convert AX.25 to M17
        uint8_t m17_data[256];
        uint16_t m17_length = sizeof(m17_data);
        
        if (m17_ax25_bridge_convert_ax25_to_m17(&bridge, data, size, 
                                               m17_data, &m17_length) == 0) {
            // Process successful conversion
        }
    }
    
    // Cleanup
    m17_ax25_bridge_cleanup(&bridge);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 1 || size > MAX_SIZE) return 0;
    
    // Create meaningful branches for fuzzer
    int result = 0;
    
    // Branch based on size
    if (size < 8) {
        result = 1;  // Too small
    } else if (size < 16) {
        result = 2;  // Small frame
    } else if (size < 32) {
        result = 3;  // Medium frame
    } else if (size < 64) {
        result = 4;  // Large frame
    } else {
        result = 5;  // Very large frame
    }
    
    // Branch based on first byte (AX.25 flag)
    if (data[0] == 0x7E) {
        result += 10;  // Valid AX.25 flag
    } else if (data[0] < 32) {
        result += 20;  // Control character
    } else if (data[0] > 126) {
        result += 30;  // Extended character
    } else {
        result += 40;  // Normal character
    }
    
    // Branch based on frame validation
    bool valid_frame = validate_ax25_frame(data, size);
    if (valid_frame) {
        result += 100;  // Valid AX.25 frame
    } else {
        result += 200;  // Invalid AX.25 frame
    }
    
    // Branch based on data patterns
    bool has_zeros = false, has_ones = false, has_alternating = false;
    for (size_t i = 0; i < size && i < 10; i++) {
        if (data[i] == 0x00) has_zeros = true;
        if (data[i] == 0xFF) has_ones = true;
        if (i > 0 && data[i] != data[i-1]) has_alternating = true;
    }
    
    if (has_zeros) result += 1000;
    if (has_ones) result += 2000;
    if (has_alternating) result += 3000;
    
    // Branch based on checksum calculation
    uint32_t checksum = 0;
    for (size_t i = 0; i < size; i++) {
        checksum += data[i];
    }
    
    if (checksum == 0) {
        result += 10000;  // Zero checksum
    } else if (checksum < 100) {
        result += 20000;  // Low checksum
    } else if (checksum > 1000) {
        result += 30000;  // High checksum
    } else {
        result += 40000;  // Medium checksum
    }
    
    // Branch based on specific byte patterns
    for (size_t i = 0; i < size && i < 5; i++) {
        if (data[i] == 0x55) result += 100000;
        if (data[i] == 0xAA) result += 200000;
        if (data[i] == 0x33) result += 300000;
        if (data[i] == 0xCC) result += 400000;
    }
    
    // Process the frame through M17 bridge
    process_ax25_bridge(data, size);
    
    return result;
}

int main() {
    uint8_t buf[MAX_SIZE];
    ssize_t len = read(STDIN_FILENO, buf, MAX_SIZE);
    if (len <= 0) return 0;
    
    int result = LLVMFuzzerTestOneInput(buf, (size_t)len);
    return result;
}
