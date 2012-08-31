from ..nusmv.parser import parser as nsparser
from ..nusmv.opt import opt as nsopt
from ..nusmv.node import node as nsnode
from ..nusmv.compile import compile as nscompile
from ..nusmv.enc import enc as nsenc
from ..nusmv.enc.bool import bool as nsboolenc
from ..nusmv.enc.bdd import bdd as nsbddenc
from ..nusmv.enc.base import base as nsbaseenc
from ..nusmv.prop import prop as nsprop
from ..nusmv.compile.symb_table import symb_table as nssymb_table
from ..nusmv.fsm import fsm as nsfsm
from ..nusmv.set import set as nsset
from ..nusmv.opt import opt as nsopt
from ..nusmv.fsm.bdd import bdd as nsbddfsm
from ..nusmv.trace import trace as nstrace
from ..nusmv.trace.exec import exec as nstraceexec

from ..enc.enc import BddEnc
from ..parser.parser import Error, NuSMVParsingException
from ..prop.propDb import PropDb

class Globals:
    """Provide some functions to access global fsm-related structures."""
    
    _parsed_tree = None
    _flat_hierarchy = None
    _bdd_encoding = None
    # TODO Get global symbols table (don't forget reset_globals function too)
    
    
    @classmethod
    def reset_globals(cls):
        """Reset the globals"""
        
        cls._parsed_tree = None
        cls._flat_hierarchy = None
        cls._bdd_encoding = None
        
        # Reset cmps
        nscompile.cmp_struct_reset(nscompile.cvar.cmps)

    
    @classmethod
    def load_from_file(cls, filepath):
        """Load a model from an SMV file."""
        
        # Check cmps. Need reset_nusmv if a model is already read
        if nscompile.cmp_struct_get_read_model(nscompile.cvar.cmps):
            raise NuSMVModelAlreadyReadException("A model is already read.")
        
        # Set the input file
        nsopt.set_input_file(nsopt.OptsHandler_get_instance(), filepath)
        
        # Call the parser
        # ret = 0 => OK
        # ret = 1 => syntax error (and there are registered syntax errors)
        # ret = 2 => lexer error
        ret = nsparser.Parser_ReadSMVFromFile(filepath)
        if ret == 2:
            # ret = 2 means lexer error
            raise NuSMVLexerException("An error with NuSMV lexer occured.")
            
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
            raise NuSMVParsingException(tuple(errlist))
            
        # Update cmps
        nscompile.cmp_struct_set_read_model(nscompile.cvar.cmps)
        
        # Store the parsed tree for further use
        # TODO Wrap the pointer?
        cls._parsed_tree = nsparser.cvar.parsed_tree
        
 
# TODO Decide if wrapping the pointer before exposing this   
#    @classmethod
#    def parsed_tree(cls):
#        """Return the parsed tree of the last loaded SMV file."""
#        # Check that a file is read
#        if cls._parsed_tree is None:
#            raise NuSMVNoReadFileException("No read file.")
#        return cls._parsed_tree
        
        
    @classmethod
    def flatten_hierarchy(cls):
        """
        Flatten the read model.
        
        If no model is read, raise a NuSMVNoReadFileException.
        If an error occured during flattening,
            raise a NuSMVCannotFlattenException.
        """
        
        # Check cmps
        if not nscompile.cmp_struct_get_read_model(nscompile.cvar.cmps):
            raise NuSMVNoReadFileException("Cannot flatten; no read file.")
            
        if nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
            raise NUSMVModelAlreadyFlattenedException(
                    "Model already flattened.")
            
        
        # Flatten hierarchy
        ret = nscompile.compile_flatten_smv(0)
        if ret != 0:
            raise NuSMVCannotFlattenException("Cannot flatten the model.")
        
        # TODO Wrap the pointer?
        cls._flat_hierarchy = nscompile.cvar.mainFlatHierarchy
        

# TODO Decide if wrapping the pointer before exposing this        
#    @classmethod
#    def flat_hierarchy(cls):
#        """
#        Compute (if needed) and return the main flat hierarchy
#        of the current model.
#        
#        If no model is read, raise a NuSMVNoReadFileException.
#        If an error occured during flattening,
#            raise a NuSMVCannotFlattenException.
#        """
#        # Flatten hierarchy if needed
#        if cls._flat_hierarchy is None:
#            cls.flatten_hierarchy()
#        return cls._flat_hierarchy
        
    
    @classmethod    
    def encode_variables(cls):
        """
        Encode the BDD variables of the current model.
        """
        # Check cmps
        if not nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
            raise NuSMVNeedFlatHierarchyException("Need flat hierarchy.")
        if nscompile.cmp_struct_get_encode_variables(nscompile.cvar.cmps):
            raise NUSMVModelAlreadyEncodedException(
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
        cls._bdd_encoding = BddEnc(nsenc.Enc_get_bdd_encoding())
        
    
    @classmethod    
    def bdd_encoding(cls):
        """
        Compute (if needed) and return the main bdd encoding
        of the current model.
        
        If no model is read, raise a NuSMVNoReadFileException.
        If an error occured during flattening,
            raise a NuSMVCannotFlattenException.
        """
        # Encode variables if needed
        if cls._bdd_encoding is None:
            if nscompile.cmp_struct_get_encode_variables(nscompile.cvar.cmps):
                cls._bdd_encoding = BddEnc(nsenc.Enc_get_bdd_encoding())
            else:
                cls.encode_variables()
        return cls._bdd_encoding
        
        
    @classmethod
    def build_flat_model(cls):
        """
        Build the Sexp FSM of the current model.
        """
        # Check cmps
        if not nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
            raise NuSMVNeedFlatHierarchyException("Need flat hierarchy.")
        if nscompile.cmp_struct_get_build_flat_model(nscompile.cvar.cmps):
            raise NUSMVFlatModelAlreadyBuiltException(
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
        
        
    @classmethod
    def build_model(cls):
        """
        Build the BDD FSM of the current model.
        """
        
        # Check cmps
        if not nscompile.cmp_struct_get_build_flat_model(nscompile.cvar.cmps):
            raise NuSMVNeedFlatModelException("Need flat model.")
        if not nscompile.cmp_struct_get_encode_variables(nscompile.cvar.cmps):
            raise NuSMVNeedVariablesEncodedException("Need variables encoded.")
        if nscompile.cmp_struct_get_build_model(nscompile.cvar.cmps):
            raise NUSMVModelAlreadyBuiltException("The model is already built.")
        
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
        
        
    @classmethod
    def prop_database(cls):
        """
        Return the global properties database.
        """
        if not nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
            # Need a flat hierarchy
            raise NuSMVNeedFlatHierarchyException("Need flat hierarchy.")
        return PropDb(nsprop.PropPkg_get_prop_database())
        
        
    @classmethod
    def compute_model(cls):
        """
        Compute the read model.
        
        Perform all the steps that are not yet performed:
        flattening, encoding, building flat model, building model.
        """
        
        if not nscompile.cmp_struct_get_read_model(nscompile.cvar.cmps):
            raise NuSMVNoReadFileException("No read file.")
        
        # Check cmps and perform what is needed
        if not nscompile.cmp_struct_get_flatten_hrc(nscompile.cvar.cmps):
            cls.flatten_hierarchy()
        if not nscompile.cmp_struct_get_encode_variables(nscompile.cvar.cmps):
            cls.encode_variables()
        if not nscompile.cmp_struct_get_build_flat_model(nscompile.cvar.cmps):
            cls.build_flat_model()
        if not nscompile.cmp_struct_get_build_model(nscompile.cvar.cmps):
            cls.build_model()
   
        
class NuSMVLexerException(Exception):
    """An error with NuSMV lexer."""
    pass

class NuSMVNoReadFileException(Exception):
    """No SMV model has been read yet."""
    pass
    
class NuSMVModelAlreadyReadException(Exception):
    """A model is already read."""
    pass

class NuSMVCannotFlattenException(Exception):
    """No SMV model has been read yet."""
    pass

class NUSMVModelAlreadyFlattenedException(Exception):
    """The model is already flattened."""
    pass
    
class NuSMVNeedFlatHierarchyException(Exception):
    """The model must be flattened."""
    pass
    
class NUSMVModelAlreadyEncodedException(Exception):
    """The model is already encoded."""
    pass
    
class NUSMVFlatModelAlreadyBuiltException(Exception):
    """The flat model is already built."""
    pass
    
class NuSMVNeedFlatModelException(Exception):
    """The model must be simplified."""
    pass
    
class NUSMVModelAlreadyBuiltException(Exception):
    """The BDD model is already built."""
    pass
    
class NuSMVNeedVariablesEncodedException(Exception):
    """The variables of the model must be encoded."""
    pass