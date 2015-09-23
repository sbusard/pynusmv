"""
The :mod:`pynusmv.parser` module provides functions to parse strings
and return corresponding ASTs. This module includes three types of
functionalities:

* :func:`parse_simple_expression`, :func:`parse_next_expression` and
  :func:`parse_identifier` are direct access to NuSMV parser, returning
  wrappers to NuSMV internal data structures representing the language AST.
* :data:`identifier`, :data:`simple_expression`, :data:`constant`,
  :data:`next_expression`, :data:`type_identifier`, :data:`var_section`,
  :data:`ivar_section`, :data:`frozenvar_section`, :data:`define_section`,
  :data:`constants_section`, :data:`assign_constraint`,
  :data:`init_constraint`, :data:`trans_constraint`, :data:`invar_constraint`,
  :data:`fairness_constraint`, :data:`justice_constraint`,
  :data:`compassion_constraint`, :data:`module` and :data:`model` are pyparsing
  parsers parsing the corresponding elements of a NuSMV model (see NuSMV
  documentation for more information on these elements of the language).
* :func:`parseAllString` is a helper function to directly return ASTs for
  strings parsed with pyparsing parsers.

"""


__all__ = [
    'parse_simple_expression',
    'parse_next_expression',
    'parse_identifier']


from .exception import NuSMVParsingError, _Error

from .nusmv.parser import parser as nsparser
from .nusmv.node import node as nsnode



def parse_simple_expression(expression, context=None):
    """
    Parse the simple `expression` in `context`.

    :param string expression: the expression to parse
    :param string context: the context, if not None
    :raise: a :exc:`NuSMVParsingError
            <pynusmv.exception.NuSMVParsingError>`
            if a parsing error occurs

    .. warning:: Returned value is a SWIG wrapper for the NuSMV node_ptr.
       It is the responsibility of the caller to manage it.

    """
    context = "" if context is None else " IN " + context
    node, err = nsparser.ReadSimpExprFromString(expression + context)
    if err:
        errors = nsparser.Parser_get_syntax_errors_list()
        raise NuSMVParsingError.from_nusmv_errors_list(errors)
    else:
        node = nsnode.car(node)  # Get rid of the top SIMPWFF node
        if node.type is nsparser.CONTEXT and nsnode.car(node) is None:
            # Get rid of the top empty context if any
            return nsnode.cdr(node)
        else:
            return node


def parse_next_expression(expression, context=None):
    """
    Parse the "next" `expression` in `context`.

    :param string expression: the expression to parse
    :param string context: the context, if not None
    :raise: a :exc:`NuSMVParsingError
            <pynusmv.exception.NuSMVParsingError>`
            if a parsing error occurs

    .. warning:: Returned value is a SWIG wrapper for the NuSMV node_ptr.
       It is the responsibility of the caller to manage it.

    """
    context = "" if context is None else " IN " + context
    node, err = nsparser.ReadNextExprFromString(expression + context)
    if err:
        errors = nsparser.Parser_get_syntax_errors_list()
        raise NuSMVParsingError.from_nusmv_errors_list(errors)
    else:
        node = nsnode.car(node)  # Get rid of the top NEXTWFF node
        if node.type is nsparser.CONTEXT and nsnode.car(node) is None:
            # Get rid of the top empty context if any
            return nsnode.cdr(node)
        else:
            return node
        


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
        errors = nsparser.Parser_get_syntax_errors_list()
        raise NuSMVParsingError.from_nusmv_errors_list(errors)
    else:
        node = nsnode.car(node)  # Get rid of the top COMPID node
        if node.type is nsparser.CONTEXT and nsnode.car(node) is None:
            # Get rid of the top empty context if any
            return nsnode.cdr(node)
        else:
            return node