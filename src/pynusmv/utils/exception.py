"""
Provide all the exceptions used in PyNuSMV.

Every particular exception raised by a PyNuSMV function is a sub-class
of the class PyNuSMVError, such that one can catch all PyNuSMV while letting
the other go by catching PyNuSMVError exceptions.
"""

from collections import namedtuple


class PyNuSMVError(Exception):
    """A generic PyNuSMV Error, superclass of all PyNuSMV Errors."""
    pass

class MissingManagerError(PyNuSMVError):
    """Error for missing BDD manager."""
    pass
    
class NuSMVLexerError(PyNuSMVError):
    """An error with NuSMV lexer."""
    pass

class NuSMVNoReadFileError(PyNuSMVError):
    """No SMV model has been read yet."""
    pass
    
class NuSMVModelAlreadyReadError(PyNuSMVError):
    """A model is already read."""
    pass

class NuSMVCannotFlattenError(PyNuSMVError):
    """No SMV model has been read yet."""
    pass

class NuSMVModelAlreadyFlattenedError(PyNuSMVError):
    """The model is already flattened."""
    pass
    
class NuSMVNeedFlatHierarchyError(PyNuSMVError):
    """The model must be flattened."""
    pass
    
class NuSMVModelAlreadyEncodedError(PyNuSMVError):
    """The model is already encoded."""
    pass
    
class NuSMVFlatModelAlreadyBuiltError(PyNuSMVError):
    """The flat model is already built."""
    pass
    
class NuSMVNeedFlatModelError(PyNuSMVError):
    """The model must be simplified."""
    pass
    
class NuSMVModelAlreadyBuiltError(PyNuSMVError):
    """The BDD model is already built."""
    pass
    
class NuSMVNeedVariablesEncodedError(PyNuSMVError):
    """The variables of the model must be encoded."""
    pass
    
class NuSMVInitError(PyNuSMVError):
    """NuSMV initialisation-related exception."""
    pass

class NuSMVParserError(PyNuSMVError):
    """An error occured while parsing a string with NuSMV."""
    pass

class NuSMVTypeCheckingError(PyNuSMVError):
    """An expression is wrongly typed."""
    pass
    
        
Error = namedtuple('Error', ('line', 'token', 'message'))
Error.__str__ = lambda self: "Error at line {}, token '{}': {}".format(
                            *self)
Error.__repr__ = lambda self: "Error at line {}, token '{}': {}".format(
                            *self)    
    
class NuSMVParsingError(PyNuSMVError):
    """
    A parsing exception. Contains several errors accessible through
    the "errors" attribute.
    """
    
    def __init__(self, errors):
        """
        Initialize this exception with errors.
        
        errors -- a tuple of errors. An error is
                  an Error(filename, line number, token, message).
        """
        super().__init__(self)
        self._errors = errors
        
    def __str__(self):
        return str(self._errors)
        
    def __repr__(self):
        return repr(self._errors)
        
        
    @property
    def errors(self):
        return self._errors