# -*- coding: utf-8 -*-
"""
M17 to AX.25 converter hierarchical block.

This module provides a GNU Radio hierarchical block for converting
M17 digital radio frames to AX.25 packet radio frames.
"""

from gnuradio import gr
from . import m17_bridge_swig as m17_bridge_swig


class m17_to_ax25(gr.hier_block2):
    """
    M17 to AX.25 protocol converter hierarchical block.
    
    This block converts M17 digital radio frames to AX.25 packet radio frames.
    It supports callsign mapping, FEC (Forward Error Correction), and APRS integration.
    
    Args:
        callsign (str): Source callsign for AX.25 frames (default: "N0CALL")
        destination (str): Destination callsign for AX.25 frames (default: "APRS")
        enable_fec (bool): Enable FX.25 Forward Error Correction (default: False)
    """
    
    def __init__(self, callsign="N0CALL", destination="APRS", 
                 enable_fec=False):
        """
        Initialize the M17 to AX.25 converter.
        
        Args:
            callsign (str): Source callsign for AX.25 frames
            destination (str): Destination callsign for AX.25 frames
            enable_fec (bool): Enable FX.25 Forward Error Correction
        """
        gr.hier_block2.__init__(
            self, "m17_to_ax25",
            gr.io_signature(1, 1, gr.sizeof_char),
            gr.io_signature(1, 1, gr.sizeof_char)
        )
        
        self.m17_to_ax25 = m17_bridge_swig.m17_to_ax25_make(
            callsign, destination, enable_fec)
        
        self.connect((self, 0), (self.m17_to_ax25, 0))
        self.connect((self.m17_to_ax25, 0), (self, 0))
    
    def set_destination(self, destination):
        """
        Set the destination callsign for AX.25 frames.
        
        Args:
            destination (str): New destination callsign
        """
        self.m17_to_ax25.set_destination(destination)
    
    def set_callsign(self, callsign):
        """
        Set the source callsign for AX.25 frames.
        
        Args:
            callsign (str): New source callsign
        """
        self.m17_to_ax25.set_callsign(callsign)
    
    def set_fec_enabled(self, enabled):
        """
        Enable or disable Forward Error Correction.
        
        Args:
            enabled (bool): True to enable FX.25 FEC, False to disable
        """
        self.m17_to_ax25.set_fec_enabled(enabled)
    
