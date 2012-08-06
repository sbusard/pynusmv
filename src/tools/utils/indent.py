"""
Indent module allows to indent a given string.
"""

import sys

__value = "   "
__count = 0

def reset():
    """Reset the indentation to 0"""
    global __count
    __count = 0
    
def inc():
    """Increase the indentation"""
    global __count
    __count += 1
    
def dec():
    """Decrease the indentation"""
    global __count
    __count -= 1
    
def indent(s):
    """Indent s with the current indentation"""
    global __value
    global __count
    return __value * __count + s