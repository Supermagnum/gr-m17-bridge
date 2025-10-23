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

#include "m17_to_ax25_impl.h"

#include <cstring>
#include <gnuradio/io_signature.h>
#include <gnuradio/math.h>
#include <iostream>
#include <m17_ax25_bridge.h>
#include <volk/volk.h>

namespace gr {
namespace m17_bridge {

/*!
 * \brief Create M17 to AX.25 converter block
 * \param callsign Source callsign for AX.25 frames
 * \param destination Destination callsign for AX.25 frames
 * \param enable_fec Enable FX.25 Forward Error Correction
 * \return Shared pointer to the converter block
 */
m17_to_ax25::sptr m17_to_ax25::make(const std::string& callsign, const std::string& destination,
                                    bool enable_fec) {
    return gnuradio::make_block_sptr<m17_to_ax25_impl>(callsign, destination, enable_fec);
}

/*!
 * \brief Constructor for M17 to AX.25 converter implementation
 * \param callsign Source callsign for AX.25 frames
 * \param destination Destination callsign for AX.25 frames
 * \param enable_fec Enable FX.25 Forward Error Correction
 */
m17_to_ax25_impl::m17_to_ax25_impl(const std::string& callsign, const std::string& destination,
                                   bool enable_fec)
    : gr::sync_block("m17_to_ax25", gr::io_signature::make(1, 1, sizeof(uint8_t)),
                     gr::io_signature::make(1, 1, sizeof(uint8_t))),
      d_callsign(callsign), d_destination(destination), d_enable_fec(enable_fec), d_frame_buffer(),
      d_frame_length(0), d_state(STATE_IDLE), d_frame_counter(0) {
    // Initialize the M17-AX.25 bridge
    if (m17_ax25_bridge_init(&d_bridge) != 0) {
        throw std::runtime_error("Failed to initialize M17-AX.25 bridge");
    }

    // Configure the bridge
    bridge_config_t config;
    config.m17_enabled = true;
    config.ax25_enabled = true;
    config.fx25_enabled = enable_fec;
    config.il2p_enabled = false;
    config.auto_detect = false;
    config.m17_frequency = 144800000;
    config.ax25_frequency = 144800000;
    config.m17_can = 0;
    strncpy(config.ax25_callsign, callsign.c_str(), sizeof(config.ax25_callsign) - 1);
    config.ax25_callsign[sizeof(config.ax25_callsign) - 1] = '\0';
    config.ax25_ssid = 0;
    config.fx25_rs_type = FX25_RS_255_239;
    config.il2p_debug = 0;

    if (m17_ax25_bridge_set_config(&d_bridge, &config) != 0) {
        throw std::runtime_error("Failed to configure M17-AX.25 bridge");
    }

    // Set up message ports for control
    message_port_register_in(pmt::mp("control"));
    set_msg_handler(pmt::mp("control"), [this](pmt::pmt_t msg) { handle_control_message(msg); });
}

m17_to_ax25_impl::~m17_to_ax25_impl() {
    // Cleanup the M17-AX.25 bridge
    m17_ax25_bridge_cleanup(&d_bridge);
}

/*!
 * \brief Main processing function for M17 to AX.25 conversion
 * \param noutput_items Number of output items to produce
 * \param input_items Input data (M17 frames)
 * \param output_items Output data (AX.25 frames)
 * \return Number of items produced
 */
int m17_to_ax25_impl::work(int noutput_items, gr_vector_const_void_star& input_items,
                           gr_vector_void_star& output_items) {
    const uint8_t* in = (const uint8_t*)input_items[0];
    uint8_t* out = (uint8_t*)output_items[0];

    int consumed = 0;
    int produced = 0;

    // Process input data in chunks
    for (int i = 0; i < noutput_items; i++) {
        // Add data to frame buffer
        d_frame_buffer.push_back(in[i]);
        consumed++;

        // Check for M17 frame markers (0x5D 0x5F)
        if (d_frame_buffer.size() >= 2 && d_frame_buffer[d_frame_buffer.size() - 2] == 0x5D &&
            d_frame_buffer[d_frame_buffer.size() - 1] == 0x5F) {

            // We have a potential M17 frame, try to convert it
            uint8_t ax25_data[256];
            uint16_t ax25_length = sizeof(ax25_data);

            if (m17_ax25_bridge_convert_m17_to_ax25(&d_bridge, d_frame_buffer.data(),
                                                    d_frame_buffer.size(), ax25_data,
                                                    &ax25_length) == 0) {

                // Copy converted AX.25 data to output
                int to_copy = std::min((int)ax25_length, noutput_items - produced);
                memcpy(&out[produced], ax25_data, to_copy);
                produced += to_copy;

                // If we couldn't fit all the data, store the rest
                if (to_copy < ax25_length) {
                    d_output_buffer.assign(ax25_data + to_copy, ax25_data + ax25_length);
                }
            }

            // Clear frame buffer for next frame
            d_frame_buffer.clear();
        }
    }

    // Output any remaining buffered data
    if (!d_output_buffer.empty()) {
        int to_copy = std::min((int)d_output_buffer.size(), noutput_items - produced);
        memcpy(&out[produced], d_output_buffer.data(), to_copy);
        d_output_buffer.erase(d_output_buffer.begin(), d_output_buffer.begin() + to_copy);
        produced += to_copy;
    }

    return produced;
}

void m17_to_ax25_impl::set_destination(const std::string& destination) {
    d_destination = destination;

    // Update bridge configuration
    bridge_config_t config;
    if (m17_ax25_bridge_get_config(&d_bridge, &config) == 0) {
        strncpy(config.ax25_callsign, destination.c_str(), sizeof(config.ax25_callsign) - 1);
        config.ax25_callsign[sizeof(config.ax25_callsign) - 1] = '\0';
        m17_ax25_bridge_set_config(&d_bridge, &config);
    }
}

void m17_to_ax25_impl::set_callsign(const std::string& callsign) {
    d_callsign = callsign;

    // Update bridge configuration
    bridge_config_t config;
    if (m17_ax25_bridge_get_config(&d_bridge, &config) == 0) {
        strncpy(config.ax25_callsign, callsign.c_str(), sizeof(config.ax25_callsign) - 1);
        config.ax25_callsign[sizeof(config.ax25_callsign) - 1] = '\0';
        m17_ax25_bridge_set_config(&d_bridge, &config);
    }
}

void m17_to_ax25_impl::set_fec_enabled(bool enabled) {
    d_enable_fec = enabled;

    // Update bridge configuration
    bridge_config_t config;
    if (m17_ax25_bridge_get_config(&d_bridge, &config) == 0) {
        config.fx25_enabled = enabled;
        m17_ax25_bridge_set_config(&d_bridge, &config);
    }
}

void m17_to_ax25_impl::handle_control_message(pmt::pmt_t msg) {
    if (pmt::is_dict(msg)) {
        // Handle configuration changes
        if (pmt::dict_has_key(msg, pmt::mp("destination"))) {
            d_destination =
                pmt::symbol_to_string(pmt::dict_ref(msg, pmt::mp("destination"), pmt::mp("")));
            initialize_ax25_frame();
        }

        if (pmt::dict_has_key(msg, pmt::mp("callsign"))) {
            d_callsign =
                pmt::symbol_to_string(pmt::dict_ref(msg, pmt::mp("callsign"), pmt::mp("")));
            initialize_ax25_frame();
        }
    }
}

void m17_to_ax25_impl::set_destination(const std::string& destination) {
    d_destination = destination;
    initialize_ax25_frame();
}

void m17_to_ax25_impl::set_callsign(const std::string& callsign) {
    d_callsign = callsign;
    initialize_ax25_frame();
}

void m17_to_ax25_impl::set_fec_enabled(bool enabled) {
    d_enable_fec = enabled;
}

} // namespace m17_bridge
} // namespace gr
