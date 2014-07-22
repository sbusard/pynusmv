from pyparsing import Suppress, SkipTo, Forward, ZeroOrMore, Literal, Group

from .ast import (TrueExp, FalseExp,
                  Atom, Not, And, Or, Implies, Iff, 
                  CEF, CEG, CEX, CEU, CEW, CAF, CAG, CAX, CAU, CAW)
                  
                  
def _left_(clss, tokens):
    """
    Parse tokens and return an AST, assuming left associativity.
    
    Given a list of tokens [v1, op, v2, op, ..., op, vn],
    return res, a hierarchy of instances of clss such that
    res = clss(clss(...clss(v1, v2), ..., vn).
    
    This is a helper function to parse logical operators.
    """
    if len(tokens) == 1:
        return tokens[0]
    else:
        return clss(_left_(clss, tokens[:-2]), tokens[-1])


def _right_(clss, tokens):
    """
    Parse tokens and return an AST, assuming right associativity.
    
    Given a list of tokens [v1, op, v2, op, ..., op, vn],
    return res, a hierarchy of instances of clss such that
    res = clss(v1, clss(v2, ..., vn)...).
    
    This is a helper function to parse logical operators.
    """
    if len(tokens) == 1:
        return tokens[0]
    else:
        return clss(tokens[0], _right_(clss, tokens[2:]))
        
        
def _logicals_(atomic):
    """
    Return a new parser parsing logical expressions on atomics.
    
    This parser recognizes the following grammar, with precedences
    parser := atomic | '~' parser | parser '&' parser | parser '|' parser |
              parser '->' parser | parser '<->' parser
              
    Returned AST uses .ast package's classes.
    """
    parser = Forward()

    atom = (atomic | Suppress("(") + parser + Suppress(")"))

    notstrict = "~" + atom
    notstrict.setParseAction(lambda tokens: Not(tokens[1]))
    not_ = notstrict | atom
    and_ = not_ + ZeroOrMore("&" + not_)
    and_.setParseAction(lambda tokens: _left_(And, tokens))
    or_ = and_ + ZeroOrMore("|" + and_)
    or_.setParseAction(lambda tokens: _left_(Or, tokens))
    implies = ZeroOrMore(or_ + "->") + or_
    implies.setParseAction(lambda tokens: _right_(Implies, tokens))
    iff = implies + ZeroOrMore("<->" + implies)
    iff.setParseAction(lambda tokens: _left_(Iff, tokens))

    parser <<= iff
    
    return parser
        

"""
ATL parsing tool.

_atl       := _atom | _logical | _strategic
_logical    := '~' __atl | '(' _logical ')' | __atl '&' __atl |
               __atl '|' __atl | __atl '->' __atl | __atl '<->' __atl
_strategic  := '<' _group '>' 'F' __atl | '<' _group '>' 'G' __atl |
               '<' _group '>' 'X' __atl |
               '<' _group '>' '[' __atl 'U' __atl ']' |
               '<' _group '>' '[' __atl 'W' __atl ']' |
               '[' _group ']' 'F' __atl | '[' _group ']' 'G' __atl |
               '[' _group ']' 'X' __atl |
               '[' _group ']' '[' __atl 'U' __atl ']' |
               '[' _group ']' '[' __atl 'W' __atl ']'
_agent      := _atom
_group      := _agent | _agent ',' _group

               
_atom is defined by any string surrounded by single quotes.

_logical are specified with usual precedences and associativity,
i.e.
prec : ~, &, |, ->, <->
assoc : &, |, <-> left assoc, ->, ~ right assoc


The parser returns a structure embedding the structure of the parsed
expression, represented using AST classes of .ast module.
"""

__atl = None

def parseATL(spec):
    """Parse the spec and return the list of possible ASTs."""
    global __atl
    if __atl is None:
        true = Literal("True")
        true.setParseAction(lambda tokens: TrueExp())
        false = Literal("False")
        false.setParseAction(lambda tokens: FalseExp())
        
        atom = "'" + SkipTo("'") + "'"
        atom.setParseAction(lambda tokens: Atom(tokens[1]))
        
        agent = atom
        group = Group(ZeroOrMore(agent + Suppress(",")) + agent)
        
        proposition = true | false | atom
        
        __atl = Forward()

        notproposition = "~" + proposition
        notproposition.setParseAction(lambda tokens: Not(tokens[1]))
        formula = (proposition | notproposition |
                   Suppress("(") + __atl + Suppress(")"))

        logical = Forward()
        
        
        cax = Literal("[") + group + "]" + "X" + logical
        cax.setParseAction(lambda tokens: CAX(tokens[1], tokens[4]))
        cex = Literal("<") + group + ">" + "X" + logical
        cex.setParseAction(lambda tokens: CEX(tokens[1], tokens[4]))
         
        caf = Literal("[") + group + "]" + "F" + logical
        caf.setParseAction(lambda tokens: CAF(tokens[1], tokens[4]))
        cef = Literal("<") + group + ">" + "F" + logical
        cef.setParseAction(lambda tokens: CEF(tokens[1], tokens[4]))
         
        cag = Literal("[") + group + "]" + "G" + logical
        cag.setParseAction(lambda tokens: CAG(tokens[1], tokens[4]))
        ceg = Literal("<") + group + ">" + "G" + logical           
        ceg.setParseAction(lambda tokens: CEG(tokens[1], tokens[4]))
         
        cau = Literal("[") + group + "]" + "[" + __atl + "U" + __atl + "]"
        cau.setParseAction(lambda tokens: CAU(tokens[1], tokens[4], tokens[6]))
        ceu = Literal("<") + group + ">" + "[" + __atl + "U" + __atl + "]"   
        ceu.setParseAction(lambda tokens: CEU(tokens[1], tokens[4], tokens[6]))
                                                                
        caw = Literal("[") + group + "]" + "[" + __atl + "W" + __atl + "]"   
        caw.setParseAction(lambda tokens: CAW(tokens[1], tokens[4], tokens[6]))
        cew = Literal("<") + group + ">" + "[" + __atl + "W" + __atl + "]"   
        cew.setParseAction(lambda tokens: CEW(tokens[1], tokens[4], tokens[6]))

        strategic = (cax | cex | caf | cef | cag | ceg | cau | ceu | caw | cew)
        
        logical <<= (formula | strategic)

        __atl <<= (_logicals_(logical))
    
    return __atl.parseString(spec, parseAll = True)