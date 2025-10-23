/* -*- c++ -*- */
/*
 * Copyright 2024 M17 Bridge Project
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "m17_to_ax25.h"
#include "ax25_to_m17.h"
#include "protocol_converter.h"
#include "callsign_mapper.h"

namespace py = pybind11;

void bind_m17_to_ax25(py::module& m)
{
    using m17_to_ax25 = gr::m17_bridge::m17_to_ax25;

    py::class_<m17_to_ax25, gr::sync_block, gr::block, gr::basic_block,
               std::shared_ptr<m17_to_ax25>>(m, "m17_to_ax25")

        .def(py::init(&m17_to_ax25::make),
             py::arg("callsign"),
             py::arg("destination"),
             py::arg("enable_fec") = false)

        .def("set_destination", &m17_to_ax25::set_destination)
        .def("set_callsign", &m17_to_ax25::set_callsign)
        .def("set_fec_enabled", &m17_to_ax25::set_fec_enabled);
}

void bind_ax25_to_m17(py::module& m)
{
    using ax25_to_m17 = gr::m17_bridge::ax25_to_m17;

    py::class_<ax25_to_m17, gr::sync_block, gr::block, gr::basic_block,
               std::shared_ptr<ax25_to_m17>>(m, "ax25_to_m17")

        .def(py::init(&ax25_to_m17::make),
             py::arg("callsign"),
             py::arg("destination"),
             py::arg("enable_fec") = false)

        .def("set_destination", &ax25_to_m17::set_destination)
        .def("set_callsign", &ax25_to_m17::set_callsign)
        .def("set_fec_enabled", &ax25_to_m17::set_fec_enabled);
}

void bind_protocol_converter(py::module& m)
{
    using protocol_converter = gr::m17_bridge::protocol_converter;

    py::class_<protocol_converter, gr::sync_block, gr::block, gr::basic_block,
               std::shared_ptr<protocol_converter>>(m, "protocol_converter")

        .def(py::init(&protocol_converter::make),
             py::arg("m17_callsign"),
             py::arg("m17_destination"),
             py::arg("ax25_callsign"),
             py::arg("ax25_destination"),
             py::arg("enable_fx25") = false,
             py::arg("enable_il2p") = false)

        .def("set_conversion_mode", &protocol_converter::set_conversion_mode)
        .def("set_m17_callsign", &protocol_converter::set_m17_callsign)
        .def("set_m17_destination", &protocol_converter::set_m17_destination)
        .def("set_ax25_callsign", &protocol_converter::set_ax25_callsign)
        .def("set_ax25_destination", &protocol_converter::set_ax25_destination)
        .def("set_fx25_enabled", &protocol_converter::set_fx25_enabled)
        .def("set_il2p_enabled", &protocol_converter::set_il2p_enabled);

    py::enum_<protocol_converter::conversion_mode_t>(m, "conversion_mode")
        .value("CONVERSION_AUTO", protocol_converter::CONVERSION_AUTO)
        .value("CONVERSION_M17_TO_AX25", protocol_converter::CONVERSION_M17_TO_AX25)
        .value("CONVERSION_AX25_TO_M17", protocol_converter::CONVERSION_AX25_TO_M17);
}

void bind_callsign_mapper(py::module& m)
{
    using callsign_mapper = gr::m17_bridge::callsign_mapper;

    py::class_<callsign_mapper, gr::sync_block, gr::block, gr::basic_block,
               std::shared_ptr<callsign_mapper>>(m, "callsign_mapper")

        .def(py::init(&callsign_mapper::make))

        .def("add_mapping", &callsign_mapper::add_mapping)
        .def("remove_mapping", &callsign_mapper::remove_mapping)
        .def("get_ax25_callsign", &callsign_mapper::get_ax25_callsign)
        .def("get_m17_callsign", &callsign_mapper::get_m17_callsign)
        .def("set_auto_mapping_enabled", &callsign_mapper::set_auto_mapping_enabled)
        .def("is_auto_mapping_enabled", &callsign_mapper::is_auto_mapping_enabled)
        .def("get_mapping_table", &callsign_mapper::get_mapping_table)
        .def("clear_mappings", &callsign_mapper::clear_mappings)
        .def("load_mappings_from_file", &callsign_mapper::load_mappings_from_file)
        .def("save_mappings_to_file", &callsign_mapper::save_mappings_to_file);
}

PYBIND11_MODULE(m17_bridge_swig, m)
{
    m.doc() = "M17 Bridge - Protocol conversion between M17 and AX.25";

    bind_m17_to_ax25(m);
    bind_ax25_to_m17(m);
    bind_protocol_converter(m);
    bind_callsign_mapper(m);
}
