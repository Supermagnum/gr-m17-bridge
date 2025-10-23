# -*- coding: utf-8 -*-
"""
Bidirectional protocol converter between M17 and AX.25.

This module provides a GNU Radio hierarchical block for bidirectional
conversion between M17 digital radio and AX.25 packet radio protocols.
"""

from gnuradio import gr
from . import m17_bridge_swig as m17_bridge_swig


class protocol_converter(gr.hier_block2):
    """
    Bidirectional protocol converter between M17 and AX.25.
    
    This block provides bidirectional conversion between M17 digital radio
    and AX.25 packet radio protocols. It supports callsign mapping, FEC,
    and modern protocols like IL2P.
    
    Args:
        m17_callsign (str): M17 source callsign (default: "N0CALL")
        m17_destination (str): M17 destination callsign (default: "APRS")
        ax25_callsign (str): AX.25 source callsign (default: "N0CALL")
        ax25_destination (str): AX.25 destination callsign (default: "APRS")
        enable_fx25 (bool): Enable FX.25 Forward Error Correction (default: False)
        enable_il2p (bool): Enable IL2P modern protocol (default: False)
    """
    
    def __init__(self, m17_callsign="N0CALL", m17_destination="APRS",
                 ax25_callsign="N0CALL", ax25_destination="APRS",
                 enable_fx25=False, enable_il2p=False):
        """
        Initialize the bidirectional protocol converter.
        
        Args:
            m17_callsign (str): M17 source callsign
            m17_destination (str): M17 destination callsign
            ax25_callsign (str): AX.25 source callsign
            ax25_destination (str): AX.25 destination callsign
            enable_fx25 (bool): Enable FX.25 Forward Error Correction
            enable_il2p (bool): Enable IL2P modern protocol
        """
        gr.hier_block2.__init__(
            self, "protocol_converter",
            gr.io_signature(2, 2, gr.sizeof_char),
            gr.io_signature(2, 2, gr.sizeof_char)
        )
        
        self.protocol_converter = m17_bridge_swig.protocol_converter_make(
            m17_callsign, m17_destination, ax25_callsign, ax25_destination,
            enable_fx25, enable_il2p)
        
        # M17 input/output
        self.connect((self, 0), (self.protocol_converter, 0))
        self.connect((self.protocol_converter, 0), (self, 0))
        
        # AX.25 input/output
        self.connect((self, 1), (self.protocol_converter, 1))
        self.connect((self.protocol_converter, 1), (self, 1))
    
    def set_conversion_mode(self, mode):
        """
        Set the conversion mode.
        
        Args:
            mode (int): Conversion mode (AUTO, M17_TO_AX25, AX25_TO_M17)
        """
        self.protocol_converter.set_conversion_mode(mode)
    
    def set_m17_callsign(self, callsign):
        """
        Set the M17 source callsign.
        
        Args:
            callsign (str): New M17 source callsign
        """
        self.protocol_converter.set_m17_callsign(callsign)
    
    def set_m17_destination(self, destination):
        """
        Set the M17 destination callsign.
        
        Args:
            destination (str): New M17 destination callsign
        """
        self.protocol_converter.set_m17_destination(destination)
    
    def set_ax25_callsign(self, callsign):
        """
        Set the AX.25 source callsign.
        
        Args:
            callsign (str): New AX.25 source callsign
        """
        self.protocol_converter.set_ax25_callsign(callsign)
    
    def set_ax25_destination(self, destination):
        """
        Set the AX.25 destination callsign.
        
        Args:
            destination (str): New AX.25 destination callsign
        """
        self.protocol_converter.set_ax25_destination(destination)
    
    def set_fx25_enabled(self, enabled):
        """
        Enable or disable FX.25 Forward Error Correction.
        
        Args:
            enabled (bool): True to enable FX.25 FEC, False to disable
        """
        self.protocol_converter.set_fx25_enabled(enabled)
    
    def set_il2p_enabled(self, enabled):
        """
        Enable or disable IL2P modern protocol.
        
        Args:
            enabled (bool): True to enable IL2P, False to disable
        """
        self.protocol_converter.set_il2p_enabled(enabled)
    
