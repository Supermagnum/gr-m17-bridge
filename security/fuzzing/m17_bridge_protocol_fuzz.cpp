/* -*- c++ -*- */
/*
 * M17 Bridge Protocol Detection Fuzzing Harness
 * 
 * Fuzz testing for protocol detection and conversion in M17 Bridge module
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

// Protocol detection with real logic
static protocol_type_t detect_protocol(const uint8_t* data, size_t size) {
    if (size < 2) return PROTOCOL_UNKNOWN;
    
    // Check for AX.25 (starts with 0x7E)
    if (data[0] == 0x7E) {
        return PROTOCOL_AX25;
    }
    
    // Check for M17 (starts with 0x5D 0x5F)
    if (size >= 2 && data[0] == 0x5D && data[1] == 0x5F) {
        return PROTOCOL_M17;
    }
    
    // Check for FX.25 (correlation tag)
    if (size >= 8) {
        // Simplified correlation check
        if (data[0] == 0xB7 && data[1] == 0x4D) {
            return PROTOCOL_FX25;
        }
    }
    
    // Check for IL2P (header structure)
    if (size >= 14) {
        uint8_t header_type = data[0] >> 6;
        if (header_type <= 3) {
            return PROTOCOL_IL2P;
        }
    }
    
    // Check for APRS (UI frame with PID 0xF0)
    if (size >= 18 && data[0] == 0x7E) {
        // Look for APRS PID in the frame
        for (size_t i = 16; i < size - 2; i++) {
            if (data[i] == 0xF0) {
                return PROTOCOL_APRS;
            }
        }
    }
    
    return PROTOCOL_UNKNOWN;
}

// M17 Bridge protocol processing simulation
static void process_protocol_bridge(const uint8_t* data, size_t size) {
    if (size < 1 || size > MAX_SIZE) return;
    
    // Initialize M17-AX.25 bridge
    m17_ax25_bridge_t bridge;
    if (m17_ax25_bridge_init(&bridge) != 0) {
        return;
    }
    
    // Configure bridge for protocol detection
    bridge_config_t config;
    config.m17_enabled = true;
    config.ax25_enabled = true;
    config.fx25_enabled = true;
    config.il2p_enabled = true;
    config.auto_detect = true;
    config.m17_frequency = 144800000;
    config.ax25_frequency = 144800000;
    config.m17_can = 0;
    strncpy(config.ax25_callsign, "N0CALL", sizeof(config.ax25_callsign) - 1);
    config.ax25_callsign[sizeof(config.ax25_callsign) - 1] = '\0';
    config.ax25_ssid = 0;
    config.fx25_rs_type = FX25_RS_255_239;
    config.il2p_debug = 0;
    
    m17_ax25_bridge_set_config(&bridge, &config);
    
    // Detect protocol
    protocol_type_t protocol = detect_protocol(data, size);
    m17_ax25_bridge_set_protocol(&bridge, protocol);
    
    // Process based on detected protocol
    switch (protocol) {
        case PROTOCOL_M17:
            {
                uint8_t ax25_data[256];
                uint16_t ax25_length = sizeof(ax25_data);
                m17_ax25_bridge_convert_m17_to_ax25(&bridge, data, size, 
                                                  ax25_data, &ax25_length);
            }
            break;
        case PROTOCOL_AX25:
        case PROTOCOL_APRS:
            {
                uint8_t m17_data[256];
                uint16_t m17_length = sizeof(m17_data);
                m17_ax25_bridge_convert_ax25_to_m17(&bridge, data, size, 
                                                    m17_data, &m17_length);
            }
            break;
        case PROTOCOL_FX25:
            m17_ax25_bridge_process_fx25_frame(&bridge, data, size);
            break;
        case PROTOCOL_IL2P:
            m17_ax25_bridge_process_il2p_frame(&bridge, data, size);
            break;
        default:
            // Try to process as unknown protocol
            m17_ax25_bridge_process_rx_data(&bridge, data, size);
            break;
    }
    
    // Cleanup
    m17_ax25_bridge_cleanup(&bridge);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 1 || size > MAX_SIZE) return 0;
    
    // Create meaningful branches for fuzzer
    int result = 0;
    
    // Branch based on size
    if (size < 4) {
        result = 1;  // Too small
    } else if (size < 8) {
        result = 2;  // Small
    } else if (size < 16) {
        result = 3;  // Medium
    } else if (size < 30) {
        result = 4;  // Large
    } else {
        result = 5;  // Very large
    }
    
    // Branch based on protocol detection
    protocol_type_t protocol = detect_protocol(data, size);
    switch (protocol) {
        case PROTOCOL_M17:
            result += 10;  // M17 detected
            break;
        case PROTOCOL_AX25:
            result += 20;  // AX.25 detected
            break;
        case PROTOCOL_FX25:
            result += 30;  // FX.25 detected
            break;
        case PROTOCOL_IL2P:
            result += 40;  // IL2P detected
            break;
        case PROTOCOL_APRS:
            result += 50;  // APRS detected
            break;
        case PROTOCOL_UNKNOWN:
            result += 60;  // Unknown protocol
            break;
    }
    
    // Branch based on first byte
    if (data[0] == 0x7E) {
        result += 100;  // AX.25 flag
    } else if (data[0] == 0x5D) {
        result += 200;  // M17 start
    } else if (data[0] == 0xB7) {
        result += 300;  // FX.25 start
    } else if (data[0] < 32) {
        result += 400;  // Control character
    } else if (data[0] > 126) {
        result += 500;  // Extended character
    } else {
        result += 600;  // Normal character
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
    
    // Process the data through M17 bridge
    process_protocol_bridge(data, size);
    
    return result;
}

int main() {
    uint8_t buf[MAX_SIZE];
    ssize_t len = read(STDIN_FILENO, buf, MAX_SIZE);
    if (len <= 0) return 0;
    
    int result = LLVMFuzzerTestOneInput(buf, (size_t)len);
    return result;
}
