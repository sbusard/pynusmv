/**CFile***********************************************************************

  FileName    [cmdMisc.c]

  PackageName [cmd]

  Synopsis    [Variable table; miscellaneous commands related to the general
  system.]

  Author      [Adapted to NuSMV by Marco Roveri]

  Copyright   [
  This file is part of the ``cmd'' package of NuSMV version 2.
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

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "cmdInt.h"
#include "utils/error.h"
#include "cinit/cinit.h"

#include <stdio.h>

#if NUSMV_HAVE_STRING_H
#include <string.h> /* for strdup */
#else
char* strdup(const char*); /* forward declaration */
#endif

static char rcsid[] UTIL_UNUSED = "$Id: cmdMisc.c,v 1.12.2.1.4.9.4.12 2010-02-17 09:29:25 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define MAX_STR         65536

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

#if NUSMV_HAVE_LIBREADLINE
typedef char **rl_completion_func_t(const char *, int, int);
typedef char *rl_compentry_func_t(const char *, int);
#endif

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
avl_tree *cmdAliasTable;

static boolean fileCreated;

/* Initialized by CmdInit, used by command 'time' */
static long start_time = -1;

#if NUSMV_HAVE_LIBREADLINE
EXTERN const char *rl_readline_name;
EXTERN int rl_completion_type;
EXTERN rl_completion_func_t *rl_attempted_completion_function;
#endif

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int CommandTime ARGS((int argc, char ** argv));
static int CommandEcho ARGS((int argc, char ** argv));
static int CommandMemoryProfile ARGS((int argc, char ** argv));
static int CommandQuit ARGS((int argc, char ** argv));
static int CommandUsage ARGS((int argc, char ** argv));
static int CommandWhich ARGS((int argc, char ** argv));
static int CommandHistory ARGS((int argc, char ** argv));
static int CommandAlias ARGS((int argc, char ** argv));
static int CommandUnalias ARGS((int argc, char ** argv));
static int CommandHelp ARGS((int argc, char ** argv));
static int CommandSource ARGS((int argc, char ** argv));
static int CommandShowHelp ARGS((int argc, char ** argv));

static void print_alias ARGS((char * value));
static char * command_alias_help ARGS((char * command));
static void FlushBuffers ARGS((int sigtype));

#if NUSMV_HAVE_LIBREADLINE
static char ** command_completion ARGS((const char *text, int start, int end));
static char *command_completion_generator ARGS((const char *text, int state));
EXTERN char **rl_completion_matches ARGS((const char *, rl_compentry_func_t *));
#endif

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Initializes the command package.]

  SideEffects [Commands are added to the command table.]

  SeeAlso     [Cmd_End]

******************************************************************************/
void Cmd_Init()
{
  cmdCommandTable = avl_init_table(strcmp);
  cmdAliasTable = avl_init_table(strcmp);
  cmdCommandHistoryArray = array_alloc(char *, 0);;

  Cmd_CommandAdd("alias", CommandAlias, 0, true);
  Cmd_CommandAdd("echo", CommandEcho, 0, true);
  Cmd_CommandAdd("help", CommandHelp, 0, true);
  Cmd_CommandAdd("quit", CommandQuit, 0, true);
  Cmd_CommandAdd("source", CommandSource, 0, true);
  Cmd_CommandAdd("unalias", CommandUnalias, 0, true);
  Cmd_CommandAdd("time", CommandTime, 0, true);
  Cmd_CommandAdd("usage", CommandUsage, 0, true);
  Cmd_CommandAdd("history", CommandHistory, 0, true);
  Cmd_CommandAdd("which", CommandWhich, 0, true);
  Cmd_CommandAdd("_memory_profile", CommandMemoryProfile, 0, true);
  Cmd_CommandAdd("_show_help", CommandShowHelp, 0, true);
  fileCreated = FALSE;

  /* Program the signal of type USR1 to flush nusmv_stdout and nusmv_stderr */
#ifdef SIGUSR1
  (void) signal(SIGUSR1, FlushBuffers);
#endif

  /* Initialize libreadline's completion machinery */
#if NUSMV_HAVE_LIBREADLINE
  rl_readline_name = "NuSMV";
  /*
   * rl_completion_type specifies the type of completion readline is
   * currently attempting.  `!' means to display all of the possible
   * completions, if there is more than one, as well as performing
   * partial completion.
   */
  rl_completion_type = '!';
  rl_attempted_completion_function = command_completion;
#endif

  nusmv_assert(start_time == -1);
  start_time = util_cpu_time();
}


/**Function********************************************************************

  Synopsis    [Ends the command package.]

  Description [Ends the command package. Tables are freed.]

  SideEffects []

  SeeAlso     [Cmd_Init]

******************************************************************************/
void
Cmd_End()
{
  avl_free_table(cmdCommandTable, (void (*)()) 0, CmdCommandFree);
  avl_free_table(cmdAliasTable, (void (*)()) 0, CmdAliasFree);

  {
    int c;
    char *dummy;

    for (c = array_n(cmdCommandHistoryArray); c-- > 0; ){
      dummy = array_fetch(char *, cmdCommandHistoryArray, c);
      FREE(dummy);
    }
  }

  array_free(cmdCommandHistoryArray);

  if (fileCreated == TRUE) {
    fprintf(nusmv_stdout, "Purify has created a temporary file. The file");
    fprintf(nusmv_stdout, " must be deleted.\n");
  }
}



/**Function********************************************************************

  Synopsis    [Opens a pipe with a pager]

  Description [Returns NULL if an error occurs]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
FILE* CmdOpenPipe(int useMore)
{
#if NUSMV_HAVE_POPEN
  FILE* rf = NIL(FILE);
  char* pager = NIL(char);

#if NUSMV_HAVE_GETENV
  pager = getenv("PAGER");
  if (pager == (char*) NULL) {
    rf = popen("more", "w");
    if (rf == (FILE*)NULL) {
      fprintf(nusmv_stderr, "Unable to open pipe with \"more\".\n");
    }
  }
  else {
    rf = popen(pager, "w");
    if (rf == NULL) {
      fprintf(nusmv_stderr, "Unable to open pipe with \"%s\".\n", pager);
    }
  }
#else
  rf = popen("more", "w");
  if (rf == (FILE*) NULL) {
    fprintf(nusmv_stderr, "Unable to open pipe with \"more\".\n");
  }
#endif
  return rf;

#else /* NUSMV_HAVE_POPEN */
  return (FILE*) NULL;
#endif
}


/**Function********************************************************************

  Synopsis    [Closes a previously opened pipe]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void CmdClosePipe(FILE* file)
{
#if NUSMV_HAVE_POPEN
  pclose(file);
#endif
}


/**Function********************************************************************

  Synopsis    [Open a file whose name is given]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
FILE* CmdOpenFile(const char* filename)
{
  FILE* rf = NIL(FILE);

  if (filename != NIL(char)) {
    rf = fopen(filename, "w");
    if (rf == (FILE*) NULL) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n", filename);
    }
  } else {
    fprintf(nusmv_stderr, "CmdOpenFile: file name is NULL\n");
  }
  return(rf);
}


/**Function********************************************************************

  Synopsis    [Closes a previously opened file]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void CmdCloseFile(FILE* file)
{
  fflush(file);
  fclose(file);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CmdFreeArgv(int  argc,  char ** argv)
{
  int i;

  for(i = 0; i < argc; i++) {
    FREE(argv[i]);
  }
  FREE(argv);
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CmdAliasFree(
  char * value)
{
  CmdAliasDescr_t *alias = (CmdAliasDescr_t *) value;

  CmdFreeArgv(alias->argc, alias->argv);
  FREE(alias->name);            /* same as key */
  FREE(alias);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis          [Implements the time command.]

  CommandName       [time]

  CommandSynopsis   [Provides a simple CPU elapsed time value]

  CommandArguments [\[-h\]]

  CommandDescription [Prints the processor time used since the last invocation
  of the \"time\" command, and the total processor time used since NuSMV
  was started.]

  SideEffects        []

******************************************************************************/
static int CommandTime(int  argc, char ** argv)
{
  static long last_time = 0;
  long time;
  int c;

  nusmv_assert(start_time >= 0);

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"h")) != EOF){
    switch(c){
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }

  if (argc != util_optind) {
    goto usage;
  }

  time = util_cpu_time();
  fprintf( nusmv_stdout,
           "elapse: %2.2f seconds, total: %2.2f seconds\n",
           (time - start_time - last_time) / 1000.0,
           (time - start_time) / 1000.0 );
  last_time = time;
  return 0;

usage:
  fprintf(nusmv_stderr, "usage: time [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Implements the echo command.]

  CommandName        [echo]

  CommandSynopsis    [Merely echoes the arguments. File redirection is allowed.]

  CommandArguments   [\[-h\] \[-2\] \[-n\] \[-o filename \[-a\]\] &lt;args&gt;]

  CommandDescription [Echoes its arguments to standard output.
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-2</tt>
     <dd> Redirects output to the standard error instead of the
     standard output. This cannot be used in combination with -o.
    <dt> <tt>-n</tt>
     <dd> Does not output the trailing newline.
    <dt> <tt>-o filename</tt>
     <dd> Echoes on the specified file instead of on the standard output.
    <dt> <tt>-a</tt>
     <dd> When used with option -o, appends the output to the specified file
          instead of overwriting it.
  </dl>]

  SideEffects        []

******************************************************************************/
static int CommandEcho(int  argc, char ** argv)
{
  int init_idx = 1;
  int i;
  int c;

  FILE* fout = nusmv_stdout;
  char* fname = (char*) NULL;
  boolean must_append = false;
  boolean trailing_nl = true;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "ha2no:")) != EOF) {
    switch(c) {
    case 'h':
        goto usage;
        break;

    case 'o':
      if (fname != (char*) NULL) FREE(fname);
      fname = ALLOC(char, strlen(util_optarg)+1);
      nusmv_assert(fname != (char*) NULL);
      strcpy(fname, util_optarg);
      init_idx += 2;
      break;

    case 'a':
      must_append = true;
      init_idx += 1;
      break;

    case '2':
      fout = nusmv_stderr;
      init_idx += 1;
      break;

    case 'n':
      trailing_nl = false;
      init_idx += 1;
      break;

    default:
      if (fname != (char*) NULL) FREE(fname);
      goto usage;
    }
  }

  if (fname != (char*) NULL) {
    /* the user asked to dump to a file */
    if (must_append) fout = fopen(fname, "a");
    else fout = fopen(fname, "w");

    if (fout == (FILE*) NULL) {
      /* counld not successfully open */
      fprintf(nusmv_stderr, "echo: unable to open file %s for writing.\n",
              fname);
      FREE(fname);
      rpterr("echo: an error occured");
    }

    FREE(fname);
  }

  for (i = init_idx; i < argc; i++) { fprintf(fout, "%s ", argv[i]); }
  if (trailing_nl) fprintf(fout, "\n");

  if (fout != nusmv_stdout && fout != nusmv_stderr) fclose(fout);
  return 0;

  usage:
  fprintf(nusmv_stderr, "usage: echo [-h] [-2] [-n] [[-o filename] [-a]] string \n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -2 \t\tRedirects to the standard error.\n");
  fprintf(nusmv_stderr, "   -n \t\tDoes not output the trailing newline.\n");
  fprintf(nusmv_stderr, "   -o filename \tRedirects the output to the specified file.\n");
  fprintf(nusmv_stderr,
          "   -a \t\tAppends the output to the end of the file specified \n"\
          "      \t\tby the option -o\n");
  return 1;
}

/**Function********************************************************************

  Synopsis    [Implements the _memory_profile command.]

  CommandName [_memory_profile]

  CommandSynopsis [It shows the amount of memory used by every package.]

  CommandArguments [\[-f &lt;filename&gt;\] \[-h\] \[-p\] \[-u &lt;units&gt;\]]

  CommandDescription [This command intregrates the output from purify with a
  function map generated by a perlscript plus another perlscript to generate a
  memory profile of NuSMV.<p>

  This command relies on the output of purify to a file to call the script
  "memoryaccount" and produces a summary of how much memory has been allocated
  by each package. Although this command may appear simple it requires the
  interaction of two scripts and three files, so special care should be taken
  when attempting to modify it.<p>

  Here is the way it works. The code in this command is conditionally compiled
  depending on the definition of the symbol <tt>PURIFY</tt>. If the symbol is
  not defined, the program prints a message notifying that the command is not
  operative in this executable. If <tt>PURIFY</tt> has been defined, there are
  certain things that are assumed. The executable has been linked with
  purify. The output of purify is being redirected to a file with name
  <tt>purify.log</tt>. The perl script <tt>memoryaccount</tt> is in
  <tt>$NuSMV_LIBRARY_PATH/common/share</tt> and it is
  executable. There exists a file whose name is <tt>.fmap</tt>, located
  in the same directory which script memoryaccount is located in. This file
  maps function names to packages which contain them.<p>

  The command then calls <tt>purify_all_inuse()</tt> to force purify to dump to
  the file <tt>purify.log</tt> all information about the memory that is
  currently visible to the program. This memory is not the total memory
  allocated by the program since there may be leaked memory that is no longer
  accessible. A temporary file is created and the script <tt>memoryaccount</tt>
  is called to analyze the file <tt>purify.log</tt> and write in the temporary
  file the memory profile obtained from it. Once the script is done, the
  temporary file is dumped to <tt>nusmv_stdout</tt> and deleted.<p>

  Since most of the computation in this command is done by the pearlscript
  <tt>memoryaccount</tt>, for more information please refer to the message
  printed when the script is invoked with the option <tt>-h</tt>.

  Command options:<p>

  <dl>
     <dt> -f &lt;filename&gt;
        <dd> File to read the dump from. The default is
             purify.log. This option should be used if and only if the
             option <tt>-log-file</tt> has been used at the linking
             stage when building the executable.
     <dt> -p
          <dd> Prints also the packages that did not allocated any detectable
          memory
     <dt> -u &lt;units&gt;
         <dd> Units to print the memory usage in. It may be "b" for
               bytes, "k" for kilobytes, "m" for megabytes and "g" for
               gigabytes. The default is bytes.
  </dl>
  ]

  SideEffects []

******************************************************************************/
static int
CommandMemoryProfile(
  int  argc,
  char ** argv)
{

  int   c;
  int   verbose;
  char  options[128];
#ifdef PURIFY
  char  tmpFileName[128];
  FILE  *fp;
  char  command[256];
  char  *NuSMVDirectoryName;
  int   systemStatus;
#endif

  /* Default values */
  verbose = 0;
  /*
   * Parse command line options.
   */
  options[0] = 0;
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "f:hpu:")) != EOF) {
    switch(c) {
      case 'f':
        strcat(options, " -f ");
        strcat(options, util_optarg);
        break;
      case 'h':
        goto usage;
        break;
      case 'p':
        strcat(options, " -p ");
        break;
      case 'u':
        strcat(options, " -u ");
        strcat(options, util_optarg);
        break;
      default:
        goto usage;
    }
  }


#if defined(PURIFY) && NUSMV_HAVE_SYSTEM
  /* Flag to remember that a file has been created by purify */
  fileCreated = TRUE;

  /* Obtain the name of a temporary file */
  tmpnam(tmpFileName);

  /* Kick purify to dump the data in the file */
  purify_all_inuse();

  /* Obtain the path to the perl script */
  NuSMVDirectoryName = CInit_NuSMVObtainLibrary();

  /* Prepare the string to be sent to a shell */
  c = snprintf(command, sizeof(command), "%s/memoryaccount %s %s/.fmap ./.fmap >%s",
               NuSMVDirectoryName, options, NuSMVDirectoryName,
               tmpFileName);
  SNPRINTF_CHECK(c, sizeof(command));

  /* Effectively execute the perlscript */
  systemStatus = system(command);
  if (systemStatus != 0) {
    return 1;
  }

  fp = Cmd_FileOpen(tmpFileName, "r", NIL(char *), 1);

  /* Check if the open has been successful */
  if (fp == NIL(FILE)) {
    fprintf(nusmv_stderr, "File %s was not found\n", tmpFileName);
    return 1;
  }

  /* Dump the contents of the result file in nusmv_stdout */
  while(fgets(command, 128, fp) != NIL(char)) {
    fprintf(nusmv_stdout, "%s", command);
  }
  fclose(fp);

  /* Remove the temporary file */
#if NUSMV_HAVE_UNLINK
  unlink(tmpFileName);
#endif
#else
  fprintf(nusmv_stderr, "Command not available: " \
          NUSMV_PACKAGE_NAME " has not been ");
  fprintf(nusmv_stderr, "compiled with purify.\n");
#endif

  return 0;             /* normal exit */

  usage:
  fprintf(nusmv_stderr, "usage: _memory_profile [-h] [-f <filename>]");
  fprintf(nusmv_stderr, "[-p] [-u <units>] <filenames>\n");
  fprintf(nusmv_stderr, "   -h\t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -f <file>\tFile to read the purify dump");
  fprintf(nusmv_stderr, " from. The default is \"purify.log\".\n");
  fprintf(nusmv_stderr, "   -p\t\tPrints also the packages that do not ");
  fprintf(nusmv_stderr, "allocate any memory.\n");
  fprintf(nusmv_stderr, "   -u <units>\tUnits to print the memory usage");
  fprintf(nusmv_stderr, " in. It may be b for bytes\n");
  fprintf(nusmv_stderr, "     \t\tk for kilobytes, m for megabytes and ");
  fprintf(nusmv_stderr, "g for gigabytes.\n");
  return 1;             /* error exit */
}


/**Function********************************************************************

  Synopsis          [Implements the quit command.]

  Description [A return value of -1 indicates a quick quit, -2 return frees
  the memory, -4 an instant quit]

  CommandName       [quit]

  CommandSynopsis   [exits NuSMV]

  CommandArguments  [\[-h\] \[-s\] \[-x\]]

  CommandDescription [Stops the program.  Does not save the current network
  before exiting.<p>

  Command options:<p>
  <dl>
     <dt> -s
     <dd> Frees all the used memory before quitting.
          This is slower, and it is used for finding memory leaks.
     <dt> -x
     <dd> Leave immediately. Skip all the cleanup code, leave it to
          the OS. This can save quite a long time.

  </dl>
  ]

  SideEffects        []

******************************************************************************/
static int
CommandQuit(
  int  argc,
  char ** argv)
{
  int c;

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"hsx")) != EOF){
    switch(c){
      case 'h':
        goto usage;
        break;
      case 's':
        return -2;
      case 'x':
        return -4;

      default:
        goto usage;
    }
  }

  if ( argc != util_optind){
    goto usage;
  }
  return -1;

  usage:
    fprintf(nusmv_stderr, "usage: quit [-h] [-s] | [-x] \n");
    fprintf(nusmv_stderr, "   -h  Prints the command usage.\n");
    fprintf(nusmv_stderr, "   -s  Frees all the used memory before quitting.\n");
    fprintf(nusmv_stderr, "   -x  Exits abruptly and silently.\n");
    return 1;
}

/**Function********************************************************************

  Synopsis          [Implements the usage command.]

  CommandName       [usage]

  CommandSynopsis   [Provides a dump of process statistics]

  CommandArguments  [\[-h\]]

  CommandDescription [Prints a formatted dump of processor-specific usage
  statistics. For Berkeley Unix, this includes all of the information in the
  getrusage() structure.]

  SideEffects        []

******************************************************************************/
static int
CommandUsage(
  int  argc,
  char ** argv)
{
  int c;

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"h")) != EOF){
    switch(c){
      case 'h':
        goto usage;
        break;
      default:
        goto usage;
    }
  }

  if (argc != util_optind){
    goto usage;
  }
  util_print_cpu_stats(nusmv_stdout);
  return 0;

  usage:
    fprintf(nusmv_stderr, "usage: usage [-h]\n");
    fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
    return 1;
}

/**Function********************************************************************

  Synopsis          [Implements the which command.]

  CommandName       [which]

  CommandSynopsis   [Looks for a file called \"file_name\"]

  CommandArguments  [\[-h\] &lt;file_name&gt;]

  CommandDescription [Looks for a file in a set of directories
  which includes the current directory as well as those in the NuSMV path.
  If it finds the specified file, it reports the found file's path.
  The searching path is specified through the "<tt>set open_path</tt>" command
  in \"<tt>.nusmvrc</tt>\".<p>

  Command options:<p>
  <dl>
     <dt> &lt;file_name&gt;
         <dd> File to be searched
  </dl>]

  SideEffects       []

  SeeAlso           [set]

******************************************************************************/
static int
CommandWhich(
  int  argc,
  char ** argv)
{
  FILE *fp;
  char *filename;
  int c;

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"h")) != EOF){
    switch(c){
      case 'h':
        goto usage;
        break;
      default:
        goto usage;
    }
  }

  if (argc-1 != util_optind){
    goto usage;
  }

  fp = Cmd_FileOpen(argv[1], "r", &filename, 0);
  if (fp != 0) {
    fprintf(nusmv_stdout, "%s\n", filename);
    (void) fclose(fp);
  }
  FREE(filename);
  return 0;

  usage:
    fprintf(nusmv_stderr,"usage: which [-h] file_name\n");
    fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
    return 1;
}


/**Function********************************************************************

  Synopsis          [Implements the history command.]

  CommandName       [history]

  CommandSynopsis   [list previous commands and their event numbers]

  CommandArguments  [\[-h\] \[&lt;num&gt;\]]

  CommandDescription [Lists previous commands and their event numbers.
  This is a UNIX-like history mechanism inside the NuSMV shell.<p>
  Command options:<p>
  <dl>
     <dt> &lt;num&gt;
         <dd> Lists the last &lt;num&gt; events.  Lists the last 30
              events if &lt;num&gt; is not specified.
  </dl><p>

  History Substitution:<p>

  The history substitution mechanism is a simpler version of the csh history
  substitution mechanism.  It enables you to reuse words from previously typed
  commands.<p>

  The default history substitution character is the `%' (`!' is default for
  shell escapes, and `#' marks the beginning of a comment). This can be changed
  using the "set" command. In this description '%' is used as the history_char.
  The `%' can appear anywhere in a line.  A line containing a history
  substitution is echoed to the screen after the substitution takes place.
  `%' can be preceded by a `\\' in order to escape the substitution,
  for example, to enter a `%' into an alias or to set the prompt.<br><p>

  Each valid line typed at the prompt is saved.  If the "history" variable
  is set (see help page for "set"), each line is also echoed to the history
  file.  You can use the "history" command to list the previously typed
  commands. <p>

  Substitutions: <p>

  At any point in a line these history substitutions are
  available.<p>
        <dl><dt>%:0   <dd>  Initial word of last command.</dl>
        <dl><dt>%:n   <dd>   n-th argument of last command.</dl>
        <dl><dt>%$    <dd>   Last argument of last command.</dl>
        <dl><dt>%*    <dd>   All but initial word of last command.</dl>

        <dl><dt>%%    <dd>   Last command.</dl>
        <dl><dt>%stuf <dd>   Last command beginning with "stuf".</dl>
        <dl><dt>%n    <dd>   Repeat the n-th command.</dl>
        <dl><dt>%-n   <dd>   Repeat the n-th previous command.</dl>
        <dl><dt>^old^new  <dd>       Replace "old" with "new" in previous command.
        Trailing spaces are significant during substitution.
        Initial spaces are not significant.</dl>  ]

  SideEffects        []

  SeeAlso            [set]

******************************************************************************/
static int
CommandHistory(
  int  argc,
  char ** argv)
{
  int i, num, lineno;
  int size;
  int c;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch(c) {
      case 'h':
        goto usage;
        break;
      default:
        goto usage;
    }
  }

  if (argc > 3) {
    goto usage;
  }
  num = 30;
  lineno = 1;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'h') {
        lineno = 0;
      }
      else {
        goto usage;
      }
    }
    else {
      num = atoi(argv[i]);
      if (num <= 0) {
        goto usage;
      }
    }
  }
  size = array_n(cmdCommandHistoryArray);
  num = (num < size) ? num : size;
  for (i = size - num; i < size; i++) {
    if (lineno != 0) {
      fprintf(nusmv_stdout, "%d\t", i + 1);
    }
    fprintf(nusmv_stdout, "%s\n", array_fetch(char *, cmdCommandHistoryArray, i));
  }
  return(0);

usage:
  fprintf(nusmv_stderr, "usage: history [-h] [num]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   num \t\tPrints the last num commands.\n");
  return(1);
}

/**Function********************************************************************

  Synopsis          [Implements the alias command.]

  CommandName       [alias]

  CommandSynopsis   [Provides an alias for a command]

  CommandArguments  [\[-h\] \[&lt;name&gt; \[&lt;string&gt;\]\]]

  CommandDescription [The "alias" command, if given no arguments, will print
  the definition of all current aliases.  <p>

  Given a single argument, it will print the definition of that alias (if any).

  Given two arguments, the keyword <tt>&lt;name&gt;</tt> becomes an alias for
  the command string <tt>&lt;string&gt;</tt>, replacing any other alias with
  the same name.<p>

  Command options:
  <dl>
     <dt> &lt;name&gt;
        <dd> Alias
     <dt> &lt;string&gt;
        <dd> Command string
  </dl>

  It is possible to create aliases that take arguments by using the history
  substitution mechanism. To protect the history substitution
  character `<tt>%</tt>' from immediate expansion, it must be preceded
  by a `<tt>\\</tt>' when entering the alias.<p>

  For example:<p>
  <code>
   NuSMV> alias read "read_model -i \\%:1.smv ; set input_order_file \\%:1.ord"
   NuSMV> read short
  </code><p>
  will create an alias `read', execute "read_model -i short.smv;
    set input_order_file short.ord".<p>

  And again:<p>
  <code>
  NuSMV> alias echo2 "echo Hi ; echo \\%* !" <br>
  NuSMV> echo2 happy birthday
  </code><p>

  will print:<p>

  <code>
  Hi<br>
  happy birthday !
  </code><br>

  CAVEAT: Currently there is no check to see if there is a circular
  dependency in the alias definition. e.g.<p>

  <code>
  NuSMV> alias foo "echo print_bdd_stats; foo"
  </code><br>

  creates an alias which refers to itself. Executing the command <tt>foo</tt>
  will result an infinite loop during which the command
  <tt>print_bdd_stats</tt> will be executed.
  ]

  SideEffects        []

  SeeAlso            [unalias]

******************************************************************************/
static int
CommandAlias(
  int  argc,
  char ** argv)
{
  int i;
  char *key, *value;
  CmdAliasDescr_t *alias;
  avl_generator *gen;
  int status;
  int c;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch(c) {
      case 'h':
        goto usage;
        break;
      default:
        goto usage;
    }
  }


  if (argc == 1) {
    avl_foreach_item(cmdAliasTable, gen, AVL_FORWARD, &key, &value) {
      print_alias(value);
    }
    return 0;

  }
  else if (argc == 2) {
    if (avl_lookup(cmdAliasTable, argv[1], &value)) {
      print_alias(value);
    }
    return 0;
  }

  /* delete any existing alias */
  key = argv[1];
  if (avl_delete(cmdAliasTable, &key, &value)) {
    CmdAliasFree(value);
  }

  alias = ALLOC(CmdAliasDescr_t, 1);
  alias->name = util_strsav(argv[1]);
  alias->argc = argc - 2;
  alias->argv = ALLOC(char *, alias->argc);
  for(i = 2; i < argc; i++) {
    alias->argv[i-2] = util_strsav(argv[i]);
  }
  status = avl_insert(cmdAliasTable, alias->name, (char *) alias);
  nusmv_assert(!status);  /* error here in SIS version, TRS, 8/4/95 */
  return 0;

  usage:
    fprintf(nusmv_stderr, "usage: alias [-h] [command [string]]\n");
    fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
    return (1);
}


/**Function********************************************************************

  Synopsis           [Implements the unalias command.]

  CommandName        [unalias]

  CommandSynopsis    [Removes the definition of an alias.]

  CommandArguments   [\[-h\] &lt;alias-names&gt;]

  CommandDescription [Removes the definition of an alias specified via the
  alias command.<p>

  Command options:<p>
  <dl>
     <dt> &lt;alias-names&gt;
     <dd> Aliases to be removed
  </dl>]

  SideEffects        []

  SeeAlso            [alias]

******************************************************************************/
static int
CommandUnalias(
  int  argc,
  char ** argv)
{
  int i;
  char *key, *value;
  int c;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch(c) {
      case 'h':
        goto usage;
        break;
      default:
        goto usage;
    }
  }

  if (argc < 2) {
    goto usage;
  }

  for(i = 1; i < argc; i++) {
    key = argv[i];
    if (avl_delete(cmdAliasTable, &key, &value)) {
      CmdAliasFree(value);
    }
  }
  return 0;

  usage:
    fprintf(nusmv_stderr, "usage: unalias [-h] alias_names\n");
    fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
    return 1;
}


/**Function********************************************************************

  Synopsis           [Implements the help command.]

  CommandName        [help]

  CommandSynopsis    [Provides on-line information on commands]

  CommandArguments   [\[-a\] \[-h\] \[&lt;command&gt;\]]

  CommandDescription [If invoked with no arguments "help" prints the list of
  all commands known to the command interpreter.
  If a command name is given, detailed information for that command will be
  provided.<p>

  Command options:<p>
  <dl>
      <dt> -a
          <dd> Provides a list of all internal commands, whose names begin
          with the underscore character ('_') by convention.
  </dl>]

  SideEffects        []

******************************************************************************/
static int CommandHelp(int  argc, char ** argv)
{
  int c, i, all;
  char *key;
  avl_generator *gen;
  char buffer[1024];
  char fname[1024];
  char *command;
  char *lib_name;
#if NUSMV_HAVE_GETENV
  char *pager;
#endif

  util_getopt_reset();
  all = 0;
  while ((c = util_getopt(argc, argv, "ah")) != EOF) {
    switch(c) {
    case 'a':
      all = 1;
      break;
    case 'h':
      goto usage;
      break;
    default:
      goto usage;
    }
  }

  if (argc - util_optind == 0) {
    i = 0;
    boolean nl_printed = false;

    avl_foreach_item(cmdCommandTable, gen, AVL_FORWARD, &key, NIL(char *)) {
      if ((key[0] == '_') == all) {
        boolean ow = (strlen(key) >= 35);

        /* If command should be printed on the second column, but it
           is too width, newline */
        if ((i % 2) == 1 && ow) {
          fprintf(nusmv_stdout, "\n");
        }

        fprintf(nusmv_stdout, "%-35s", key);
        ++i;

        if ((i % 2) == 0 || ow) {
          fprintf(nusmv_stdout, "\n");
          nl_printed = true;
        }
        else { nl_printed = false; }

        /* One long command takes 2 columns! If a newline has been
           printed because of a long command, next iteration we need
           to skip one newline printing! */
        if (ow && (i % 2) == 1) ++i;
      }
    }
    if (!nl_printed) {
      fprintf(nusmv_stdout, "\n");
    }
  }
  else if (argc - util_optind == 1) {
#if NUSMV_HAVE_SYSTEM
    command = command_alias_help(argv[util_optind]);
    lib_name = CInit_NuSMVObtainLibrary();
#if NUSMV_HAVE_GETENV
    pager = getenv("PAGER");
    if (pager != NULL) {
      c = snprintf(buffer, sizeof(buffer), "%s %s/help/%sCmd.txt", pager, lib_name, command);
      SNPRINTF_CHECK(c, sizeof(buffer));
    } else {
      c = snprintf(buffer, sizeof(buffer), "more %s/help/%sCmd.txt", lib_name, command);
      SNPRINTF_CHECK(c, sizeof(buffer));
    }
#else
    c = snprintf(buffer, sizeof(buffer), "more %s/help/%sCmd.txt", lib_name, command);
    SNPRINTF_CHECK(c, sizeof(buffer));
#endif
    c = snprintf(fname, sizeof(fname), "%s/help/%sCmd.txt", lib_name, command);
    SNPRINTF_CHECK(c, sizeof(fname));

    {
      FILE * test;
      if ((test = fopen(fname, "r")) == NULL) {
        fprintf(nusmv_stderr, "The manual for the command \"%s\" is not available.\n", command);
        if (strcmp(command, "help") == 0) {
          fprintf(nusmv_stderr,
                  "In order to make the command \"help\" working, you should\n"
                  "first set the \"NuSMV_LIBRARY_PATH\" environment variable to\n"
                  "<your-nusmv-install-path>/share directory. If you still have\n"
                  "trouble with the on-line help, it may be that you didn't build\n"
                  "the NuSMV documentation. Type \"make docs\" in your NuSMV\n"
                  "source directory in order to do that\n");
        }
        else {
          fprintf(nusmv_stderr, "Type \"help help\" for a possible solution\n");
        }
      } else {
        (void) fclose(test);
        (void) system(buffer);
      }
    }
    FREE(lib_name);

#else /* NUSMV_HAVE_SYSTEM */
    fprintf(nusmv_stderr, "The manual is not available.\n");
#endif
  }
  else {
    goto usage;
  }

  return 0;

 usage:
  fprintf(nusmv_stderr, "usage: help [-a] [-h] [command]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -a \t\tPrints help for all internal (hidden) commands.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis          [Implements the source command.]

  CommandName       [source]

  CommandSynopsis   [Executes a sequence of commands from a file]

  CommandArguments  [\[-h\] \[-p\] \[-s\] \[-x\] &lt;file&gt; \[&lt;args&gt;\]]

  CommandDescription [Reads and executes commands from a file.<p>
  Command options:<p>
  <dl>
    <dt> -p
       <dd> Prints a prompt before reading each command.
    <dt> -s
       <dd> Silently ignores an attempt to execute commands from a nonexistent file.
    <dt> -x
       <dd> Echoes each command before it is executed.
    <dt> &lt;file&gt;
       <dd> File name
  </dl>

  Arguments on the command line after the filename are remembered but not
  evaluated.  Commands in the script file can then refer to these arguments
  using the history substitution mechanism.<p>

  EXAMPLE:<p>

  Contents of test.scr:<p>

  <br><code>
  read_model -i %:2<br>
  flatten_hierarchy<br>
  build_variables<br>
  build_model<br>
  </code><br>

  Typing "source test.scr short.smv" on the command line will execute the
  sequence<p>

  <br><code>
  read_model -i short.smv<br>
  flatten_hierarchy<br>
  build_variables<br>
  build_model<br>
  </code><br>

  (In this case <code>%:0</code> gets "source", <code>%:1</code> gets
  "test.scr", and <code>%:2</code> gets "short.smv".)
  If you type "alias st source test.scr" and then type "st short.smv bozo",
  you will execute<p>

  <br><code>
  read_model -i bozo<br>
  flatten_hierarchy<br>
  build_variables<br>
  build_model<br>
  </code><br>

  because "bozo" was the second argument on the last command line typed.  In
  other words, command substitution in a script file depends on how the script
  file was invoked. Switches passed to a command are also counted as
  positional parameters. Therefore, if you type "st -x short.smv bozo",
  you will execute

  <br><code>
  read_model -i short.smv<br>
  flatten_hierarchy<br>
  build_variables<br>
  build_model<br>
  </code><br>

  To pass the "-x" switch (or any other switch) to "source" when the
  script uses positional parameters, you may define an alias. For
  instance, "alias srcx source -x".<p>

  returns -3 if an error occurs and the flag 'on_failure_script_quits'
  is set.  ]

  SideEffects        []

  SeeAlso            [history]

******************************************************************************/
static int
CommandSource(
  int  argc,
  char ** argv)
{
  int c, echo, prompt, silent, interactive, quit_count, lp_count;
  int status = 0; /* initialize so that lint doesn't complain */
  int lp_file_index, did_subst;
  char *real_filename, line[MAX_STR], *command;
  FILE *fp;

  interactive = silent = prompt = echo = 0;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hipsx")) != EOF) {
    switch(c) {
        case 'h':
          goto usage ;
          break;
        case 'i':               /* a hack to distinguish EOF from stdin */
          interactive = 1;
          break;
        case 'p':
          prompt = 1;
          break;
        case 's':
          silent = 1;
          break;
        case 'x':
          echo = 1;
          break;
      default:
          goto usage;
    }
  }

  /* added to avoid core-dumping when no script file is specified */
  if (argc == util_optind){
    goto usage;
  }

  lp_file_index = util_optind;
  lp_count = 0;

  /*
   * FIX (Tom, 5/7/95):  I'm not sure what the purpose of this outer do loop
   * is. In particular, lp_file_index is never modified in the loop, so it
   * looks it would just read the same file over again.  Also, SIS had
   * lp_count initialized to -1, and hence, any file sourced by SIS (if -l or
   * -t options on "source" were used in SIS) would actually be executed
   * twice.
   */
  do {
    lp_count ++; /* increment the loop counter */

    fp = Cmd_FileOpen(argv[lp_file_index], "r", &real_filename, silent);
    if (fp == NULL) {
      FREE(real_filename);
      return ! silent;  /* error return if not silent */
    }

    quit_count = 0;
    do {
      char* prompt_string = (char*) NULL;

      if (prompt) {
        char* stmp = ALLOC(char, strlen(NuSMVCore_get_prompt_string())+1);
        nusmv_assert(stmp != (char*) NULL);

        stmp[0] = '\0';
        strcat(stmp, NuSMVCore_get_prompt_string());

        prompt_string = stmp;
      }

      /* clear errors -- e.g., EOF reached from stdin */
      clearerr(fp);

      /* read another command line */
      if (CmdFgetsFilec(line, MAX_STR, fp, prompt_string) == NULL) {

        if (prompt_string != (char*) NULL) FREE(prompt_string);

        if (interactive) {
          if (quit_count++ < 5) {
            fprintf(nusmv_stderr, "\nUse \"quit\" to leave %s.\n",
                    get_pgm_name(OptsHandler_get_instance()));
            continue;
          }
          status = -1;          /* fake a 'quit' */
        }
        else {
          status = 0;           /* successful end of 'source' ; loop? */
        }
        break;
      }
      else if (prompt_string != (char*) NULL) FREE(prompt_string);

      quit_count = 0;

      if (echo) {
        fprintf(nusmv_stdout, "%s", line);
      }
      command = CmdHistorySubstitution(line, &did_subst);
      if (command == NIL(char)) {
        status = 1;
        break;
      }
      if (did_subst) {
        if (interactive) {
          fprintf(stdout, "%s\n", command);
        }
      }
      if (command != line) {
        (void) strcpy(line, command);
      }
      if (interactive && *line != '\0') {
        array_insert_last(char *, cmdCommandHistoryArray, util_strsav(line));
        if (nusmv_historyFile != NIL(FILE)) {
          fprintf(nusmv_historyFile, "%s\n", line);
          (void) fflush(nusmv_historyFile);
        }
      }

      status = Cmd_CommandExecute(line);

    } while (status == 0);

    if (fp != stdin) {
      if (status > 0) {
        fprintf(nusmv_stderr, "aborting 'source %s'\n", real_filename);
      }
      (void) fclose(fp);
    }
    FREE(real_filename);

  } while ((status == 0) && (lp_count <= 0));

  /* An error occured during script execution */
  if (opt_on_failure_script_quits(OptsHandler_get_instance()) && (status > 0)) return -3;

  return status;

usage:
  fprintf(nusmv_stderr, "source [-h] [-p] [-s] [-x] file_name [args]\n");
  fprintf(nusmv_stderr, "\t-h Prints the command usage.\n");
  fprintf(nusmv_stderr, "\t-p Supplies prompt before reading each line.\n");
  fprintf(nusmv_stderr, "\t-s Silently ignores nonexistent file.\n");
  fprintf(nusmv_stderr, "\t-x Echoes each line as it is executed.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Implements the _show_help command.]

  CommandName        [_show_help]

  CommandSynopsis    [Provides on-line information for all commands]

  CommandArguments   [\[-f\] \[-h\] \[&lt;command&gt;\]]

  CommandDescription [If invoked with no arguments prints the short help
  for all commands known to the command interpreter including
  hidden commands (those whose name starts with _).<p>

  Command options:<p>
  <dl>
      <dt> -f </dt>
          <dd> Prints for each command the long help.</dd>

  </dl>]

  SideEffects        []

******************************************************************************/
static int CommandShowHelp(int  argc, char ** argv)
{
  int c, longmaual;
  char *key, *lib_name;
  char ch;
  avl_generator *gen;
  char command[1024];
  char fname[1024];
  FILE* fileid;

  util_getopt_reset();
  longmaual = 0;
  while ((c = util_getopt(argc, argv, "f")) != EOF) {
    switch(c) {
    case 'f':
      longmaual = 1;
      break;
    case 'h':
      goto usage;
      break;
    default:
      goto usage;
    }
  }

  lib_name = CInit_NuSMVObtainLibrary();

  avl_foreach_item(cmdCommandTable, gen, AVL_FORWARD, &key, NIL(char *)) {
    fprintf(nusmv_stderr,
            "==============================================================================\n");
    if (1 == longmaual) {
      int c = snprintf(fname, 1023, "%s/help/%sCmd.txt", lib_name, key);
      SNPRINTF_CHECK(c, 1023);

      fileid = fopen(fname, "r");
      if ((FILE *)NULL == fileid) {
        fprintf(nusmv_stderr, "The manual for the command '%s'"\
                "is not available.\n", key);
      } else {
        while(!feof(fileid)) {
          ch = fgetc(fileid);
          if (EOF != ch) fputc(ch, nusmv_stderr);
        }
        fclose(fileid);
      }
    }
    else {
      int c = 0;
      fprintf(nusmv_stderr, "COMMAND = %s\n", key);
      c = snprintf(command, 1023, "%s -h", key);
      SNPRINTF_CHECK(c, 1023);
      Cmd_CommandExecute(command);
    }
    fprintf(nusmv_stderr,
            "==============================================================================\n");
  }
  FREE(lib_name);

  return 0;

  usage:
    fprintf(nusmv_stderr, "usage: _show_help [-f] [-h]\n");
    fprintf(nusmv_stderr, "   -f \t\tPrints the long help for all commands.\n");
    fprintf(nusmv_stderr, "      \t\tBy default the short help is printed.\n");
    return 1;
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
print_alias(
  char * value)
{
  int i;
  CmdAliasDescr_t *alias;

  alias = (CmdAliasDescr_t *) value;
  fprintf(nusmv_stdout, "%s\t", alias->name);
  for(i = 0; i < alias->argc; i++) {
    fprintf(nusmv_stdout, " %s", alias->argv[i]);
  }
  fprintf(nusmv_stdout, "\n");
}

#if NUSMV_HAVE_LIBREADLINE
/**Function********************************************************************

  Synopsis    [Generate completion matches for readline.]

  Description [Based on the partial input and the list of installed commands
                generates the possible completions.]

  SideEffects [none]

  SeeAlso     [Cmd_Completion]

******************************************************************************/
static char *command_completion_generator(const char *text, int state)
{
  static int list_index, len;
  char* key;
  avl_generator* gen;
  int cnt;

  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  cnt = 0;
  avl_foreach_item(cmdCommandTable, gen, AVL_FORWARD, &key, NIL(char *)) {
    if (strncmp(text, key, len) == 0) { // possible match
      if (cnt++ == list_index) {
        list_index++;
        return strdup(key);
      }
    }
  }

  return NULL;
}

/**Function********************************************************************

  Synopsis    [Sets up command or filename completion on reading user input.]

  Description [We use the regular NuSMV command completion function for the
                first word on the line, and filename completion for the rest.]

  SideEffects [none]

  SeeAlso     [CmdCompletion_Generator]

******************************************************************************/
static char ** command_completion(const char *text, int start, int end)
{
  if (start) return NULL; /* not on line beginning: filename */
  else return rl_completion_matches(text, command_completion_generator);
}
#endif

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static char *
command_alias_help(
  char * command)
{
  char *value;
  CmdAliasDescr_t *alias;

  if (!avl_lookup(cmdAliasTable, command, &value)) {
    return command;
  }
  alias = (CmdAliasDescr_t *) value;
  return alias->argv[0];
}

/**Function********************************************************************

  Synopsis           [Function to flush nusmv_stdout and nusmv_stderr.]

  Description [This function is the signal handler for the SIGUSR1
  signal. Whenever that signal is received, this function is executed and the
  output channels of NuSMV are flushed.]

  SideEffects        []

  SeeAlso            [Cmd_Init]

******************************************************************************/
static void
FlushBuffers(
  int sigtype)
{
  fflush(nusmv_stdout);
  fflush(nusmv_stderr);

  /* Reprogram again the handler */
#ifdef SIGUSR1
  (void) signal(SIGUSR1, FlushBuffers);
#endif
} /* End of FlushBuffers */
