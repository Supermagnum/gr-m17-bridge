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

#include "ax25_to_m17_impl.h"

#include <cstring>
#include <gnuradio/io_signature.h>
#include <gnuradio/math.h>
#include <iostream>
#include <m17_ax25_bridge.h>
#include <volk/volk.h>

namespace gr {
namespace m17_bridge {

/*!
 * \brief Create AX.25 to M17 converter block
 * \param callsign Source callsign for M17 frames
 * \param destination Destination callsign for M17 frames
 * \param enable_fec Enable FX.25 Forward Error Correction
 * \return Shared pointer to the converter block
 */
ax25_to_m17::sptr ax25_to_m17::make(const std::string& callsign, const std::string& destination,
                                    bool enable_fec) {
    return gnuradio::make_block_sptr<ax25_to_m17_impl>(callsign, destination, enable_fec);
}

ax25_to_m17_impl::ax25_to_m17_impl(const std::string& callsign, const std::string& destination,
                                   bool enable_fec)
    : gr::sync_block("ax25_to_m17", gr::io_signature::make(1, 1, sizeof(uint8_t)),
                     gr::io_signature::make(1, 1, sizeof(uint8_t))),
      d_callsign(callsign), d_destination(destination), d_enable_fec(enable_fec), d_frame_buffer(),
      d_frame_length(0), d_state(STATE_IDLE), d_frame_counter(0), d_bit_stuffer(0), d_bit_count(0) {
    // Initialize M17 frame structure
    initialize_m17_frame();

    // Set up message ports for control
    message_port_register_in(pmt::mp("control"));
    set_msg_handler(pmt::mp("control"), [this](pmt::pmt_t msg) { handle_control_message(msg); });
}

ax25_to_m17_impl::~ax25_to_m17_impl() {}

void ax25_to_m17_impl::initialize_m17_frame() {
    // M17 frame structure: sync word + payload + CRC
    d_m17_frame.clear();

    // M17 sync word (0x5D)
    d_m17_frame.push_back(0x5D);

    // Frame type (data frame)
    d_m17_frame.push_back(0x00);
}

int ax25_to_m17_impl::work(int noutput_items, gr_vector_const_void_star& input_items,
                           gr_vector_void_star& output_items) {
    const uint8_t* in = (const uint8_t*)input_items[0];
    uint8_t* out = (uint8_t*)output_items[0];

    int consumed = 0;
    int produced = 0;

    for (int i = 0; i < noutput_items; i++) {
        switch (d_state) {
        case STATE_IDLE:
            // Look for AX.25 frame start (flag 0x7E)
            if (in[i] == 0x7E) {
                d_state = STATE_FRAME_START;
                d_frame_length = 0;
                d_frame_buffer.clear();
                d_bit_stuffer = 0;
                d_bit_count = 0;
            }
            consumed++;
            break;

        case STATE_FRAME_START:
            // Check for end of frame (flag 0x7E)
            if (in[i] == 0x7E && d_frame_length > 0) {
                process_ax25_frame();
                d_state = STATE_IDLE;
            } else {
                d_frame_buffer.push_back(in[i]);
                d_frame_length++;
            }
            consumed++;
            break;
        }
    }

    // Output any converted M17 frames
    if (!d_output_buffer.empty()) {
        int to_copy = std::min((int)d_output_buffer.size(), noutput_items - produced);
        memcpy(&out[produced], d_output_buffer.data(), to_copy);
        d_output_buffer.erase(d_output_buffer.begin(), d_output_buffer.begin() + to_copy);
        produced += to_copy;
    }

    return produced;
}

void ax25_to_m17_impl::process_ax25_frame() {
    if (d_frame_buffer.size() < 10) { // Minimum AX.25 frame size
        return;                       // Incomplete frame
    }

    // Extract AX.25 frame components
    std::vector<uint8_t> ax25_payload;

    // Skip AX.25 header (destination, source, control, PID)
    size_t header_size = 14; // 6 + 1 + 6 + 1 for dest/src addresses and control
    if (d_frame_buffer.size() > header_size + 2) { // +2 for FCS
        ax25_payload.assign(d_frame_buffer.begin() + header_size,
                            d_frame_buffer.end() - 2); // Skip FCS
    }

    // Convert AX.25 payload to M17 frame
    convert_ax25_to_m17(ax25_payload);
}

void ax25_to_m17_impl::convert_ax25_to_m17(const std::vector<uint8_t>& ax25_payload) {
    // Create M17 frame
    std::vector<uint8_t> m17_frame = d_m17_frame;

    // Add M17 payload (AX.25 information field)
    m17_frame.insert(m17_frame.end(), ax25_payload.begin(), ax25_payload.end());

    // Pad to M17 frame size if necessary
    while (m17_frame.size() < 48) {
        m17_frame.push_back(0x00);
    }

    // Calculate and add M17 CRC
    uint16_t crc = calculate_m17_crc(m17_frame);
    m17_frame.push_back(crc & 0xFF);
    m17_frame.push_back((crc >> 8) & 0xFF);

    // Store for output
    d_output_buffer.insert(d_output_buffer.end(), m17_frame.begin(), m17_frame.end());

    d_frame_counter++;
}

uint16_t ax25_to_m17_impl::calculate_m17_crc(const std::vector<uint8_t>& frame) {
    // M17 uses CRC-16-CCITT
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

void ax25_to_m17_impl::handle_control_message(pmt::pmt_t msg) {
    if (pmt::is_dict(msg)) {
        // Handle configuration changes
        if (pmt::dict_has_key(msg, pmt::mp("destination"))) {
            d_destination =
                pmt::symbol_to_string(pmt::dict_ref(msg, pmt::mp("destination"), pmt::mp("")));
        }

        if (pmt::dict_has_key(msg, pmt::mp("callsign"))) {
            d_callsign =
                pmt::symbol_to_string(pmt::dict_ref(msg, pmt::mp("callsign"), pmt::mp("")));
        }
    }
}

void ax25_to_m17_impl::set_destination(const std::string& destination) {
    d_destination = destination;
}

void ax25_to_m17_impl::set_callsign(const std::string& callsign) {
    d_callsign = callsign;
}

void ax25_to_m17_impl::set_fec_enabled(bool enabled) {
    d_enable_fec = enabled;
}

} // namespace m17_bridge
} // namespace gr
