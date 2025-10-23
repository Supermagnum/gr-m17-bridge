# -*- coding: utf-8 -*-
"""
AX.25 to M17 converter hierarchical block.

This module provides a GNU Radio hierarchical block for converting
AX.25 packet radio frames to M17 digital radio frames.
"""

from gnuradio import gr
from . import m17_bridge_swig as m17_bridge_swig


class ax25_to_m17(gr.hier_block2):
    """
    AX.25 to M17 protocol converter hierarchical block.
    
    This block converts AX.25 packet radio frames to M17 digital radio frames.
    It supports callsign mapping, FEC (Forward Error Correction), and APRS integration.
    
    Args:
        callsign (str): Source callsign for M17 frames (default: "N0CALL")
        destination (str): Destination callsign for M17 frames (default: "APRS")
        enable_fec (bool): Enable FX.25 Forward Error Correction (default: False)
    """
    
    def __init__(self, callsign="N0CALL", destination="APRS",
                 enable_fec=False):
        """
        Initialize the AX.25 to M17 converter.
        
        Args:
            callsign (str): Source callsign for M17 frames
            destination (str): Destination callsign for M17 frames
            enable_fec (bool): Enable FX.25 Forward Error Correction
        """
        gr.hier_block2.__init__(
            self, "ax25_to_m17",
            gr.io_signature(1, 1, gr.sizeof_char),
            gr.io_signature(1, 1, gr.sizeof_char)
        )
        
        self.ax25_to_m17 = m17_bridge_swig.ax25_to_m17_make(
            callsign, destination, enable_fec)
        
        self.connect((self, 0), (self.ax25_to_m17, 0))
        self.connect((self.ax25_to_m17, 0), (self, 0))
    
    def set_destination(self, destination):
        """
        Set the destination callsign for M17 frames.
        
        Args:
            destination (str): New destination callsign
        """
        self.ax25_to_m17.set_destination(destination)
    
    def set_callsign(self, callsign):
        """
        Set the source callsign for M17 frames.
        
        Args:
            callsign (str): New source callsign
        """
        self.ax25_to_m17.set_callsign(callsign)
    
    def set_fec_enabled(self, enabled):
        """
        Enable or disable Forward Error Correction.
        
        Args:
            enabled (bool): True to enable FX.25 FEC, False to disable
        """
        self.ax25_to_m17.set_fec_enabled(enabled)
    
