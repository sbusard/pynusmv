from pyparsing import Suppress, SkipTo, Forward, ZeroOrMore, Literal, Group

from .ast import (TrueExp, FalseExp, Init, Reachable,
                  Atom, Not, And, Or, Implies, Iff, 
                  AF, AG, AX, AU, AW, EF, EG, EX, EU, EW,
                  nK, nE, nD, nC, K, E, D, C,
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

    parser << iff
    
    return parser
        

"""
ARCTL parsing tool.

_atlk       := _atom | _logical | _temporal | _epistemic | _strategic
_logical    := '~' _atlk | '(' _logical ')' | _atlk '&' _atlk |
               _atlk '|' _atlk | _atlk '->' _atlk | _atlk '<->' _atlk
_temporal   := 'A' 'F' _atlk | 'A' 'G' _atlk | 'A' 'X' _atlk |
               'A' '[' _atlk 'U' _atlk ']' | 'A' '[' _atlk 'W' _atlk ']' |
               'E' 'F' _atlk | 'E' 'G' _atlk | 'E' 'X' _atlk |
               'E' '[' _atlk 'U' _atlk ']' | 'E' '[' _atlk 'W' _atlk ']'
_epistemic  := 'nK' '<' _agent '>' _atlk | 'nE' '<' _group '>' _atlk |
               'nD' '<' _group '>' _atlk | 'nC' '<' _group '>' _atlk |
               'K' '<' _agent '>' _atlk | 'E' '<' _group '>' _atlk |
               'D' '<' _group '>' _atlk | 'C' '<' _group '>' _atlk
_strategic  := '<' _group '>' 'F' _atlk | '<' _group '>' 'G' _atlk |
               '<' _group '>' 'X' _atlk |
               '<' _group '>' '[' _atlk 'U' _atlk ']' |
               '<' _group '>' '[' _atlk 'W' _atlk ']' |
               '[' _group ']' 'F' _atlk | '[' _group ']' 'G' _atlk |
               '[' _group ']' 'X' _atlk |
               '[' _group ']' '[' _atlk 'U' _atlk ']' |
               '[' _group ']' '[' _atlk 'W' _atlk ']'
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

_atlk = None

def parseATLK(spec):
    """Parse the spec and return the list of possible ASTs."""
    global _atlk
    if _atlk is None:
        true = Literal("True")
        true.setParseAction(lambda tokens: TrueExp())
        false = Literal("False")
        false.setParseAction(lambda tokens: FalseExp())
        init = Literal("Init")
        init.setParseAction(lambda tokens: Init())
        reachable = Literal("Reachable")
        reachable.setParseAction(lambda tokens: Reachable())
        
        atom = "'" + SkipTo("'") + "'"
        atom.setParseAction(lambda tokens: Atom(tokens[1]))
        
        agent = atom
        group = Group(ZeroOrMore(agent + Suppress(",")) + agent)
        
        proposition = true | false | init | reachable | atom
        
        _atlk = Forward()

        notproposition = "~" + proposition
        notproposition.setParseAction(lambda tokens: Not(tokens[1]))
        formula = (proposition | notproposition |
                   Suppress("(") + _atlk + Suppress(")"))

        logical = Forward()
        

        ex = Literal("E") + "X" + logical
        ex.setParseAction(lambda tokens: EX(tokens[2]))
        ax = Literal("A") + "X" + logical
        ax.setParseAction(lambda tokens: AX(tokens[2]))
         
        ef = Literal("E") + "F" + logical
        ef.setParseAction(lambda tokens: EF(tokens[2]))
        af = Literal("A") + "F" + logical
        af.setParseAction(lambda tokens: AF(tokens[2]))
         
        eg = Literal("E") + "G" + logical
        eg.setParseAction(lambda tokens: EG(tokens[2]))
        ag = Literal("A") + "G" + logical           
        ag.setParseAction(lambda tokens: AG(tokens[2]))
         
        eu = Literal("E") + "[" + _atlk + "U" + _atlk + "]"
        eu.setParseAction(lambda tokens: EU(tokens[2], tokens[4]))
        au = Literal("A") + "[" + _atlk + "U" + _atlk + "]"   
        au.setParseAction(lambda tokens: AU(tokens[2], tokens[4]))
                                                                
        ew = Literal("E") + "[" + _atlk + "W" + _atlk + "]"   
        ew.setParseAction(lambda tokens: EW(tokens[2], tokens[4]))
        aw = Literal("A") + "[" + _atlk + "W" + _atlk + "]"   
        aw.setParseAction(lambda tokens: AW(tokens[2], tokens[4]))

        temporal = (ex | ax | ef | af | eg | ag | eu | au | ew | aw)
        
        
        nk = Literal("nK") + "<" + agent + ">" + logical
        nk.setParseAction(lambda tokens: nK(tokens[2], tokens[4]))
        k = Literal("K") + "<" + agent + ">" + logical
        k.setParseAction(lambda tokens: K(tokens[2], tokens[4]))
        
        ne = Literal("nE") + "<" + group + ">" + logical
        ne.setParseAction(lambda tokens: nE(list(tokens[2]), tokens[4]))
        e = Literal("E") + "<" + group + ">" + logical
        e.setParseAction(lambda tokens: E(list(tokens[2]), tokens[4]))
        
        nd = Literal("nD") + "<" + group + ">" + logical
        nd.setParseAction(lambda tokens: nD(list(tokens[2]), tokens[4]))
        d = Literal("D") + "<" + group + ">" + logical
        d.setParseAction(lambda tokens: D(list(tokens[2]), tokens[4]))
        
        nc = Literal("nC") + "<" + group + ">" + logical
        nc.setParseAction(lambda tokens: nC(list(tokens[2]), tokens[4]))
        c = Literal("C") + "<" + group + ">" + logical
        c.setParseAction(lambda tokens: C(list(tokens[2]), tokens[4]))
        
        epistemic = (nk | k | ne | e | nd | d | nc | c)
        
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
         
        cau = Literal("[") + group + "]" + "[" + _atlk + "U" + _atlk + "]"
        cau.setParseAction(lambda tokens: CAU(tokens[1], tokens[4], tokens[6]))
        ceu = Literal("<") + group + ">" + "[" + _atlk + "U" + _atlk + "]"   
        ceu.setParseAction(lambda tokens: CEU(tokens[1], tokens[4], tokens[6]))
                                                                
        caw = Literal("[") + group + "]" + "[" + _atlk + "W" + _atlk + "]"   
        caw.setParseAction(lambda tokens: CAW(tokens[1], tokens[4], tokens[6]))
        cew = Literal("<") + group + ">" + "[" + _atlk + "W" + _atlk + "]"   
        cew.setParseAction(lambda tokens: CEW(tokens[1], tokens[4], tokens[6]))

        strategic = (cax | cex | caf | cef | cag | ceg | cau | ceu | caw | cew)
        
        logical << (formula | epistemic | temporal | strategic)

        _atlk << (_logicals_(logical))
    
    return _atlk.parseString(spec, parseAll = True)