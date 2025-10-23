/* -*- c++ -*- */
/*
 * Copyright 2024 M17 Bridge Project
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_M17_BRIDGE_CALLSIGN_MAPPER_H
#define INCLUDED_M17_BRIDGE_MAPPER_H

#include <gnuradio/sync_block.h>
#include <m17_bridge/api.h>
#include <map>
#include <string>

namespace gr {
namespace m17_bridge {

/*!
 * \brief Callsign mapping between M17 and AX.25 protocols
 * \ingroup m17_bridge
 *
 * This block provides callsign mapping functionality between M17 and AX.25
 * protocols, allowing automatic translation of callsigns between the two
 * systems.
 */
class M17_BRIDGE_API callsign_mapper : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<callsign_mapper> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of m17_bridge::callsign_mapper.
     */
    static sptr make();

    /*!
     * \brief Add a callsign mapping
     * \param m17_callsign M17 callsign
     * \param ax25_callsign Corresponding AX.25 callsign
     */
    virtual void add_mapping(const std::string& m17_callsign,
                           const std::string& ax25_callsign) = 0;

    /*!
     * \brief Remove a callsign mapping
     * \param m17_callsign M17 callsign to remove
     */
    virtual void remove_mapping(const std::string& m17_callsign) = 0;

    /*!
     * \brief Get AX.25 callsign for M17 callsign
     * \param m17_callsign M17 callsign
     * \return Corresponding AX.25 callsign
     */
    virtual std::string get_ax25_callsign(const std::string& m17_callsign) = 0;

    /*!
     * \brief Get M17 callsign for AX.25 callsign
     * \param ax25_callsign AX.25 callsign
     * \return Corresponding M17 callsign
     */
    virtual std::string get_m17_callsign(const std::string& ax25_callsign) = 0;

    /*!
     * \brief Enable or disable auto-mapping
     * \param enabled Enable auto-mapping
     */
    virtual void set_auto_mapping_enabled(bool enabled) = 0;

    /*!
     * \brief Check if auto-mapping is enabled
     * \return True if auto-mapping is enabled
     */
    virtual bool is_auto_mapping_enabled() const = 0;

    /*!
     * \brief Get the current mapping table
     * \return Map of M17 to AX.25 callsign mappings
     */
    virtual std::map<std::string, std::string> get_mapping_table() const = 0;

    /*!
     * \brief Clear all mappings
     */
    virtual void clear_mappings() = 0;

    /*!
     * \brief Load mappings from file
     * \param filename Configuration file path
     */
    virtual void load_mappings_from_file(const std::string& filename) = 0;

    /*!
     * \brief Save mappings to file
     * \param filename Configuration file path
     */
    virtual void save_mappings_to_file(const std::string& filename) = 0;
};

} // namespace m17_bridge
} // namespace gr

#endif /* INCLUDED_M17_BRIDGE_CALLSIGN_MAPPER_H */
