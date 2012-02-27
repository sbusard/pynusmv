/**CFile***********************************************************************

  FileName    [sbmcCmd.c]

  PackageName [bmc.sbmc]

  Synopsis    [Bmc.Cmd module]

  Description [This module contains all the sbmc commands implementation.
  Options parsing and checking is performed here, than the high-level SBMC
  layer is called]

  SeeAlso     [bmcPkg.c, bmcBmc.c]

  Author      [Timo Latvala, Tommi Juntilla, Marco Roveri

  Copyright   [
  This file is part of the ``bmc.sbmc'' package of NuSMV version 2.
  Copyright (C) 2004 Timo Latvala <timo.latvala@tkk.fi>
  Copyright (C) 2006 Tommi Junttila.

  NuSMV version 2 is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

  For more information of NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "sbmcCmd.h"
#include "sbmcBmc.h"
#include "sbmcBmcInc.h"
#include "sbmcPkg.h"

#include "bmc/bmc.h"
#include "bmc/bmcInt.h"
#include "bmc/bmcUtils.h"

#include "prop/Prop.h"
#include "prop/PropDb.h"
#include "prop/propPkg.h"
#include "enc/enc.h"


static char rcsid[] UTIL_UNUSED = "$Id: sbmcCmd.c,v 1.1.2.9.4.7 2010-02-19 15:05:22 nusmv Exp $";

/* ---------------------------------------------------------------------- */


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int UsageSBMCCheckLtlSpec  ARGS((void));
static int UsageSBMCGenLtlSpec    ARGS((void));
static int UsageSBMCIncCheck      ARGS((void));

static Outcome
sbmc_cmd_options_handling ARGS((int argc, char** argv,
                                Prop_Type prop_type,

                                /* output parameters: */
                                Prop_ptr* res_prop,
                                int* res_k,
                                int* res_l,
                                char** res_o,
                                boolean* res_N,
                                boolean* res_c,
                                boolean* res_1));

/**AutomaticEnd***************************************************************/



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Uses Kepa's and Timo's method for doing bmc]

  Description        [Model check using sbmc]

  SideEffects        [None]

  SeeAlso            []

  CommandName        [check_ltlspec_sbmc]

  CommandSynopsis    [Finds error up to depth k]

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\]\ | -P
  "name"\] \[-k max_length\] \[-l loopback\] \[-1\] \[-o filename\]]

  CommandDescription [
  This command generates one or more problems, and calls
  SAT solver for each one. Each problem is related to a specific problem
  bound, which increases from zero (0) to the given maximum problem
  length. Here "<i>length</i>" is the bound of the problem that system
  is going to generate and/or solve. <BR>
  In this context the maximum problem bound is represented by the
  <i>-k</i> command parameter, or by its default value stored in the
  environment variable <i>bmc_length</i>.<BR>
  The single generated problem also depends on the "<i>loopback</i>"
  parameter you can explicitly specify by the <i>-l</i> option, or by its
  default value stored in the environment variable <i>bmc_loopback</i>. <BR>
  The property to be checked may be specified using the <i>-n idx</i> or
  the <i>-p "formula"</i> options. <BR>
  If you need to generate a dimacs dump file of all generated problems, you
  must use the option <i>-o "filename"</i>. <BR>
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification
       formula actually located in the properties database. <BR>
    <dt> <tt>-p "formula" \[IN context\]</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-P name</tt>
       <dd> Checks the LTLSPEC property with name <tt>name</tt> in the property
            database.
    <dt> <tt>-k <i>max_length</i></tt>
       <dd> <i>max_length</i> is the maximum problem bound must be reached.
       Only natural number are valid values for this option. If no value
       is given the environment variable <i>bmc_length</i> is considered
       instead.
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>max_length-1</i>). Positive sign ('+') can
       be also used as prefix of the number. Any invalid combination of length
       and loopback will be skipped during the generation/solving process.<BR>
       - a negative number in (-1, -<i>bmc_length</i>). In this case
       <i>loopback</i> is considered a value relative to <i>max_length</i>.
       Any invalid combination of length and loopback will be skipped
       during the generation/solving process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to
       <i>length-1</i>"
    <dt> <tt>-1</tt>
       <dd> Generates and solves a single problem with length <tt>k</tt>
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of the dumped dimacs file.
       It may contain special symbols which will be macro-expanded to form
       the real file name. Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @k: current problem bound <BR>
       - @l: current loopback value <BR>
       - @n: index of the currently processed formula in the properties
       database <BR>
       - @@: the '@' character
  </dl>

  For further information about this implementation see:
  T. Latvala, A. Biere, K. Heljanko, and T. Junttila. Simple is
  Better: Efficient Bounded Model Checking for Past LTL. In: R. Cousot
  (ed.), Verification, Model Checking, and Abstract Interpretation,
  6th International Conference VMCAI 2005, Paris, France, Volume 3385
  of LNCS, pp. 380-395, Springer, 2005.  Copyright ©
  Springer-Verlag.

  ]

******************************************************************************/
int Sbmc_CommandCheckLtlSpecSBmc(int argc, char** argv)
{
  Prop_ptr ltlprop=(Prop_ptr)NULL;   /* The property being processed */
  Outcome opt_handling_res;
  int k = get_bmc_pb_length(OptsHandler_get_instance());
  boolean single_prob = false;
  char *fname = (char *)NULL;
  int relative_loop =
    Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(OptsHandler_get_instance()), NULL);

  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = sbmc_cmd_options_handling(argc, argv,
                                               Prop_Ltl, &ltlprop,
                                               &k, &relative_loop,
                                               &fname,
                                               NULL, /* -N */
                                               NULL /* -c */,
                                               &single_prob);

  if (opt_handling_res == OUTCOME_SUCCESS_REQUIRED_HELP) {
    return UsageSBMCCheckLtlSpec();
  }
  if (opt_handling_res != OUTCOME_SUCCESS) {
    if (fname != (char *)NULL) FREE(fname);
    return 1;
  }

  /* makes sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* ----------------------------------------------------------------------- */

  /* prepares the list of properties if no property was selected: */
  if (ltlprop == NIL_PTR(Prop_ptr)) {
    lsList props;
    lsGen  iterator;
    Prop_ptr prop;

    if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
      props = PropDb_get_ordered_props_of_type(PropPkg_get_prop_database(),
                                               mainFlatHierarchy,
                                               Prop_Ltl);
    else props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                          Prop_Ltl);

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_SBMCGenSolveLtl(prop, k, relative_loop,
                          !single_prob, /* iterate on k */
                          true, /* solve */
                          (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
                          fname) != 0) {
        if (fname != (char*) NULL) FREE(fname);
        lsDestroy(props, NULL); /* the list is no longer needed */
        return 1;
      }
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* it's time to solve (a single property): */
    if (Bmc_SBMCGenSolveLtl(ltlprop, k, relative_loop,
                        !single_prob, /* iterate on k */
                        true, /* solve */
                        (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
                        fname) != 0) {
      if (fname != (char*) NULL) FREE(fname);
      return 1;
    }
  }

  if (fname != (char*) NULL) FREE(fname);

  return 0;
}

static int UsageSBMCCheckLtlSpec(void)
{
  fprintf(nusmv_stderr, "\nusage: check_ltlspec_sbmc [-h | -n idx | -p \"formula\"] [-k max_length] [-l loopback]\n\t\t\t [-1] [-o <filename>]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx>\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property\n");
  fprintf(nusmv_stderr, "  -P \"name\"\tChecks the LTL property specified with <name>.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties.\n");
  fprintf(nusmv_stderr, "  -k max_length\tChecks the property using <max_length> value instead of using the\n\t\tvariable <bmc_length> value\n");
  fprintf(nusmv_stderr, "  -l loopback\tChecks the property using <loopback> value instead of using the\n\t\tvariable <bmc_loopback> value\n");
  fprintf(nusmv_stderr, "  -1 \t\tGenerates and solves a single problem.\n");
  fprintf(nusmv_stderr, "  -o filename\tGenerates dimacs output file too. <filename> may contain patterns\n\n");

  return 1;
}


/**Function********************************************************************

  Synopsis           [Generate length_max+1 problems iterating the problem
  bound from zero to length_max, and dumps each problem to a dimacs file.
  Uses Kepa's and Timo's method for doing bmc]

  Description        [Each problem is dumped for the given LTL specification,
  or for all LTL specifications if no formula is given.
  Generation parameters are the maximum bound and the loopback values. <BR>
  After command line processing it calls the function Bmc_SBMCGenSolveLtl
  to generate and dump all problems from zero to k.Uses Kepa's and Timo's method for doing bmc ]

  SideEffects        [None]

  SeeAlso            []

  CommandName        [gen_ltlspec_sbmc]

  CommandSynopsis    [Dumps into one or more dimacs files the given LTL
  specification, or all LTL specifications if no formula is given.
  Generation and dumping parameters are the maximum bound and the loopback
  values. Uses Kepa's and Timo's method for doing bmc.]

 CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\] | -P "name"\]
  \[-k max_length\] \[-l loopback\] \[-1\] \[-o filename\]]

  CommandDescription [ This command generates one or more problems, and
  dumps each problem into a dimacs file. Each problem is related to a specific
  problem bound, which increases from zero (0) to the given maximum problem
  bound. In this short description "<i>length</i>" is the bound of the
  problem that system is going to dump out. Uses Kepa's and Timo's method for doing bmc. <BR>
  In this context the maximum problem bound is represented by the
  <i>max_length</i> parameter, or by its default value stored in the
  environment variable <i>bmc_length</i>.<BR>
  Each dumped problem also depends on the loopback you can explicitly
  specify by the <i>-l</i> option, or by its default value stored in the
  environment variable <i>bmc_loopback</i>. <BR>
  The property to be checked may be specified using the <i>-n idx</i> or
  the <i>-p "formula"</i> options. <BR>
  You may specify dimacs file name by using the option <i>-o "filename"</i>,
  otherwise the default value stored in the environment variable
  <i>bmc_dimacs_filename</i> will be considered.<BR>
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification
       formula actually located in the properties database. <BR>
    <dt> <tt>-p "formula" \[IN context\]</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-P name</tt>
       <dd> Checks the LTLSPEC property with name <tt>name</tt> in the property
            database.
    <dt> <tt>-k <i>max_length</i></tt>
       <dd> <i>max_length</i> is the maximum problem bound must be reached.
       Only natural number are valid values for this option. If no value
       is given the environment variable <i>bmc_length</i> is considered
       instead.
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>max_length-1</i>). Positive sign ('+') can
       be also used as prefix of the number. Any invalid combination of length
       and loopback will be skipped during the generation/solving process.<BR>
       - a negative number in (-1, -<i>bmc_length</i>). In this case
       <i>loopback</i> is considered a value relative to <i>max_length</i>.
       Any invalid combination of length and loopback will be skipped
       during the generation/solving process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to
       <i>length-1</i>"
    <dt> <tt>-1</tt>
       <dd> Generates a single problem with length <tt>k</tt>
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of the dumped dimacs file.
       It may contain special symbols which will be macro-expanded to form
       the real file name. Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @k: current problem bound <BR>
       - @l: current loopback value <BR>
       - @n: index of the currently processed formula in the properties
       database <BR>
       - @@: the '@' character
  </dl>

  For further information about this implementation see:
  T. Latvala, A. Biere, K. Heljanko, and T. Junttila. Simple is
  Better: Efficient Bounded Model Checking for Past LTL. In: R. Cousot
  (ed.), Verification, Model Checking, and Abstract Interpretation,
  6th International Conference VMCAI 2005, Paris, France, Volume 3385
  of LNCS, pp. 380-395, Springer, 2005.  Copyright ©
  Springer-Verlag.
  ]

******************************************************************************/
int Sbmc_CommandGenLtlSpecSBmc(int argc, char** argv)
{
  Prop_ptr ltlprop=(Prop_ptr)NULL;   /* The property being processed */
  Outcome opt_handling_res;
  int k = get_bmc_pb_length(OptsHandler_get_instance());
  boolean single_prob = false;
  char *fname = (char *)NULL;
  int relative_loop =
    Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(OptsHandler_get_instance()), NULL);

  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = sbmc_cmd_options_handling(argc, argv, Prop_Ltl,
                                               &ltlprop,
                                               &k, &relative_loop,
                                               &fname,
                                               NULL, NULL /* -N -c */,
                                               &single_prob);

  if (opt_handling_res == OUTCOME_SUCCESS_REQUIRED_HELP) {
    return UsageSBMCGenLtlSpec();
  }
  if (opt_handling_res != OUTCOME_SUCCESS) {
    if (fname != (char *)NULL) FREE(fname);
    return 1;
  }

  /* makes sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* ----------------------------------------------------------------------- */

  if (fname == (char*) NULL) {
    fname = util_strsav(get_bmc_dimacs_filename(OptsHandler_get_instance()));
  }

  /* prepares the list of properties if no property was selected: */
  if (ltlprop == NIL_PTR(Prop_ptr)) {
    lsList props;
    lsGen  iterator;
    Prop_ptr prop;

    if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
      props = PropDb_get_ordered_props_of_type(PropPkg_get_prop_database(),
                                               mainFlatHierarchy,
                                               Prop_Ltl);
    else props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                          Prop_Ltl);

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_SBMCGenSolveLtl(prop, k, relative_loop,
                          true, /* iterate on k */
                          false, /* do not solve */
                          BMC_DUMP_DIMACS,
                          fname) != 0) {
        if (fname != (char*) NULL) FREE(fname);
        return 1;
      }
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to solve (a single property): */
    if (Bmc_SBMCGenSolveLtl(ltlprop, k, relative_loop,
                        !single_prob, /* iterate on k */
                        false, /* do not solve */
                        BMC_DUMP_DIMACS,
                        fname) != 0) {
      if (fname != (char*) NULL) FREE(fname);
      return 1;
    }
  }

  if (fname != (char*) NULL) FREE(fname);

  return 0;
}

static int UsageSBMCGenLtlSpec(void)
{
  fprintf(nusmv_stderr, "\nusage: gen_ltlspec_sbmc [-h | -n idx | -p \"formula\"] [-k max_length] [-l loopback]\n\t\t\t [-1] [-o <filename>]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx>\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property\n");
  fprintf(nusmv_stderr, "  -P \"name\"\tChecks the LTL property specified with <name>.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties.\n");
  fprintf(nusmv_stderr, "  -k max_length\tChecks the property using <max_length> value instead of using the\n\t\tvariable <bmc_length> value\n");
  fprintf(nusmv_stderr, "  -l loopback\tChecks the property using <loopback> value instead of using the\n\t\tvariable <bmc_loopback> value\n");
  fprintf(nusmv_stderr, "  -1 \t\tGenerates and solves a single problem.\n");
  fprintf(nusmv_stderr, "  -o filename\tGenerates dimacs output file too. <filename> may contain patterns\n\n");

  return 1;
}


/**Function********************************************************************

  Synopsis           [Uses Kepa's and Timo's method for doing incremental bmc]

  Description        [

  For further information about this implementation see: K. Heljanko,
  T. Junttila and T. Latvala.  Incremental and Complete Bounded Model
  Checking for Full PLTL.  In K. Etessami and S. Rajamani (eds.),
  Computer Aided Verification, Edinburgh, Scotland, Volume 3576 of
  LNCS, pp. 98-111, Springer, 2005. Copyright © Springer-Verlag.]

    SideEffects        [None]

  SeeAlso            []

  CommandName        [check_ltlspec_sbmc_inc]

  CommandSynopsis    [Incremental SBMC LTL model checking]

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\]\ | -P
  "name"\] \[-k max_length\] \[-c\] \[-N\]]

  CommandDescription [
  This command generates one or more problems, and calls
  SAT solver for each one. Each problem is related to a specific problem
  bound, which increases from zero (0) to the given maximum problem
  length. Here "<i>length</i>" is the bound of the problem that system
  is going to generate and/or solve. <BR>
  In this context the maximum problem bound is represented by the
  <i>-k</i> command parameter, or by its default value stored in the
  environment variable <i>bmc_length</i>.<BR>
  The property to be checked may be specified using the <i>-n idx</i>,
  <i>-p "formula"</i>, or <i>-P "property_name"</i> options. <BR>
  Completeness check, although slower, can be used to determine whether
  the property holds.<BR>
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification
       formula actually located in the properties database. <BR>
    <dt> <tt>-p "formula" \[IN context\]</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-P name</tt>
       <dd> Checks the LTLSPEC property with name <tt>name</tt> in the property
            database.
    <dt> <tt>-k <i>max_length</i></tt>
       <dd> <i>max_length</i> is the maximum problem bound must be reached.
       Only natural number are valid values for this option. If no value
       is given the environment variable <i>bmc_length</i> is considered
       instead.
    <dt> <tt>-c</tt>
       <dd> Performs completeness check at every step. This can be
       effectively used to determine whether a property holds.</tt>
    <dt><tt>-N</tt>
       <dd> Does not perform virtual unrolling.
  </dl>

  For further information about this implementation see:
  T. Latvala, A. Biere, K. Heljanko, and T. Junttila. Simple is
  Better: Efficient Bounded Model Checking for Past LTL. In: R. Cousot
  (ed.), Verification, Model Checking, and Abstract Interpretation,
  6th International Conference VMCAI 2005, Paris, France, Volume 3385
  of LNCS, pp. 380-395, Springer, 2005.  Copyright ©
  Springer-Verlag.

  ]

******************************************************************************/
int Sbmc_CommandLTLCheckZigzagInc(int argc, char** argv)
{
  Prop_ptr ltlprop = PROP(NULL);   /* The property being processed */
  Outcome opt_handling_res;
  int k = get_bmc_pb_length(OptsHandler_get_instance());
  boolean do_virtual_unrolling = true;
  boolean do_completeness_check = false;

  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = sbmc_cmd_options_handling(argc, argv,
                                               Prop_Ltl, &ltlprop,
                                               &k,
                                               NULL, NULL, /* l, o */
                                               &do_virtual_unrolling,
                                               &do_completeness_check,
                                               NULL /* single problem */
                                               );

  if (opt_handling_res == OUTCOME_SUCCESS_REQUIRED_HELP) {
    return UsageSBMCIncCheck();
  }
  if (opt_handling_res != OUTCOME_SUCCESS)  return 1;

  /* makes sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    return 1;
  }
  /* ----------------------------------------------------------------------- */

  /* prepares the list of properties if no property was selected: */
  if (ltlprop == PROP(NULL)) {
    lsGen  iterator;
    Prop_ptr prop;
    lsList props;

    if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
      props = PropDb_get_ordered_props_of_type(PropPkg_get_prop_database(),
                                               mainFlatHierarchy,
                                               Prop_Ltl);
    else props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                          Prop_Ltl);

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Sbmc_zigzag_incr(prop, k, do_virtual_unrolling,
                              do_completeness_check) != 0)
        return 1;
    }
    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to solve (a single property): */
    if (Sbmc_zigzag_incr(ltlprop, k, do_virtual_unrolling,
                         do_completeness_check) != 0)
      return 1;
  }

  return 0;
}

static int UsageSBMCIncCheck(void)
{
  fprintf(nusmv_stderr, "\nusage: check_ltlspec_sbmc_inc [-h | -n idx | -p \"formula\"] [-k max_length] [-N] [-c]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx>\n"
                        "        \t(using incremental algorithms).\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property\n");
  fprintf(nusmv_stderr, "  -P \"name\"\tChecks the LTL property specified with <name>.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties (using \n"
                        "\t\tincremental algorithms).\n");
  fprintf(nusmv_stderr, "  -k max_length\tChecks the property using <max_length> value instead of using \n\t\tthe variable <bmc_length> value.\n");
  fprintf(nusmv_stderr, "  -N \t\tDoes not perform virtual unrolling.\n");
  fprintf(nusmv_stderr, "  -c \t\tPerforms completeness check.\n");

  return 1;
}




/**Function********************************************************************

  Synopsis           [Top-level function for bmc of PSL properties]

  Description        [The parameters are:
  - prop is the PSL property to be checked
  - dump_prob is true if the problem must be dumped as DIMACS file (default filename
  from system corresponding variable)
  - inc_sat is true if incremental sat must be used. If there is no
  support for inc sat, an internal error will occur.
  - single_prob is true if k must be not incremented from 0 to k_max
    (single problem)
  - k and rel_loop are the bmc parameters.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int Sbmc_check_psl_property(Prop_ptr prop,
                            boolean dump_prob,
                            boolean inc_sat,
                            boolean do_completeness_check,
                            boolean do_virtual_unrolling,
                            boolean single_prob,
                            int k, int rel_loop)
{
  nusmv_assert(prop != PROP(NULL));
  nusmv_assert(Prop_get_type(prop) == Prop_Psl);

  /* checks the property is LTL compatible */
  if (!Prop_is_psl_ltl(prop)) {
    fprintf (nusmv_stderr, "SBMC can be used only with Psl/ltl properies.\n");
    return 1;
  }

  /* SBMC for ltl: makes sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    return 1;
  }

  if (inc_sat) {
#if NUSMV_HAVE_INCREMENTAL_SAT
    return Bmc_GenSolveLtlInc(prop, k, rel_loop, !single_prob);
#else
    internal_error("Sbmc_check_psl_property: Inc SAT Solving requested when not supported.\n");
#endif
  }

  if (single_prob && inc_sat) {
    fprintf(nusmv_stderr,
            "Error: single problem generation (option -1) with incremental "\
            "solvers is an unsupported feature of SBMC.\n");
    return 1;
  }

  if (dump_prob && inc_sat) {
    fprintf(nusmv_stderr,
            "Error: problem cannot be dumped when incremental sat solving is used.\n");
    return 1;
  }

  if (inc_sat) {
    if (Sbmc_zigzag_incr(prop, k, do_virtual_unrolling,
                         do_completeness_check) != 0)
      return 1;
  }
  else {
    if (Bmc_SBMCGenSolveLtl(prop, k, rel_loop,
                            !single_prob, /* iterate on k */
                            true, /* solve */
                            (dump_prob) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
                            get_bmc_dimacs_filename(OptsHandler_get_instance())) != 0) {
      return 1;
    }
  }

  return 0;
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis [Sbmc commands options handling for commands (optionally)
  acceping options -k -l -o -p -n -N -c]

  Description [ Output variables called res_* are pointers to
  variables that will be changed if the user specified a value for the
  corresponding option. For example if the user specified "-k 2", then
  *res_k will be assigned to 2. The caller can selectively choose which
  options can be specified by the user, by passing either a valid pointer
  as output parameter, or NULL to disable the corresponding option.
  For example by passing NULL as actual parameter of res_l, option -l will
  be not accepted.

  If both specified, k and l will be checked for mutual consistency.
  Loop will contain a relative value, like the one the user specified.

  prop_type is the expected property type, if specified.

  All integers values will not be changed if the corresponding options
  had not be specified by the user, so the caller might assign them to
  default values before calling this function.

  All strings will be allocated by the function if the corresponding
  options had been used by the user. In this case it is responsability
  of the caller to free them. Strings will be assigned to NULL if the
  user had not specified any corresponding option.

  Returns OUTCOME_GENERIC_ERROR if an error has occurred;
  Returns OUTCOME_SUCCESS_REQUIRED_HELP if -h options had been specified;
  Returns OUTCOME_SUCCESS in all other cases.
  ]

  SideEffects        [Result parameters might change]

  SeeAlso            []

******************************************************************************/
static Outcome
sbmc_cmd_options_handling(int argc, char** argv,
                          Prop_Type prop_type,

                          /* output parameters: */
                          Prop_ptr* res_prop,
                          int* res_k,
                          int* res_l,
                          char** res_o,
                          boolean* res_N,
                          boolean* res_c,
                          boolean* res_1)
{
  int c;
  int prop_idx;
  char* formula_name = (char*) NULL;
  char* str_formula = (char*) NULL;
  char* str_loop = (char*) NULL;

  boolean k_specified = false;
  boolean l_specified = false;

  /* If one or more options are added here, the size of this array
     must be changed. At the moment eight options are supported.  */
  char opt_string[9*2+1];


  /* ---------------------------------------------------------------------- */
  /* Fills up the string to pass to util_getopt, depending on which
     options are actually required */
  strcpy(opt_string, "h");  /* h is always needed */

  if (res_prop != (Prop_ptr*) NULL) {
    *res_prop = (Prop_ptr) NULL;
    strcat(opt_string, "n:p:P:");
  }

  if (res_k != (int*) NULL) strcat(opt_string, "k:");
  if (res_l != (int*) NULL) strcat(opt_string, "l:");

  if (res_o != (char**) NULL) {
    *res_o = (char*) NULL;
    strcat(opt_string, "o:");
  }

  if (res_N != (boolean*) NULL) strcat(opt_string, "N");
  if (res_c != (boolean*) NULL) strcat(opt_string, "c");
  if (res_1 != (boolean*) NULL) strcat(opt_string, "1");


  util_getopt_reset();
  while ((c = util_getopt((int)argc, (char**) argv, opt_string)) != EOF) {
    switch (c) {
    case 'h':
      return OUTCOME_SUCCESS_REQUIRED_HELP;

    case 'n': {
      char* str_prop_idx = (char*) NULL;

      nusmv_assert(res_prop != (Prop_ptr*) NULL);

      /* check if a formula has already been specified: */
      if ((*res_prop != PROP(NULL)) || (str_formula != (char*) NULL)) {
        error_property_already_specified();
        return OUTCOME_GENERIC_ERROR;
      }

      str_prop_idx = util_strsav(util_optarg);

      /* check if property idx is ok */
      prop_idx = PropDb_get_prop_index_from_string(PropPkg_get_prop_database(),
                                                   str_prop_idx);
      FREE(str_prop_idx);

      if (prop_idx == -1) {
        /* error messages have already been shown */
        return OUTCOME_GENERIC_ERROR;
      }

      /* here property idx is ok */
      *res_prop = PropDb_get_prop_at_index(PropPkg_get_prop_database(),
                                           prop_idx);
      if ( Prop_check_type(*res_prop, prop_type) != 0 ) {
        /* specified property's type is not what the caller expected */
        return OUTCOME_GENERIC_ERROR;
      }

      break;
    } /* case 'n' */

    case 'P':
      {
        nusmv_assert(res_prop != (Prop_ptr*) NULL);

        /* check if a formula has already been specified: */
        if ((*res_prop != PROP(NULL)) || (str_formula != (char*) NULL) || (formula_name != (char*) NULL)) {
          error_property_already_specified();
          return OUTCOME_GENERIC_ERROR;
        }

        formula_name = util_strsav(util_optarg);

        prop_idx = PropDb_prop_parse_name(PropPkg_get_prop_database(),
                                          formula_name);

        if (prop_idx == -1) {
          fprintf(nusmv_stderr, "No property named \"%s\"\n", formula_name);
          FREE(formula_name);
          /* error messages have already been shown */
          return OUTCOME_GENERIC_ERROR;
        }

        FREE(formula_name);

        /* here property idx is ok */
        *res_prop = PropDb_get_prop_at_index(PropPkg_get_prop_database(),
                                             prop_idx);
        if ( Prop_check_type(*res_prop, prop_type) != 0 ) {
          /* specified property's type is not what the caller expected */
          return OUTCOME_GENERIC_ERROR;
        }

        break;
      } /* case 'P' */

    case 'p':
      nusmv_assert(res_prop != (Prop_ptr*) NULL);

      /* check if a formula has already been specified: */
      if ((*res_prop != PROP(NULL)) || (str_formula != (char*) NULL)) {
        error_property_already_specified();
        return OUTCOME_GENERIC_ERROR;
      }

      str_formula = util_strsav(util_optarg);
      break;

    case 'k': {
      char* str_k;
      int k;

      nusmv_assert(res_k != (int*) NULL);

      /* check if a value has already been specified: */
      if (k_specified) {
        fprintf(nusmv_stderr,
                "Option -k cannot be specified more than once.\n");
        return OUTCOME_GENERIC_ERROR;
      }

      str_k = util_strsav(util_optarg);

      if (util_str2int(str_k, &k) != 0) {
        error_invalid_number(str_k);
        FREE(str_k);
        return OUTCOME_GENERIC_ERROR;
      }

      if (k < 0) {
        error_invalid_number(str_k);
        FREE(str_k);
        return OUTCOME_GENERIC_ERROR;
      }

      FREE(str_k);
      *res_k = k;
      k_specified = true;
      break;
    }

    case 'l':
      nusmv_assert(res_l != (int*) NULL);

      /* check if a value has already been specified: */
      if (l_specified) {
        fprintf(nusmv_stderr,
                "Option -l cannot be specified more than once.\n");
        return OUTCOME_GENERIC_ERROR;
      }

      str_loop = util_strsav(util_optarg);
      l_specified = true;
      /* checking of loopback value is delayed after command line
         processing to allow any -k option evaluation before (see the
         cheking code below) */
      break;

    case 'o':
      nusmv_assert(res_o != (char**) NULL);

      *res_o = util_strsav(util_optarg);
      break;

    case 'N':
      nusmv_assert(res_N != (boolean*) NULL);
      *res_N = false;
      break;

    case '1':
      nusmv_assert(res_1 != (boolean*) NULL);
      *res_1 = true;
      break;

    case 'c':
      nusmv_assert(res_c != (boolean*) NULL);
      *res_c = true;
      break;

    default:  return OUTCOME_GENERIC_ERROR;
    } /* switch case */
  } /* end of cmd line processing */

  /* checks if there are unexpected options: */
  if (argc != util_optind) {
    fprintf(nusmv_stderr, "You specified one or more invalid options.\n\n");
    return OUTCOME_GENERIC_ERROR;
  }

  /* Checking of k,l constrains: */
  if (str_loop != (char*) NULL) {
    Outcome res;
    int rel_loop;

    rel_loop = Bmc_Utils_ConvertLoopFromString(str_loop, &res);

    if (res != OUTCOME_SUCCESS) {
      error_invalid_number(str_loop);
      FREE(str_loop);
      return OUTCOME_GENERIC_ERROR;
    }
    FREE(str_loop);

    if (Bmc_Utils_Check_k_l(*res_k,
                            Bmc_Utils_RelLoop2AbsLoop(rel_loop, *res_k))
        != OUTCOME_SUCCESS) {
      error_bmc_invalid_k_l(*res_k, rel_loop);
      return OUTCOME_GENERIC_ERROR;
    }

    *res_l = rel_loop;
  } /* k,l consistency check */


  /* Formula checking and commitment: */
  if (str_formula != (char*) NULL) {
    int idx;

    /* make sure bmc has been set up */
    if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
      FREE(str_formula);
      return OUTCOME_GENERIC_ERROR;
    }

    idx = PropDb_prop_parse_and_add(PropPkg_get_prop_database(),
                                    Compile_get_global_symb_table(),
                                    str_formula, prop_type);
    if (idx == -1) {
      FREE(str_formula);
      return OUTCOME_GENERIC_ERROR;
    }

    /* index is ok */
    nusmv_assert(*res_prop == PROP(NULL));
    *res_prop = PropDb_get_prop_at_index(PropPkg_get_prop_database(), idx);

    FREE(str_formula);
  } /* formula checking and commit */

  return OUTCOME_SUCCESS;
}
