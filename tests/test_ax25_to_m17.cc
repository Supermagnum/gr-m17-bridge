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
#include <gnuradio/m17_bridge/ax25_to_m17.h>

class TestAX25ToM17 : public ::testing::Test
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

TEST_F(TestAX25ToM17, BasicCreation)
{
    // Test basic block creation
    auto block = gr::m17_bridge::ax25_to_m17::make(
        "N0CALL", "APRS", false);
    
    ASSERT_NE(block, nullptr);
}

TEST_F(TestAX25ToM17, CallsignSetting)
{
    auto block = gr::m17_bridge::ax25_to_m17::make(
        "N0CALL", "APRS", false);
    
    // Test callsign setting
    block->set_callsign("W1AW");
    block->set_destination("APRS");
    
    // These should not throw exceptions
    SUCCEED();
}

TEST_F(TestAX25ToM17, FECSetting)
{
    auto block = gr::m17_bridge::ax25_to_m17::make(
        "N0CALL", "APRS", false);
    
    // Test FEC setting
    block->set_fec_enabled(true);
    block->set_fec_enabled(false);
    
    // These should not throw exceptions
    SUCCEED();
}
