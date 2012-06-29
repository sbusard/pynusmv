"""
Indent module allows to indent a given string.
"""

__value = "   "
__count = 0

def reset():
    """Reset the indentation to 0"""
    __count = 0
    
def inc():
    """Increase the indentation"""
    __count += 1
    
def dec():
    """Decrease the indentation"""
    __count -= 1
    
def indent(s):
    """Indent s with the current indentation"""
    return __value * __count + s
    
def prt(*args, **kwargs):
    """Print indented args. Bind to print."""
    print(__value * __count, *args, **kwargs)