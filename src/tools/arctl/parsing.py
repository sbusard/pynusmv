from pyparsing import Suppress, SkipTo, Forward

from .ast import (Atom, Not, And, Or, Implies, Iff, 
                  AaF, AaG, AaX, AaU, AaW,
                  EaF, EaG, EaX, EaU, EaW)

"""
ARCTL parsing tool.

_arctl       := _atom | _logical | _temporal
_logical     := '~' _arctl | '(' _arctl '&' _arctl ')' |
               '(' _arctl '|' _arctl ')' | '(' _arctl '->' _arctl ')' |
               '(' _arctl '<->' _arctl ')'
_temporal    := 'A' _action 'F' _arctl | 'A' _action 'G' _arctl |
               'A' _action 'X' _arctl | 'A' _action '[' _arctl 'U' _arctl ']' |
               'A' _action '[' _arctl 'W' _arctl']' |
               'E' _action 'F' _arctl | 'E' _action 'G' _arctl |
               'E' _action 'X' _arctl | 'E' _action '[' _arctl 'U' _arctl ']' |
               'E' _action '[' _arctl 'W' _arctl']'
_action      := '<' _atom '>'
               
_atom is defined by any string surrounded by single quotes.


The parser returns a special structure embedding the structure of the parsed
expression, represented using special classes defined in this module.
"""

def parseArctl(spec):
    """Parse the spec and return its AST."""
    
    _arctl = Forward()

    _atom = "'" + SkipTo("'") + "'"
    _atom.setParseAction(lambda tokens: Atom(tokens[1]))

    _action = Suppress("<") + _atom + Suppress(">")


    _not = "~" + _arctl
    _not.setParseAction(lambda tokens: Not(tokens[1]))

    _and = "(" + _arctl + "&" + _arctl + ")"
    _and.setParseAction(lambda tokens: And(tokens[1], tokens[3]))

    _or = "(" + _arctl + "|" + _arctl + ")"
    _or.setParseAction(lambda tokens: Or(tokens[1], tokens[3]))

    _implies = "(" + _arctl + "->" + _arctl + ")"
    _implies.setParseAction(lambda tokens: Implies(tokens[1], tokens[3]))

    _iff = "(" + _arctl + "<->" + _arctl + ")"
    _iff.setParseAction(lambda tokens: Iff(tokens[1], tokens[3]))

    _logical = _not | _and | _or | _implies | _iff


    _eaf = "E" + _action + "F" + _arctl
    _eaf.setParseAction(lambda tokens: EaF(tokens[1], tokens[3]))

    _eag = "E" + _action + "G" + _arctl
    _eag.setParseAction(lambda tokens: EaG(tokens[1], tokens[3]))

    _eax = "E" + _action + "X" + _arctl
    _eax.setParseAction(lambda tokens: EaX(tokens[1], tokens[3]))

    _eau = "E" + _action + "[" + _arctl + "U" + _arctl + "]"
    _eau.setParseAction(lambda tokens: EaU(tokens[1], tokens[3], tokens[5]))

    _eaw = "E" + _action + "[" + _arctl + "W" + _arctl + "]"
    _eaw.setParseAction(lambda tokens: EaW(tokens[1], tokens[3], tokens[5]))

    _aaf = "A" + _action + "F" + _arctl
    _aaf.setParseAction(lambda tokens: AaF(tokens[1], tokens[3]))

    _aag = "A" + _action + "G" + _arctl
    _aag.setParseAction(lambda tokens: AaG(tokens[1], tokens[3]))

    _aax = "A" + _action + "X" + _arctl
    _aax.setParseAction(lambda tokens: AaX(tokens[1], tokens[3]))

    _aau = "A" + _action + "[" + _arctl + "U" + _arctl + "]"
    _aau.setParseAction(lambda tokens: AaU(tokens[1], tokens[3], tokens[5]))

    _aaw = "A" + _action + "[" + _arctl + "W" + _arctl + "]"
    _aaw.setParseAction(lambda tokens: AaW(tokens[1], tokens[3], tokens[5]))

    _temporal = (_eaf | _eag | _eax | _eau | _eaw |
                 _aaf | _aag | _aax | _aau | _aaw)

    _arctl << (_logical | _temporal | _atom)
    
    return _arctl.parseString(spec, parseAll = True)