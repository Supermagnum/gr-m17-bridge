# Find GNU Radio
# This module defines:
#  Gnuradio_FOUND - whether GNU Radio was found
#  Gnuradio_INCLUDE_DIRS - GNU Radio include directories
#  Gnuradio_LIBRARIES - GNU Radio libraries
#  Gnuradio_VERSION - GNU Radio version

find_package(PkgConfig REQUIRED)

# Find GNU Radio using pkg-config
pkg_check_modules(Gnuradio REQUIRED gnuradio)

# Set variables for compatibility
set(Gnuradio_INCLUDE_DIRS ${Gnuradio_INCLUDE_DIRS})
set(Gnuradio_LIBRARIES ${Gnuradio_LIBRARIES})

# Create imported target
if(NOT TARGET gnuradio::gnuradio)
    add_library(gnuradio::gnuradio INTERFACE IMPORTED)
    set_target_properties(gnuradio::gnuradio PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Gnuradio_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${Gnuradio_LIBRARIES}"
    )
endif()

# Print found information
if(Gnuradio_FOUND)
    message(STATUS "Found GNU Radio: ${Gnuradio_VERSION}")
    message(STATUS "  Include dirs: ${Gnuradio_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${Gnuradio_LIBRARIES}")
else()
    message(FATAL_ERROR "GNU Radio not found")
endif()
