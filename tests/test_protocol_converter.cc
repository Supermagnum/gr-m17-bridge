/* -*- c++ -*- */
/*
 * Copyright 2024 M17 Bridge Project
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gtest/gtest.h>
#include <gnuradio/m17_bridge/protocol_converter.h>

class TestProtocolConverter : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up test fixtures
    }
    
    void TearDown() override
    {
        // Clean up test fixtures
    }
};

TEST_F(TestProtocolConverter, BasicCreation)
{
    // Test basic block creation
    auto block = gr::m17_bridge::protocol_converter::make(
        "N0CALL", "APRS", "N0CALL", "APRS", false, false);
    
    ASSERT_NE(block, nullptr);
}

TEST_F(TestProtocolConverter, ConversionModeSetting)
{
    auto block = gr::m17_bridge::protocol_converter::make(
        "N0CALL", "APRS", "N0CALL", "APRS", false, false);
    
    // Test conversion mode setting
    block->set_conversion_mode(gr::m17_bridge::protocol_converter::CONVERSION_AUTO);
    block->set_conversion_mode(gr::m17_bridge::protocol_converter::CONVERSION_M17_TO_AX25);
    block->set_conversion_mode(gr::m17_bridge::protocol_converter::CONVERSION_AX25_TO_M17);
    
    // These should not throw exceptions
    SUCCEED();
}

TEST_F(TestProtocolConverter, CallsignSetting)
{
    auto block = gr::m17_bridge::protocol_converter::make(
        "N0CALL", "APRS", "N0CALL", "APRS", false, false);
    
    // Test callsign setting
    block->set_m17_callsign("W1AW");
    block->set_m17_destination("APRS");
    block->set_ax25_callsign("W1AW");
    block->set_ax25_destination("APRS");
    
    // These should not throw exceptions
    SUCCEED();
}

TEST_F(TestProtocolConverter, FeatureSetting)
{
    auto block = gr::m17_bridge::protocol_converter::make(
        "N0CALL", "APRS", "N0CALL", "APRS", false, false);
    
    // Test feature setting
    block->set_fx25_enabled(true);
    block->set_il2p_enabled(true);
    
    // These should not throw exceptions
    SUCCEED();
}
