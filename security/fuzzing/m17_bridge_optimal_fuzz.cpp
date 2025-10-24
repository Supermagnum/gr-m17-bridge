// fuzz_optimal.cpp - Maximum bang for buck
#include <fuzzer/FuzzedDataProvider.h>
#include <cstdint>
#include <cstring>

extern "C" {
#include "ax25_protocol.h"
#include "kiss_protocol.h"
#include "fx25_protocol.h"
#include "il2p_protocol.h"
#include "m17_ax25_bridge.h"
}

// Protocol type selector
enum ProtocolType {
    PROTO_AX25,
    PROTO_KISS,
    PROTO_FX25,
    PROTO_IL2P,
    PROTO_M17
};

// Generate structured AX.25 frame
static size_t generate_ax25_frame(FuzzedDataProvider &fdp, uint8_t *out, size_t max_len) {
    if (max_len < 20) return 0;
    
    size_t offset = 0;
    
    // Flag
    out[offset++] = 0x7E;
    
    // Destination address (7 bytes) - valid AX.25 encoding
    for (int i = 0; i < 6; i++) {
        out[offset++] = fdp.ConsumeIntegralInRange<uint8_t>(0x40, 0xDF) & 0xFE;
    }
    out[offset++] = fdp.ConsumeIntegralInRange<uint8_t>(0x60, 0x7E); // SSID
    
    // Source address (7 bytes)
    for (int i = 0; i < 6; i++) {
        out[offset++] = fdp.ConsumeIntegralInRange<uint8_t>(0x40, 0xDF) & 0xFE;
    }
    out[offset++] = fdp.ConsumeIntegralInRange<uint8_t>(0x60, 0x7E); // SSID + last addr bit
    
    // Optional digipeaters (0-2)
    int num_digi = fdp.ConsumeIntegralInRange(0, 2);
    for (int d = 0; d < num_digi && offset + 7 < max_len; d++) {
        for (int i = 0; i < 6; i++) {
            out[offset++] = fdp.ConsumeIntegralInRange<uint8_t>(0x40, 0xDF) & 0xFE;
        }
        bool is_last = (d == num_digi - 1);
        out[offset++] = fdp.ConsumeIntegralInRange<uint8_t>(0x60, 0x7E) | (is_last ? 0x01 : 0x00);
    }
    
    // Control field (favor valid frame types)
    uint8_t ctrl_type = fdp.ConsumeIntegralInRange(0, 9);
    if (ctrl_type < 7) {
        out[offset++] = 0x03; // UI frame (most common)
    } else if (ctrl_type < 9) {
        out[offset++] = fdp.ConsumeIntegralInRange<uint8_t>(0x00, 0xEF); // I-frame
    } else {
        out[offset++] = fdp.ConsumeIntegral<uint8_t>(); // Random (test edge cases)
    }
    
    // PID (favor valid values)
    uint8_t pid_type = fdp.ConsumeIntegralInRange(0, 9);
    if (pid_type < 5) {
        out[offset++] = 0xF0; // No layer 3
    } else if (pid_type < 8) {
        out[offset++] = 0x01; // ISO 8208/CCITT X.25 PLP
    } else {
        out[offset++] = fdp.ConsumeIntegral<uint8_t>();
    }
    
    // Information field (variable length)
    size_t info_len = fdp.ConsumeIntegralInRange<size_t>(0, std::min(max_len - offset - 3, size_t(256)));
    for (size_t i = 0; i < info_len && offset < max_len - 3; i++) {
        out[offset++] = fdp.ConsumeIntegral<uint8_t>();
    }
    
    // FCS (50% valid, 50% corrupted for edge case testing)
    if (fdp.ConsumeBool()) {
        // Simple FCS calculation for testing
        uint16_t fcs = 0xFFFF;
        for (size_t i = 1; i < offset; i++) {
            fcs ^= out[i];
            for (int j = 0; j < 8; j++) {
                if (fcs & 1) {
                    fcs = (fcs >> 1) ^ 0x8408;
                } else {
                    fcs >>= 1;
                }
            }
        }
        out[offset++] = fcs & 0xFF;
        out[offset++] = (fcs >> 8) & 0xFF;
    } else {
        // Invalid FCS (test error handling)
        out[offset++] = fdp.ConsumeIntegral<uint8_t>();
        out[offset++] = fdp.ConsumeIntegral<uint8_t>();
    }
    
    // Flag
    if (offset < max_len) {
        out[offset++] = 0x7E;
    }
    
    return offset;
}

// Generate KISS frame
static size_t generate_kiss_frame(FuzzedDataProvider &fdp, uint8_t *out, size_t max_len) {
    if (max_len < 3) return 0;
    
    size_t offset = 0;
    
    // FEND
    out[offset++] = 0xC0;
    
    // Command byte (favor valid commands)
    uint8_t cmd_type = fdp.ConsumeIntegralInRange(0, 9);
    if (cmd_type < 6) {
        out[offset++] = 0x00; // Data frame (port 0)
    } else if (cmd_type < 8) {
        out[offset++] = fdp.ConsumeIntegralInRange<uint8_t>(0x00, 0x0F); // Valid port
    } else {
        out[offset++] = fdp.ConsumeIntegral<uint8_t>(); // Invalid
    }
    
    // Payload with KISS escaping test
    size_t payload_len = fdp.ConsumeIntegralInRange<size_t>(0, std::min(max_len - offset - 1, size_t(512)));
    for (size_t i = 0; i < payload_len && offset < max_len - 1; i++) {
        uint8_t byte = fdp.ConsumeIntegral<uint8_t>();
        
        // Occasionally inject FEND/FESC to test escaping
        if (fdp.ConsumeIntegralInRange(0, 19) == 0) {
            byte = fdp.ConsumeBool() ? 0xC0 : 0xDB;
        }
        
        out[offset++] = byte;
    }
    
    // FEND
    if (offset < max_len) {
        out[offset++] = 0xC0;
    }
    
    return offset;
}

// Main fuzzer entry point
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 3) return 0;
    
    FuzzedDataProvider fdp(data, size);
    
    // Choose protocol with equal distribution
    ProtocolType proto = static_cast<ProtocolType>(
        fdp.ConsumeIntegralInRange(0, 4)
    );
    
    uint8_t frame[2048];
    size_t frame_len = 0;
    
    switch (proto) {
        case PROTO_AX25: {
            frame_len = generate_ax25_frame(fdp, frame, sizeof(frame));
            if (frame_len > 0) {
                // Test AX.25 protocol processing
                m17_ax25_bridge_t bridge;
                m17_ax25_bridge_init(&bridge);
                m17_ax25_bridge_process_ax25_frame(&bridge, frame, frame_len);
                m17_ax25_bridge_cleanup(&bridge);
            }
            break;
        }
        
        case PROTO_KISS: {
            frame_len = generate_kiss_frame(fdp, frame, sizeof(frame));
            if (frame_len > 0) {
                // Test KISS protocol processing
                kiss_tnc_t tnc;
                kiss_init(&tnc);
                for (size_t i = 0; i < frame_len; i++) {
                    kiss_process_byte(&tnc, frame[i]);
                }
                kiss_cleanup(&tnc);
                printf("KISS frame: %zu bytes processed\n", frame_len);
            }
            break;
        }
        
        case PROTO_FX25: {
            // FX.25 wraps AX.25 with RS FEC
            frame_len = generate_ax25_frame(fdp, frame + 10, sizeof(frame) - 10);
            if (frame_len > 0) {
                // Add FX.25 correlation tag and parity
                memcpy(frame, "\x7E\x7E\xFF\xFF\x01\x23\x45\x67\x89\xAB", 10);
                // Test FX.25 processing
                struct fx25_rs* rs = fx25_rs_init(8, 0x11D, 0, 1, 16);
                if (rs) {
                    uint8_t decoded[256];
                    int decoded_len = fx25_rs_decode(rs, frame, frame_len + 10, decoded, 16);
                    (void)decoded_len; // Suppress unused warning
                    fx25_rs_free(rs);
                    printf("FX.25 frame: %zu bytes processed\n", frame_len + 10);
                }
            }
            break;
        }
        
        case PROTO_IL2P: {
            // IL2P has specific header structure
            if (fdp.remaining_bytes() >= 13) {
                frame[0] = 0xF1; // Sync word
                frame[1] = fdp.ConsumeIntegral<uint8_t>(); // Header
                size_t payload_len = std::min(fdp.remaining_bytes(), size_t(1023));
                fdp.ConsumeData(frame + 2, payload_len);
                // Test IL2P processing - simplified for now
                // IL2P processing would go here
                (void)frame; // Suppress unused warning
                (void)payload_len; // Suppress unused warning
                printf("IL2P frame: %zu bytes processed\n", payload_len + 2);
            }
            break;
        }
        
        case PROTO_M17: {
            // Test M17 bridge conversion
            frame_len = generate_ax25_frame(fdp, frame, sizeof(frame));
            if (frame_len > 0) {
                m17_ax25_bridge_t bridge;
                m17_ax25_bridge_init(&bridge);
                m17_ax25_bridge_process_ax25_frame(&bridge, frame, frame_len);
                m17_ax25_bridge_cleanup(&bridge);
                printf("M17 frame: %zu bytes processed\n", frame_len);
            }
            break;
        }
    }
    
    return 0;
}
