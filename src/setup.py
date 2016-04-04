from setuptools import setup, find_packages
from setuptools.extension import Extension
import sys
import os

include_dirs = ['nusmv', 'nusmv/src', 'cudd-2.4.1.1/include']
if sys.version_info[0] >= 3:
    swig_opts = ['-py3']
else:
    swig_opts = []
libraries = ['nusmv']
runtime_library_dirs = ['lib']
library_dirs = ['lib']
extensions = []

# addons_core modules
extensions.append(
    Extension('pynusmv.nusmv.addons_core._addons_core',
                ['pynusmv/nusmv/addons_core/addons_core.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/addons_core/addonsCore.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/addons_core/addons_core.py',
                            'pynusmv/nusmv/addons_core/addons_core_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.addons_core.compass._compass',
                ['pynusmv/nusmv/addons_core/compass/compass.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/addons_core/compass/compass.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/addons_core/compass/compass.py',
                            'pynusmv/nusmv/addons_core/compass/compass_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.addons_core.compass.compile._compile',
                ['pynusmv/nusmv/addons_core/compass/compile/compile.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/addons_core/compass/compile/ProbAssign.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/addons_core/compass/compile/compile.py',
                            'pynusmv/nusmv/addons_core/compass/compile/compile_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.addons_core.compass.parser.ap._ap',
                ['pynusmv/nusmv/addons_core/compass/parser/ap/ap.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/addons_core/compass/parser/ap/ap_grammar.h',
                            'nusmv/src/addons_core/compass/parser/ap/ParserAp.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/addons_core/compass/parser/ap/ap.py',
                            'pynusmv/nusmv/addons_core/compass/parser/ap/ap_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.addons_core.compass.parser.prob._prob',
                ['pynusmv/nusmv/addons_core/compass/parser/prob/prob.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/addons_core/compass/parser/prob/ParserProb.h',
                            'nusmv/src/addons_core/compass/parser/prob/prob_grammar.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/addons_core/compass/parser/prob/prob.py',
                            'pynusmv/nusmv/addons_core/compass/parser/prob/prob_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# be module
extensions.append(
     Extension('pynusmv.nusmv.be._be',
                ['pynusmv/nusmv/be/be.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/be/be.h',
                            'nusmv/src/be/bePkg.h',
                            'nusmv/src/be/beRbcManager.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/be/be.py',
                            'pynusmv/nusmv/be/be_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )

# bmc modules
extensions.append(
     Extension('pynusmv.nusmv.bmc._bmc',
                ['pynusmv/nusmv/bmc/bmc.i'],
                depends = [ 'nusmv/src/utils/defs.h',
                            'nusmv/src/bmc/bmc.h',
                            'nusmv/src/bmc/bmcBmc.h',
                            'nusmv/src/bmc/bmcCheck.h',
                            'nusmv/src/bmc/bmcCmd.h',
                            'nusmv/src/bmc/bmcConv.h',
                            'nusmv/src/bmc/bmcDump.h',
                            'nusmv/src/bmc/bmcGen.h',
                            'nusmv/src/bmc/bmcModel.h',
                            'nusmv/src/bmc/bmcPkg.h',
                            'nusmv/src/bmc/bmcSimulate.h',
                            'nusmv/src/bmc/bmcTableau.h',
                            'nusmv/src/bmc/bmcUtils.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/bmc/bmc.py',
                            'pynusmv/nusmv/bmc/bmc_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.bmc.sbmc._sbmc',
                ['pynusmv/nusmv/bmc/sbmc/sbmc.i'],
                depends = [ 'nusmv/src/utils/defs.h',
                            'nusmv/src/bmc/sbmc/sbmcBmc.h',
                            'nusmv/src/bmc/sbmc/sbmcBmcInc.h',
                            'nusmv/src/bmc/sbmc/sbmcCmd.h',
                            'nusmv/src/bmc/sbmc/sbmcGen.h',
                            'nusmv/src/bmc/sbmc/sbmcHash.h',
                            'nusmv/src/bmc/sbmc/sbmcNodeStack.h',
                            'nusmv/src/bmc/sbmc/sbmcPkg.h',
                            'nusmv/src/bmc/sbmc/sbmcStructs.h',
                            'nusmv/src/bmc/sbmc/sbmcTableau.h',
                            'nusmv/src/bmc/sbmc/sbmcTableauInc.h',
                            'nusmv/src/bmc/sbmc/sbmcTableauIncLTLformula.h',
                            'nusmv/src/bmc/sbmc/sbmcTableauLTLformula.h',
                            'nusmv/src/bmc/sbmc/sbmcUtils.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/bmc/sbmc/sbmc.py',
                            'pynusmv/nusmv/bmc/sbmc/sbmc_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# cinit module
extensions.append(
     Extension('pynusmv.nusmv.cinit._cinit',
                ['pynusmv/nusmv/cinit/cinit.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/cinit/cinit.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/cinit/cinit.py',
                            'pynusmv/nusmv/cinit/cinit_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# cmd module
extensions.append(
     Extension('pynusmv.nusmv.cmd._cmd',
                ['pynusmv/nusmv/cmd/cmd.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/cmd/cmd.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/cmd/cmd.py',
                            'pynusmv/nusmv/cmd/cmd_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# compile modules
extensions.append(
     Extension('pynusmv.nusmv.compile._compile',
                ['pynusmv/nusmv/compile/compile.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/compile/compile.h',
                            'nusmv/src/compile/FlatHierarchy.h',
                            'nusmv/src/compile/PredicateExtractor.h',
                            'nusmv/src/compile/PredicateNormaliser.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/compile/compile.py',
                            'pynusmv/nusmv/compile/compile_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.compile.symb_table._symb_table',
                ['pynusmv/nusmv/compile/symb_table/symb_table.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/compile/symb_table/NFunction.h',
                            'nusmv/src/compile/symb_table/ResolveSymbol.h',
                            'nusmv/src/compile/symb_table/symb_table.h',
                            'nusmv/src/compile/symb_table/SymbCache.h',
                            'nusmv/src/compile/symb_table/SymbLayer.h',
                            'nusmv/src/compile/symb_table/SymbTable.h',
                            'nusmv/src/compile/symb_table/SymbType.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/compile/symb_table/symb_table.py',
                            'pynusmv/nusmv/compile/symb_table/symb_table_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.compile.type_checking._type_checking',
                ['pynusmv/nusmv/compile/type_checking/type_checking.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/compile/type_checking/TypeChecker.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/compile/type_checking/type_checking.py',
                            'pynusmv/nusmv/compile/type_checking/type_checking_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.compile.type_checking.checkers._checkers',
                ['pynusmv/nusmv/compile/type_checking/checkers/checkers.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/compile/type_checking/checkers/CheckerBase.h',
                            'nusmv/src/compile/type_checking/checkers/CheckerCore.h',
                            'nusmv/src/compile/type_checking/checkers/CheckerPsl.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/compile/type_checking/checkers/checkers.py',
                            'pynusmv/nusmv/compile/type_checking/checkers/checkers_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# dag module
extensions.append(
     Extension('pynusmv.nusmv.dag._dag',
                ['pynusmv/nusmv/dag/dag.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/dag/dag.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/dag/dag.py',
                            'pynusmv/nusmv/dag/dag_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# dd module
extensions.append(
     Extension('pynusmv.nusmv.dd._dd',
                ['pynusmv/nusmv/dd/dd.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/dd/dd.h',
                            'nusmv/src/dd/VarsHandler.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/dd/dd.py',
                            'pynusmv/nusmv/dd/dd_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# enc modules
extensions.append(
     Extension('pynusmv.nusmv.enc._enc',
                ['pynusmv/nusmv/enc/enc.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/enc/enc.h',
                            'nusmv/src/enc/operators.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/enc/enc.py',
                            'pynusmv/nusmv/enc/enc_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.enc.base._base',
                ['pynusmv/nusmv/enc/base/base.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/enc/base/BaseEnc.h',
                            'nusmv/src/enc/base/BoolEncClient.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/enc/base/base.py',
                            'pynusmv/nusmv/enc/base/base_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.enc.bdd._bdd',
                ['pynusmv/nusmv/enc/bdd/bdd.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/enc/bdd/bdd.h',
                            'nusmv/src/enc/bdd/BddEnc.h',
                            'nusmv/src/enc/bdd/BddEncCache.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/enc/bdd/bdd.py',
                            'pynusmv/nusmv/enc/bdd/bdd_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.enc.be._be',
                ['pynusmv/nusmv/enc/be/be.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/enc/be/BeEnc.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/enc/be/be.py',
                            'pynusmv/nusmv/enc/be/be_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.enc.bool._bool',
                ['pynusmv/nusmv/enc/bool/bool.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/enc/bool/BitValues.h',
                            'nusmv/src/enc/bool/BoolEnc.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/enc/bool/bool.py',
                            'pynusmv/nusmv/enc/bool/bool_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.enc.utils._utils',
                ['pynusmv/nusmv/enc/utils/utils.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/enc/utils/AddArray.h',
                            'nusmv/src/enc/utils/OrdGroups.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/enc/utils/utils.py',
                            'pynusmv/nusmv/enc/utils/utils_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# fsm modules
extensions.append(
     Extension('pynusmv.nusmv.fsm._fsm',
                ['pynusmv/nusmv/fsm/fsm.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/fsm/fsm.h',
                            'nusmv/src/fsm/FsmBuilder.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/fsm/fsm.py',
                            'pynusmv/nusmv/fsm/fsm_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.fsm.bdd._bdd',
                ['pynusmv/nusmv/fsm/bdd/bdd.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/fsm/bdd/bdd.h',
                            'nusmv/src/fsm/bdd/BddFsm.h',
                            'nusmv/src/fsm/bdd/FairnessList.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/fsm/bdd/bdd.py',
                            'pynusmv/nusmv/fsm/bdd/bdd_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.fsm.be._be',
                ['pynusmv/nusmv/fsm/be/be.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/fsm/be/BeFsm.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/fsm/be/be.py',
                            'pynusmv/nusmv/fsm/be/be_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.fsm.sexp._sexp',
                ['pynusmv/nusmv/fsm/sexp/sexp.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/fsm/sexp/BoolSexpFsm.h',
                            'nusmv/src/fsm/sexp/Expr.h',
                            'nusmv/src/fsm/sexp/sexp.h',
                            'nusmv/src/fsm/sexp/SexpFsm.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/fsm/sexp/sexp.py',
                            'pynusmv/nusmv/fsm/sexp/sexp_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# hrc modules
extensions.append(
     Extension('pynusmv.nusmv.hrc._hrc',
                ['pynusmv/nusmv/hrc/hrc.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/hrc/hrc.h',
                            'nusmv/src/hrc/hrcCmd.h',
                            'nusmv/src/hrc/HrcFlattener.h',
                            'nusmv/src/hrc/HrcNode.h',
                            'nusmv/src/hrc/hrcPrefixUtils.h',
                            'nusmv/src/hrc/HrcVarDependencies.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/hrc/hrc.py',
                            'pynusmv/nusmv/hrc/hrc_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.hrc.dumpers._dumpers',
                ['pynusmv/nusmv/hrc/dumpers/dumpers.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/hrc/dumpers/HrcDumper.h',
                            'nusmv/src/hrc/dumpers/HrcDumperDebug.h',
                            'nusmv/src/hrc/dumpers/HrcDumperSmv.h',
                            'nusmv/src/hrc/dumpers/HrcDumperXml.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/hrc/dumpers/dumpers.py',
                            'pynusmv/nusmv/hrc/dumpers/dumpers_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# ltl modules
extensions.append(
     Extension('pynusmv.nusmv.ltl._ltl',
                ['pynusmv/nusmv/ltl/ltl.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/ltl/ltl.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/ltl/ltl.py',
                            'pynusmv/nusmv/ltl/ltl_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.ltl.ltl2smv._ltl2smv',
                ['pynusmv/nusmv/ltl/ltl2smv/ltl2smv.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/ltl/ltl2smv/ltl2smv.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/ltl/ltl2smv/ltl2smv.py',
                            'pynusmv/nusmv/ltl/ltl2smv/ltl2smv_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# mc module
extensions.append(
     Extension('pynusmv.nusmv.mc._mc',
                ['pynusmv/nusmv/mc/mc.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/mc/mc.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/mc/mc.py',
                            'pynusmv/nusmv/mc/mc_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# node modules
extensions.append(
     Extension('pynusmv.nusmv.node._node',
                ['pynusmv/nusmv/node/node.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/node/MasterNodeWalker.h',
                            'nusmv/src/node/node.h',
                            'nusmv/src/node/NodeWalker.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/node/node.py',
                            'pynusmv/nusmv/node/node_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.node.normalizers._normalizers',
                ['pynusmv/nusmv/node/normalizers/normalizers.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/node/normalizers/MasterNormalizer.h',
                            'nusmv/src/node/normalizers/NormalizerBase.h',
                            'nusmv/src/node/normalizers/NormalizerCore.h',
                            'nusmv/src/node/normalizers/NormalizerPsl.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/node/normalizers/normalizers.py',
                            'pynusmv/nusmv/node/normalizers/normalizers_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.node.printers._printers',
                ['pynusmv/nusmv/node/printers/printers.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/node/printers/MasterPrinter.h',
                            'nusmv/src/node/printers/PrinterBase.h',
                            'nusmv/src/node/printers/PrinterIWffCore.h',
                            'nusmv/src/node/printers/PrinterPsl.h',
                            'nusmv/src/node/printers/PrinterSexpCore.h',
                            'nusmv/src/node/printers/PrinterWffCore.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/node/printers/printers.py',
                            'pynusmv/nusmv/node/printers/printers_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# opt module
extensions.append(
     Extension('pynusmv.nusmv.opt._opt',
                ['pynusmv/nusmv/opt/opt.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/opt/opt.h',
                            'nusmv/src/opt/OptsHandler.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/opt/opt.py',
                            'pynusmv/nusmv/opt/opt_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# parser modules
extensions.append(
     Extension('pynusmv.nusmv.parser._parser',
                ['pynusmv/nusmv/parser/parser.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/node/node.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/parser/grammar.h',
                            'nusmv/src/parser/parser.h',
                            'nusmv/src/parser/symbols.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/parser/parser.py',
                            'pynusmv/nusmv/parser/parser_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.parser.idlist._idlist',
                ['pynusmv/nusmv/parser/idlist/idlist.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/node/node.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/parser/idlist/idlist_grammar.h',
                            'nusmv/src/parser/idlist/ParserIdList.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/parser/idlist/idlist.py',
                            'pynusmv/nusmv/parser/idlist/idlist_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.parser.ord._ord',
                ['pynusmv/nusmv/parser/ord/ord.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/node/node.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/parser/ord/ord_grammar.h',
                            'nusmv/src/parser/ord/ParserOrd.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/parser/ord/ord.py',
                            'pynusmv/nusmv/parser/ord/ord_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.parser.psl._psl',
                ['pynusmv/nusmv/parser/psl/psl.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/node/node.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/parser/psl/psl_grammar.h',
                            'nusmv/src/parser/psl/psl_symbols.h',
                            'nusmv/src/parser/psl/pslExpr.h',
                            'nusmv/src/parser/psl/pslNode.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/parser/psl/psl.py',
                            'pynusmv/nusmv/parser/psl/psl_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# prop module
extensions.append(
     Extension('pynusmv.nusmv.prop._prop',
                ['pynusmv/nusmv/prop/prop.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/prop/Prop.h',
                            'nusmv/src/prop/PropDb.h',
                            'nusmv/src/prop/propPkg.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/prop/prop.py',
                            'pynusmv/nusmv/prop/prop_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )

# rbc modules
extensions.append(
     Extension('pynusmv.nusmv.rbc._rbc',
                ['pynusmv/nusmv/rbc/rbc.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/rbc/ConjSet.h',
                            'nusmv/src/rbc/InlineResult.h',
                            'nusmv/src/rbc/rbc.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/rbc/rbc.py',
                            'pynusmv/nusmv/rbc/rbc_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.rbc.clg._clg',
                ['pynusmv/nusmv/rbc/clg/clg.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/rbc/clg/clg.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/rbc/clg/clg.py',
                            'pynusmv/nusmv/rbc/clg/clg_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# sat module
extensions.append(
     Extension('pynusmv.nusmv.sat._sat',
                ['pynusmv/nusmv/sat/sat.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/sat/sat.h',
                            'nusmv/src/sat/SatIncSolver.h',
                            'nusmv/src/sat/SatSolver.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/sat/sat.py',
                            'pynusmv/nusmv/sat/sat_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# set module
extensions.append(
     Extension('pynusmv.nusmv.set._set',
                ['pynusmv/nusmv/set/set.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/set/set.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/set/set.py',
                            'pynusmv/nusmv/set/set_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# sexp module
extensions.append(
     Extension('pynusmv.nusmv.sexp._sexp',
                ['pynusmv/nusmv/sexp/sexp.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/sexp/SexpInliner.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/sexp/sexp.py',
                            'pynusmv/nusmv/sexp/sexp_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# simulate module
extensions.append(
     Extension('pynusmv.nusmv.simulate._simulate',
                ['pynusmv/nusmv/simulate/simulate.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/simulate/simulate.h',
                            'nusmv/src/simulate/simulateTransSet.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/simulate/simulate.py',
                            'pynusmv/nusmv/simulate/simulate_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )

# trace modules
extensions.append(
     Extension('pynusmv.nusmv.trace._trace',
                ['pynusmv/nusmv/trace/trace.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/trace/pkg_trace.h',
                            'nusmv/src/trace/Trace.h',
                            'nusmv/src/trace/TraceLabel.h',
                            'nusmv/src/trace/TraceManager.h',
                            'nusmv/src/trace/TraceOpt.h',
                            'nusmv/src/trace/TraceXml.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/trace/trace.py',
                            'pynusmv/nusmv/trace/trace_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.trace.eval._eval',
                ['pynusmv/nusmv/trace/eval/eval.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/trace/eval/BaseEvaluator.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/trace/eval/eval.py',
                            'pynusmv/nusmv/trace/eval/eval_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.trace.exec_._exec_',
                ['pynusmv/nusmv/trace/exec_/exec_.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/trace/exec/BaseTraceExecutor.h',
                            'nusmv/src/trace/exec/BDDCompleteTraceExecutor.h',
                            'nusmv/src/trace/exec/BDDPartialTraceExecutor.h',
                            'nusmv/src/trace/exec/CompleteTraceExecutor.h',
                            'nusmv/src/trace/exec/PartialTraceExecutor.h',
                            'nusmv/src/trace/exec/SATCompleteTraceExecutor.h',
                            'nusmv/src/trace/exec/SATPartialTraceExecutor.h',
                            'nusmv/src/trace/exec/traceExec.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/trace/exec_/exec_.py',
                            'pynusmv/nusmv/trace/exec_/exec__wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.trace.loaders._loaders',
                ['pynusmv/nusmv/trace/loaders/loaders.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/trace/loaders/TraceLoader.h',
                            'nusmv/src/trace/loaders/TraceXmlLoader.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/trace/loaders/loaders.py',
                            'pynusmv/nusmv/trace/loaders/loaders_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.trace.plugins._plugins',
                ['pynusmv/nusmv/trace/plugins/plugins.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/trace/plugins/TraceCompact.h',
                            'nusmv/src/trace/plugins/TraceExplainer.h',
                            'nusmv/src/trace/plugins/TracePlugin.h',
                            'nusmv/src/trace/plugins/TraceTable.h',
                            'nusmv/src/trace/plugins/TraceXmlDumper.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/trace/plugins/plugins.py',
                            'pynusmv/nusmv/trace/plugins/plugins_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )   

# trans modules
extensions.append(
     Extension('pynusmv.nusmv.trans._trans',
                ['pynusmv/nusmv/trans/trans.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/trans/trans.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/trans/trans.py',
                            'pynusmv/nusmv/trans/trans_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.trans.bdd._bdd',
                ['pynusmv/nusmv/trans/bdd/bdd.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/trans/bdd/bdd.h',
                            'nusmv/src/trans/bdd/BddTrans.h',
                            'nusmv/src/trans/bdd/Cluster.h',
                            'nusmv/src/trans/bdd/ClusterList.h',
                            'nusmv/src/trans/bdd/ClusterOptions.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/trans/bdd/bdd.py',
                            'pynusmv/nusmv/trans/bdd/bdd_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.trans.generic._generic',
                ['pynusmv/nusmv/trans/generic/generic.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/trans/generic/GenericTrans.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/trans/generic/generic.py',
                            'pynusmv/nusmv/trans/generic/generic_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# utils module
extensions.append(
     Extension('pynusmv.nusmv.utils._utils',
                ['pynusmv/nusmv/utils/utils.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/utils/utils.h',
                            'nusmv/src/utils/array.h',
                            'nusmv/src/utils/assoc.h',
                            'nusmv/src/utils/avl.h',
                            'nusmv/src/utils/error.h',
                            'nusmv/src/utils/heap.h',
                            'nusmv/src/utils/list.h',
                            'nusmv/src/utils/NodeGraph.h',
                            'nusmv/src/utils/NodeList.h',
                            'nusmv/src/utils/object.h',
                            'nusmv/src/utils/Olist.h',
                            'nusmv/src/utils/Pair.h',
                            'nusmv/src/utils/portability.h',
                            'nusmv/src/utils/range.h',
                            'nusmv/src/utils/Slist.h',
                            'nusmv/src/utils/Sset.h',
                            'nusmv/src/utils/Stack.h',
                            'nusmv/src/utils/TimerBench.h',
                            'nusmv/src/utils/Triple.h',
                            'nusmv/src/utils/ucmd.h',
                            'nusmv/src/utils/ustring.h',
                            'nusmv/src/utils/utils_io.h',
                            'nusmv/src/utils/WordNumber.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/utils/utils.py',
                            'pynusmv/nusmv/utils/utils_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
    
# wff modules
extensions.append(
     Extension('pynusmv.nusmv.wff._wff',
                ['pynusmv/nusmv/wff/wff.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/wff/wff.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/wff/wff.py',
                            'pynusmv/nusmv/wff/wff_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )
extensions.append(
     Extension('pynusmv.nusmv.wff.w2w._w2w',
                ['pynusmv/nusmv/wff/w2w/w2w.i'],
                depends = [ 'nusmv/nusmv-config.h',
                            'nusmv/src/utils/defs.h',
                            'nusmv/src/wff/w2w/w2w.h',
                            'lib/libnusmv.so',
                            
                            'pynusmv/nusmv/typedefs.tpl',
                            
                            'pynusmv/nusmv/wff/w2w/w2w.py',
                            'pynusmv/nusmv/wff/w2w/w2w_wrap.c'],
                swig_opts = swig_opts, libraries = libraries,
                library_dirs = library_dirs, include_dirs = include_dirs,
                runtime_library_dirs = runtime_library_dirs)
    )


def get_packages(packages):
    """
    Return all the packages and (recursively) sub-packages of packages.
    
    packages -- a list of package names.
    """
    top_packages = packages
    packages = list(top_packages)
    for package in top_packages:
        packages += [package + "." + sub for sub in find_packages(package)]
    return packages

# Get all packages (recursively)
packages = get_packages(["pynusmv", "tools"])
if os.path.isfile('pyparsing.py'):
    packages.append(".")


setup(  name = "PyNuSMV",
        version = "0.11",
        author = "Simon Busard",
        author_email = "simon.busard@uclouvain.be",
        url = "http://lvl.info.ucl.ac.be/Tools/PyNuSMV",
        description = "Python interface for NuSMV.",
        packages=packages,
        # Add files in runtime_library_dirs (to include libnusmv library)
        data_files=[(rld,
                     [os.path.join(rld, f)
                      for f in os.listdir(rld)
                      if os.path.isfile(os.path.join(rld, f))]
                     )
                    for rld in runtime_library_dirs],
        ext_modules = extensions)