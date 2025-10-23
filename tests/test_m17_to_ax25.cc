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
#include <gnuradio/m17_bridge/m17_to_ax25.h>

class TestM17ToAX25 : public ::testing::Test
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

TEST_F(TestM17ToAX25, BasicCreation)
{
    // Test basic block creation
    auto block = gr::m17_bridge::m17_to_ax25::make(
        "N0CALL", "APRS", false);
    
    ASSERT_NE(block, nullptr);
}

TEST_F(TestM17ToAX25, CallsignSetting)
{
    auto block = gr::m17_bridge::m17_to_ax25::make(
        "N0CALL", "APRS", false);
    
    // Test callsign setting
    block->set_callsign("W1AW");
    block->set_destination("APRS");
    
    // These should not throw exceptions
    SUCCEED();
}

TEST_F(TestM17ToAX25, FECSetting)
{
    auto block = gr::m17_bridge::m17_to_ax25::make(
        "N0CALL", "APRS", false);
    
    // Test FEC setting
    block->set_fec_enabled(true);
    block->set_fec_enabled(false);
    
    // These should not throw exceptions
    SUCCEED();
}
