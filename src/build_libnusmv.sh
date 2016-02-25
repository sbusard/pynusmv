#! /usr/bin/env bash
set -e


# Check the existence of libexpat, libreadline and libncurses
MACHINE=`gcc -dumpmachine`;

if [[ $MACHINE == *"darwin"* ]] # Mac OS
then
    if [ -z "$(ld -lexpat 2>&1 | grep "library not found for")" ]
    then
        libexpat="-lexpat"
    else
        libexpat=""
    fi;

    if [ -z "$(ld -lreadline 2>&1 | grep "library not found for")" ]
    then
        libreadline="-lreadline"
    else
        libreadline=""
    fi;

    if [ -z "$(ld -lncurses 2>&1 | grep "library not found for")" ]
    then
        libncurses="-lncurses"
    else
        libncurses=""
    fi;
else # Linux
    if [ -n "$(find /usr/include/ -iname *expat*)" ]
    then
        libexpat="-lexpat"
    else
        libexpat=""
    fi;

    if [ -n "$(find /usr/include/ -iname *readline*)" ]
    then
        libreadline="-lreadline"
    else
        libreadline=""
    fi;

    if [ -n "$(find /usr/include/ -iname *ncurses*)" ]
    then
        libncurses="-lncurses"
    else
        libncurses=""
    fi;
fi


here=`pwd`

mkdir -p $here/lib/
g++ -shared $libexpat $libreadline $libncurses -o lib/libnusmv.so \
nusmv/cinitDummyMac.o \
nusmv/main.o \
nusmv/src/addons_core/addonsCore.o \
nusmv/src/be/beCnf.o \
nusmv/src/be/beManager.o \
nusmv/src/be/bePkg.o \
nusmv/src/be/beRbcManager.o \
nusmv/src/bmc/bmcBmcInc.o \
nusmv/src/bmc/bmcBmcNonInc.o \
nusmv/src/bmc/bmcCheck.o \
nusmv/src/bmc/bmcCmd.o \
nusmv/src/bmc/bmcConv.o \
nusmv/src/bmc/bmcDump.o \
nusmv/src/bmc/bmcGen.o \
nusmv/src/bmc/bmcInt.o \
nusmv/src/bmc/bmcModel.o \
nusmv/src/bmc/bmcOpt.o \
nusmv/src/bmc/bmcPkg.o \
nusmv/src/bmc/bmcSimulate.o \
nusmv/src/bmc/bmcTableauLTLformula.o \
nusmv/src/bmc/bmcTableau.o \
nusmv/src/bmc/bmcTableauPLTLformula.o \
nusmv/src/bmc/bmcTest.o \
nusmv/src/bmc/bmcTrace.o \
nusmv/src/bmc/bmcUtils.o \
nusmv/src/cinit/cinitCmd.o \
nusmv/src/cinit/cinitData.o \
nusmv/src/cinit/cinitInit.o \
nusmv/src/cinit/cinitMisc.o \
nusmv/src/cinit/cinitVers.o \
nusmv/src/cmd/cmdCmd.o \
nusmv/src/cmd/cmdFile.o \
nusmv/src/cmd/cmdMisc.o \
nusmv/src/compile/compileBEval.o \
nusmv/src/compile/compileCheck.o \
nusmv/src/compile/compileCmd.o \
nusmv/src/compile/compileCone.o \
nusmv/src/compile/compileFlatten.o \
nusmv/src/compile/compileStruct.o \
nusmv/src/compile/compileUtil.o \
nusmv/src/compile/compileWrite.o \
nusmv/src/compile/compileWriteUdg.o \
nusmv/src/compile/FlatHierarchy.o \
nusmv/src/compile/PredicateExtractor.o \
nusmv/src/compile/PredicateNormaliser.o \
nusmv/src/dag/dagDfs.o \
nusmv/src/dag/dagEnStat.o \
nusmv/src/dag/dagManager.o \
nusmv/src/dag/dagStat.o \
nusmv/src/dag/dagVertex.o \
nusmv/src/dd/ddCmd.o \
nusmv/src/dd/dd.o \
nusmv/src/dd/VarsHandler.o \
nusmv/src/enc/encCmd.o \
nusmv/src/enc/enc.o \
nusmv/src/enc/operators.o \
nusmv/src/fsm/FsmBuilder.o \
nusmv/src/fsm/fsm.o \
nusmv/src/hrc/hrcCmd.o \
nusmv/src/hrc/hrcDump.o \
nusmv/src/hrc/HrcFlattener.o \
nusmv/src/hrc/HrcNode.o \
nusmv/src/hrc/hrc.o \
nusmv/src/hrc/hrcPrefixUtils.o \
nusmv/src/hrc/HrcVarDependencies.o \
nusmv/src/hrc/hrcWrite.o \
nusmv/src/ltl/ltlCmd.o \
nusmv/src/ltl/ltlCompassion.o \
nusmv/src/ltl/ltl.o \
nusmv/src/ltl/ltlRewrite.o \
nusmv/src/mc/mcAGonly.o \
nusmv/src/mc/mcCmd.o \
nusmv/src/mc/mcEval.o \
nusmv/src/mc/mcExplain.o \
nusmv/src/mc/mcInvar.o \
nusmv/src/mc/mcLE.o \
nusmv/src/mc/mcMc.o \
nusmv/src/mc/mcTrace.o \
nusmv/src/node/MasterNodeWalker.o \
nusmv/src/node/node.o \
nusmv/src/node/nodePkg.o \
nusmv/src/node/nodePrint.o \
nusmv/src/node/NodeWalker.o \
nusmv/src/node/nodeWffPrint.o \
nusmv/src/opt/optCmd.o \
nusmv/src/opt/OptsHandler.o \
nusmv/src/parser/grammar.o \
nusmv/src/parser/input.o \
nusmv/src/parser/parserCmd.o \
nusmv/src/parser/parserUtil.o \
nusmv/src/prop/propCmd.o \
nusmv/src/prop/PropDb.o \
nusmv/src/prop/Prop.o \
nusmv/src/prop/propPkg.o \
nusmv/src/rbc/ConjSet.o \
nusmv/src/rbc/InlineResult.o \
nusmv/src/rbc/rbcCnfCompact.o \
nusmv/src/rbc/rbcCnf.o \
nusmv/src/rbc/rbcCnfSimple.o \
nusmv/src/rbc/rbcFormula.o \
nusmv/src/rbc/rbcInline.o \
nusmv/src/rbc/rbcManager.o \
nusmv/src/rbc/rbcOutput.o \
nusmv/src/rbc/rbcStat.o \
nusmv/src/rbc/rbcSubst.o \
nusmv/src/rbc/rbcUtils.o \
nusmv/src/sat/SatIncSolver.o \
nusmv/src/sat/SatSolver.o \
nusmv/src/sat/satUtils.o \
nusmv/src/set/setSet.o \
nusmv/src/sexp/SexpInliner.o \
nusmv/src/simulate/simulateCmd.o \
nusmv/src/simulate/simulate.o \
nusmv/src/simulate/simulateTransSet.o \
nusmv/src/trace/pkg_trace.o \
nusmv/src/trace/traceCmd.o \
nusmv/src/trace/traceEval.o \
nusmv/src/trace/TraceLabel.o \
nusmv/src/trace/TraceManager.o \
nusmv/src/trace/Trace.o \
nusmv/src/trace/TraceOpt.o \
nusmv/src/trace/Trace_private.o \
nusmv/src/trace/traceTest.o \
nusmv/src/trace/traceUtils.o \
nusmv/src/trace/TraceXml.o \
nusmv/src/utils/array.o \
nusmv/src/utils/assoc.o \
nusmv/src/utils/avl.o \
nusmv/src/utils/error.o \
nusmv/src/utils/heap.o \
nusmv/src/utils/list.o \
nusmv/src/utils/NodeGraph.o \
nusmv/src/utils/NodeList.o \
nusmv/src/utils/object.o \
nusmv/src/utils/Olist.o \
nusmv/src/utils/Pair.o \
nusmv/src/utils/portability.o \
nusmv/src/utils/range.o \
nusmv/src/utils/Slist.o \
nusmv/src/utils/Sset.o \
nusmv/src/utils/Stack.o \
nusmv/src/utils/TimerBench.o \
nusmv/src/utils/Triple.o \
nusmv/src/utils/ucmd.o \
nusmv/src/utils/ustring.o \
nusmv/src/utils/utils_io.o \
nusmv/src/utils/utils.o \
nusmv/src/utils/WordNumber.o \
nusmv/src/wff/exprWff.o \
nusmv/src/wff/wffPkg.o \
nusmv/src/addons_core/compass/compassCmd.o \
nusmv/src/addons_core/compass/compassPkg.o \
nusmv/src/bmc/sbmc/sbmcBmcInc.o \
nusmv/src/bmc/sbmc/sbmcBmc.o \
nusmv/src/bmc/sbmc/sbmcCmd.o \
nusmv/src/bmc/sbmc/sbmcGen.o \
nusmv/src/bmc/sbmc/sbmcHash.o \
nusmv/src/bmc/sbmc/sbmcNodeStack.o \
nusmv/src/bmc/sbmc/sbmcPkg.o \
nusmv/src/bmc/sbmc/sbmcStructs.o \
nusmv/src/bmc/sbmc/sbmcTableauIncLTLformula.o \
nusmv/src/bmc/sbmc/sbmcTableauInc.o \
nusmv/src/bmc/sbmc/sbmcTableauLTLformula.o \
nusmv/src/bmc/sbmc/sbmcTableau.o \
nusmv/src/bmc/sbmc/sbmcUtils.o \
nusmv/src/compile/symb_table/NFunction.o \
nusmv/src/compile/symb_table/ResolveSymbol.o \
nusmv/src/compile/symb_table/SymbCache.o \
nusmv/src/compile/symb_table/SymbLayer.o \
nusmv/src/compile/symb_table/symb_table.o \
nusmv/src/compile/symb_table/SymbTable.o \
nusmv/src/compile/symb_table/SymbType.o \
nusmv/src/compile/type_checking/TypeChecker.o \
nusmv/src/enc/base/BaseEnc.o \
nusmv/src/enc/base/BoolEncClient.o \
nusmv/src/enc/bdd/BddEncBddPrintWff.o \
nusmv/src/enc/bdd/BddEncCache.o \
nusmv/src/enc/bdd/BddEnc.o \
nusmv/src/enc/be/BeEnc.o \
nusmv/src/enc/bool/BitValues.o \
nusmv/src/enc/bool/BoolEnc.o \
nusmv/src/enc/utils/AddArray.o \
nusmv/src/enc/utils/OrdGroups.o \
nusmv/src/enc/utils/utils.o \
nusmv/src/fsm/bdd/bddCmd.o \
nusmv/src/fsm/bdd/BddFsmCache.o \
nusmv/src/fsm/bdd/BddFsm.o \
nusmv/src/fsm/bdd/bddMisc.o \
nusmv/src/fsm/bdd/FairnessList.o \
nusmv/src/fsm/be/BeFsm.o \
nusmv/src/fsm/sexp/BoolSexpFsm.o \
nusmv/src/fsm/sexp/Expr.o \
nusmv/src/fsm/sexp/SexpFsm.o \
nusmv/src/hrc/dumpers/HrcDumperDebug.o \
nusmv/src/hrc/dumpers/HrcDumper.o \
nusmv/src/hrc/dumpers/HrcDumperSmv.o \
nusmv/src/hrc/dumpers/HrcDumperXml.o \
nusmv/src/ltl/ltl2smv/ltl2smv.o \
nusmv/src/node/normalizers/MasterNormalizer.o \
nusmv/src/node/normalizers/NormalizerBase.o \
nusmv/src/node/normalizers/NormalizerCore.o \
nusmv/src/node/normalizers/NormalizerPsl.o \
nusmv/src/node/printers/MasterPrinter.o \
nusmv/src/node/printers/PrinterBase.o \
nusmv/src/node/printers/PrinterIWffCore.o \
nusmv/src/node/printers/PrinterPsl.o \
nusmv/src/node/printers/PrinterSexpCore.o \
nusmv/src/node/printers/PrinterWffCore.o \
nusmv/src/parser/idlist/idlist_grammar.o \
nusmv/src/parser/idlist/idlist_input.o \
nusmv/src/parser/idlist/ParserIdList.o \
nusmv/src/parser/ord/ord_grammar.o \
nusmv/src/parser/ord/ord_input.o \
nusmv/src/parser/ord/ParserOrd.o \
nusmv/src/parser/psl/pslConv.o \
nusmv/src/parser/psl/pslExpr.o \
nusmv/src/parser/psl/psl_grammar.o \
nusmv/src/parser/psl/psl_input.o \
nusmv/src/parser/psl/pslNode.o \
nusmv/src/rbc/clg/clgClg.o \
nusmv/src/trace/eval/BaseEvaluator.o \
nusmv/src/trace/exec/BaseTraceExecutor.o \
nusmv/src/trace/exec/BDDCompleteTraceExecutor.o \
nusmv/src/trace/exec/BDDPartialTraceExecutor.o \
nusmv/src/trace/exec/CompleteTraceExecutor.o \
nusmv/src/trace/exec/PartialTraceExecutor.o \
nusmv/src/trace/exec/SATCompleteTraceExecutor.o \
nusmv/src/trace/exec/SATPartialTraceExecutor.o \
nusmv/src/trace/exec/traceExec.o \
nusmv/src/trace/loaders/TraceLoader.o \
nusmv/src/trace/loaders/TraceXmlLoader.o \
nusmv/src/trace/plugins/TraceCompact.o \
nusmv/src/trace/plugins/TraceExplainer.o \
nusmv/src/trace/plugins/TracePlugin.o \
nusmv/src/trace/plugins/TraceTable.o \
nusmv/src/trace/plugins/TraceXmlDumper.o \
nusmv/src/trans/bdd/BddTrans.o \
nusmv/src/trans/bdd/ClusterList.o \
nusmv/src/trans/bdd/Cluster.o \
nusmv/src/trans/bdd/ClusterOptions.o \
nusmv/src/trans/generic/GenericTrans.o \
nusmv/src/wff/w2w/wff2nnf.o \
nusmv/src/addons_core/compass/compile/compile.o \
nusmv/src/addons_core/compass/compile/ProbAssign.o \
nusmv/src/addons_core/compass/sigref/sigrefWrite.o \
nusmv/src/compile/type_checking/checkers/CheckerBase.o \
nusmv/src/compile/type_checking/checkers/CheckerCore.o \
nusmv/src/compile/type_checking/checkers/CheckerPsl.o \
nusmv/src/compile/type_checking/checkers/checkersInt.o \
nusmv/src/compile/type_checking/checkers/CheckerStatement.o \
nusmv/src/addons_core/compass/parser/ap/ap_grammar.o \
nusmv/src/addons_core/compass/parser/ap/ap_input.o \
nusmv/src/addons_core/compass/parser/ap/ParserAp.o \
nusmv/src/addons_core/compass/parser/prob/ParserProb.o \
nusmv/src/addons_core/compass/parser/prob/prob_grammar.o \
nusmv/src/addons_core/compass/parser/prob/prob_input.o \
cudd-2.4.1.1/lib/*.a