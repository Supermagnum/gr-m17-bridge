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

#include "callsign_mapper_impl.h"

#include <algorithm>
#include <cstring>
#include <gnuradio/io_signature.h>
#include <gnuradio/math.h>
#include <iostream>
#include <map>
#include <volk/volk.h>

namespace gr {
namespace m17_bridge {

callsign_mapper::sptr callsign_mapper::make() {
    return gnuradio::make_block_sptr<callsign_mapper_impl>();
}

callsign_mapper_impl::callsign_mapper_impl()
    : gr::sync_block("callsign_mapper", gr::io_signature::make(1, 1, sizeof(uint8_t)),
                     gr::io_signature::make(1, 1, sizeof(uint8_t))),
      d_mapping_table(), d_reverse_mapping_table(), d_auto_mapping_enabled(true),
      d_mapping_counter(0) {
    // Set up message ports for control
    message_port_register_in(pmt::mp("control"));
    set_msg_handler(pmt::mp("control"), [this](pmt::pmt_t msg) { handle_control_message(msg); });

    // Initialize default mappings
    initialize_default_mappings();
}

callsign_mapper_impl::~callsign_mapper_impl() {}

void callsign_mapper_impl::initialize_default_mappings() {
    // Add some common amateur radio callsign mappings
    add_mapping("N0CALL", "N0CALL");
    add_mapping("W1AW", "W1AW");
    add_mapping("VE3KCL", "VE3KCL");
    add_mapping("G0ABC", "G0ABC");
    add_mapping("JA1ABC", "JA1ABC");
}

int callsign_mapper_impl::work(int noutput_items, gr_vector_const_void_star& input_items,
                               gr_vector_void_star& output_items) {
    const uint8_t* in = (const uint8_t*)input_items[0];
    uint8_t* out = (uint8_t*)output_items[0];

    // Process input data and apply callsign mapping
    for (int i = 0; i < noutput_items; i++) {
        out[i] = in[i]; // Default: pass through unchanged

        // Apply callsign mapping if needed
        if (d_auto_mapping_enabled) {
            // This is a simplified implementation
            // In a real implementation, you would parse the frame
            // and replace callsigns based on the mapping table
            apply_callsign_mapping(&out[i], 1);
        }
    }

    return noutput_items;
}

void callsign_mapper_impl::add_mapping(const std::string& m17_callsign,
                                       const std::string& ax25_callsign) {
    d_mapping_table[m17_callsign] = ax25_callsign;
    d_reverse_mapping_table[ax25_callsign] = m17_callsign;
}

void callsign_mapper_impl::remove_mapping(const std::string& m17_callsign) {
    auto it = d_mapping_table.find(m17_callsign);
    if (it != d_mapping_table.end()) {
        std::string ax25_callsign = it->second;
        d_mapping_table.erase(it);
        d_reverse_mapping_table.erase(ax25_callsign);
    }
}

std::string callsign_mapper_impl::get_ax25_callsign(const std::string& m17_callsign) {
    auto it = d_mapping_table.find(m17_callsign);
    if (it != d_mapping_table.end()) {
        return it->second;
    }

    // Auto-mapping: create a new mapping if auto-mapping is enabled
    if (d_auto_mapping_enabled) {
        std::string ax25_callsign = m17_callsign; // Default: same callsign
        add_mapping(m17_callsign, ax25_callsign);
        return ax25_callsign;
    }

    return m17_callsign; // Return original if no mapping found
}

std::string callsign_mapper_impl::get_m17_callsign(const std::string& ax25_callsign) {
    auto it = d_reverse_mapping_table.find(ax25_callsign);
    if (it != d_reverse_mapping_table.end()) {
        return it->second;
    }

    // Auto-mapping: create a new mapping if auto-mapping is enabled
    if (d_auto_mapping_enabled) {
        std::string m17_callsign = ax25_callsign; // Default: same callsign
        add_mapping(m17_callsign, ax25_callsign);
        return m17_callsign;
    }

    return ax25_callsign; // Return original if no mapping found
}

void callsign_mapper_impl::apply_callsign_mapping(uint8_t* data, int length) {
    // This is a simplified implementation
    // In a real implementation, you would:
    // 1. Parse the frame to identify callsign fields
    // 2. Look up the callsign in the mapping table
    // 3. Replace the callsign with the mapped value
    // 4. Update any checksums or CRCs

    // For now, just pass through the data unchanged
    // This would need to be implemented based on the specific
    // frame format being processed
}

void callsign_mapper_impl::handle_control_message(pmt::pmt_t msg) {
    if (pmt::is_dict(msg)) {
        if (pmt::dict_has_key(msg, pmt::mp("add_mapping"))) {
            pmt::pmt_t mapping = pmt::dict_ref(msg, pmt::mp("add_mapping"), pmt::PMT_NIL);
            if (pmt::is_dict(mapping)) {
                std::string m17_callsign =
                    pmt::symbol_to_string(pmt::dict_ref(mapping, pmt::mp("m17"), pmt::mp("")));
                std::string ax25_callsign =
                    pmt::symbol_to_string(pmt::dict_ref(mapping, pmt::mp("ax25"), pmt::mp("")));
                add_mapping(m17_callsign, ax25_callsign);
            }
        }

        if (pmt::dict_has_key(msg, pmt::mp("remove_mapping"))) {
            std::string m17_callsign =
                pmt::symbol_to_string(pmt::dict_ref(msg, pmt::mp("remove_mapping"), pmt::mp("")));
            remove_mapping(m17_callsign);
        }

        if (pmt::dict_has_key(msg, pmt::mp("auto_mapping"))) {
            d_auto_mapping_enabled =
                pmt::to_bool(pmt::dict_ref(msg, pmt::mp("auto_mapping"), pmt::PMT_F));
        }
    }
}

void callsign_mapper_impl::set_auto_mapping_enabled(bool enabled) {
    d_auto_mapping_enabled = enabled;
}

bool callsign_mapper_impl::is_auto_mapping_enabled() const {
    return d_auto_mapping_enabled;
}

std::map<std::string, std::string> callsign_mapper_impl::get_mapping_table() const {
    return d_mapping_table;
}

void callsign_mapper_impl::clear_mappings() {
    d_mapping_table.clear();
    d_reverse_mapping_table.clear();
}

void callsign_mapper_impl::load_mappings_from_file(const std::string& filename) {
    // This would load mappings from a configuration file
    // For now, just a placeholder
    std::cout << "Loading mappings from file: " << filename << std::endl;
}

void callsign_mapper_impl::save_mappings_to_file(const std::string& filename) {
    // This would save mappings to a configuration file
    // For now, just a placeholder
    std::cout << "Saving mappings to file: " << filename << std::endl;
}

} // namespace m17_bridge
} // namespace gr
