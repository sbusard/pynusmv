from pynusmv.trans.trans import BddTrans as SuperBddTrans
from pynusmv.parser.parser import (parse_next_expression, parse_identifier,
                                   parse_simple_expression)
from pynusmv.utils.exception import NuSMVTypeCheckingError

from . import glob
from pynusmv.glob import glob as superGlob

from pynusmv.nusmv.compile import compile as nscompile
from pynusmv.nusmv.fsm import fsm as nsfsm
from pynusmv.nusmv.trans.bdd import bdd as nsbddtrans
from pynusmv.nusmv.opt import opt as nsopt
from pynusmv.nusmv.enc.bdd import bdd as nsbddenc
from pynusmv.nusmv.compile.symb_table import symb_table as nssymb_table
from pynusmv.nusmv.compile.type_checking import type_checking as nstype_checking
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.parser import parser as nsparser

class BddTrans(SuperBddTrans):
    
    def _free(self):
        if self._freeit and self._ptr is not None:
            # Free it because such a BddTrans is not owned by anyone
            nsbddtrans.BddTrans_free(self._ptr)
            self._freeit = False
            
    
    @staticmethod
    def from_trans(symb_table, trans, context=None):
        """
        Return a new BddTrans from the given trans.
        
        symb_table -- the symbols table used to flatten the trans.
        trans -- the given trans. Not flattened. Already parsed.
        context -- an additional context, in which trans will be flattened,
                   if not None. Already parsed.
        """
        
        flattrans = nscompile.Compile_FlattenSexp(symb_table, trans, context)
        
        # Build the BDD trans
        fsmbuilder = superGlob.fsm_builder()
        enc = superGlob.bdd_encoding()
        ddmanager = enc.DDmanager
        
        clusters = nsfsm.FsmBuilder_clusterize_expr(fsmbuilder, enc._ptr,
                                                    flattrans)
        cluster_options = nsbddtrans.ClusterOptions_create(
                            nsopt.OptsHandler_get_instance())
        
        newtransptr = nsbddtrans.BddTrans_create(
                            ddmanager._ptr,
                            clusters,
                            nsbddenc.BddEnc_get_state_vars_cube(enc._ptr),
                            nsbddenc.BddEnc_get_input_vars_cube(enc._ptr),
                            nsbddenc.BddEnc_get_next_state_vars_cube(enc._ptr),
                            nsopt.get_partition_method(
                                            nsopt.OptsHandler_get_instance()),
                            cluster_options)
                            
        return BddTrans(newtransptr, enc, ddmanager, freeit = True)
        
    
    @staticmethod    
    def from_string(symb_table, strtrans, strcontext=None):
        """
        Return a new BddTrans from the given strtrans, in given strcontex.
        
        symb_table -- the symbols table used to flatten the trans.
        strtrans -- the given trans as a string.
        context -- an additional context, in which trans will be flattened,
                   if not None. A string representing the context.
        """
        type_checker = nssymb_table.SymbTable_get_type_checker(symb_table)
        
        if strcontext is not None:
            strtrans = "(" + strtrans + ")" + " IN " + strcontext
        
        # Parse the string
        trans = parse_next_expression(strtrans)
        
        #if trans.type == nsparser.CONTEXT and nsnode.car(trans) is None:
        #    # trans is CONTEXT(None, trans body)
        #    # we need trans body
        #    trans = nsnode.cdr(trans)
        #
        ## Parse the context if needed
        #if strcontext is not None:
        #    context = parse_identifier(strcontext)
        #    # No type checking since there is no way to make sure the context
        #    # is a good module instance.
        #    
        #    # we need DOT(None, context parsed tree)
        #    context = nsnode.find_node(nsparser.DOT, None, context)
        #    trans = nsnode.find_node(nsparser.CONTEXT, context, trans)
        
        # Type check
        expr_type = nstype_checking.TypeChecker_get_expression_type(
                                                   type_checker, trans, None)
        if not nssymb_table.SymbType_is_boolean(expr_type):
            raise NuSMVTypeCheckingError("The given TRANS is wrongly typed.")
            
        # Call from_trans method
        return BddTrans.from_trans(symb_table, trans)