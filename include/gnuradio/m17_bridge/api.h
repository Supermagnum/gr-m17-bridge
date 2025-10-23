/* -*- c++ -*- */
/*
 * Copyright 2024 M17 Bridge Project
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_M17_BRIDGE_API_H
#define INCLUDED_M17_BRIDGE_API_H

#include <gnuradio/attributes.h>

#ifdef gnuradio_m17_bridge_EXPORTS
#define M17_BRIDGE_API __GR_ATTR_EXPORT
#else
#define M17_BRIDGE_API __GR_ATTR_IMPORT
#endif

#endif /* INCLUDED_M17_BRIDGE_API_H */
