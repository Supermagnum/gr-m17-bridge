cmake_minimum_required(VERSION 3.16)

project(gr-m17-bridge)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Volk REQUIRED)
find_package(Gnuradio REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Define the library sources
set(GR_M17_BRIDGE_SOURCES
    lib/m17_to_ax25_impl.cc
    lib/ax25_to_m17_impl.cc
    lib/protocol_converter_impl.cc
    lib/callsign_mapper_impl.cc
    lib/m17_ax25_bridge.c
    lib/ax25_protocol.c
    lib/fx25_protocol.c
    lib/il2p_protocol.c
    lib/kiss_protocol.c
)

# Create the library
add_library(gnuradio-m17-bridge SHARED ${GR_M17_BRIDGE_SOURCES})

# Link libraries
target_link_libraries(gnuradio-m17-bridge
    gnuradio::gnuradio
    Volk::volk
)

# Set target properties
set_target_properties(gnuradio-m17-bridge PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Install the library
install(TARGETS gnuradio-m17-bridge
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install headers
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Python bindings
if(ENABLE_PYTHON)
    find_package(Python3 COMPONENTS Interpreter Development)
    find_package(pybind11 REQUIRED)
    
    # Create Python bindings
    pybind11_add_module(m17_bridge_swig
        python/m17_bridge/m17_bridge_swig.cc
    )
    
    target_link_libraries(m17_bridge_swig
        gnuradio-m17-bridge
        gnuradio::gnuradio
        Volk::volk
    )
    
    # Install Python module
    install(TARGETS m17_bridge_swig
        DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/m17_bridge
    )
    
    # Install Python files
    install(DIRECTORY python/
        DESTINATION ${CMAKE_INSTALL_PYTHONDIR}
        FILES_MATCHING PATTERN "*.py"
    )
endif()

# GNU Radio Companion blocks
if(ENABLE_GRC)
    install(DIRECTORY grc/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/gnuradio/grc/blocks
        FILES_MATCHING PATTERN "*.yml"
    )
endif()

# Examples
if(ENABLE_EXAMPLES)
    install(DIRECTORY examples/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/gnuradio/examples/m17-bridge
        FILES_MATCHING PATTERN "*.grc"
    )
endif()

# Documentation
if(ENABLE_DOXYGEN)
    find_package(Doxygen REQUIRED)
    
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/html
    )
endif()

# Testing
if(ENABLE_TESTING)
    enable_testing()
    
    # Add test subdirectories
    add_subdirectory(tests)
endif()

# Print configuration summary
message(STATUS "M17 Bridge Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Python support: ${ENABLE_PYTHON}")
message(STATUS "  GRC support: ${ENABLE_GRC}")
message(STATUS "  Examples: ${ENABLE_EXAMPLES}")
message(STATUS "  Documentation: ${ENABLE_DOXYGEN}")
message(STATUS "  Testing: ${ENABLE_TESTING}")
