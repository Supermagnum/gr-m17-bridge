# Test configuration for M17 Bridge

if(ENABLE_TESTING)
    # Find required testing frameworks
    find_package(CTest REQUIRED)
    find_package(GTest REQUIRED)
    
    # Add test executable
    add_executable(test_m17_bridge
        test_m17_to_ax25.cc
        test_ax25_to_m17.cc
        test_protocol_converter.cc
        test_callsign_mapper.cc
    )
    
    # Link test executable
    target_link_libraries(test_m17_bridge
        gnuradio-m17-bridge
        gnuradio::gnuradio
        Volk::volk
        GTest::gtest
        GTest::gtest_main
    )
    
    # Add test to CTest
    add_test(NAME m17_bridge_tests COMMAND test_m17_bridge)
    
    # Set test properties
    set_tests_properties(m17_bridge_tests PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
