/**CFile***********************************************************************

  FileName    [ddCmd.c]

  PackageName [dd]

  Synopsis    [The shell interface of the DD package]

  Description [Shell interface of the DD package. here are provided
  the shell commands to modyfy all the modifiable DD options.]

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``dd'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and FBK-irst. 

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

  For more information on NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/

#include "ddInt.h" 

static char rcsid[] UTIL_UNUSED = "$Id: ddCmd.c,v 1.6.16.2 2006-05-15 09:06:16 nusmv Exp $";

int CommandDynamicVarOrdering ARGS((int argc, char **argv));
int CommandSetBddParameters ARGS((int argc, char ** argv));
int CommandPrintBddStats ARGS((int argc, char ** argv));

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int UsageDynamicVarOrdering ARGS((void));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

void dd_AddCmd(void)
{
  Cmd_CommandAdd("dynamic_var_ordering", CommandDynamicVarOrdering, 0, true);
  Cmd_CommandAdd("set_bdd_parameters" , CommandSetBddParameters, 0, true);
  Cmd_CommandAdd("print_bdd_stats" , CommandPrintBddStats, 0, true);

}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Implements the dynamic_var_ordering command.]

  CommandName        [dynamic_var_ordering] 	   

  CommandSynopsis    [Deals with the dynamic variable ordering.]  

  CommandArguments   [\[-d\] \[-e &lt;method&gt;\] \[-f &lt;method&gt;\] \[-h\]]  

  CommandDescription [
  Controls the application and the modalities of (dynamic) variable
  ordering. Dynamic ordering is a technique to reorder the BDD variables
  to reduce the size of the existing BDDs. When no options are specified,
  the current status of dynamic ordering is displayed. At most one of the
  options <tt>-e</tt>, <tt>-f</tt>, and <tt>-d</tt> should be specified.<p>

  Dynamic ordering may be time consuming, but can often reduce the size of
  the BDDs dramatically. A good point to invoke dynamic ordering
  explicitly (using the <tt>-f</tt> option) is after the commands
  <tt>build_model</tt>, once the transition relation has been built.  It is
  possible to save the ordering found using <tt>write_order</tt> in order to
  reuse it (using <tt>build_model -i order-file</tt>) in the future.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-d</tt>
       <dd> Disable dynamic ordering from triggering automatically.
    <dt> <tt>-e &lt;method&gt;</tt>
       <dd> Enable dynamic ordering to trigger automatically whenever a
            certain threshold on the overall BDD size is reached.
            <tt>&lt;method&gt;</tt> must be one of the following:
            <ul>
            <li> <b>sift</b>: Moves each variable throughout the order to
                 find an optimal position for that variable (assuming all
                 other variables are fixed).  This generally achieves
                 greater size reductions than the window method, but is slower.
            <li> <b>random</b>: Pairs of variables are randomly chosen, and
                 swapped in the order. The swap is performed by a series of
                 swaps of adjacent variables. The best order among those
                 obtained by the series of swaps is retained. The number of
                 pairs chosen for swapping equals the number of variables
                 in the diagram.
            <li> <b>random_pivot</b>: Same as <b>random</b>, but the two
                 variables are chosen so that the first is above the
                 variable with the largest number of nodes, and the second
                 is below that variable.  In case there are several
                 variables tied for the maximum number of nodes, the one
                 closest to the root is used.
            <li> <b>sift_converge</b>: The <b>sift</b> method is iterated
                 until no further improvement is obtained.
            <li> <b>symmetry_sift</b>: This method is an implementation of
                 symmetric sifting. It is similar to sifting, with one
                 addition: Variables that become adjacent during sifting are
                 tested for symmetry. If they are symmetric, they are linked
                 in a group. Sifting then continues with a group being
                 moved, instead of a single variable.
            <li> <b>symmetry_sift_converge</b>: The <b>symmetry_sift</b>
                 method is iterated until no further improvement is obtained.
            <li> <b>window{2,3,4}</b>: Permutes the variables within windows
                 of "n" adjacent variables, where "n" can be either 2, 3 or 4,
                 so as to minimize the overall BDD size.<p>
            <li> <b>window{2,3,4}_converge</b>: The <b>window{2,3,4}</b> method
                 is iterated until no further improvement is obtained.
            <li> <b>group_sift</b>: This method is similar to
                 <b>symmetry_sift</b>, but uses more general criteria to
                 create groups.
            <li> <b>group_sift_converge</b>: The <b>group_sift</b> method is
                 iterated until no further improvement is obtained.
            <li> <b>annealing</b>: This method is an implementation of
                 simulated annealing for variable ordering. This method is
                 potentially very slow.
            <li> <b>genetic</b>: This method is an implementation of a
                 genetic algorithm for variable ordering. This method is
                 potentially very slow.
            <li> <b>exact</b>: This method implements a dynamic programming
                 approach to exact reordering. It only stores a BDD
                 at a time. Therefore, it is relatively efficient in
                 terms of memory. Compared to other reordering
                 strategies, it is very slow, and is not recommended
                 for more than 16 boolean variables.
            <li> <b>linear</b>: This method is a combination of
                 sifting and linear transformations.
            <li> <b>linear_converge</b>: The <b>linear</b> method is
                 iterated until no further improvement is obtained.
            </ul><br>
    <dt> <tt>-f &lt;method&gt;</tt>
     <dd> Force dynamic ordering to be invoked immediately. The values for
          <tt>&lt;method&gt;</tt> are the same as in option <tt>-e</tt>.
  </dl>]  

  SideEffects        []

******************************************************************************/
int CommandDynamicVarOrdering(int argc, char **argv)
{
  int c;
  dd_reorderingtype currentMethod;
  int dynOrderingMethod = REORDER_NONE; /* for lint */
  boolean currentlyEnabled = false;
  boolean disableFlag   = false;
  boolean enableFlag    = false;
  boolean forceFlag     = false;

  /*
   * Parse the command line.
  */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "df:e:h")) != EOF) {
    switch (c) {
    case 'h': return(UsageDynamicVarOrdering());
    case 'f':
      forceFlag = true;
      dynOrderingMethod = StringConvertToDynOrderType(util_optarg);
      if (dynOrderingMethod == REORDER_NONE) {
        fprintf(nusmv_stderr, "unknown method: %s\n", util_optarg);
        return(UsageDynamicVarOrdering());
      }
      break;
    case 'e':
      enableFlag = true;
      dynOrderingMethod = StringConvertToDynOrderType(util_optarg);
      if (dynOrderingMethod == REORDER_NONE) {
        fprintf(nusmv_stderr, "unknown method: %s\n", util_optarg);
        return(UsageDynamicVarOrdering());
      }
      break;
    case 'd':
      disableFlag = true;
      break;
    default:
      return(UsageDynamicVarOrdering());
    }
  }

  if (dd_manager == NIL(DdManager)) {
    fprintf(nusmv_stderr, "The DD Manager has not been created yet.\n");
    return 1;
  }
  /* At most one option is allowed. */
  if ((disableFlag && enableFlag) ||
      (disableFlag && forceFlag) ||
      (enableFlag && forceFlag)) {
    fprintf(nusmv_stderr, "Only one of -d, -f, -e is allowed.\n");
    return 1;
  }
  /*
   * Get the current method for reading and to save in case temporarily
   * overwritten.
  */
  currentlyEnabled = dd_reordering_status(dd_manager, &currentMethod);
  if (!(disableFlag || enableFlag || forceFlag)) {
    if (currentlyEnabled) {
      fprintf(nusmv_stdout, "Dynamic variable ordering is enabled ");
      fprintf(nusmv_stdout, "with method: \"%s\".\n",
                     DynOrderTypeConvertToString(currentMethod));
    }
    else {
      fprintf(nusmv_stdout, "Dynamic variable ordering is disabled.\n");
    }
  }
  if (disableFlag) {
    if (currentMethod == REORDER_NONE) {
      fprintf(nusmv_stdout, "Dynamic variable ordering is already disabled.\n");
    }
    else {
      fprintf(nusmv_stdout, "Dynamic variable ordering is disabled.\n");
      dd_autodyn_disable(dd_manager);
      unset_dynamic_reorder(OptsHandler_get_instance());
    }
  }

  /*
   * Set the dynamic ordering method.
  */
  if (enableFlag) {
    dd_autodyn_enable(dd_manager, dynOrderingMethod);
    set_reorder_method(OptsHandler_get_instance(), dynOrderingMethod);
    set_dynamic_reorder(OptsHandler_get_instance());
    fprintf(nusmv_stdout, "Dynamic variable ordering is enabled ");
    fprintf(nusmv_stdout, "with method \"%s\".\n",
                   DynOrderTypeConvertToString(dynOrderingMethod));
  }

  /*
   * Force a reordering.  Note that the DdManager has to have the method set
   * before calling bdd_reorder.
  */
  if (forceFlag) {
    fprintf(nusmv_stdout, "Dynamic variable ordering forced ");
    fprintf(nusmv_stdout, "with method %s....\n",
                   DynOrderTypeConvertToString(dynOrderingMethod));
    dd_reorder(dd_manager, dynOrderingMethod, DEFAULT_MINSIZE);
  }

  return 0;
}

static int UsageDynamicVarOrdering(void)
{
  fprintf(nusmv_stderr, "usage: dynamic_var_ordering [[-d] | [-e method] | [-f method] [-h]]\n");
  fprintf(nusmv_stderr, "   -d \t\tDisables dynamic ordering\n");
  fprintf(nusmv_stderr, "   -e method \tEnables dynamic ordering with method\n");
  fprintf(nusmv_stderr, "   -f method \tForces dynamic ordering with method\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage\n");
  return 1;
}


/**Function********************************************************************

  Synopsis    [Implements the set_bdd_parameters command.]

  SideEffects []

  CommandName [set_bdd_parameters]

  CommandSynopsis [Creates a table with the value of all currently
  active NuSMV flags and change accordingly the configurable parameters
  of the BDD package.]

  CommandArguments [\[-h\] \[-s\]]

  CommandDescription [Applies the variables table of the NuSMV environnement
  to the BDD package, so the user can set specific BDD parameters to the 
  given value. This command works in conjunction with the 
  <tt>print_bdd_stats</tt> and <tt>set</tt> commands.<p>

  <tt>print_bdd_stats</tt> first prints a report of the parameters and
  statistics of the current bdd_manager. By using the command <tt>set</tt>,
  the user may modify the value of any of the parameters of the
  underlying BDD package. The way to do it is by setting a value in
  the variable <tt>BDD.parameter name</tt> where <tt>parameter
  name</tt> is the name of the parameter exactly as printed by the
  <tt>print_bdd_stats</tt> command.<p>

  Command options:<p>

  <dl>
    <dt> -s
       <dd> Prints the BDD parameter and statistics after the modification.

  </dl>
  ]
  
******************************************************************************/
int CommandSetBddParameters(int  argc, char ** argv)
{
  boolean  showAfter;
  int c;

  showAfter = FALSE;

  /*
   * Parse the command line.
   */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hs")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
        break;
      case 's':
	showAfter = TRUE;
	break;
      default:
        goto usage;
    }
  }

  /* flatten_hierarchy and static_order must have been invoked already. */
  if (dd_manager == NIL(DdManager)) {
    fprintf(nusmv_stderr, "The DD Manager has not been created yet.\n");
    return 1;
  }
  
  /* Create the table of variable->value */
  dd_set_parameters(dd_manager, OptsHandler_get_instance(), nusmv_stdout);
  if (showAfter) {
    dd_print_stats(dd_manager, nusmv_stdout);
  }
  return 0;  /* Everything okay */

usage:
  fprintf(nusmv_stderr, "usage: set_bdd_parameters [-h | -s]\n");
  fprintf(nusmv_stderr, "   -h  Prints the command usage.\n");
  fprintf(nusmv_stderr, "   -s  Prints also the bdd statistics.\n");

  return 1;
}

/**Function********************************************************************

  Synopsis    [Implements the print_bdd_stats command.]

  SideEffects []

  CommandName [print_bdd_stats]

  CommandSynopsis [Prints out the BDD statistics and parameters]

  CommandArguments [\[-h\]]

  CommandDescription [Prints the statistics for the BDD package. The
  amount of information depends on the BDD package configuration
  established at compilation time. The configurtion parameters are
  printed out too. More information about statistics and parameters
  can be found in the documentation of the CUDD Decision Diagram
  package.]
  
******************************************************************************/
int CommandPrintBddStats(int argc, char ** argv)
{
  int c;
  /*
   * Parse the command line.
   */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
        break;
      default:
        goto usage;
    }
  }

  if (dd_manager == NIL(DdManager)) {
    fprintf(nusmv_stderr, "The DD Manager has not been created yet.\n");
    return 1;
  }

  dd_print_stats(dd_manager, nusmv_stdout);
  return 0;  /* Everything okay */

usage:
  fprintf(nusmv_stderr, "usage: print_bdd_stats [-h]\n");
  fprintf(nusmv_stderr, "   -h  Prints the command usage.\n");

  return 1;
}
