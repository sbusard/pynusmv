__all__ = ['reset_globals', 'load_from_file', 'flatten_hierarchy', 'symb_table',
           'encode_variables', 'bdd_encoding', 'build_flat_model',
           'build_model', 'prop_database', 'compute_model']

from .nusmv.parser import parser as nsparser
from .nusmv.opt import opt as nsopt
from .nusmv.node import node as nsnode
from .nusmv.compile import compile as nscompile
from .nusmv.enc import enc as nsenc
from .nusmv.enc.bool import bool as nsboolenc
from .nusmv.enc.bdd import bdd as nsbddenc
from .nusmv.enc.base import base as nsbaseenc
from .nusmv.prop import prop as nsprop
from .nusmv.compile.symb_table import symb_table as nssymb_table
from .nusmv.fsm import fsm as nsfsm
from .nusmv.set import set as nsset
from .nusmv.opt import opt as nsopt
from .nusmv.fsm.bdd import bdd as nsbddfsm
from .nusmv.trace import trace as nstrace
from .nusmv.trace.exec import exec as nstraceexec

from .enc import BddEnc
from .symb_table import SymbTable
from .parser import Error, NuSMVParsingError
from .prop.propDb import PropDb
from .utils.exception import (NuSMVLexerError,
                               NuSMVNoReadFileError,
                               NuSMVModelAlreadyReadError,
                               NuSMVCannotFlattenError,
                               NuSMVModelAlreadyFlattenedError,
                               NuSMVNeedFlatHierarchyError,
                               NuSMVModelAlreadyEncodedError,
                               NuSMVFlatModelAlreadyBuiltError,
                               NuSMVNeedFlatModelError,
                               NuSMVModelAlreadyBuiltError,
                               NuSMVNeedVariablesEncodedError)
                               
import os

"""Provide some functions to access global fsm-related structures."""

_bdd_encoding = None
_prop_database = None
_symb_table = None


def reset_globals():
    """Reset the globals"""
    
    global _bdd_encoding, _prop_database, _symb_table
    _bdd_encoding = None
    _prop_database = None
    _symb_table = None
    
    # Reset cmps
    nscompile.cmp_struct_reset(nscompile.cvar.cmps)


def load_from_file(filepath):
    """Load a model from an SMV file."""
    
    # Check file
    if not os.path.exists(filepath):
        raise IOError("File {} does not exist".format(filepath))
    
    # Check cmps. Need reset_nusmv if a model is already read
    if nscompile.cmp_struct_get_read_model(nscompile.cvar.cmps):
        raise NuSMVModelAlreadyReadError("A model is already read.")
    
    # Set the input file
    nsopt.set_input_file(nsopt.OptsHandler_get_instance(), filepath)
    
    # Call the parser
    # ret = 0 => OK
    # ret = 1 => syntax error (and there are registered syntax errors)
    # ret = 2 => lexer error
    ret = nsparser.Parser_ReadSMVFromFile(filepath)
    if ret == 2:
        # ret = 2 means lexer error
        raise NuSMVLexerError("An error with NuSMV lexer occured.")
        
    # When parsing a model with parser_is_lax enabled (this is the case
    # since this is enabled in init_nusmv), the parser gets
    # as many correct parts of the model as possible and build a partial
    # model with it.        
    
    # Raise exceptions if needed
    errors = nsparser.Parser_get_syntax_errors_list()
    if errors is not None:
        errlist = []
        while errors is not None:
            error = nsnode.car(errors)
            err = nsparser.Parser_get_syntax_error(error)
            errlist.append(Error(*err[1:]))
            errors = nsnode.cdr(errors)
        raise NuSMVParsingError(tuple(errlist))
        
    # Update cmps
    nscompile.cmp_struct_set_read_model(nscompile.cvar.cmps)
    
    
def flatten_hierarchy():
    """
    Flatten the read model.
    
    If no model is read, raise a NuSMVNoReadFileError.
    If an error occured during flattening,
        raise a NuSMVCannotFlattenError.
    If the model is already flattened, raise a NuSMVModelAlreadyFlattenedError.
        
    In case of type checking errors, a message is printed at stderr and
    a NuSMVCannotFlattenError is raised.
    """
    
    # Check cmps
    if not nscompile.cmp_struct_get_read_model(nscompile.cvar.cmps):
        raise NuSMVNoReadFileError("Cannot flatten; no read file.")
        
    if nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
        raise NuSMVModelAlreadyFlattenedError(
                "Model already flattened.")
        
    
    # Flatten hierarchy
    ret = nscompile.flatten_hierarchy()
    if ret != 0:
        raise NuSMVCannotFlattenError("Cannot flatten the model.")
    
    global _symb_table
    _symb_table = SymbTable(nscompile.Compile_get_global_symb_table())
    
    
def symb_table():
    """
    Compute (if needed) and return the main symbols table
    of the current model.
    """
    # Flatten hierarchy if needed
    global _symb_table
    if _symb_table is None:
        if nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
            _symb_table = SymbTable(nscompile.Compile_get_global_symb_table())
        else:
            flatten_hierarchy()
    return _symb_table
    
    
def encode_variables():
    """
    Encode the BDD variables of the current model.
    
    If the model is not flattened, raise a NuSMVNeedFlatHierarchyError;
    if the variables are already encoded, raise a NuSMVModelAlreadyEncodedError.
    """
    # Check cmps
    if not nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
        raise NuSMVNeedFlatHierarchyError("Need flat hierarchy.")
    if nscompile.cmp_struct_get_encode_variables(nscompile.cvar.cmps):
        raise NuSMVModelAlreadyEncodedError(
                                    "The variables are already encoded.")
    
    # Encode variables
    nsenc.Enc_init_bool_encoding()
    bool_enc = nsenc.Enc_get_bool_encoding()
    base_enc = nsboolenc.boolenc2baseenc(bool_enc)
    nsbaseenc.BaseEnc_commit_layer(base_enc, "model")

    nsenc.Enc_init_bdd_encoding()
    bdd_enc = nsenc.Enc_get_bdd_encoding()
    base_enc = nsbddenc.bddenc2baseenc(bdd_enc)
    nsbaseenc.BaseEnc_commit_layer(base_enc, "model")
    
    # Update cmps
    nscompile.cmp_struct_set_encode_variables(nscompile.cvar.cmps)
    
    # Get global encoding
    global _bdd_encoding
    _bdd_encoding = BddEnc(nsenc.Enc_get_bdd_encoding())
    

def bdd_encoding():
    """
    Compute (if needed) and return the main bdd encoding
    of the current model.
    """
    # Encode variables if needed
    global _bdd_encoding
    if _bdd_encoding is None:
        if nscompile.cmp_struct_get_encode_variables(nscompile.cvar.cmps):
            _bdd_encoding = BddEnc(nsenc.Enc_get_bdd_encoding())
        else:
            encode_variables()
    return _bdd_encoding
    

def build_flat_model():
    """
    Build the Sexp FSM of the current model.
    
    If the model is not flattened, raise a NuSMVNeedFlatHierarchyError;
    if the Sexp FSM is already built, raise a NuSMVFlatModelAlreadyBuiltError.
    """
    # Check cmps
    if not nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
        raise NuSMVNeedFlatHierarchyError("Need flat hierarchy.")
    if nscompile.cmp_struct_get_build_flat_model(nscompile.cvar.cmps):
        raise NuSMVFlatModelAlreadyBuiltError(
                                        "The flat model is already built.")
    
    # Simplify the model
    st = nscompile.Compile_get_global_symb_table()
    layer = nssymb_table.SymbTable_get_layer(st, "model")
    ite = nssymb_table.gen_iter(layer, nssymb_table.STT_VAR)
    variables = nssymb_table.SymbLayer_iter_to_set(layer, ite)
    
    sexp_fsm = nsfsm.FsmBuilder_create_scalar_sexp_fsm(
                    nscompile.Compile_get_global_fsm_builder(),
                    nscompile.cvar.mainFlatHierarchy,
                    variables)
                    
    nsset.Set_ReleaseSet(variables)
    
    nsprop.PropDb_master_set_scalar_sexp_fsm(
            nsprop.PropPkg_get_prop_database(),
            sexp_fsm)
            
    # Update cmps
    nscompile.cmp_struct_set_build_flat_model(nscompile.cvar.cmps)
    

def build_model():
    """
    Build the BDD FSM of the current model.
    
    If the Sexp FSM of the model is not built yet,
        raise a NuSMVNeedFlatModelError;
    if the variables of the model are not encoded yet,
        raise a NuSMVNeedVariablesEncodedError;
    if the BDD FSM of the model is already built,
        raise a NuSMVModelAlreadyBuiltError
    """
    
    # Check cmps
    if not nscompile.cmp_struct_get_build_flat_model(nscompile.cvar.cmps):
        raise NuSMVNeedFlatModelError("Need flat model.")
    if not nscompile.cmp_struct_get_encode_variables(nscompile.cvar.cmps):
        raise NuSMVNeedVariablesEncodedError("Need variables encoded.")
    if nscompile.cmp_struct_get_build_model(nscompile.cvar.cmps):
        raise NuSMVModelAlreadyBuiltError("The model is already built.")
    
    # Build the model
    pd = nsprop.PropPkg_get_prop_database()
    sexp_fsm = nsprop.PropDb_master_get_scalar_sexp_fsm(pd)
    bdd_fsm = nsfsm.FsmBuilder_create_bdd_fsm(
                nscompile.Compile_get_global_fsm_builder(),
                nsenc.Enc_get_bdd_encoding(),
                sexp_fsm,
                nsopt.get_partition_method(
                            nsopt.OptsHandler_get_instance()))

    nsprop.PropDb_master_set_bdd_fsm(pd, bdd_fsm)
    
    # Register executors
    enc = nsbddfsm.BddFsm_get_bdd_encoding(bdd_fsm)

    nstrace.TraceManager_register_complete_trace_executor(
                nstrace.TracePkg_get_global_trace_manager(),
                "bdd", "BDD partial trace execution",
                nstraceexec.bddCompleteTraceExecutor2completeTraceExecutor(
                    nstraceexec.BDDCompleteTraceExecutor_create(bdd_fsm,
                                                                 enc)))

    nstrace.TraceManager_register_partial_trace_executor(
                nstrace.TracePkg_get_global_trace_manager(),
                "bdd", "BDD complete trace execution",
                nstraceexec.bddPartialTraceExecutor2partialTraceExecutor(
                    nstraceexec.BDDPartialTraceExecutor_create(bdd_fsm,
                                                                  enc)))
                                                                  
    # Update cmps
    nscompile.cmp_struct_set_build_model(nscompile.cvar.cmps)
    

def prop_database():
    """
    Return the global properties database.
    """
    if not nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
        # Need a flat hierarchy
        raise NuSMVNeedFlatHierarchyError("Need flat hierarchy.")
    global _prop_database
    if _prop_database is None:
        _prop_database = PropDb(nsprop.PropPkg_get_prop_database())
    return _prop_database
    

def compute_model():
    """
    Compute the read model.
    
    Perform all the steps that are not yet performed:
    flattening, encoding, building flat model, building model.
    """
    
    if not nscompile.cmp_struct_get_read_model(nscompile.cvar.cmps):
        raise NuSMVNoReadFileError("No read file.")
    
    # Check cmps and perform what is needed
    if not nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
        flatten_hierarchy()
    if not nscompile.cmp_struct_get_encode_variables(nscompile.cvar.cmps):
        encode_variables()
    if not nscompile.cmp_struct_get_build_flat_model(nscompile.cvar.cmps):
        build_flat_model()
    if not nscompile.cmp_struct_get_build_model(nscompile.cvar.cmps):
        build_model()