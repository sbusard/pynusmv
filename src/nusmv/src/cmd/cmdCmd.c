/**CFile***********************************************************************

  FileName    [cmdCmd.c]

  PackageName [cmd]

  Synopsis    [Command table and command execution.]

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
#include "nusmv-config.h"
#endif
#include "cmdInt.h"
#include "utils/error.h" /* for CATCH */

#if NUSMV_HAVE_SIGNAL_H
#include <signal.h>
#endif

/* 
 * Support to define unused varibles
 */
static char rcsid[] UTIL_UNUSED = "$Id: cmdCmd.c,v 1.8.2.1.4.4.4.6 2009-08-03 11:45:38 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
avl_tree *cmdCommandTable;
array_t *cmdCommandHistoryArray;

static char NuSMVShellChar = '!';      /* can be reset using the "set shell_char" */

static int autoexec;		/* indicates currently in autoexec */


/* to check currently executed command reentrancy capability */
static boolean is_curr_cmd_reentrant = false; 
static void 
cmd_set_curr_reentrant(boolean value) {is_curr_cmd_reentrant = value;}
static boolean cmd_is_curr_reentrant(void) {return is_curr_cmd_reentrant;}


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int com_dispatch ARGS((int argc, char ** argv));
static int apply_alias ARGS((int * argcp, char *** argvp, int * loop));
static void variableInterpolation ARGS((int argc, char **argv));
static char * variableInterpolationRecur ARGS((char *str));
static char * split_line ARGS((char * command, int * argc, char *** argv));
static int check_shell_escape ARGS((char * p, int * status));
static void disarm_signal_andler ARGS((void));
static void arm_signal_andler ARGS((void));

#if NUSMV_HAVE_SIGNAL_H
static void sigterm ARGS((int sig));
#endif

/**AutomaticEnd***************************************************************/

/**Function********************************************************************

  Synopsis    [Adds a command to the command table.]

  Description [Adds a command to the command table.  If name already defines
  an existing command, its definition is replaced.  FuncFp is a function
  pointer to code of the form: <p>

                int <br>
		CommandTest(argc, argv)<br>
                  int argc;<br>
                  char **argv;<br>
                {<br>
		    return 0;<br>
		}<p>

  argv\[0\] will generally
  be the command name, and argv\[1\] ... argv\[argc-1\] are the arguments for the
  command.  util_getopt() can be used to parse the arguments, but
  util_getopt_reset() must be used before calling util_getopt().  The command
  function should return 0 for normal operation, 1 for any error.  The changes
  flag is used to automatically save the hmgr before executing the command (in
  order to support undo).
  The flag reentrant is true if the command execution can be interrupted without
  leaving the internal status inconsistent.
  ]

  SideEffects []

******************************************************************************/
void Cmd_CommandAdd(char* name, PFI  funcFp, int  changes, boolean reentrant)
{
  char *key, *value;
  CommandDescr_t *descr;
  int status;

  key = name;
  if (avl_delete(cmdCommandTable, &key, &value)) {
    /* delete existing definition for this command */
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "warning: redefining '%s'\n", name);
    }
    CmdCommandFree(value);
  }

  descr = ALLOC(CommandDescr_t, 1);
  descr->name = util_strsav(name);
  descr->command_fp = funcFp;
  descr->changes_hmgr = changes;
  descr->reentrant = reentrant;
  status = avl_insert(cmdCommandTable, descr->name, (char *) descr);
  nusmv_assert(!status);  /* error here in SIS version, TRS, 8/4/95 */
}


/**Function********************************************************************

  Synopsis    [Removes given command from the command table.]

  Description [Returns true if command was found and removed,
  false if not found]

  SideEffects []

******************************************************************************/
boolean Cmd_CommandRemove(const char* name)
{
  char *key, *value;
  boolean status;

  key = (char*) name;
  status = (avl_delete(cmdCommandTable, &key, &value) != 0);
  if (status) CmdCommandFree(value);
  
  return status;
}


/**Function********************************************************************

  Synopsis    [True iff a command named 'name' is defined.]

  Description []

  SideEffects []

******************************************************************************/
boolean Cmd_CommandDefined(const char* name)
{
  char *key;
  boolean status;

  key = (char*) name;
  status = (avl_lookup(cmdCommandTable, key, (char**) NULL) != 0);
  
  return status;
}


/**Function********************************************************************

  Synopsis    [Returns the command stored under 'name' in the command table.]

  Description [Returned value does not belong to caller.]

  SideEffects []

******************************************************************************/
CommandDescr_t *Cmd_CommandGet(const char* name)
{
  char *key;
  CommandDescr_t *value;
  boolean status;

  key = (char*) name;
  status = (avl_lookup(cmdCommandTable, key, (char**) &value) != 0);

  if (status) {
    return value;
  } else {
    return (CommandDescr_t*) NULL;
  }
}


/**Function********************************************************************

  Synopsis    [Executes a command line.]

  Description [Executes a command line.  This is the top-level of the command
  interpreter, and supports multiple commands (separated by ;), alias
  substitution, etc.  For many simple operations, Cmd_CommandExecute() is the
  easiest way to accomplish a given task. For example, to set a variable, use
  the code: Cmd_CommandExecute("set color blue").]

  SideEffects []

******************************************************************************/
int Cmd_CommandExecute(char* command)
{
  int status, argc;
  int loop;
  char *commandp, **argv;

  disarm_signal_andler();

  commandp = command;
  do {
    if (check_shell_escape(commandp, &status)) break;
    
    commandp = split_line(commandp, &argc, &argv);
    loop = 0;
    status = apply_alias(&argc, &argv, &loop);
    if (status == 0) {
      variableInterpolation(argc, argv);
      status = com_dispatch(argc, argv);
    }
    CmdFreeArgv(argc, argv);
  } while (status == 0 && *commandp != '\0');

  return status;
}


/**Function********************************************************************

  Synopsis    [Secure layer for Cmd_CommandExecute]

  Description [This version is securly callable from scripting languages. 
  Do not call Cmd_CommandExecute directly from a scripting language, otherwise
  the script execution could be aborted without any warning.]

  SideEffects []

******************************************************************************/
int Cmd_SecureCommandExecute(char* command)
{
  int res;

  CATCH {
    res = Cmd_CommandExecute(command);
  } FAIL {
    res = 1;
  }
  return(res);
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
CmdCommandFree(
  char * value)
{
  CommandDescr_t *command = (CommandDescr_t *) value;

  FREE(command->name);		/* same as key */
  FREE(command);
}


/**Function********************************************************************

  Synopsis    [Copies value.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
CommandDescr_t *
CmdCommandCopy(
  CommandDescr_t * value)
{
  CommandDescr_t * res;

  nusmv_assert(value != (CommandDescr_t*) NULL);

  res = ALLOC(CommandDescr_t, 1);
  nusmv_assert(res != (CommandDescr_t*) NULL);

  res->name         = util_strsav(value->name);
  res->command_fp   = value->command_fp;
  res->changes_hmgr = value->changes_hmgr;
  res->reentrant    = value->reentrant;

  return res;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static int
com_dispatch(
  int  argc,
  char ** argv)
{
  int status;
  char *value;
  CommandDescr_t *descr;

  if (argc == 0) {		/* empty command */
    return 0;
  }

  if (! avl_lookup(cmdCommandTable, argv[0], &value)) {
    fprintf(nusmv_stderr, "unknown command '%s'\n", argv[0]);
    return 1;
  }

  descr = (CommandDescr_t *) value;

  arm_signal_andler();

  CATCH {
    cmd_set_curr_reentrant(descr->reentrant);
    status = (*descr->command_fp)(argc, argv);
    cmd_set_curr_reentrant(true);

    /* automatic execution of arbitrary command after each command */
    /* usually this is a passive command ... */
    if (status == 0 && ! autoexec) {
      OptsHandler_ptr opt = OptsHandler_get_instance();
      if (OptsHandler_is_option_registered(opt, "autoexec")) {
        value = OptsHandler_get_string_option_value(opt, "autoexec");

        nusmv_assert((char*)NULL != value);

        autoexec = 1;
        status = Cmd_CommandExecute(value);
        autoexec = 0;
      }
    }
  } FAIL { 
    return(1);
  }

  disarm_signal_andler();

  return status;
}


/**Function********************************************************************

  Synopsis    [Applies alias.]

  Description [Applies alias.  If perform a history substitution in expanding
  an alias, remove all the orginal trailing arguments.  For example:<p>

    > alias t rl \!:1<br>
    > t lion.blif  would otherwise expand to   rl lion.blif lion.blif <br>]

  SideEffects []

  SeeAlso     [optional]

******************************************************************************/
static int
apply_alias(
  int * argcp,
  char *** argvp,
  int * loop)
{
  int i, argc, stopit, added, offset, did_subst, subst, status, newc, j;
  char *arg, **argv, **newv;
  CmdAliasDescr_t *alias;

  argc = *argcp;
  argv = *argvp;
  stopit = 0;
  for(; *loop < 20; (*loop)++) {
    if (argc == 0) {
      return 0;
    }
    if (stopit != 0 || avl_lookup(cmdAliasTable, argv[0], (char **) &alias) == 0) {
      return 0;
    }
    if (strcmp(argv[0], alias->argv[0]) == 0) {
      stopit = 1;
    }
    FREE(argv[0]);
    added = alias->argc - 1;

    /* shift all the arguments to the right */
    if (added != 0) {
      argv = REALLOC(char *, argv, argc + added);
      for (i = argc - 1; i >= 1; i--) {
        argv[i + added] = argv[i];
      }
      for (i = 1; i <= added; i++) {
        argv[i] = NIL(char);
      }
      argc += added;
    }
    subst = 0;
    for (i = 0, offset = 0; i < alias->argc; i++, offset++) {
      arg = CmdHistorySubstitution(alias->argv[i], &did_subst);
      if (arg == NIL(char)) {
        *argcp = argc;
        *argvp = argv;
        return(1);
      }
      if (did_subst != 0) {
        subst = 1;
      }
      status = 0;
      do {
        arg = split_line(arg, &newc, &newv);
        /*
         * If there's a complete `;' terminated command in `arg',
         * when split_line() returns arg[0] != '\0'.
         */
        if (arg[0] == '\0') {   /* just a bunch of words */
          break;
        }
        status = apply_alias(&newc, &newv, loop);
        if (status == 0) {
          status = com_dispatch(newc, newv);
        }
        CmdFreeArgv(newc, newv);
      } while (status == 0);
      if (status != 0) {
        *argcp = argc;
        *argvp = argv;
        return(1);
      }
      added = newc - 1;
      if (added != 0) {
        argv = REALLOC(char *, argv, argc + added);
        for (j = argc - 1; j > offset; j--) {
          argv[j + added] = argv[j];
        }
        argc += added;
      }
      for (j = 0; j <= added; j++) {
        argv[j + offset] = newv[j];
      }
      FREE(newv);
      offset += added;
    }
    if (subst == 1) {
      for (i = offset; i < argc; i++) {
        FREE(argv[i]);
      }
      argc = offset;
    }
    *argcp = argc;
    *argvp = argv;
  }

  fprintf(nusmv_stderr, "error -- alias loop\n");
  return 1;
}


/**Function********************************************************************

  Synopsis    [Allows interpolation of variables]

  Description [Allows interpolation of variables. Here it is implemented by
  allowing variables to be referred to with the prefix of '$'. The variables
  are set using the "set" command. So for example, the following can be done <p>

  <code>
  NuSMV> set foo bar <br>
  NuSMV> echo $foo <br>
  bar <br>
  </code>
  
  The last line "bar" will the output produced by NuSMV.

  The following can also be done: <p>
  
  <code>
  NuSMV> set foo $foo:foobar <br>
  NuSMV> echo $foobar <br>
  bar:foobar <br>
  </code>
  The last line will be the output produced by NuSMV. <p>

  These variables can
  be used in recursive definitions. The following termination characters are
  recognized for the variables \\n, \\0, ' ', \\t,  :,  ;,  #,  /.

  Although the set command allows the usage of the some of the
  above termination characters between quotes, 
  the variable interpolation procedure has the restriction
  that the two characters ':' and '/' may not be used with quotes.
  A variable with spaces in it may be used only if it is enclosed
  within quotes. ]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/

static void
variableInterpolation(int argc, char **argv)
{
  int i;          /* to iterate through the arguments */
  char *newStr;   /* string returned by the expanded value */
  char dollar;    /* character to store reference to the variable, now '$' */


  dollar = '$';

  /* step through all argvs */
  for (i = 0; i < argc; i++) {
    if (strchr((char *)argv[i], (int)dollar) != NULL) {
      /* expanded string returned by the procedure */
      newStr = variableInterpolationRecur(argv[i]);
      FREE(argv[i]);
      /* replace old value with new */
      argv[i] = newStr;
    }
  } /* end of iterating through all arguments */
}/* end of variable interpolation */


/**Function********************************************************************

  Synopsis    [Recursive procedure that expands the interpolation variables]

  Description [Recursive procedure that expands the interpolation variables.
  This procedure is designed to handle multiple occurrences of variables
  in a string and recursive definitions. If the expanded variable has another
  variable, then the procedure is called recursively. The existence of a
  variable is identified by the $ sign in the string. But since this may be
  an environment variable too, the variable is untouched if not found in
  this table. A sophisticated check can be made to see if this variable
  exists in the environment, but it is NOT done here. Therefore, detection
  of bogus values cannot be done. The procedure steps through the string
  to see if any variables are present. If a termination character (one of
  :, /) is found after the '$', then the variable
  is identified and looked up in the flag table. If the returned string again
  has a dollar, then the procedure is called recursively. If not, the returned
  value replaces the variable and the stepping through continues. If the
  variable is not found, then it might be an environment variable.So the
  procedure leaves the variable there. ]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/

static char *
variableInterpolationRecur(char *str)
{
  int i;               /* iterators */
  int findEndDollar;   /* flag to denote that a $ has been found. So
                        * search till end of variable
                        */
  int endDollarIndex;  /* index in the current string of the end of the
                        * variable
                        */
  int dollarIndex;     /* index in the current string of the dollar sign */  
  int singleQuote;     /* flag that symbolizes that a quote is started */ 
  int doubleQuote;     /* flag that symbolizes that a quote is started */
  char *value;         /* value of the variable that is returned by the table */
  char *subStr;        /* string to store the variable */
  int curStrIndex;     /* index to step through the current string */
  int subLen;          /* length of the variable */
  int index;           /* variable use to step through the various strings */
  char *curStr;        /* current string which may change as substitutions
                        * take place
                        */
  char *newCurStr;     /* new string pieced together with the expanded value */
  char c;              /* current character in the string */

  int freeNewValue;    /* new value of string returned by recursion needs
                        * to be freed.
                        */
  char dollar;         /* character that stores the dollar sign */
  int lastPos;         /* position of the last character of the variable
                        * in the string.
                        */
  int envVar;           /* flag to say that the variable is not found in
                         * the table, hence may be an environment variable
                         */

  dollar = '$';
  curStrIndex = 0;
  value = (char*) NULL;
  subLen = 0;
  findEndDollar = 0;
  singleQuote = 0;
  doubleQuote = 0;
  dollarIndex = -1;
  endDollarIndex = -1;
  /* make a local copy since the string may change */
  curStr = ALLOC(char, strlen(str)+1);
  curStr = strncpy(curStr, str, strlen(str)+1);
  /* search through the end of string including te \0 character to detect
   * end of variable, if required.
   */
  while (curStrIndex <= strlen(curStr)) {
    /* current character */
    c = curStr[curStrIndex];
    /* redundant here since split_line already strips out the quotes */
    if ((c == '\"') || (c == '\'')) {
      /* also termination charactrers for $ */
      /* set flags for quote found */
      singleQuote = !singleQuote;
      doubleQuote = !doubleQuote;
      /* also a variable termination */
      if (findEndDollar) {
        findEndDollar = 0;
        endDollarIndex = curStrIndex;
      }
    }
    /* detect a $ if not within quotes */
    if ((c == '$') && (!singleQuote) && (!doubleQuote)) {
      if (findEndDollar == 1) {
        (void)fprintf(nusmv_stderr, "Cannot have nested $ signs, not found termination\n");
        break;
      }
      /* note the beginning of the dollar position */
      dollarIndex = curStrIndex;
      /* start quest for end of dollar */
      findEndDollar = 1;
      endDollarIndex = -1;
    }
    /* termination characters are \0, :, / when not within quotes.
     * Although, some of these may never be encountered
     * since this is called after split_line and apply_alias
     * Termination characters except '\0' are ignored within quotes
     */
    if ((findEndDollar) &&
        ((c == '\0') ||
         ((!singleQuote) && (!doubleQuote) &&
          ((c == ':') || (c == '/'))))) {
      /*     if (((c == '\n') || (c == '\t') || (isspace(c)) ||
             (c == ':') || (c == ';') || (c == '\0') ||
             (c == '#') || (c == '/')) && (findEndDollar)) { */
      findEndDollar = 0;
      endDollarIndex = curStrIndex;
    } /* end of find termination characters */

    /* found the interpolation variable and its end*/
    if (!findEndDollar && (endDollarIndex != -1)) {
      /* found an interpolation variable */
      subLen = 0;
      freeNewValue = 0;
      envVar = 0;
      subStr = NULL;
      if (endDollarIndex > (dollarIndex +1)) {
        OptsHandler_ptr opt = OptsHandler_get_instance();
        /* if not empty string */
        subStr = ALLOC(char, endDollarIndex - dollarIndex);
        /* copy the variable into another string */
        for ( i = 0; i <  endDollarIndex - dollarIndex - 1; i++) {
          subStr[i] = curStr[dollarIndex+1+i];
        }
        subStr[i] = '\0';
        /* quiet if of the form var$:iable or var$foo:iable and
         * $foo not in flag table
         */
        if (OptsHandler_is_option_registered(opt, subStr)) {

          value = OptsHandler_get_string_representation_option_value(opt,
                                                                     subStr);
          /* NULL strings are returned as "NULL" string (4 chars +
             delimiter) instead of NULL pointer (0x0). */
          nusmv_assert((char*)NULL != value);

          /* found the variable in the alias table */
          if (strchr((char *)value, (int)dollar) != NULL) {
            /* if more $s in the value */
            value = variableInterpolationRecur(value);
            subLen = strlen(value);
            /* to be freed later since variableInterpolationRecur
             * returns a new string to be freed later.
             */
            freeNewValue = 1;

          }  else {
            /* if no dollars in the value, substitute the return value
             * in the string
             */
            subLen = strlen(value);
          }
        } else { 
          /* if the variable is not found, it might be an
           * environment variable and so keep it. This might be
           * a hazard for bogus variables but that is upto the user.
           */
          value = subStr;
          /* for environment variable keep the $ sign */
          subLen = strlen(value) +1;
          envVar = 1;
        }

      } /* end of interpolation variable not trivial */
      /* prefix + strlen(substituted value) + suffix */
      newCurStr = ALLOC(char, dollarIndex + 
                        subLen +
                        strlen(curStr) - endDollarIndex + 1);


      /* copy prefix */
      newCurStr = strncpy(newCurStr, curStr, dollarIndex);
      i = dollarIndex;
      if (subLen) {
        /* copy substituted value */
        if (envVar) {
          /* if it is an environment variable, keep the $ sign */
          newCurStr[i++] = '$';
        }
        index = 0;
        while (value[index] != '\0') {
          newCurStr[i++] = value[index++];
        }
        if (freeNewValue) {
          FREE(value);
        }
      }
      /* freed here cos value might be subStr in one case */
      if (subStr != NULL) {
        FREE(subStr);
      }
      /* copy suffix */
      index = endDollarIndex;
      /* figure out where to start the next search */
      lastPos = i;
      while (curStr[index] != '\0') {
        newCurStr[i++] = curStr[index++];
      }
      newCurStr[i] = '\0';
      FREE(curStr);
      curStr = newCurStr;
      /* reset counter for further search. Due to recursive calling of this
       * function eventually, the value that is substituted will not have a $
       */
      curStrIndex = lastPos;
      dollarIndex = -1;
      endDollarIndex = -1;
      /* end of found a variable */   
    } else { /* if a variable is not found, keep going */
      curStrIndex++;
    }
  } /* end of stepping through the string */
  return(curStr);
} /* end of variableInterpolationRecur */



/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static char *
split_line(
  char * command,
  int * argc,
  char *** argv)
{
  register char *p, *start, c;
  register int i, j;
  register char *new_arg;
  array_t *argv_array;
  boolean single_quote, double_quote;

  argv_array = array_alloc(char *, 5);

  p = command;
  while (true) {
    /* skip leading white space */
    while (isspace(*p)) p++;

    /* skip until end of this token */
    single_quote = double_quote = false;

    for (start = p; (c = *p) != '\0'; p++) {
      if (c == ';' || c == '#' || isspace(c)) {
        if (!single_quote && !double_quote) break;
      }

      if (c == '\'') single_quote = !single_quote;
      else if (c == '"') double_quote = !double_quote;
    }
    if (single_quote || double_quote) {
      fprintf(nusmv_stderr, "ignoring unbalanced quote ...\n");
    }
    if (start == p) break;

    new_arg = ALLOC(char, p - start + 1);
    nusmv_assert(new_arg != (char*) NULL);

    single_quote=double_quote=false;
    j = 0;
    for (i = 0; i < p - start; i++) {
      c = start[i];

      if (c == '\'' && !double_quote) single_quote = !single_quote;
      else if (c=='"' && !single_quote) double_quote = !double_quote;

      if ((c != '\'' || double_quote) && (c != '\"' || single_quote)) {
        new_arg[j++] = isspace(c) ? ' ' : c;
      }
    }
    new_arg[j] = '\0';
    array_insert_last(char *, argv_array, new_arg);
  }

  *argc = array_n(argv_array);
  *argv = array_data(char *, argv_array);
  array_free(argv_array);
  if (*p == ';') {
    p++;
  }
  else if (*p == '#') {
    for(; *p != 0; p++) ;		/* skip to end of line */
  }
  return p;
}    


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static int check_shell_escape(char* p, int* status)
{
#if NUSMV_HAVE_SYSTEM
  OptsHandler_ptr opt = OptsHandler_get_instance();
  while (isspace(*p)) p++;

  if (OptsHandler_is_option_registered(opt, "shell_char")) {
    const char* tmp = OptsHandler_get_string_option_value(opt, "shell_char");
    NuSMVShellChar = tmp[0];
  }
  if (*p == NuSMVShellChar) {
    *status = system(p+1);
    return 1;
  }
#else
#warning "Shell escape is not supported"
#endif
  return 0;
}


#if NUSMV_HAVE_SIGNAL_H
/**Function********************************************************************

  Synopsis    [Signal handler.]

  SideEffects []

  SeeAlso     [com_dispatch]

******************************************************************************/
static void
sigterm(int sig)
{
  fprintf(nusmv_stdout, "Interrupt\n");
  if (!opt_batch(OptsHandler_get_instance()) && !cmd_is_curr_reentrant()) {
    fprintf(nusmv_stderr, 
            "Warning: %s status may be not consistent. Use 'reset' "\
            "command if needed.\n", get_pgm_name(OptsHandler_get_instance()));
  }

  (void) signal(sig, sigterm);
  util_longjmp();
}
#endif /* NUSMV_HAVE_SIGNAL_H */


/**Function********************************************************************

  Synopsis [Enable signal trapping depending on the interactive/batch
  mode.]

  SideEffects []

  SeeAlso     [com_dispatch]

******************************************************************************/
static void arm_signal_andler() 
{
  if (! opt_batch(OptsHandler_get_instance())) { /* interactive mode */
#if NUSMV_HAVE_SIGNAL_H
    (void) signal(SIGINT, sigterm);
#endif
  }
}

/**Function********************************************************************

  Synopsis [Enable signal trapping depending on the interactive/batch
  mode.]

  SideEffects []

  SeeAlso     [com_dispatch]

******************************************************************************/
static void disarm_signal_andler() 
{
  if (! opt_batch(OptsHandler_get_instance())) { /* interactive mode */
#if NUSMV_HAVE_SIGNAL_H
    (void) signal(SIGINT, SIG_IGN);
#endif
  }
}
