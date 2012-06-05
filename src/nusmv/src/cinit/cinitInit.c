/**CFile***********************************************************************

  FileName    [cinitInit.c]

  PackageName [cinit]

  Synopsis    [Initializes and ends NuSMV.]

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

#include <string.h>

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "cinitInt.h"

#include "enc/enc.h"
#include "trace/pkg_trace.h"
#include "utils/error.h"
#include "utils/ustring.h"
#include "utils/assoc.h"
#include "utils/utils_io.h"
#include "utils/NodeList.h"
#include "compile/FlatHierarchy.h"
#include "utils/WordNumber_private.h" /* for WordNumber_init and ..._quit */
#include "prop/propPkg.h"
#include "dd/VarsHandler.h"
#include "wff/wff.h"

/* package initializers/deinitializers */
#include "fsm/fsm.h"
#include "hrc/hrc.h"

/*---------------------------------------------------------------------------*/
/* Constants declarations                                                    */
/*---------------------------------------------------------------------------*/

#define NUSMV_CPP_NAME "cpp"
#define NUSMV_M4_NAME "m4"

/* number of fields in structure preprocessors_list: */
#define PP_FIELDS_NUM  3

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

FILE* nusmv_stderr;
FILE* nusmv_stdout;
FILE* nusmv_stdin;
FILE* nusmv_historyFile;
FILE* nusmv_stdpipe;
DdManager* dd_manager = (DdManager*) NULL;
VarsHandler_ptr dd_vars_handler = VARS_HANDLER(NULL);

FILE* def_nusmv_stderr;
FILE* def_nusmv_stdout;

/**Variable********************************************************************

  Synopsis    [The list of pre-processor names and how to call them.]

  Description [This array is used to store the names of the avaliable
pre-processors on the system, as well as the command to excecute them. The
names are stored in even indices, with the corresponding command stored and the
location immediately succeeding the name. The last two entries are NULL to
indicate that there are no more entries.]

  SeeAlso     []

******************************************************************************/
static char** preprocessors_list = (char**) NULL;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void init_preprocessors ARGS((void));

static void quit_preprocessors ARGS((void));

static char* get_executable_name ARGS((const char* command));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Shuts down and restarts the system, shut down part]

  Description [Shuts down and restarts the system, shut down part]

  SideEffects []

  SeeAlso     [CInit_reset_last]

******************************************************************************/
void CInit_reset_first()
{
  TracePkg_quit();
  PropPkg_quit();

#if NUSMV_HAVE_SAT_SOLVER
  Bmc_Quit();
#endif

  Simulate_End();
  Fsm_quit();
  Enc_quit_encodings();
  Compile_quit();
  Hrc_quit();
  Parser_Quit();

  #ifdef DEBUG
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
    int result = dd_checkzeroref(dd_manager);
    if (result != 0) {
      fprintf(nusmv_stderr,
              "\n%d non-zero DD reference counts after dereferencing.\n",
              result);
    }
  }
  #endif

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Clearing DD and node packages....");
  }

  VarsHandler_destroy(dd_vars_handler);
  quit_dd_package(dd_manager);

  WordNumber_quit();

  wff_pkg_quit();
  set_pkg_quit();
  node_pkg_quit();

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Done\n");
  }
}

/**Function********************************************************************

  Synopsis    [Shuts down and restarts the system, restart part]

  Description [Shuts down and restarts the system, restart part]

  SideEffects []

  SeeAlso     [CInit_reset_first]

******************************************************************************/
void CInit_reset_last()
{
  node_pkg_init();
  init_the_node();
  set_pkg_init();
  wff_pkg_init();
  WordNumber_init();

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Restarting the DD manager....");
  }
  dd_manager = init_dd_package();
  dd_vars_handler = VarsHandler_create(dd_manager);

  /* restores the dynamic reordering method if enabled */
  if (opt_dynamic_reorder(OptsHandler_get_instance())) {
    dd_autodyn_enable(dd_manager,
                      get_reorder_method(OptsHandler_get_instance()));
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Done\n");
    fprintf(nusmv_stderr, "Restarting the compiler....\n");
  }

  Parser_Init();
  Hrc_init();
  Compile_init();

  Enc_init_encodings();
  Fsm_init();

  PropPkg_init();
  TracePkg_init();

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Done\n");
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Calls the initialization routines of all the packages.]

  SideEffects [Sets the global variables nusmv_stdout, nusmv_stderr,
  nusmv_historyFile.]

  SeeAlso     [SmEnd]

******************************************************************************/
void CInit_init()
{
  init_memory();
  nusmv_stdout     = stdout;
  nusmv_stderr     = stderr;
  nusmv_stdin      = stdin;
  def_nusmv_stdout = nusmv_stdout;
  def_nusmv_stderr = nusmv_stderr;
  nusmv_historyFile = NIL(FILE);

  /* win32 platforms seem to be "lazy" when they need to empty stream
     buffers. In this case we force buffers to be emptied explicitly */
#if NUSMV_HAVE_SETVBUF && (defined(__MINGW32__) || defined(__CYGWIN__))
# if SETVBUF_REVERSED
  setvbuf(nusmv_stdout, _IOLBF, (char *) NULL, 0);
  setvbuf(nusmv_stderr, _IONBF, (char *) NULL, 0);
# else
  setvbuf(nusmv_stdout, (char *) NULL, _IOLBF, 0);
  setvbuf(nusmv_stderr, (char *) NULL, _IONBF, 0);
# endif
#endif

  init_string();

  init_preprocessors();
  init_options();

  node_pkg_init();
  init_the_node();
  set_pkg_init();

  wff_pkg_init();

  dd_manager = init_dd_package();
  dd_vars_handler = VarsHandler_create(dd_manager);

  WordNumber_init();

  Cmd_Init();

  PropPkg_init();
  PropPkg_init_cmd();

  Parser_Init();
  Hrc_init();
  Compile_init();
  Compile_init_cmd();

  Enc_init_encodings();
  Enc_add_commands();

  Fsm_init();

  /* Other init here */
  cinit_AddCmd();
  dd_AddCmd();

  Bdd_Init();
  Mc_Init();
  Ltl_Init();
  Simulate_Init();
#if NUSMV_HAVE_SAT_SOLVER
  Bmc_init_opt();
  Bmc_AddCmd();
#endif
  TracePkg_init();
  traceCmd_init();

  Utils_pkg_init();
}


/**Function********************************************************************

  Synopsis    [Calls the end routines of all the packages.]

  SideEffects [Closes the output files if not the standard ones.]

  SeeAlso     [CInit_Init]

******************************************************************************/
void CInit_end()
{
  boolean print_final = false;

  Utils_pkg_quit();

  TracePkg_quit();
  PropPkg_quit_cmd();
  PropPkg_quit();
#if NUSMV_HAVE_SAT_SOLVER
  Bmc_Quit();
#endif
  Simulate_End();
  Enc_quit_encodings();
  Compile_quit();
  Hrc_quit();
  Parser_Quit();

  quit_preprocessors();

  /* commands */
  Bdd_End();
  Mc_End();
  Cmd_End();

#ifdef DEBUG
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
    int result = dd_checkzeroref(dd_manager);
    if (result != 0) {
      fprintf(stderr,
              "%d non-zero DD reference counts after dereferencing\n",
              result);
    }
  }
#endif

  VarsHandler_destroy(dd_vars_handler);
  quit_dd_package(dd_manager);

  WordNumber_quit();

  wff_pkg_quit();

  set_pkg_quit();
  node_pkg_quit();

  print_final = opt_verbose_level_gt(OptsHandler_get_instance(), 0);

  quit_string();
  deinit_options();
  if (print_final) {
    fprintf(nusmv_stderr, "\nSuccessful termination\n");
  }

  if (nusmv_stdout != stdout) fclose(nusmv_stdout);
  if (nusmv_stderr != stderr) fclose(nusmv_stderr);
  if (nusmv_historyFile != NIL(FILE)) fclose(nusmv_historyFile);
  if (nusmv_stdin != stdin) fclose(nusmv_stdin);
  nusmv_stdout = stdout;
  nusmv_stderr = stderr;
  nusmv_stdin  = stdin;
  nusmv_historyFile = NIL(FILE);
}


/**Function********************************************************************

  Synopsis    [Initializes information about the pre-processors avaliable.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void init_preprocessors()
{
  char* cpp_call = (char*) NULL;

  nusmv_assert(preprocessors_list == (char**) NULL);

  /* two triplets preprocessor/filename/command, one triplet for
     termination */
  preprocessors_list = ALLOC(char*, PP_FIELDS_NUM*3);
  nusmv_assert(preprocessors_list != (char**) NULL);

  /* sets the executable file for cpp preprocessor */
#if NUSMV_HAVE_GETENV
  cpp_call = getenv("CPP");
#endif

  if (cpp_call == (char*) NULL) {
#if NUSMV_HAVE_CPP
    cpp_call = NUSMV_PROG_CPP;
#else
    /* Tries anyway an executable: */
    cpp_call = NUSMV_CPP_NAME;
#endif
  }

  if (cpp_call == (char*) NULL) {
    internal_error("The pre-proprocessor could not be found.\n");
  }

  /* Stores the names of avaliable pre-processors along with the
     command to actually execute them. The NUL entries signify the end
     of the list: */

  /* cpp */
  preprocessors_list[0] = util_strsav(NUSMV_CPP_NAME);
  preprocessors_list[1] = get_executable_name(cpp_call);
  preprocessors_list[2] = util_strsav(cpp_call);

  /* m4 */
  preprocessors_list[3] = util_strsav(NUSMV_M4_NAME);
  preprocessors_list[4] = get_executable_name(NUSMV_M4_NAME);
  preprocessors_list[5] = util_strsav(NUSMV_M4_NAME);

  /* terminators: */
  preprocessors_list[6] = (char*) NULL;
  preprocessors_list[7] = (char*) NULL;
  preprocessors_list[8] = (char*) NULL;
}


/**Function********************************************************************

  Synopsis [Given a command, returns the executable file name (with
  extension if required)]

  SideEffects [If not already specified, extension suffix is appended
  to the returned string. Returned string must be freed.]

  SeeAlso     []

******************************************************************************/
static char* get_executable_name(const char* command)
{
  char* space;
  char* exec_name = (char*) NULL;
  size_t exec_len;
  size_t exeext_len;

  space = strchr(command, ' ');
  if (space != (char*) NULL) exec_len = (size_t) (space - command);
  else exec_len = strlen(command);

  exeext_len = strlen(NUSMV_EXEEXT);

  exec_name = ALLOC(char, exec_len + exeext_len + 1);
  nusmv_assert(exec_name != (char*) NULL);

  strncpy(exec_name, command, exec_len);
  exec_name[exec_len] = '\0'; /* adds a terminator */

  if ((exec_len > exeext_len) && (exeext_len > 0)) {
    /* the command might already contain NUSMV_EXEEXT: */
    char* pos;
    pos = strstr(exec_name, NUSMV_EXEEXT);
    if ( (pos == (char*) NULL) ||
         (((int) (pos - exec_name)) < (exec_len-exeext_len)) ) {
      /* add the suffix: */
      strcat(exec_name, NUSMV_EXEEXT);
    }
  }
  else {
    /* it can't contain the suffix: add it */
    strcat(exec_name, NUSMV_EXEEXT);
  }

  return exec_name;
}


/**Function********************************************************************

  Synopsis    [Removes information regarding the avaliable pre-processors.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void quit_preprocessors(void)
{
  char** iter;

  nusmv_assert(preprocessors_list != (char**) NULL);
  iter = preprocessors_list;
  while (*iter != (char*) NULL) {
    char * n = *iter;
    iter += 1;
    FREE(n);
  }

  FREE(preprocessors_list);
  preprocessors_list = (char**) NULL;
}


/**Function********************************************************************

  Synopsis    [Gets the command line call for the specified pre-processor
  name. Returns NULL if given name is not available, or a string that must be
  NOT freed]

  SideEffects []

  SeeAlso     []

******************************************************************************/
char* get_preprocessor_call(const char* name)
{
  char* res = (char*) NULL;
  char** iter;

  iter = preprocessors_list;
  while (*iter != (char*) NULL) {
    if (strncmp(*iter, name, strlen(name) + 1) == 0) {
      res = *(iter + 2);
      break;
    }

    iter += PP_FIELDS_NUM;
  }

  return (char*) res;
}


/**Function********************************************************************

  Synopsis    [Gets the actual program name of the specified pre-processor.
  Returns NULL if given name is not available, or a string that must be
  freed]

  SideEffects []

  SeeAlso     []

******************************************************************************/
char* get_preprocessor_filename(const char* name)
{
  char* res = (char*) NULL;
  char** iter;

  iter = preprocessors_list;
  while (*iter != (char*) NULL) {
    if (strncmp(*iter, name, strlen(name) + 1) == 0) {
      res = *(iter + 1);
      break;
    }

    iter += PP_FIELDS_NUM;
  }

  return (char*) res;
}


/**Function********************************************************************

  Synopsis    [Returns the number of available proprocessors]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int get_preprocessors_num()
{
  int len = 0;
  char** iter;

  iter = preprocessors_list;
  while (*iter != (char*) NULL) {
    ++len;
    iter += PP_FIELDS_NUM;
  }

  return len;
}


/**Function********************************************************************

  Synopsis    [Gets the names of the avaliable pre-processors. Returned
  string must be freed]

  SideEffects []

  SeeAlso     []

******************************************************************************/
char* get_preprocessor_names()
{
  int len;
  char* names;
  char** iter;

  /* length of the string: */
  len = 0;
  iter = preprocessors_list;
  while (*iter != (char*) NULL) {
    len += strlen(*iter) + 1; /* for the additional space */
    iter += PP_FIELDS_NUM;
  }

  names = ALLOC(char, len+1);
  nusmv_assert(names != (char*) NULL);

  names[0] = '\0';
  iter = preprocessors_list;
  while (*iter != (char*) NULL) {
    strncat(names, *iter, strlen(*iter));
    strncat(names, " ", 1);
    iter += PP_FIELDS_NUM;
  }

  names[len] = '\0';
  return names;
}
