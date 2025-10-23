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
#include <gnuradio/m17_bridge/callsign_mapper.h>

class TestCallsignMapper : public ::testing::Test
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

TEST_F(TestCallsignMapper, BasicCreation)
{
    // Test basic block creation
    auto block = gr::m17_bridge::callsign_mapper::make();
    
    ASSERT_NE(block, nullptr);
}

TEST_F(TestCallsignMapper, MappingOperations)
{
    auto block = gr::m17_bridge::callsign_mapper::make();
    
    // Test mapping operations
    block->add_mapping("N0CALL", "N0CALL");
    block->add_mapping("W1AW", "W1AW");
    
    // Test getting mappings
    std::string ax25_callsign = block->get_ax25_callsign("N0CALL");
    std::string m17_callsign = block->get_m17_callsign("N0CALL");
    
    ASSERT_EQ(ax25_callsign, "N0CALL");
    ASSERT_EQ(m17_callsign, "N0CALL");
    
    // Test removing mapping
    block->remove_mapping("N0CALL");
    
    // Test clearing mappings
    block->clear_mappings();
    
    // These should not throw exceptions
    SUCCEED();
}

TEST_F(TestCallsignMapper, AutoMappingSetting)
{
    auto block = gr::m17_bridge::callsign_mapper::make();
    
    // Test auto-mapping setting
    block->set_auto_mapping_enabled(true);
    ASSERT_TRUE(block->is_auto_mapping_enabled());
    
    block->set_auto_mapping_enabled(false);
    ASSERT_FALSE(block->is_auto_mapping_enabled());
}

TEST_F(TestCallsignMapper, MappingTableOperations)
{
    auto block = gr::m17_bridge::callsign_mapper::make();
    
    // Add some mappings
    block->add_mapping("N0CALL", "N0CALL");
    block->add_mapping("W1AW", "W1AW");
    
    // Test getting mapping table
    auto table = block->get_mapping_table();
    ASSERT_EQ(table.size(), 2);
    
    // Test file operations (these should not throw exceptions)
    block->load_mappings_from_file("test_mappings.conf");
    block->save_mappings_to_file("test_mappings.conf");
    
    SUCCEED();
}
