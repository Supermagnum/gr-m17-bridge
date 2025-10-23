/* -*- c++ -*- */
/*
 * Copyright 2024 M17 Bridge Project
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_M17_BRIDGE_PROTOCOL_CONVERTER_H
#define INCLUDED_M17_BRIDGE_PROTOCOL_CONVERTER_H

#include <gnuradio/sync_block.h>
#include <m17_bridge/api.h>

namespace gr {
namespace m17_bridge {

/*!
 * \brief Protocol converter between M17 and AX.25
 * \ingroup m17_bridge
 *
 * This block provides bidirectional conversion between M17 digital radio
 * and AX.25 packet radio protocols, with support for FX.25 FEC and IL2P.
 */
class M17_BRIDGE_API protocol_converter : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<protocol_converter> sptr;

    enum conversion_mode_t {
        CONVERSION_AUTO = 0,
        CONVERSION_M17_TO_AX25 = 1,
        CONVERSION_AX25_TO_M17 = 2
    };

    /*!
     * \brief Return a shared_ptr to a new instance of m17_bridge::protocol_converter.
     *
     * \param m17_callsign M17 source callsign
     * \param m17_destination M17 destination callsign
     * \param ax25_callsign AX.25 source callsign
     * \param ax25_destination AX.25 destination callsign
     * \param enable_fx25 Enable FX.25 Forward Error Correction
     * \param enable_il2p Enable IL2P protocol support
     */
    static sptr make(const std::string& m17_callsign,
                     const std::string& m17_destination,
                     const std::string& ax25_callsign,
                     const std::string& ax25_destination,
                     bool enable_fx25 = false,
                     bool enable_il2p = false);

    /*!
     * \brief Set the conversion mode
     */
    virtual void set_conversion_mode(conversion_mode_t mode) = 0;

    /*!
     * \brief Set M17 callsign
     */
    virtual void set_m17_callsign(const std::string& callsign) = 0;

    /*!
     * \brief Set M17 destination
     */
    virtual void set_m17_destination(const std::string& destination) = 0;

    /*!
     * \brief Set AX.25 callsign
     */
    virtual void set_ax25_callsign(const std::string& callsign) = 0;

    /*!
     * \brief Set AX.25 destination
     */
    virtual void set_ax25_destination(const std::string& destination) = 0;

    /*!
     * \brief Enable or disable FX.25 FEC
     */
    virtual void set_fx25_enabled(bool enabled) = 0;

    /*!
     * \brief Enable or disable IL2P protocol
     */
    virtual void set_il2p_enabled(bool enabled) = 0;
};

} // namespace m17_bridge
} // namespace gr

#endif /* INCLUDED_M17_BRIDGE_PROTOCOL_CONVERTER_H */
