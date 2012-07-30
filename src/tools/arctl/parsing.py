from pyparsing import Suppress, SkipTo, Forward, opAssoc, operatorPrecedence

from .ast import (Atom, Not, And, Or, Implies, Iff, 
                  AaF, AaG, AaX, AaU, AaW,
                  EaF, EaG, EaX, EaU, EaW)

"""
ARCTL parsing tool.

_arctl       := _atom | _logical | _temporal
_logical     := '~' _arctl | '(' _arctl '&' _arctl ')' |
               '(' _arctl '|' _arctl ')' | '(' _arctl '->' _arctl ')' |
               '(' _arctl '<->' _arctl ')'
_temporal    := 'A' '<' _action '>' 'F' _arctl |
                'A' '<' _action '>' 'G' _arctl |
                'A' '<' _action '>' 'X' _arctl |
                'A' '<' _action '>' '[' _arctl 'U' _arctl ']' |
                'A' '<' _action '>' '[' _arctl 'W' _arctl']' |
                'E' '<' _action '>' 'F' _arctl |
                'E' '<' _action '>' 'G' _arctl |
                'E' '<' _action '>' 'X' _arctl |
                'E' '<' _action '>' '[' _arctl 'U' _arctl ']' |
                'E' '<' _action '>' '[' _arctl 'W' _arctl']'
_action      := _atom | '(' _action ')' | '~' _action | _action '&' _action |
                _action '|' _action | _action '->' _action |
                _action '<->' _action
               
_atom is defined by any string surrounded by single quotes.

_action is specified with usual precedences and associativity, i.e.
prec : ~, &, |, ->, <->
assoc : &, |, <-> left assoc, -> right assoc


The parser returns a special structure embedding the structure of the parsed
expression, represented using special classes defined in this module.
"""

def _ast_(clss, tokens):
    """
    Parser tokens and return an AST.
    
    Given a list of tokens [v1, op, v2, op, ..., op, vn],
    return res, a hierarchy of instances of clss such that
    res = clss(clss(...clss(v1, v2), ..., vn).
    
    This is an helper function to parse logical operators.
    """
    if len(tokens) == 1:
        return tokens[0]
    else:
        return clss(_ast_(clss, tokens[:-2]), tokens[-1])

def parseArctl(spec):
    """Parse the spec and return its AST."""
    
    _arctl = Forward()


    _atom = "'" + SkipTo("'") + "'"
    _atom.setParseAction(lambda tokens: Atom(tokens[1]))


    _action = Forward()
    _acoperators = (
        ("~", 1, opAssoc.RIGHT, lambda tokens: Not(tokens[0][1])),
        ("&", 2, opAssoc.LEFT, lambda tokens: _ast_(And, tokens[0])),
        ("|", 2, opAssoc.LEFT, lambda tokens: _ast_(Or, tokens[0])),
        ("->", 2, opAssoc.RIGHT, lambda tokens: Implies(tokens[0][0], tokens[0][2])),
        ("<->", 2, opAssoc.LEFT, lambda tokens: _ast_(And, tokens[0]))
    )
    _action << (Suppress("(") + _action + Suppress(")") |
                operatorPrecedence(_atom, _acoperators) |
                _atom)


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

    _logical = (_not | _and | _or | _implies | _iff)


    _eaf = "E" + Suppress("<") + _action + Suppress(">") + "F" + _arctl
    _eaf.setParseAction(lambda tokens: EaF(tokens[1], tokens[3]))

    _eag = "E" + Suppress("<") + _action + Suppress(">") + "G" + _arctl
    _eag.setParseAction(lambda tokens: EaG(tokens[1], tokens[3]))

    _eax = "E" + Suppress("<") + _action + Suppress(">") + "X" + _arctl
    _eax.setParseAction(lambda tokens: EaX(tokens[1], tokens[3]))

    _eau = ("E" + Suppress("<") + _action + Suppress(">") +
            "[" + _arctl + "U" + _arctl + "]")
    _eau.setParseAction(lambda tokens: EaU(tokens[1], tokens[3], tokens[5]))

    _eaw = ("E" + Suppress("<") + _action + Suppress(">") +
            "[" + _arctl + "W" + _arctl + "]")
    _eaw.setParseAction(lambda tokens: EaW(tokens[1], tokens[3], tokens[5]))

    _aaf = "A" + Suppress("<") + _action + Suppress(">") + "F" + _arctl
    _aaf.setParseAction(lambda tokens: AaF(tokens[1], tokens[3]))

    _aag = "A" + Suppress("<") + _action + Suppress(">") + "G" + _arctl
    _aag.setParseAction(lambda tokens: AaG(tokens[1], tokens[3]))

    _aax = "A" + Suppress("<") + _action + Suppress(">") + "X" + _arctl
    _aax.setParseAction(lambda tokens: AaX(tokens[1], tokens[3]))

    _aau = ("A" + Suppress("<") + _action + Suppress(">") +
            "[" + _arctl + "U" + _arctl + "]")
    _aau.setParseAction(lambda tokens: AaU(tokens[1], tokens[3], tokens[5]))

    _aaw = ("A" + Suppress("<") + _action + Suppress(">") +
            "[" + _arctl + "W" + _arctl + "]")
    _aaw.setParseAction(lambda tokens: AaW(tokens[1], tokens[3], tokens[5]))

    _temporal = (_eaf | _eag | _eax | _eau | _eaw |
                 _aaf | _aag | _aax | _aau | _aaw)


    _arctl << (_logical | _temporal | _atom)
    
    return _arctl.parseString(spec, parseAll = True)