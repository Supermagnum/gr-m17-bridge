/* -*- c++ -*- */
/*
 * Copyright 2024 M17 Bridge Project
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_M17_BRIDGE_AX25_TO_M17_H
#define INCLUDED_M17_BRIDGE_AX25_TO_M17_H

#include <gnuradio/sync_block.h>
#include <m17_bridge/api.h>

namespace gr {
namespace m17_bridge {

/*!
 * \brief Convert AX.25 frames to M17 frames
 * \ingroup m17_bridge
 *
 * This block converts AX.25 packet radio frames to M17 digital radio frames.
 * It handles the protocol conversion, callsign mapping, and frame formatting.
 */
class M17_BRIDGE_API ax25_to_m17 : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<ax25_to_m17> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of m17_bridge::ax25_to_m17.
     *
     * \param callsign Source callsign for M17 frames
     * \param destination Destination callsign for M17 frames
     * \param enable_fec Enable Forward Error Correction
     */
    static sptr make(const std::string& callsign,
                     const std::string& destination,
                     bool enable_fec = false);

    /*!
     * \brief Set the destination callsign
     */
    virtual void set_destination(const std::string& destination) = 0;

    /*!
     * \brief Set the source callsign
     */
    virtual void set_callsign(const std::string& callsign) = 0;

    /*!
     * \brief Enable or disable FEC
     */
    virtual void set_fec_enabled(bool enabled) = 0;
};

} // namespace m17_bridge
} // namespace gr

#endif /* INCLUDED_M17_BRIDGE_AX25_TO_M17_H */
