#!/bin/bash
# Quick fuzzing test for M17 Bridge module
# Runs a 10-minute fuzzing session for quick validation

echo "M17 Bridge Quick Fuzzing Test"
echo "=============================="

# Run the main fuzzing script with ultra-quick mode
exec ./fuzz-testing.sh ultra-quick
