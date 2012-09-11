"""
Provide some functions to compute the model and build a multimodal FSM from
a given SMV model.
"""

from pynusmv.nusmv.compile import compile as nscompile
from pynusmv.nusmv.compile.symb_table import symb_table as nssymb_table
from pynusmv.nusmv.hrc import hrc as nshrc
from pynusmv.nusmv.prop import prop as nsprop
from pynusmv.nusmv.trace import trace as nstrace
from pynusmv.nusmv.parser import parser as nsparser
from pynusmv.nusmv.utils import utils as nsutils
from pynusmv.nusmv.node import node as nsnode

from pynusmv.utils.exception import (NuSMVCannotFlattenError,
                                     NuSMVModelAlreadyFlattenedError)
from pynusmv.glob.glob import (load_from_file, _symb_table,
                               compute_model as _compute_model,
                               prop_database as _prop_database,
                               symb_table, bdd_encoding)

from .mmFsm import MMFsm
from .bddTrans import BddTrans


def _flatten_and_remove_trans():
    """
    Flatten the current model, remove its TRANS statements and return them,
    in their context.
    
    The returned value is a (maybe empty) list of TRANS in their context.
    """
    
    # Check cmps
    if not nscompile.cmp_struct_get_read_model(nscompile.cvar.cmps):
        raise NuSMVNoReadFileError("Cannot flatten; no read file.")
        
    if nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
        raise NuSMVModelAlreadyFlattenedError(
                "Model already flattened.")
        
    # Flatten hierarchy
    nscompile.CompileFlatten_init_flattener()

    st = nscompile.Compile_get_global_symb_table()
    layer = nssymb_table.SymbTable_create_layer(
                st, nscompile.MODEL_LAYER_NAME,
                nssymb_table.SYMB_LAYER_POS_BOTTOM)
    
    nssymb_table.SymbTable_layer_add_to_class(
                st, nscompile.MODEL_LAYER_NAME,
                nscompile.MODEL_LAYERS_CLASS)
    nssymb_table.SymbTable_set_default_layers_class_name(
                st, nscompile.MODEL_LAYERS_CLASS)

    if nshrc.cvar.mainHrcNode is not None:
        nshrc.HrcNode_cleanup(nshrc.cvar.mainHrcNode)                
    
    # ----- Compile_FlattenHierarchy ---------------------------------------
    main_mod_name = nscompile.sym_intern("main")
       
    nscompile.cvar.mainFlatHierarchy = nscompile.FlatHierarchy_create(st)
    result = nscompile.cvar.mainFlatHierarchy
    instances = nsutils.new_assoc()
    
    if nshrc.cvar.mainHrcNode is not None:
        mod_def = nscompile.lookup_module_hash(
                                            nsnode.find_atom(main_mod_name))
        if mod_def is None:
            pass # TODO Error, module is undefined
        nshrc.HrcNode_set_symbol_table(nshrc.cvar.mainHrcNode, st)
        nshrc.HrcNode_set_lineno(nshrc.cvar.mainHrcNode, 
                                 nsnode.node_get_lineno(mod_def))
        nshrc.HrcNode_set_name(nshrc.cvar.mainHrcNode, main_mod_name)
        nshrc.HrcNode_set_instance_name(nshrc.cvar.mainHrcNode, None)


    nscompile.Compile_ConstructHierarchy(st, layer, main_mod_name,
                                         None, None, result,
                                         nshrc.cvar.mainHrcNode, instances)
                                         
    # Get and remove TRANS from result
    trans = nscompile.FlatHierarchy_get_trans(result)
    nscompile.FlatHierarchy_set_trans(result, None)

    nscompile.Compile_ProcessHierarchy(st, layer, result, None,
                       1, 0)
                       
    if nshrc.cvar.mainHrcNode is not None:
        if nshrc.HrcNode_get_undef(nshrc.cvar.mainHrcNode) is not None:
            pass
            # TODO ??? Warning
            # "WARNING *** The model contains PROCESSes or ISAs. ***\n"
            # "WARNING *** The HRC hierarchy will not be usable. ***\n"

    nsutils.free_assoc(instances)
    # ----------------------------------------------------------------------

        
    if nshrc.cvar.mainHrcNode is not None:
        if nshrc.HrcNode_get_undef(nshrc.cvar.mainHrcNode) is not None:
            nshrc.HrcNode_destroy_recur(nshrc.cvar.mainHrcNode)
            nshrc.cvar.mainHrcNode = None

    propErr = nsprop.PropDb_fill(nsprop.PropPkg_get_prop_database(),
                st,
                nscompile.FlatHierarchy_get_spec(
                    nscompile.cvar.mainFlatHierarchy),
                nscompile.FlatHierarchy_get_compute(
                    nscompile.cvar.mainFlatHierarchy),
                nscompile.FlatHierarchy_get_ltlspec(
                    nscompile.cvar.mainFlatHierarchy),
                nscompile.FlatHierarchy_get_pslspec(
                    nscompile.cvar.mainFlatHierarchy),
                nscompile.FlatHierarchy_get_invarspec(
                    nscompile.cvar.mainFlatHierarchy))

    if 0 != propErr:
        nscompile.FlatHierarchy_destroy(nscompile.cvar.mainFlatHierarchy)
        mainFlatHierarchy = None
        nssymb_table.SymbTable_remove_layer(st, layer)
        
        # Clean up before raising exception
        nsprop.PropDb_clean(nsprop.PropPkg_get_prop_database())
        nscompile.CompileFlatten_quit_flattener()
        nscompile.cmp_struct_unset_read_model(nscompile.cvar.cmps)
        
        raise NuSMVCannotFlattenError("Cannot flatten the model.")

    
    nstrace.TraceManager_register_layer(
            nstrace.TracePkg_get_global_trace_manager(),
            nscompile.MODEL_LAYER_NAME)

    if nsparser.Parser_get_syntax_errors_list() is not None:
        # A partial model has been built
        nscompile.cmp_struct_set_hrc_built(nscompile.cvar.cmps)
        # TODO Warning? Error?
        
    else:
        nscompile.cmp_struct_set_hrc_built(nscompile.cvar.cmps)
        nscompile.cmp_struct_set_flatten_hrc(nscompile.cvar.cmps)   
    
        # TODO Wrap the pointers?
        global _symb_table
        _symb_table = nscompile.Compile_get_global_symb_table()
        
        
    # Return the got TRANS
    # The TRANS is an AND-list of individual (isolated) TRANS of the model,
    # in their context
    # The structure is the following:
    #  AND -car-> None
    #      -cdr-> AND   -car-> TRANS
    #                   -cdr-> TRANS
    translist = []
    while trans is not None:
        translist.append(nsnode.cdr(trans))
        trans = nsnode.car(trans)
    
    return translist
    

def mm_fsm():
    """Return the multi modal FSM represented by the loaded module."""
    
    global _mm_fsm
    if _mm_fsm is None:        
        # Flatten and remove TRANS, compute the model
        translist = _flatten_and_remove_trans()
        _compute_model()
        
        st = symb_table()
    
        # Compute the TRANS
        transbymod = {}
        for trans in translist:
            context = nsnode.sprint_node(nsnode.car(trans))
            if context not in transbymod:
                transbymod[context] = None
            transbymod[context] = nsnode.find_node(nsparser.AND,
                                                   transbymod[context],
                                                   trans)
        
        bddtrans = {}                              
        for cont in transbymod:
            bddtrans[cont] = BddTrans.from_trans(st, transbymod[cont], None)
        
        # Build the mmFsm and return it
        fsm = _prop_database().master.bddFsm
        _mm_fsm = MMFsm(fsm._ptr, bddtrans, freeit = False)
        
    return _mm_fsm


# The current fsm
_mm_fsm = None


def reset_globals():
    """
    Reset multimodal.glob related global variables.
    
    Must be called whenever (and before) pynusmv.init.init.deinit_nusmv
    is called.    
    """
    global _mm_fsm
    _mm_fsm = None