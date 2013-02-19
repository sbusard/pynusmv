"""
The :mod:`pynusmv.parser` module provides functions to parse strings
and return corresponding ASTs.

"""


__all__ = ['parse_simple_expression', 'parse_next_expression',
           'parse_identifier']


from .exception import NuSMVParsingError, Error
     
from .nusmv.parser import parser as nsparser
from .nusmv.node import node as nsnode


def parse_simple_expression(expression):
    """
    Parse a simple expression.
    
    :param string expression: the expression to parse
    :raise: a :exc:`NuSMVParsingError
            <pynusmv.exception.NuSMVParsingError>`
            if a parsing error occurs
    
    .. warning:: Returned value is a SWIG wrapper for the NuSMV node_ptr.
       It is the responsibility of the caller to manage it.
    
    """
    node, err = nsparser.ReadSimpExprFromString(expression)
    if err:
        errlist = []
        errors = nsparser.Parser_get_syntax_errors_list()
        while errors is not None:
            error = nsnode.car(errors)
            err = nsparser.Parser_get_syntax_error(error)
            errlist.append(Error(*err[1:]))
            errors = nsnode.cdr(errors)
        raise NuSMVParsingError(tuple(errlist))
    else:
        return nsnode.car(node) # Get rid of the top SIMPWFF node
        
        
def parse_next_expression(expression):
    """
    Parse a "next" expression.
    
    :param string expression: the expression to parse
    :raise: a :exc:`NuSMVParsingError
            <pynusmv.exception.NuSMVParsingError>`
            if a parsing error occurs
    
    .. warning:: Returned value is a SWIG wrapper for the NuSMV node_ptr.
       It is the responsibility of the caller to manage it.
    
    """
    node, err = nsparser.ReadNextExprFromString(expression)
    if err:
        errlist = []
        errors = nsparser.Parser_get_syntax_errors_list()
        while errors is not None:
            error = nsnode.car(errors)
            err = nsparser.Parser_get_syntax_error(error)
            errlist.append(Error(*err[1:]))
            errors = nsnode.cdr(errors)
        raise NuSMVParsingError(tuple(errlist))
    else:
        return nsnode.car(node) # Get rid of the top NEXTWFF node
        
        
def parse_identifier(expression):
    """
    Parse an identifier
    
    :param string expression: the identifier to parse
    :raise: a :exc:`NuSMVParsingError
            <pynusmv.exception.NuSMVParsingError>`
            if a parsing error occurs
    
    .. warning:: Returned value is a SWIG wrapper for the NuSMV node_ptr.
       It is the responsibility of the caller to manage it.
    
    """
    node, err = nsparser.ReadIdentifierExprFromString(expression)
    if err:
        errlist = []
        errors = nsparser.Parser_get_syntax_errors_list()
        while errors is not None:
            error = nsnode.car(errors)
            err = nsparser.Parser_get_syntax_error(error)
            errlist.append(Error(*err[1:]))
            errors = nsnode.cdr(errors)
        raise NuSMVParsingError(tuple(errlist))
    else:
        return nsnode.car(node) # Get rid of the top COMPID node