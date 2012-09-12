"""
Some functions to compute Multi-Agent Systems with NuSMV.
"""

from pynusmv.nusmv.compile import compile as nscompile
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.parser import parser as nsparser
from pynusmv.nusmv.set import set as nsset
from pynusmv.nusmv.utils import utils as nsutils
from pynusmv.nusmv.compile.symb_table import symb_table as nssymb_table

from pynusmv.utils.exception import NuSMVNoReadFileError
from pynusmv.parser.parser import parse_next_expression

from tools.multimodal.glob import (load_from_file, _flatten_and_remove_trans,
                                   symb_table, bdd_encoding, _prop_database,
                                   _compute_model,
                                   reset_globals as _reset_globals)
from tools.multimodal.bddTrans import BddTrans

from .mas import MAS


# The current multi-agent system
_mas = None

def reset_globals():
    """
    Reset ctlk.glob related global variables.
    
    Must be called whenever (and before) pynusmv.init.init.deinit_nusmv
    is called.    
    """
    global _mas
    _mas = None


def _get_instances_for_module(modtree):
    """
    Return a dictionary of variable name -> variable declaration pairs,
    that are instances of modules in module modtree.
    
    modtree is a part of the AST of the SMV model. modtree.type = MODULE
    """
    # MODULE(MODTYPE(...), declarationlist)
    varlist = {}
    
    declarations = nsnode.cdr(modtree)
    while declarations is not None:
        decl = nsnode.car(declarations)
        
        if decl.type == nsparser.VAR:
            
            decl = nsnode.car(decl)
            while decl is not None:
                var = nsnode.car(decl)
                # var_id : type => COLON(ATOM, type)
                if nsnode.cdr(var).type == nsparser.MODTYPE:
                    varid = nsnode.sprint_node(nsnode.car(var))
                    if varid in varlist:
                        pass # TODO Variable already defined
                    else:
                        varlist[varid] = var
                decl = nsnode.cdr(decl)
        
        declarations = nsnode.cdr(declarations)
        
    return varlist
    
    
def _get_variables_by_instances(instances):
    """
    Return a dictionary of instance->list of variables
    """
    st = symb_table()
    flatHierarchy = nscompile.cvar.mainFlatHierarchy
    
    # Populate variables with instances
    variables = {}
    for instance in instances:
        variables[instance] = []
    
    
    varset = nscompile.FlatHierarchy_get_vars(flatHierarchy)
    varlist = nsset.Set_Set2List(varset)
    
    ite = nsutils.NodeList_get_first_iter(varlist)
    while not nsutils.ListIter_is_end(ite):
        var = nsutils.NodeList_get_elem_at(varlist, ite)
        varname = nsnode.sprint_node(var)
        isVar = nssymb_table.SymbTable_is_symbol_var(st._ptr, var)
        if isVar:
            # Put the var in the variables dictionary, under the right instance
            topcontext = varname.partition(".")[0]
            if topcontext in instances:
                variables[topcontext].append(var)                    
        ite = nsutils.ListIter_get_next(ite)
        
    return variables
        
        
def _get_epistemic_trans(variable):
    """Return a new TRANS next(variable) = variable"""
    
    varname = nsnode.sprint_node(variable)
    transexpr = "next({0}) = {0}".format(varname)
    return parse_next_expression(transexpr)
    

def mas():
    """
    Return (and compute if needed) the multi-agent system represented by
    the currently read SMV model.
    """    
    global _mas
    if _mas is None:
        # Check cmps
        if not nscompile.cmp_struct_get_read_model(nscompile.cvar.cmps):
            raise NuSMVNoReadFileError("Cannot build MAS; no read file.")
        
        # Get agents names
        tree = nsparser.cvar.parsed_tree
        main = None
        while tree is not None:
            module = nsnode.car(tree)
            if nsnode.sprint_node(nsnode.car(nsnode.car(module))) == "main":
                main = module            
            tree = nsnode.cdr(tree)
        if main is None:
            print("[ERROR] No main module.")
            return # TODO Error, cannot find main module
        instances = _get_instances_for_module(main)
        
        # Flatten the model without TRANS
        translist = _flatten_and_remove_trans()
        
        # Compute the model
        _compute_model()
        
        # Sort TRANS by agents and compute them
        transbyinst = {}
        for trans in translist:
            context = nsnode.sprint_node(nsnode.car(trans))
            topcontext = context.partition(".")[0]
            if topcontext not in transbyinst:
                transbyinst[topcontext] = None
            transbyinst[topcontext] = nsnode.find_node(nsparser.AND,
                                                       transbyinst[topcontext],
                                                       trans)
                                                       
        # Note: there may be a transition for the top context,
        # i.e. transbyinst[""] can be not null
        
        st = symb_table()                                               
        temptrans = {}                              
        for cont in transbyinst:
            temptrans[cont] = BddTrans.from_trans(st, transbyinst[cont], None)
        
        # Get agents observable variables (locals + module parameters)
        agents = list(temptrans.keys())
        if "" in agents:
            agents.remove("")
        variables = _get_variables_by_instances(agents)
        
        # Compute epistemic relation
        epistemictrans = {}
        for agent in agents:
            transexpr = None
            for var in variables[agent]:
                transexpr = nsnode.find_node(nsparser.AND,                                                       
                                             _get_epistemic_trans(var),
                                             transexpr)
            epistemictrans[agent] = BddTrans.from_trans(st, transexpr, None)                
        
        # Create the MAS
        fsm = _prop_database().master.bddFsm
        _mas = MAS(fsm._ptr, temptrans, epistemictrans, freeit = False)
        
    return _mas