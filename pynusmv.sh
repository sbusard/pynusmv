#!/bin/sh
# Start Python with pynusmv library

# PyNuSMV package path. Set it right before using the script
PYNUSMV=/path/to/pynusmv/package

export PYTHONPATH=${PYNUSMV}/src
export DYLD_LIBRARY_PATH=${PYNUSMV}/src/lib
export LD_LIBRARY_PATH=${PYNUSMV}/src/lib

python3 -i -c "import pynusmv"