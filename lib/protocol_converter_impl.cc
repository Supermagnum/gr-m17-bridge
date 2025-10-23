/* -*- c++ -*- */
/*
 * Copyright 2024 M17 Bridge Project
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "protocol_converter_impl.h"

#include <algorithm>
#include <cstring>
#include <gnuradio/io_signature.h>
#include <gnuradio/math.h>
#include <iostream>
#include <m17_ax25_bridge.h>
#include <volk/volk.h>

namespace gr {
namespace m17_bridge {

protocol_converter::sptr protocol_converter::make(const std::string& m17_callsign,
                                                  const std::string& m17_destination,
                                                  const std::string& ax25_callsign,
                                                  const std::string& ax25_destination,
                                                  bool enable_fx25, bool enable_il2p) {
    return gnuradio::make_block_sptr<protocol_converter_impl>(
        m17_callsign, m17_destination, ax25_callsign, ax25_destination, enable_fx25, enable_il2p);
}

protocol_converter_impl::protocol_converter_impl(const std::string& m17_callsign,
                                                 const std::string& m17_destination,
                                                 const std::string& ax25_callsign,
                                                 const std::string& ax25_destination,
                                                 bool enable_fx25, bool enable_il2p)
    : gr::sync_block("protocol_converter", gr::io_signature::make(2, 2, sizeof(uint8_t)),
                     gr::io_signature::make(2, 2, sizeof(uint8_t))),
      d_m17_callsign(m17_callsign), d_m17_destination(m17_destination),
      d_ax25_callsign(ax25_callsign), d_ax25_destination(ax25_destination),
      d_enable_fx25(enable_fx25), d_enable_il2p(enable_il2p), d_conversion_mode(CONVERSION_AUTO),
      d_frame_counter(0), d_error_count(0) {
    // Set up message ports for control
    message_port_register_in(pmt::mp("control"));
    set_msg_handler(pmt::mp("control"), [this](pmt::pmt_t msg) { handle_control_message(msg); });

    // Initialize protocol handlers
    initialize_protocol_handlers();
}

protocol_converter_impl::~protocol_converter_impl() {}

void protocol_converter_impl::initialize_protocol_handlers() {
    // Initialize M17 to AX.25 converter
    d_m17_to_ax25 = m17_to_ax25::make(d_m17_callsign, d_ax25_destination, d_enable_fx25);

    // Initialize AX.25 to M17 converter
    d_ax25_to_m17 = ax25_to_m17::make(d_ax25_callsign, d_m17_destination, d_enable_fx25);

    // Initialize callsign mapper
    d_callsign_mapper = callsign_mapper::make();
}

int protocol_converter_impl::work(int noutput_items, gr_vector_const_void_star& input_items,
                                  gr_vector_void_star& output_items) {
    const uint8_t* m17_in = (const uint8_t*)input_items[0];
    const uint8_t* ax25_in = (const uint8_t*)input_items[1];
    uint8_t* m17_out = (uint8_t*)output_items[0];
    uint8_t* ax25_out = (uint8_t*)output_items[1];

    int consumed_m17 = 0;
    int consumed_ax25 = 0;
    int produced_m17 = 0;
    int produced_ax25 = 0;

    // Process M17 to AX.25 conversion
    if (d_conversion_mode == CONVERSION_AUTO || d_conversion_mode == CONVERSION_M17_TO_AX25) {
        // Convert M17 frames to AX.25
        std::vector<uint8_t> m17_buffer(m17_in, m17_in + noutput_items);
        std::vector<uint8_t> ax25_result = convert_m17_to_ax25(m17_buffer);

        // Copy to output
        int to_copy = std::min((int)ax25_result.size(), noutput_items - produced_ax25);
        memcpy(&ax25_out[produced_ax25], ax25_result.data(), to_copy);
        produced_ax25 += to_copy;
        consumed_m17 += noutput_items;
    }

    // Process AX.25 to M17 conversion
    if (d_conversion_mode == CONVERSION_AUTO || d_conversion_mode == CONVERSION_AX25_TO_M17) {
        // Convert AX.25 frames to M17
        std::vector<uint8_t> ax25_buffer(ax25_in, ax25_in + noutput_items);
        std::vector<uint8_t> m17_result = convert_ax25_to_m17(ax25_buffer);

        // Copy to output
        int to_copy = std::min((int)m17_result.size(), noutput_items - produced_m17);
        memcpy(&m17_out[produced_m17], m17_result.data(), to_copy);
        produced_m17 += to_copy;
        consumed_ax25 += noutput_items;
    }

    return std::max(produced_m17, produced_ax25);
}

std::vector<uint8_t>
protocol_converter_impl::convert_m17_to_ax25(const std::vector<uint8_t>& m17_data) {
    std::vector<uint8_t> result;

    // Detect M17 frame boundaries
    for (size_t i = 0; i < m17_data.size(); i++) {
        if (m17_data[i] == 0x5D) { // M17 sync word
            // Extract M17 frame
            std::vector<uint8_t> m17_frame;
            size_t frame_start = i;
            size_t frame_end = std::min(i + 48, m17_data.size());

            m17_frame.assign(m17_data.begin() + frame_start, m17_data.begin() + frame_end);

            // Convert to AX.25
            std::vector<uint8_t> ax25_frame = convert_single_m17_to_ax25(m17_frame);
            result.insert(result.end(), ax25_frame.begin(), ax25_frame.end());

            i = frame_end - 1; // Skip processed frame
        }
    }

    return result;
}

std::vector<uint8_t>
protocol_converter_impl::convert_ax25_to_m17(const std::vector<uint8_t>& ax25_data) {
    std::vector<uint8_t> result;

    // Detect AX.25 frame boundaries
    for (size_t i = 0; i < ax25_data.size(); i++) {
        if (ax25_data[i] == 0x7E) { // AX.25 flag
            // Extract AX.25 frame
            std::vector<uint8_t> ax25_frame;
            size_t frame_start = i;
            size_t frame_end = i + 1;

            // Find end of frame
            for (size_t j = i + 1; j < ax25_data.size(); j++) {
                if (ax25_data[j] == 0x7E) {
                    frame_end = j + 1;
                    break;
                }
            }

            ax25_frame.assign(ax25_data.begin() + frame_start, ax25_data.begin() + frame_end);

            // Convert to M17
            std::vector<uint8_t> m17_frame = convert_single_ax25_to_m17(ax25_frame);
            result.insert(result.end(), m17_frame.begin(), m17_frame.end());

            i = frame_end - 1; // Skip processed frame
        }
    }

    return result;
}

std::vector<uint8_t>
protocol_converter_impl::convert_single_m17_to_ax25(const std::vector<uint8_t>& m17_frame) {
    if (m17_frame.size() < 48) {
        return {}; // Incomplete frame
    }

    // Extract M17 payload
    std::vector<uint8_t> payload;
    payload.assign(m17_frame.begin() + 2, m17_frame.end() - 2); // Skip sync and CRC

    // Create AX.25 frame
    std::vector<uint8_t> ax25_frame;

    // AX.25 header
    ax25_frame.push_back(0x7E); // Opening flag

    // Destination address
    std::string dest = d_ax25_destination;
    dest.resize(6, ' ');
    for (size_t i = 0; i < 6; i++) {
        ax25_frame.push_back(dest[i] << 1);
    }
    ax25_frame.push_back(0x00); // SSID

    // Source address
    std::string src = d_ax25_callsign;
    src.resize(6, ' ');
    for (size_t i = 0; i < 6; i++) {
        ax25_frame.push_back(src[i] << 1);
    }
    ax25_frame.push_back(0x60); // SSID

    // Control field
    ax25_frame.push_back(0x03); // I-frame

    // PID
    ax25_frame.push_back(0xF0);

    // Information field (M17 payload)
    ax25_frame.insert(ax25_frame.end(), payload.begin(), payload.end());

    // Calculate FCS
    uint16_t fcs = calculate_ax25_fcs(ax25_frame);
    ax25_frame.push_back(fcs & 0xFF);
    ax25_frame.push_back((fcs >> 8) & 0xFF);

    // Closing flag
    ax25_frame.push_back(0x7E);

    return ax25_frame;
}

std::vector<uint8_t>
protocol_converter_impl::convert_single_ax25_to_m17(const std::vector<uint8_t>& ax25_frame) {
    if (ax25_frame.size() < 10) {
        return {}; // Incomplete frame
    }

    // Extract AX.25 information field
    std::vector<uint8_t> payload;
    size_t header_size = 14; // Destination + source + control + PID
    if (ax25_frame.size() > header_size + 2) {
        payload.assign(ax25_frame.begin() + header_size, ax25_frame.end() - 3); // Skip FCS and flag
    }

    // Create M17 frame
    std::vector<uint8_t> m17_frame;

    // M17 sync word
    m17_frame.push_back(0x5D);

    // Frame type
    m17_frame.push_back(0x00);

    // Payload
    m17_frame.insert(m17_frame.end(), payload.begin(), payload.end());

    // Pad to M17 frame size
    while (m17_frame.size() < 48) {
        m17_frame.push_back(0x00);
    }

    // Calculate M17 CRC
    uint16_t crc = calculate_m17_crc(m17_frame);
    m17_frame.push_back(crc & 0xFF);
    m17_frame.push_back((crc >> 8) & 0xFF);

    return m17_frame;
}

uint16_t protocol_converter_impl::calculate_ax25_fcs(const std::vector<uint8_t>& frame) {
    uint16_t fcs = 0xFFFF;

    for (uint8_t byte : frame) {
        fcs ^= byte;
        for (int i = 0; i < 8; i++) {
            if (fcs & 0x0001) {
                fcs = (fcs >> 1) ^ 0x8408;
            } else {
                fcs >>= 1;
            }
        }
    }

    return fcs ^ 0xFFFF;
}

uint16_t protocol_converter_impl::calculate_m17_crc(const std::vector<uint8_t>& frame) {
    uint16_t crc = 0xFFFF;

    for (uint8_t byte : frame) {
        crc ^= (byte << 8);
        for (int i = 0; i < 8; i++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

void protocol_converter_impl::handle_control_message(pmt::pmt_t msg) {
    if (pmt::is_dict(msg)) {
        if (pmt::dict_has_key(msg, pmt::mp("conversion_mode"))) {
            std::string mode = pmt::symbol_to_string(
                pmt::dict_ref(msg, pmt::mp("conversion_mode"), pmt::mp("auto")));
            if (mode == "m17_to_ax25") {
                d_conversion_mode = CONVERSION_M17_TO_AX25;
            } else if (mode == "ax25_to_m17") {
                d_conversion_mode = CONVERSION_AX25_TO_M17;
            } else {
                d_conversion_mode = CONVERSION_AUTO;
            }
        }
    }
}

void protocol_converter_impl::set_conversion_mode(conversion_mode_t mode) {
    d_conversion_mode = mode;
}

void protocol_converter_impl::set_m17_callsign(const std::string& callsign) {
    d_m17_callsign = callsign;
}

void protocol_converter_impl::set_m17_destination(const std::string& destination) {
    d_m17_destination = destination;
}

void protocol_converter_impl::set_ax25_callsign(const std::string& callsign) {
    d_ax25_callsign = callsign;
}

void protocol_converter_impl::set_ax25_destination(const std::string& destination) {
    d_ax25_destination = destination;
}

void protocol_converter_impl::set_fx25_enabled(bool enabled) {
    d_enable_fx25 = enabled;
}

void protocol_converter_impl::set_il2p_enabled(bool enabled) {
    d_enable_il2p = enabled;
}

} // namespace m17_bridge
} // namespace gr
