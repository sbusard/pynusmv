/**CFile***********************************************************************

  FileName    [cinitMisc.c]

  PackageName [cinit]

  Synopsis    [This file contain the main routine for the batch use of NuSMV.]

  Description [This file contain the main routine for the batch use of
  NuSMV. The batch main executes the various model checking steps in a
  predefined order. After the processing of the input file than it
  return to the calling shell.]

  SeeAlso     []

  Author      [Adapted to NuSMV by Marco Roveri]

  Copyright   [
  This file is part of the ``cinit'' package of NuSMV version 2.
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

#include "cinitInt.h"
#include "set/set.h"
#include "fsm/bdd/BddFsm.h"

#include "utils/error.h"
#include "prop/propPkg.h"
#include "mc/mc.h"
#include "enc/enc.h"
#include "bmc/bmcUtils.h"
#include "compile/compileInt.h" /* for CompileFlatten_init_flattener */
#include "opt/opt.h"

#include <stdarg.h>

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The batch main.]

  Description [The batch main. It first read the input file, than
  flatten the hierarchy. Aftre this preliminar phase it creates the
  boolean variables necessary for the encoding and then start
  compiling the read model into BDD. Now computes the reachable states
  depending if the flag has been set. before starting verifying if the
  properties specified in the model hold or not it computes the
  fairness constraints. You can also activate the reordering and
  also choose to avoid the verification of the properties.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void cinitBatchMain()
{

  /* Necessary to have standard behavior in the batch mode */
  util_resetlongjmp();
  CATCH {

  /* ================================================== */
  /*   1: Read the model                                */
  /* ================================================== */
  if (Cmd_CommandExecute("read_model")) nusmv_exit(1);

  /* ================================================== */
  /*  2: Flatten hierarchy                              */
  /* ================================================== */
  if (Cmd_CommandExecute("flatten_hierarchy")) nusmv_exit(1);

  /* If the -lp option is used, list the properties and exit */
  if (opt_list_properties(OptsHandler_get_instance()) == true) {
    if (Cmd_CommandExecute("show_property")) nusmv_exit(1);
    return;
  }

  /* ================================================== */
  /*  3: Builds the encodings                           */
  /* ================================================== */
  if (Cmd_CommandExecute("encode_variables")) nusmv_exit(1);

  /* ================================================== */
  /*  4: Builds the flat FSMs                           */
  /* ================================================== */
  if (Cmd_CommandExecute("build_flat_model")) nusmv_exit(1);


  /* --------------------------------------------------- */
  /*  Write the flat and bool FSMs (if required)         */
  /* ----------------------------------------------------*/
  if (get_output_flatten_model_file(OptsHandler_get_instance()) != NIL(char)) {
    if (Cmd_CommandExecute("write_flat_model")) nusmv_exit(1);
  }

  if (get_output_boolean_model_file(OptsHandler_get_instance()) != NIL(char)) {
    if (Cmd_CommandExecute("build_boolean_model")) nusmv_exit(1);
    if (Cmd_CommandExecute("write_boolean_model")) nusmv_exit(1);
  }

#if NUSMV_HAVE_SAT_SOLVER
  /* ================================================== */
  /*  5.1 BMC starts                                    */
  /* ================================================== */
  if (opt_bmc_mode(OptsHandler_get_instance()) == true) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr, "Entering BMC mode...\n");
    }

    /* build_boolean_model may have been already called if the output
       boolean model was specified in the argument list. */
    if (Compile_check_if_bool_model_was_built(NULL, false)) {
      if (Cmd_CommandExecute("build_boolean_model")) nusmv_exit(1);
    }

    /* Initializes the bmc package, and commits both the model and the
       determinization layers: */
    if (Cmd_CommandExecute("bmc_setup")) nusmv_exit(1);

    if (get_prop_no(OptsHandler_get_instance()) != -1) {
      int prop_no = get_prop_no(OptsHandler_get_instance());
      Prop_ptr prop;

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
        fprintf(nusmv_stderr, "Verifying property %d...\n", prop_no);
      }

      if ((prop_no < 0) ||
          (prop_no >= PropDb_get_size(PropPkg_get_prop_database()))) {
        fprintf(nusmv_stderr,
                "Error: \"%d\" is not a valid property index\n", prop_no);
        nusmv_exit(1);
      }

      prop = PropDb_get_prop_at_index(PropPkg_get_prop_database(), prop_no);

      switch (Prop_get_type(prop)) {
      case Prop_Ltl:
        {
          int rel_loop;

          /* skip if -ils option is given */
          if (opt_ignore_ltlspec(OptsHandler_get_instance())) break;
          const char* loop = get_bmc_pb_loop(OptsHandler_get_instance());
          rel_loop = Bmc_Utils_ConvertLoopFromString(loop, NULL);

          Bmc_GenSolveLtl(prop, get_bmc_pb_length(OptsHandler_get_instance()),
                          rel_loop, /*increasing length*/TRUE,
                          TRUE, BMC_DUMP_NONE, NULL);
          break;
        }

      case Prop_Psl:
        {
          const char* loop = get_bmc_pb_loop(OptsHandler_get_instance());
          int rel_loop = Bmc_Utils_ConvertLoopFromString(loop, NULL);

          /* skip if -ips option is given */
          if (opt_ignore_pslspec(OptsHandler_get_instance())) break;

          Bmc_check_psl_property(prop, false, false, false,
                                 get_bmc_pb_length(OptsHandler_get_instance()),
                                 rel_loop);
          break;
        }

      case Prop_Invar:
        /* skip if -ii option is given */
        if (opt_ignore_invar(OptsHandler_get_instance())) break;

        Bmc_GenSolveInvar(prop, TRUE, BMC_DUMP_NONE, NULL);
        break;

      default:
        fprintf(nusmv_stderr,
                "Error: only LTL, PSL and INVAR properties"
                " can be checked in BMC mode\n");
        nusmv_exit(1);
      } /* switch on type */

    }
    else {
      /* Checks all ltlspecs, invarspecs and pslspecs */

      if (! opt_ignore_ltlspec(OptsHandler_get_instance())) {
        lsList props;
        lsGen  iterator;
        Prop_ptr prop;
        int rel_loop;

        if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
          fprintf(nusmv_stderr, "Verifying the LTL properties...\n");
        }


        props = PropDb_get_props_of_type(PropPkg_get_prop_database(), Prop_Ltl);
        nusmv_assert(props != LS_NIL);

        lsForEachItem(props, iterator, prop) {
          const char* loop = get_bmc_pb_loop(OptsHandler_get_instance());
          rel_loop = Bmc_Utils_ConvertLoopFromString(loop, NULL);

          Bmc_GenSolveLtl(prop, get_bmc_pb_length(OptsHandler_get_instance()),
                          rel_loop, /*increasing length*/ TRUE, TRUE,
                          BMC_DUMP_NONE, NULL);
        }

        lsDestroy(props, NULL); /* the list is no longer needed */
      }

      if (! opt_ignore_pslspec(OptsHandler_get_instance())) {
        lsList props;
        lsGen  iterator;
        Prop_ptr prop;
        const char* loop = get_bmc_pb_loop(OptsHandler_get_instance());
        int rel_loop = Bmc_Utils_ConvertLoopFromString(loop, NULL);

        if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
          fprintf(nusmv_stderr, "Verifying the PSL properties...\n");
        }

        props = PropDb_get_props_of_type(PropPkg_get_prop_database(), Prop_Psl);
        nusmv_assert(props != LS_NIL);

        lsForEachItem(props, iterator, prop) {
          if (Prop_is_psl_ltl(prop)) {
            int len = get_bmc_pb_length(OptsHandler_get_instance());
            Bmc_check_psl_property(prop, false, false, false,
                                   len, rel_loop);
          }
        }

        lsDestroy(props, NULL); /* the list is no longer needed */
      }

      if (! opt_ignore_invar(OptsHandler_get_instance())) {
        lsList props;
        lsGen  iterator;
        Prop_ptr prop;

        if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
          fprintf(nusmv_stderr, "Verifying the INVAR properties...\n");
        }

        props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                         Prop_Invar);
        nusmv_assert(props != LS_NIL);

        lsForEachItem(props, iterator, prop) {
          Bmc_GenSolveInvar(prop, TRUE, BMC_DUMP_NONE, NULL);
        }

        lsDestroy(props, NULL); /* the list is no longer needed */
      }
    }

    return;
  } /* end of BMC */
#endif

  /* ================================================== */
  /*  5.2 BDD-based model checking starts               */
  /* ================================================== */

  /* Builds the BDD FSM of the whole read model.
     If COI is enabled there is no reason to create global BDD FSM since
     every property will have its one instance of a BDD FSM.
  */
  if (Cmd_CommandExecute("build_model")) nusmv_exit(1);

  /* checks the fsm if required */
  if (opt_check_fsm(OptsHandler_get_instance()) == true) {
    BddFsm_ptr bdd_fsm = PropDb_master_get_bdd_fsm(PropPkg_get_prop_database());

    if (opt_cone_of_influence(OptsHandler_get_instance())) {
      fprintf(nusmv_stderr,
              "WARNING: Check for totality of the transition "
              "relation cannot currently\n"
              "performed in batch mode if the cone"
              " of influence reduction has been enabled.\n");
      nusmv_exit(1);
    }

    BddFsm_check_machine(bdd_fsm);
  }

  if (get_prop_no(OptsHandler_get_instance()) != -1) {
    const char* command_pattern = "check_property -n %d";
    /* 11 digits at most: */
    char* command_str = ALLOC(char, strlen(command_pattern) + 12); 
    int _cmd_res;
    
    sprintf(command_str, command_pattern, 
	    get_prop_no(OptsHandler_get_instance()));
    _cmd_res = Cmd_CommandExecute(command_str);

    FREE(command_str);
    if (_cmd_res) nusmv_exit(1);
  }
  else {
    /* Evaluates the Specifications */
    if (!opt_ignore_spec(OptsHandler_get_instance())) {
      PropDb_verify_all_type(PropPkg_get_prop_database(), Prop_Ctl);
    }

    if (!opt_ignore_compute(OptsHandler_get_instance())) {
      PropDb_verify_all_type(PropPkg_get_prop_database(), Prop_Compute);
    }

    /* Evaluates the LTL specifications */
    if (!opt_ignore_ltlspec(OptsHandler_get_instance())) {
      PropDb_verify_all_type(PropPkg_get_prop_database(), Prop_Ltl);
    }

    /* Evaluates the PSL specifications */
    if (!opt_ignore_pslspec(OptsHandler_get_instance())) {
      PropDb_verify_all_type(PropPkg_get_prop_database(), Prop_Psl);
    }

    /* Evaluates CHECKINVARIANTS */
    if (!opt_ignore_invar(OptsHandler_get_instance())) {
      PropDb_verify_all_type(PropPkg_get_prop_database(), Prop_Invar);
    }
  }


  /* Reporting of statistical information. */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    if (Cmd_CommandExecute("print_usage")) nusmv_exit(1);
  }

  /* Computing and Reporting of the Effect of Reordering */
  if (opt_reorder(OptsHandler_get_instance())) {
    fprintf(nusmv_stdout, "\n========= starting reordering ============\n");
    dd_reorder(dd_manager, get_reorder_method(OptsHandler_get_instance()),
               DEFAULT_MINSIZE);
    fprintf(nusmv_stdout, "\n========= after reordering ============\n");
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      if (Cmd_CommandExecute("print_usage")) nusmv_exit(1);
    }

    if (Cmd_CommandExecute("write_order")) nusmv_exit(1);
  }

  /* Reporting of Reachable States */
  if (opt_print_reachable(OptsHandler_get_instance()) == true) {
    BddFsm_ptr fsm = PropDb_master_get_bdd_fsm(PropPkg_get_prop_database());
    if (opt_cone_of_influence(OptsHandler_get_instance())) {
      fprintf(nusmv_stderr,
              "WARNING: Statistics of reachable "
              "states is not currently available\n"
              "in batch mode if cone of influence "
              "reduction has been enabled.\n");
      nusmv_exit(1);
    }

    BddFsm_print_reachable_states_info(fsm,
                                       false, /* do not print states */
                                       false, /* do not print defines */
                                       false, /* do not print formula */
                                       nusmv_stdout);
  }

  } FAIL {
    fprintf(nusmv_stderr, "\n%s terminated by a signal\n",
            NuSMVCore_get_tool_name());
    nusmv_exit(1);
  }
}


/**Function********************************************************************

  Synopsis    [Sources the .nusmvrc file.]

  Description [Sources the .nusmvrc file.  Always sources the .nusmvrc from
  library.  Then source the .nusmvrc from the home directory.  If there is none
  in the home directory, then execute the one in the current directory if one
  is present.  Returns 1 if scripts were successfully executed, else return 0.]

  SideEffects []

  SeeAlso     [optional]

******************************************************************************/
int CInit_NusmvrcSource()
{
  char *commandLine;
  char *libraryName;
  char *rcFileName;
  char *homeFile;
  char *tmpRcTilde;
  char *tmpCmd;
  struct stat home;
  struct stat cur;
  int s1;
  int s2;                       /* flags for checking the stat() call */
  int status0;
  int status1 = TRUE;
  int status2 = TRUE;
  const char* cmd_pattern = "source -s %s/master%s";
  const char* rc_tilde_pattern = "~/%s";
  const char* source_pattern = "source -s %s";

  /*
   * First execute the standard .nusmvrc.
   */
  libraryName = CInit_NuSMVObtainLibrary();
  rcFileName = NuSMVCore_get_tool_rc_file_name();
  commandLine = ALLOC(char, strlen(cmd_pattern) + strlen(libraryName) + 
		      strlen(rcFileName)+1);
  sprintf(commandLine, cmd_pattern, libraryName, rcFileName);
  FREE(libraryName);
  status0 = Cmd_CommandExecute(commandLine);
  FREE(commandLine);

  /*
   * Look in home directory and current directory for .nusmvrc.
   */
  /* ~/.nusmvrc */
  tmpRcTilde = ALLOC(char, 
		     strlen(rcFileName) + strlen(rc_tilde_pattern) + 1);
  sprintf(tmpRcTilde, rc_tilde_pattern, rcFileName);

  homeFile = util_tilde_expand(tmpRcTilde);
  s1 = stat(homeFile, &home);
  FREE(homeFile);
  /* .nusmvrc */
  s2 = stat(rcFileName, &cur);

  /*
   * If .nusmvrc is present in both the home and current directories, then read
   * it from the home directory.  Otherwise, read it from wherever it's
   * located.
   */
  if ((s1 == 0) && (s2 == 0) && (home.st_ino == cur.st_ino)){
    /* ~/.nusmvrc == .nusmvrc : Source the file only once */
    tmpCmd = ALLOC(char, strlen(source_pattern) + strlen(tmpRcTilde) + 1);
    sprintf(tmpCmd, source_pattern, tmpRcTilde);
    status1 = Cmd_CommandExecute(tmpCmd);
    FREE(tmpCmd);
  }
  else {
    if (s1 == 0) {
      tmpCmd = ALLOC(char, strlen(tmpRcTilde) + strlen(source_pattern) + 1);
      sprintf(tmpCmd, source_pattern, tmpRcTilde);
      status1 = Cmd_CommandExecute(tmpCmd);
      FREE(tmpCmd);
    }
    if (s2 == 0) {
      tmpCmd = ALLOC(char, strlen(rcFileName) + strlen(source_pattern) + 1);
      sprintf(tmpCmd, source_pattern, rcFileName);
      status2 = Cmd_CommandExecute(tmpCmd);
      FREE(tmpCmd);
    }
  }

  FREE(tmpRcTilde);

  return (status0 && status1 && status2);
}


/**Function********************************************************************

  Synopsis           [Prints usage statistic.]

  Description        [Prints on <code>nusmv_stdout</code> usage
  statistics, i.e. the amount of memory used, the amount of time
  spent, the number of BDD nodes allocated and the size of the model
  in BDD.]

  SideEffects        []

  SeeAlso            [compilePrintBddModelStatistic]
******************************************************************************/
void print_usage(FILE * file)
{
  fprintf(nusmv_stdout,
          "###################################"
          "###################################\n");
  util_print_cpu_stats(file);
  fprintf(file,
          "###################################"
          "###################################\n");
  fprintf(file, "BDD statistics\n");
  fprintf(file, "--------------------\n");
  fprintf(file, "BDD nodes allocated: %d\n", get_dd_nodes_allocated(dd_manager));
  fprintf(file, "--------------------\n");
  if (opt_cone_of_influence(OptsHandler_get_instance()) == false) {
    BddFsm_ptr fsm = PropDb_master_get_bdd_fsm(PropPkg_get_prop_database());
    if (fsm != BDD_FSM(NULL)) { BddFsm_print_info(fsm, file); }
  }
  else {
    fprintf(nusmv_stderr,
            "WARNING: Model Statistics is not currently available\n");
    fprintf(nusmv_stderr,
            "if cone of influence reduction has been enabled.\n");
  }
}


/**Function********************************************************************

  Synopsis           [Saves internally the current FILE used for stdout]

  Description [Saves internally the current FILE used for stdout, for
  later restoring with restore_nusmv_stdout. The saved values are not
  stacked, only the current value is saved.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void save_nusmv_stdout()
{
  def_nusmv_stdout = nusmv_stdout;
}

/**Function********************************************************************

  Synopsis           [Restores a previously saved file.]

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void restore_nusmv_stdout()
{
  nusmv_stdout = def_nusmv_stdout;
}

/**Function********************************************************************

  Synopsis           [Opens and sets the current nusmv_stdout]

  Description [This function does not close the previously set
  nusmv_stdout.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void set_nusmv_stdout(const char* name)
{
  nusmv_stdout = fopen(name, "w");
}

/**Function********************************************************************

  Synopsis           [Returns the previously set internal nusmv_stdout]

  Description [Use this function only as input of the nusmv's library
  functions. Do NOT use it in CRTs different from the CRT the library
  was compiled with (e.g. in MSVC CRT when the library was compiled
  with mingw), or the application will crash. For example, do not call
  fprintf on the returned FILE*, if the CRT is not the same.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
FILE* get_nusmv_stdout()
{
  return nusmv_stdout;
}

/**Function********************************************************************

  Synopsis           [Prints the given string to stdout]

  Description        [Use this function to print on stdout from a different
  CRT.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void print_nusmv_stdout(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  (void) vfprintf(nusmv_stdout, fmt, args);
  va_end(args); 
}

/**Function********************************************************************

  Synopsis           [Closes the current nusmv_stdout]

  Description        [To use stdout, you will have to set a different value
  or restore it after calling this function. Do NOT close the default stdout.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void close_nusmv_stdout()
{
  fclose(nusmv_stdout);
}


/**Function********************************************************************

  Synopsis           [Saves internally the current FILE used for stderr]

  Description [Saves internally the current FILE used for stderr, for
  later restoring with restore_nusmv_stderr. The saved values are not
  stacked, only the current value is saved.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void save_nusmv_stderr()
{
  def_nusmv_stderr = nusmv_stderr;
}

/**Function********************************************************************

  Synopsis           [Restores a previously saved file.]

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void restore_nusmv_stderr()
{
  nusmv_stderr = def_nusmv_stderr;
}

/**Function********************************************************************

  Synopsis           [Opens and sets the current nusmv_stderr]

  Description [This function does not close the previously set
  nusmv_stderr.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void set_nusmv_stderr(const char* name)
{
  nusmv_stderr = fopen(name, "w");
}

/**Function********************************************************************

  Synopsis           [Returns the previously set internal nusmv_stderr]

  Description [Use this function only as input of the nusmv's library
  functions. Do NOT use it in CRTs different from the CRT the library
  was compiled with (e.g. in MSVC CRT when the library was compiled
  with mingw), or the application will crash. For example, do not call
  fprintf on the returned FILE*, if the CRT is not the same.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
FILE* get_nusmv_stderr()
{
  return nusmv_stderr;
}

/**Function********************************************************************

  Synopsis           [Prints the given string to stderr]

  Description        [Use this function to print on stderr from a different
  CRT.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void print_nusmv_stderr(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  (void) vfprintf(nusmv_stderr, fmt, args);
  va_end(args); 
}

/**Function********************************************************************

  Synopsis           [Closes the current nusmv_stderr]

  Description        [To use stderr, you will have to set a different value
  or restore it after calling this function. Do NOT close the default stderr.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void close_nusmv_stderr()
{
  fclose(nusmv_stderr);
}


/**Function********************************************************************

  Synopsis           [Frees the memory pointed by ptr]

  Description [Use this function like FREE(), to free all memory
  allocated by the core (with ALLOC), and returned to the user
  which is responsible for its disposal. 

  WARNING! This function *must* be used instead of FREE() when the
  memory disposal has to be done from within an environment which
  uses a memory allocator different from the one used by the
  core. For example, this is the case when using nusmv as a library
  compiled with a compiler, from external software compiled with a
  _different_ compiler which uses a different CRT.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void nusmv_FREE(void* ptr)
{
  FREE(ptr);
}
