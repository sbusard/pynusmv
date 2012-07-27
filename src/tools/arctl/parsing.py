from pyparsing import Suppress, SkipTo, Forward

from .ast import (Atom, Not, And, Or, Implies, Iff, A, E, F, G, X, U, W)

"""
ARCTL parsing tool.

arctl     := atom | logical | temporal
logical     := '~' arctl | '(' arctl '&' arctl ')' |
               '(' arctl '|' arctl ')' | '(' arctl '->' arctl ')' |
               '(' arctl '<->' arctl ')'
temporal    := 'A' action path | 'E' action path
action      := '<' atom '>'
path        := 'F' arctl | 'G' arctl | 'X' arctl |
               '[' arctl 'U' arctl ']' | '[' arctl 'W' arctl']'
               
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


f = "F" + arctl
f.setParseAction(lambda tokens: F(tokens[1]))

g = "G" + arctl
g.setParseAction(lambda tokens: G(tokens[1]))

x = "X" + arctl
x.setParseAction(lambda tokens: X(tokens[1]))

u = "[" + arctl + "U" + arctl + "]"
u.setParseAction(lambda tokens: U(tokens[1], tokens[3]))

w = "[" + arctl + "W" + arctl + "]"
w.setParseAction(lambda tokens: W(tokens[1], tokens[3]))

path = f | g | x | u | w


a = "A" + action + path
a.setParseAction(lambda tokens: A(tokens[1], tokens[2]))

e = "E" + action + path
e.setParseAction(lambda tokens: E(tokens[1], tokens[2]))

temporal = a | e


arctl << (logical | temporal | atom)