/* -*- c++ -*- */
/*
 * Copyright 2024 M17 Bridge Project
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_M17_BRIDGE_M17_TO_AX25_IMPL_H
#define INCLUDED_M17_BRIDGE_M17_TO_AX25_IMPL_H

#include <gnuradio/io_signature.h>
#include <gnuradio/sync_block.h>
#include <m17_ax25_bridge.h>
#include <m17_to_ax25.h>
#include <pmt/pmt.h>

namespace gr {
namespace m17_bridge {

/*!
 * \brief Implementation of M17 to AX.25 protocol converter
 * \ingroup m17_bridge
 *
 * This block converts M17 digital radio frames to AX.25 packet radio frames.
 * It supports callsign mapping, FEC (Forward Error Correction), and APRS integration.
 */
class m17_to_ax25_impl : public m17_to_ax25 {
  private:
    std::string d_callsign;                         //!< Source callsign for AX.25 frames
    std::string d_destination;                      //!< Destination callsign for AX.25 frames
    bool d_enable_fec;                              //!< Enable FX.25 Forward Error Correction
    m17_ax25_bridge_t d_bridge;                     //!< M17-AX.25 bridge context
    std::vector<uint8_t> d_frame_buffer;            //!< Buffer for incoming M17 frames
    int d_frame_length;                             //!< Current frame length
    enum { STATE_IDLE, STATE_FRAME_START } d_state; //!< Processing state
    int d_frame_counter;                            //!< Frame counter for statistics
    std::vector<uint8_t> d_ax25_frame;              //!< AX.25 frame buffer
    std::vector<uint8_t> d_output_buffer;           //!< Output buffer for converted frames

  public:
    /*!
     * \brief Constructor for M17 to AX.25 converter
     * \param callsign Source callsign for AX.25 frames
     * \param destination Destination callsign for AX.25 frames
     * \param enable_fec Enable FX.25 Forward Error Correction
     */
    m17_to_ax25_impl(const std::string& callsign, const std::string& destination, bool enable_fec);

    /*!
     * \brief Destructor
     */
    ~m17_to_ax25_impl();

    /*!
     * \brief Main processing function
     * \param noutput_items Number of output items to produce
     * \param input_items Input data
     * \param output_items Output data
     * \return Number of items produced
     */
    int work(int noutput_items, gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);

    /*!
     * \brief Set destination callsign
     * \param destination New destination callsign
     */
    void set_destination(const std::string& destination);

    /*!
     * \brief Set source callsign
     * \param callsign New source callsign
     */
    void set_callsign(const std::string& callsign);

    /*!
     * \brief Enable or disable FEC
     * \param enabled True to enable FX.25 FEC, false to disable
     */
    void set_fec_enabled(bool enabled);

  private:
    /*!
     * \brief Handle control messages
     * \param msg Control message
     */
    void handle_control_message(pmt::pmt_t msg);
};

} // namespace m17_bridge
} // namespace gr

#endif /* INCLUDED_M17_BRIDGE_M17_TO_AX25_IMPL_H */
