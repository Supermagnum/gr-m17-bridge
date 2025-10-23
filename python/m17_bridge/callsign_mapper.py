# -*- coding: utf-8 -*-
"""
Callsign mapper for M17 and AX.25 protocols
"""

from gnuradio import gr
from . import m17_bridge_swig as m17_bridge_swig


class callsign_mapper(gr.hier_block2):
    """
    Callsign mapping between M17 and AX.25 protocols
    """
    
    def __init__(self):
        gr.hier_block2.__init__(
            self, "callsign_mapper",
            gr.io_signature(1, 1, gr.sizeof_char),
            gr.io_signature(1, 1, gr.sizeof_char)
        )
        
        self.callsign_mapper = m17_bridge_swig.callsign_mapper_make()
        
        self.connect((self, 0), (self.callsign_mapper, 0))
        self.connect((self.callsign_mapper, 0), (self, 0))
    
    def add_mapping(self, m17_callsign, ax25_callsign):
        """Add a callsign mapping"""
        self.callsign_mapper.add_mapping(m17_callsign, ax25_callsign)
    
    def remove_mapping(self, m17_callsign):
        """Remove a callsign mapping"""
        self.callsign_mapper.remove_mapping(m17_callsign)
    
    def get_ax25_callsign(self, m17_callsign):
        """Get AX.25 callsign for M17 callsign"""
        return self.callsign_mapper.get_ax25_callsign(m17_callsign)
    
    def get_m17_callsign(self, ax25_callsign):
        """Get M17 callsign for AX.25 callsign"""
        return self.callsign_mapper.get_m17_callsign(ax25_callsign)
    
    def set_auto_mapping_enabled(self, enabled):
        """Enable or disable auto-mapping"""
        self.callsign_mapper.set_auto_mapping_enabled(enabled)
    
    def is_auto_mapping_enabled(self):
        """Check if auto-mapping is enabled"""
        return self.callsign_mapper.is_auto_mapping_enabled()
    
    def get_mapping_table(self):
        """Get the current mapping table"""
        return self.callsign_mapper.get_mapping_table()
    
    def clear_mappings(self):
        """Clear all mappings"""
        self.callsign_mapper.clear_mappings()
    
    def load_mappings_from_file(self, filename):
        """Load mappings from file"""
        self.callsign_mapper.load_mappings_from_file(filename)
    
    def save_mappings_to_file(self, filename):
        """Save mappings to file"""
        self.callsign_mapper.save_mappings_to_file(filename)
