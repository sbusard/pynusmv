from pyparsing import Suppress, SkipTo, Forward

from .ast import (Atom, Not, And, Or, Implies, Iff, 
                  AaF, AaG, AaX, AaU, AaW,
                  EaF, EaG, EaX, EaU, EaW)

"""
ARCTL parsing tool.

arctl       := atom | logical | temporal
logical     := '~' arctl | '(' arctl '&' arctl ')' |
               '(' arctl '|' arctl ')' | '(' arctl '->' arctl ')' |
               '(' arctl '<->' arctl ')'
temporal    := 'A' action 'F' arctl | 'A' action 'G' arctl |
               'A' action 'X' arctl | 'A' action '[' arctl 'U' arctl ']' |
               'A' action '[' arctl 'W' arctl']' |
               'E' action 'F' arctl | 'E' action 'G' arctl |
               'E' action 'X' arctl | 'E' action '[' arctl 'U' arctl ']' |
               'E' action '[' arctl 'W' arctl']'
action      := '<' atom '>'
               
atom is defined by any string surrounded by single quotes.


The parser returns a special structure embedding the structure of the parsed
expression, represented using special classes defined in this module.
"""

arctl = Forward()

atom = "'" + SkipTo("'") + "'"
atom.setParseAction(lambda tokens: Atom(tokens[1]))

action = Suppress("<") + atom + Suppress(">")


not_ = "~" + arctl
not_.setParseAction(lambda tokens: Not(tokens[1]))

and_ = "(" + arctl + "&" + arctl + ")"
and_.setParseAction(lambda tokens: And(tokens[1], tokens[3]))

or_ = "(" + arctl + "|" + arctl + ")"
or_.setParseAction(lambda tokens: Or(tokens[1], tokens[3]))

implies = "(" + arctl + "->" + arctl + ")"
implies.setParseAction(lambda tokens: Implies(tokens[1], tokens[3]))

iff = "(" + arctl + "<->" + arctl + ")"
iff.setParseAction(lambda tokens: Iff(tokens[1], tokens[3]))

logical = not_ | and_ | or_ | implies | iff


eaf = "E" + action + "F" + arctl
eaf.setParseAction(lambda tokens: EaF(tokens[1], tokens[3]))

eag = "E" + action + "G" + arctl
eag.setParseAction(lambda tokens: EaG(tokens[1], tokens[3]))

eax = "E" + action + "X" + arctl
eax.setParseAction(lambda tokens: EaX(tokens[1], tokens[3]))

eau = "E" + action + "[" + arctl + "U" + arctl + "]"
eau.setParseAction(lambda tokens: EaU(tokens[1], tokens[3], tokens[5]))

eaw = "E" + action + "[" + arctl + "W" + arctl + "]"
eaw.setParseAction(lambda tokens: EaW(tokens[1], tokens[3], tokens[5]))

aaf = "A" + action + "F" + arctl
aaf.setParseAction(lambda tokens: AaF(tokens[1], tokens[3]))

aag = "A" + action + "G" + arctl
aag.setParseAction(lambda tokens: AaG(tokens[1], tokens[3]))

aax = "A" + action + "X" + arctl
aax.setParseAction(lambda tokens: AaX(tokens[1], tokens[3]))

aau = "A" + action + "[" + arctl + "U" + arctl + "]"
aau.setParseAction(lambda tokens: AaU(tokens[1], tokens[3], tokens[5]))

aaw = "A" + action + "[" + arctl + "W" + arctl + "]"
aaw.setParseAction(lambda tokens: AaW(tokens[1], tokens[3], tokens[5]))

temporal = eaf | eag | eax | eau | eaw | aaf | aag | aax | aau | aaw


arctl << (logical | temporal | atom)