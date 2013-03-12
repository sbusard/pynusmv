"""
Some functions to compute Multi-Agent Systems with NuSMV.
"""

from pynusmv.nusmv.compile import compile as nscompile
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.parser import parser as nsparser
from pynusmv.nusmv.set import set as nsset
from pynusmv.nusmv.utils import utils as nsutils
from pynusmv.nusmv.compile.symb_table import symb_table as nssymb_table

from pynusmv.exception import NuSMVNoReadFileError
from pynusmv.parser import parse_next_expression

from pynusmv.glob import (load_from_file,
                               flatten_hierarchy as _flatten_hierarchy,
                               symb_table, bdd_encoding,
                               prop_database as _prop_database,
                               compute_model as _compute_model)
                               
from .bddTrans import BddTrans
from .mas import MAS

import itertools


# The current multi-agent system
_mas = None

def reset_globals():
    """
    Reset mas.glob related global variables.
    
    Must be called whenever (and before) pynusmv.init.init.deinit_nusmv
    is called.    
    """
    global _mas
    _mas = None


def _get_instances_args_for_module(modtree):
    """
    Return a dictionary of instance name -> list of instance arguments pairs,
    with instances of modules in module modtree.
    
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
                        # Compute args list
                        argslist = []
                        args = nsnode.cdr(nsnode.cdr(var))
                        while args is not None:
                            arg = nsnode.car(args)
                            argslist.append(arg)
                            args = nsnode.cdr(args)
                        varlist[varid] = argslist
                decl = nsnode.cdr(decl)
        
        declarations = nsnode.cdr(declarations)
        
    return varlist
    
    
def _flatten_and_filter_variable_args(arguments):
    """
    Return a new dictionary instance name -> list of vars where
    all vars belong to arguments (under the correct instance name)
    all vars are VAR types
    all vars are flattened.
    
    arguments -- a dictionary instance name -> list of module arguments
    """
    result = {}
    st = symb_table()
    for instance in arguments:
        result[instance] = []
        for argument in arguments[instance]:
            arg, err = nscompile.FlattenSexp(st._ptr, argument, None)
            if not err and nssymb_table.SymbTable_is_symbol_var(st._ptr, arg):
                result[instance].append(arg)
    return result
    
    
def _get_variables_by_instances(agents):
    """
    Return a dictionary of instance->list of variables
    """
    st = symb_table()
    flatHierarchy = nscompile.cvar.mainFlatHierarchy
    
    # Populate variables with instances
    variables = {}
    for agent in agents:
        variables[agent] = []
    
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
            if topcontext in variables:
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
        arguments = _get_instances_args_for_module(main)
        # arguments is a dict instancename(str)->listofargs(node)
        agents = arguments.keys()
        
        # Compute the model
        _compute_model()
        
        st = symb_table()
        
        # Flatten arguments and filter on variables
        argvars = _flatten_and_filter_variable_args(arguments)
        
        # Get agents observable variables (locals + module parameters)
        localvars = _get_variables_by_instances(agents)
        #localvars is a dict instancename(str)->listofvars(node)
        
        # Merge instance variable arguments and local variables
        variables = {key: ((key in argvars and argvars[key] or []) + 
                           (key in localvars and localvars[key] or []))
                     for key in list(argvars.keys())+list(localvars.keys())}
        
        # Compute epistemic relation
        singletrans = {}
        for agent in variables:
            transexpr = None
            for var in variables[agent]:
                transexpr = nsnode.find_node(nsparser.AND,                                                       
                                             _get_epistemic_trans(var),
                                             transexpr)
            singletrans[agent] = transexpr           
        
        # Create the MAS
        fsm = _prop_database().master.bddFsm
        _mas = MAS(fsm._ptr, singletrans, freeit = False)
        
    return _mas