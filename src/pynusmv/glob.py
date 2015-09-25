"""
The :mod:`pynusmv.glob` module provide some functions to access global NuSMV
functionalities. These functions are used to feed an SMV model to NuSMV and
build the different structures representing the model, like flattening the
model, building its BDD encoding and getting the BDD-encoded FSM.

Besides the functions, this module provides an access to main globally stored
data structures like the flat hierarchy, the BDD encoding, the symbols table
and the propositions database.

"""

__all__ = ['load', 'symb_table', 'bdd_encoding', 'prop_database',
           'flatten_hierarchy', 'encode_variables', 'build_flat_model',
           'build_model', 'compute_model', 'encode_variables_for_layers',
           'flat_hierarchy']

import tempfile
import os

from .nusmv.parser import parser as nsparser
from .nusmv.opt import opt as nsopt
from .nusmv.compile import compile as nscompile
from .nusmv.enc import enc as nsenc
from .nusmv.enc.bool import bool as nsboolenc
from .nusmv.enc.bdd import bdd as nsbddenc
from .nusmv.enc.base import base as nsbaseenc
from .nusmv.prop import prop as nsprop
from .nusmv.compile.symb_table import symb_table as nssymb_table
from .nusmv.fsm import fsm as nsfsm
from .nusmv.set import set as nsset
from .nusmv.fsm.bdd import bdd as nsbddfsm
from .nusmv.trace import trace as nstrace
from .nusmv.trace.exec_ import exec_ as nstraceexec

from .fsm import BddEnc, SymbTable
from .node import FlatHierarchy
from .parser import NuSMVParsingError
from .prop import PropDb
from .exception import (NuSMVLexerError,
                        NuSMVNoReadModelError,
                        NuSMVModelAlreadyReadError,
                        NuSMVCannotFlattenError,
                        NuSMVModelAlreadyFlattenedError,
                        NuSMVNeedFlatHierarchyError,
                        NuSMVModelAlreadyEncodedError,
                        NuSMVFlatModelAlreadyBuiltError,
                        NuSMVNeedFlatModelError,
                        NuSMVModelAlreadyBuiltError,
                        NuSMVNeedVariablesEncodedError)


__bdd_encoding = None
__prop_database = None
__symb_table = None
__flat_hierarchy = None


def _reset_globals():
    """
    Reset the global variables of the module, keeping track of global data
    structures.

    """
    global __bdd_encoding, __prop_database, __symb_table, __flat_hierarchy
    __bdd_encoding = None
    __prop_database = None
    __symb_table = None
    __flat_hierarchy = None

    # Reset cmps
    nscompile.cmp_struct_reset(nscompile.cvar.cmps)


def load(*model):
    """
    Load the given model. This model can be of several forms:

    * a file path; in this case, the model is loaded from the file;
    * NuSMV modelling code; in this case, `model` is the code for the model;
    * a list of modules (list of :class:`Module <pynusmv.model.Module>`
      subclasses); in this case, the model is represented by the set of
      modules.

    """
    if len(model) == 1 and isinstance(model[0], str):
        if os.path.isfile(model[0]):
            load_from_file(model[0])
        else:
            load_from_string(model[0])
    else:  # model must be a list of modules
        load_from_modules(*model)


def load_from_string(model):
    """
    Load a model from a string representing the model.

    :param model: a String representing the model
    :type model: str

    """
    # Create temp file
    with tempfile.NamedTemporaryFile(suffix=".smv") as tmp:
        tmp.write(model.encode("UTF-8"))
        tmp.flush()
        load_from_file(tmp.name)


def load_from_modules(*modules):
    """
    Load a model from a set of modules representing the model.

    :param modules: the modules defining the NuSMV model. Must contain a
                    `main` module.
    :type modules: a list of :class:`Module <pynusmv.model.Module>`
                   subclasses

    """
    load_from_string("\n".join(str(module) for module in modules))


def load_from_file(filepath):
    """
    Load a model from an SMV file and store it in global data structures.

    :param filepath: the path to the SMV file
    :type filepath: str

    """
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
        raise NuSMVParsingError.from_nusmv_errors_list(errors)

    # Update cmps
    nscompile.cmp_struct_set_read_model(nscompile.cvar.cmps)


def flatten_hierarchy(keep_single_enum=False):
    """
    Flatten the read model and store it in global data structures.

    :param keep_single_enum: whether or not enumerations with single values
                             should be converted into defines
    :type keep_single_enum: bool

    :raise: a :exc:`NuSMVNoReadModelError
            <pynusmv.exception.NuSMVNoReadModelError>` if no model is read yet
    :raise: a :exc:`NuSMVCannotFlattenError
            <pynusmv.exception.NuSMVCannotFlattenError>` if an error occurred
            during flattening
    :raise: a :exc:`NuSMVModelAlreadyFlattenedError
            <pynusmv.exception.NuSMVModelAlreadyFlattenedError>` if the model
            is already flattened

    .. warning:: In case of type checking errors, a message is printed at
       stderr and a :exc:`NuSMVCannotFlattenError
       <pynusmv.exception.NuSMVCannotFlattenError>` is raised.

    """

    # Check cmps
    if not nscompile.cmp_struct_get_read_model(nscompile.cvar.cmps):
        raise NuSMVNoReadModelError("Cannot flatten; no read model.")

    if nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
        raise NuSMVModelAlreadyFlattenedError(
            "Model already flattened.")

    # Update options to reflect keep_single_enum
    if keep_single_enum:
        nsopt.set_keep_single_value_vars(nsopt.OptsHandler_get_instance())
    else:
        nsopt.unset_keep_single_value_vars(nsopt.OptsHandler_get_instance())

    # Flatten hierarchy
    ret = nscompile.flatten_hierarchy()
    if ret != 0:
        raise NuSMVCannotFlattenError("Cannot flatten the model.")

    global __symb_table
    __symb_table = SymbTable(nscompile.Compile_get_global_symb_table())


def symb_table():
    """
    Return the main symbols table of the current model.

    :rtype: :class:`SymbTable <pynusmv.fsm.SymbTable>`

    """
    # Flatten hierarchy if needed
    global __symb_table
    if __symb_table is None:
        if nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
            __symb_table = SymbTable(nscompile.Compile_get_global_symb_table())
        else:
            flatten_hierarchy()
    return __symb_table


def encode_variables(layers={"model"}, variables_ordering=None):
    """
    Encode the BDD variables of the current model and store it in global data
    structures.
    If variables_ordering is provided, use this ordering to encode the
    variables; otherwise, the default ordering method is used.

    :param layers: the set of layers variables to encode
    :type layers: :class:`set`
    :param variables_ordering: the file containing a custom ordering
    :type variables_ordering: path to file

    :raise: a :exc:`NuSMVNeedFlatHierarchyError
            <pynusmv.exception.NuSMVNeedFlatHierarchyError>` if the model is
            not flattened
    :raise: a :exc:`NuSMVModelAlreadyEncodedError
            <pynusmv.exception.NuSMVModelAlreadyEncodedError>`
            if the variables are already encoded

    """
    # Check cmps
    if not nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
        raise NuSMVNeedFlatHierarchyError("Need flat hierarchy.")
    if nscompile.cmp_struct_get_encode_variables(nscompile.cvar.cmps):
        raise NuSMVModelAlreadyEncodedError(
            "The variables are already encoded.")

    if variables_ordering is not None:
        nsopt.set_input_order_file(nsopt.OptsHandler_get_instance(),
                                   variables_ordering)

    encode_variables_for_layers(layers, init=True)

    # Update cmps
    nscompile.cmp_struct_set_encode_variables(nscompile.cvar.cmps)

    # Get global encoding
    global __bdd_encoding
    __bdd_encoding = BddEnc(nsenc.Enc_get_bdd_encoding())


def encode_variables_for_layers(layers={"model"}, init=False):
    """
    Encode the BDD variables of the given layers and store them in global data
    structures.

    :param layers: the set of layers variables to encode
    :type layers: :class:`set`
    :param bool init: whether or not initialize the global encodings

    .. warning: Global encodings should be initialized only once, otherwise,
                NuSMV quits unexpectingly. Note that :func:`encode_variables`
                initializes them, and should be called before any call to
                this function.

    """
    if init:
        nsenc.Enc_init_bool_encoding()
    bool_enc = nsenc.Enc_get_bool_encoding()
    base_enc = nsboolenc.boolenc2baseenc(bool_enc)
    for layer in layers:
        nsbaseenc.BaseEnc_commit_layer(base_enc, layer)

    if init:
        nsenc.Enc_init_bdd_encoding()
    bdd_enc = nsenc.Enc_get_bdd_encoding()
    base_enc = nsbddenc.bddenc2baseenc(bdd_enc)
    for layer in layers:
        nsbaseenc.BaseEnc_commit_layer(base_enc, layer)


def bdd_encoding():
    """
    Return the main bdd encoding of the current model.

    :rtype: :class:`BddEnc <pynusmv.dd.BddEnc>`

    """
    # Encode variables if needed
    global __bdd_encoding
    if __bdd_encoding is None:
        if nscompile.cmp_struct_get_encode_variables(nscompile.cvar.cmps):
            __bdd_encoding = BddEnc(nsenc.Enc_get_bdd_encoding())
        else:
            encode_variables()
    return __bdd_encoding


def build_flat_model():
    """
    Build the Sexp FSM (Simple Expression FSM) of the current model and store
    it in global data structures.

    :raise: a :exc:`NuSMVNeedFlatHierarchyError
            <pynusmv.exception.NuSMVNeedFlatHierarchyError>` if the model is
            not flattened
    :raise: a :exc:`NuSMVFlatModelAlreadyBuiltError
            <pynusmv.exception.NuSMVFlatModelAlreadyBuiltError>` if the Sexp
            FSM is already built

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
    Build the BDD FSM of the current model and store it in global data
    structures.

    :raise: a :exc:`NuSMVNeedFlatModelError
            <pynusmv.exception.NuSMVNeedFlatModelError>` if the Sexp FSM
            of the model is not built yet
    :raise: a :exc:`NuSMVNeedVariablesEncodedError
            <pynusmv.exception.NuSMVNeedVariablesEncodedError>` if the
            variables of the model are not encoded yet
    :raise: a :exc:`NuSMVModelAlreadyBuiltError
            <pynusmv.exception.NuSMVModelAlreadyBuiltError>` if the BDD FSM
            of the model is already built

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


def flat_hierarchy():
    """
    Return the global flat hierarchy.

    :rtype: :class:`FlatHierarchy <pynusmv.node.FlatHierarchy>`
    """
    if not nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
        # Need a flat hierarchy
        raise NuSMVNeedFlatHierarchyError("Need flat hierarchy.")
    global __flat_hierarchy
    if __flat_hierarchy is None:
        __flat_hierarchy = FlatHierarchy(nscompile.cvar.mainFlatHierarchy)
    return __flat_hierarchy


def prop_database():
    """
    Return the global properties database.

    :rtype: :class:`PropDb <pynusmv.prop.PropDb>`

    """
    if not nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
        # Need a flat hierarchy
        raise NuSMVNeedFlatHierarchyError("Need flat hierarchy.")
    global __prop_database
    if __prop_database is None:
        __prop_database = PropDb(nsprop.PropPkg_get_prop_database())
    return __prop_database


def compute_model(variables_ordering=None, keep_single_enum=False):
    """
    Compute the read model and store its parts in global data structures.
    This function is a shortcut for calling all the steps of the model building
    that are not yet performed.
    If variables_ordering is not None, it is used as a file containing the
    order of variables used for encoding the model into BDDs.

    
    :param variables_ordering: the file containing a custom ordering
    :type variables_ordering: path to file
    :param keep_single_enum: whether or not enumerations with single values
                             should be converted into defines
    :type keep_single_enum: bool

    """
    if not nscompile.cmp_struct_get_read_model(nscompile.cvar.cmps):
        raise NuSMVNoReadModelError("No read model.")

    # Check cmps and perform what is needed
    if not nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
        flatten_hierarchy(keep_single_enum=keep_single_enum)
    if not nscompile.cmp_struct_get_encode_variables(nscompile.cvar.cmps):
        encode_variables(variables_ordering=variables_ordering)
    if not nscompile.cmp_struct_get_build_flat_model(nscompile.cvar.cmps):
        build_flat_model()
    if not nscompile.cmp_struct_get_build_model(nscompile.cvar.cmps):
        build_model()
