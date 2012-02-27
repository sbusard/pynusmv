/**CFile***********************************************************************

  FileName    [parserUtil.c]

  PackageName [parser]

  Synopsis    [Parser utilities]

  Description [This file contains some parser utilities that allows
  for example to parse from a string, instead that from a file.]

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``parser'' package of NuSMV version 2.
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

#include "parserInt.h"
#include "symbols.h"

#include "psl/pslInt.h"

#include "cinit/cinit.h"
#include "utils/error.h"
#include "utils/ustring.h"


static char rcsid[] UTIL_UNUSED = "$Id: parserUtil.c,v 1.7.4.6.4.14.4.16 2010-02-02 10:09:34 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Macro definitions                                                         */
/*---------------------------------------------------------------------------*/

#define YY_BUF_SIZE 16384
#define YY_CURRENT_BUFFER yy_current_buffer
#define YY_END_OF_BUFFER_CHAR 0

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

static char* tmpfname1 = (char*) NULL;
static char* tmpfname2 = (char*) NULL;

/* the current buffer of the lexer - only one buffer can exist at any time*/
static YY_BUFFER_STATE yy_current_buffer = NULL;

static node_ptr parsed_errors = Nil; /* the list of found errors */
static boolean errors_reversed = false;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void parser_open_input_pp ARGS((const char* filename));
static void parser_close_input_pp ARGS((void));


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Open a file and inform the parser to read from it]

  Description        [Open a file and inform the parser to start
  reading tokens from this file. If no input file is provided, then it
  inform the parser to start reading tokens from the standard input.
  Invoke Parser_CloseInput to close the file and associated buffer. ]

  SideEffects        []

  SeeAlso            [Parser_CloseInput]

******************************************************************************/
extern FILE* psl_yyin;
void Parser_OpenInput(const char *filename)
{
  if (filename != (char*) NULL) {
    yyin = fopen(filename,"r");
    if (yyin == (FILE*) NULL) rpterr("cannot open input file %s",filename);
    yylineno = 1;
  }
  else {
    yyin = stdin;
    yylineno = 0;
  }

  psl_yyin = yyin;

  /* buffer has not been initialized before: */
  nusmv_assert(NULL == YY_CURRENT_BUFFER);
  YY_CURRENT_BUFFER = yy_create_buffer(yyin, YY_BUF_SIZE);
  /* Flushes the current input buffer */
  (void) yy_switch_to_buffer(YY_CURRENT_BUFFER);

  (void) yyrestart(yyin);
}


/**Function********************************************************************

  Synopsis           [Close the input file]

  Description        [Closes the input file and corresponding buffer used
  by the parser to read tokens.
  NB: This function should be invoked only after successive invocation
  of parser_open_input_pp.]

  SideEffects        []

  SeeAlso            [Parser_OpenInput]

******************************************************************************/
void Parser_CloseInput()
{
  nusmv_assert(NULL != YY_CURRENT_BUFFER);/* buffer should be initialized before */
  yy_delete_buffer(YY_CURRENT_BUFFER);
  YY_CURRENT_BUFFER = NULL;

  if (stdin != yyin) fclose(yyin);
}

void Parser_switch_to_psl()
{
  psl_yyrestart(psl_yyin);
}


void Parser_switch_to_smv()
{
  yyrestart(yyin);
}


/**Function********************************************************************

  Synopsis           [Parse SMV code from a given file.]

  Description        [Parse SMV code from a given file. If
  no file is provided, parse from stdin. If a parsing error occurs then
  return 1, else return 0. The result of parsing is stored in
  the global variable <tt>parsed_tree</tt> to be used from the caller.]

  SideEffects        []

  SeeAlso            []

*****************************************************************************/
int Parser_ReadSMVFromFile(const char *filename)
{
  int retval = 0;
  if (strcmp(get_pp_list(OptsHandler_get_instance()), "") != 0) parser_open_input_pp(filename);
  else Parser_OpenInput(filename);

  parsed_tree = Nil;
  parser_free_parsed_syntax_errors();
  parse_mode_flag = PARSE_MODULES;

  CATCH {
    if (yyparse()) retval = 1;
    else {
      yylineno = 0;
    }
  }
  FAIL {
    if (strcmp(get_pp_list(OptsHandler_get_instance()), "") != 0) parser_close_input_pp();
    else Parser_CloseInput();
    rpterr("Parser error");
  }

  if (strcmp(get_pp_list(OptsHandler_get_instance()), "") != 0) parser_close_input_pp();
  else Parser_CloseInput();

  return retval;
}


/**Function********************************************************************

  Synopsis           [Parse a comand from a given string.]

  Description        [Create a string for a command, and then call
  <tt>yyparse</tt> to read from the created string.
  If a parsing error occurs than return 1, else return 0.
  The result of parsing is stored in <tt>pc</tt> to be used from the caller.]

  SideEffects        []

  SeeAlso            []

*****************************************************************************/
int Parser_ReadCmdFromString(int argc, const char** argv,
                             const char* head, const char* tail,
                             node_ptr* pc)
{
  int i;
  char * old_input_file;
  int l = strlen(head);
  int status = 0;
  char* cmd = NIL(char);
  char* cmd1 = NIL(char);
  YY_BUFFER_STATE buf;

  for (i = 1; i < argc; i++) l += strlen(argv[i]) + 1;
  l += strlen(tail) +2+1; /* 2 is for last two YY_END_OF_BUFFER_CHAR */
  cmd = ALLOC(char, l);
  cmd1 = ALLOC(char, l);
  nusmv_assert(cmd != NIL(char));
  nusmv_assert(cmd1 != NIL(char));

  sprintf(cmd, "%s", head);
  for (i = 1; i < argc; i++) {
    sprintf(cmd1, "%s%s ", cmd, argv[i]);
    strcpy(cmd, cmd1);
  }
  sprintf(cmd1, "%s%s%c%c", cmd, tail,
          YY_END_OF_BUFFER_CHAR, YY_END_OF_BUFFER_CHAR);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3))
    fprintf(nusmv_stderr, "%s\n", cmd1);

  /* Get a local copy of the old input file. */
  old_input_file = get_input_file(OptsHandler_get_instance());
  if (NIL(char) != old_input_file) {
    old_input_file = util_strsav(old_input_file);
  }
  set_input_file(OptsHandler_get_instance(), "<command-line>");

  parsed_tree = Nil;
  parser_free_parsed_syntax_errors();
  parse_mode_flag = PARSE_COMMAND;
  buf = yy_scan_buffer(cmd1, l-1);
  nusmv_assert(buf != (YY_BUFFER_STATE) NULL);

  status = (yyparse() != 0);

  yy_delete_buffer(buf); /* frees the buffer */
  FREE(cmd);
  FREE(cmd1);

  /* We need to reset the input buffer */
  //yyrestart(yyin);

  set_input_file(OptsHandler_get_instance(), old_input_file);

  /* Free the local copy of the input file */
  if (NIL(char) != old_input_file) {
    FREE(old_input_file);
  }

  *pc = parsed_tree;
  return(status);
}


/**Function********************************************************************

  Synopsis           [Parse a command expression from file]

  Description [ The resulting parse tree is returned through res. If a
  parsing error occurs then return 1, else return 0. ]

  SideEffects        []

  SeeAlso            []

*****************************************************************************/
int Parser_ReadCmdFromFile(const char *filename, node_ptr* res)
{
  int status;
  char * old_input_file;

  parsed_tree = Nil;
  parser_free_parsed_syntax_errors();
  parse_mode_flag = PARSE_COMMAND;

  old_input_file = get_input_file(OptsHandler_get_instance());
  if (NIL(char) != old_input_file) {
    old_input_file = util_strsav(old_input_file);
  }
  set_input_file(OptsHandler_get_instance(),
                 (char*)(NULL == filename ? "<stdin>" : filename));

  Parser_OpenInput(filename);
  status = yyparse();
  Parser_CloseInput();

  *res = parsed_tree;

  set_input_file(OptsHandler_get_instance(), old_input_file);
  if (NIL(char) != old_input_file) {
    FREE(old_input_file);
  }

  return status;
}


/**Function********************************************************************

  Synopsis           [Parse a next expression from file]

  Description [ The resulting parse tree is returned through res. If a
  parsing error occurs then return 1, else return 0. ]

  SideEffects        []

  SeeAlso            []

*****************************************************************************/
int Parser_ReadNextExprFromFile(const char *filename, node_ptr* res)
{
  return Parser_ReadCmdFromFile(filename, res);
}


/**Function********************************************************************

  Synopsis           [Parse a simple expression from string]

  Description [ The resulting parse tree is returned through res. If a
  parsing error occurs then return 1, else return 0. ]

  SideEffects        []

  SeeAlso            []

*****************************************************************************/
int Parser_ReadSimpExprFromString(const char* str_expr, node_ptr* res)
{
  const char* argv[2];
  const char* head = "SIMPWFF ";
  const char* tail =  ";\n";

  /* prepare argv for parsing */
  argv[0] = (const char*) NULL;
  argv[1] = str_expr;

  *res = Nil;
  return Parser_ReadCmdFromString(sizeof(argv)/sizeof(argv[0]), argv,
                                  head, tail, res);
}


/**Function********************************************************************

  Synopsis           [Parse a next expression from string]

  Description [ The resulting parse tree is returned through res. If a
  parsing error occurs then return 1, else return 0. ]

  SideEffects        []

  SeeAlso            []

*****************************************************************************/
int Parser_ReadNextExprFromString(const char* str_expr, node_ptr* res)
{
  const char* argv[2];
  const char* head = "NEXTWFF ";
  const char* tail =  ";\n";

  /* prepare argv for parsing */
  argv[0] = (const char*) NULL;
  argv[1] = str_expr;

  *res = Nil;
  return Parser_ReadCmdFromString(sizeof(argv)/sizeof(argv[0]), argv,
                                  head, tail, res);
}


/**Function********************************************************************

  Synopsis           [Parse an identifier expression from string]

  Description [ The resulting parse tree is returned through res. If a
  parsing error occurs then return 1, else return 0. ]

  SideEffects        []

  SeeAlso            []

*****************************************************************************/
int Parser_ReadIdentifierExprFromString(const char* str_expr, node_ptr* res)
{
  const char* argv[2];
  const char* head = "COMPID ";
  const char* tail =  ";\n";

  /* prepare argv for parsing */
  argv[0] = (const char*) NULL;
  argv[1] = str_expr;

  *res = Nil;
  return Parser_ReadCmdFromString(sizeof(argv)/sizeof(argv[0]), argv,
                                  head, tail, res);
}


/**Function********************************************************************

  Synopsis           [Parse LTL expression from a given file.]

  Description        [Parse SMV code from a given file. If
  no file is provided, parse from stdin. If a parsing error occurs then
  return 1, else return 0. The result of parsing is stored in
  the global variable <tt>parsed_tree</tt> to be used from the caller.]

  SideEffects        []

  SeeAlso            []

*****************************************************************************/
int Parser_ReadLtlExprFromFile(const char *filename)
{
  int retval;
  char * old_input_file;

  parsed_tree = Nil;
  parser_free_parsed_syntax_errors();
  parse_mode_flag = PARSE_LTL_EXPR;

  old_input_file = get_input_file(OptsHandler_get_instance());
  if (NIL(char) != old_input_file) {
    old_input_file = util_strsav(old_input_file);
  }

  set_input_file(OptsHandler_get_instance(),
                 (char*)(NULL == filename ? "<stdin>" : filename));

  Parser_OpenInput(filename);
  retval = yyparse();
  Parser_CloseInput();

  set_input_file(OptsHandler_get_instance(), old_input_file);

  if (NIL(char) != old_input_file) {
    FREE(old_input_file);
  }

  return retval;
}


/**Function********************************************************************

  Synopsis           [Parses a PSL expression from the given string.]

  Description        [The PSL parser is directly called. The resulting
  parse tree is returned through res. 1 is returned if an error occurred.]

  SideEffects        []

  SeeAlso            []

*****************************************************************************/
int Parser_read_psl_from_string(int argc, const char** argv, node_ptr* res)
{
  /* Invokes the PSL parser directly */
  char* cmd;
  char* cmd1;
  int len = 0;
  int i;
  int status = 0;
  YY_BUFFER_STATE buf;

  *res = Nil;

  for (i=0; i < argc; ++i) {
    if (argv[i] != (char*) NULL) len += strlen(argv[i]) + 1;
  }

  len += 1+2+1;  /* semicolon, last two YY_END_OF_BUFFER_CHAR, terminator */
  cmd = ALLOC(char, len);
  cmd1 = ALLOC(char, len);

  cmd[0] = '\0';
  for (i=0; i < argc; ++i) {
    if (argv[i] != (char*) NULL) {
      strcat(cmd, argv[i]);
      strcat(cmd, " ");
    }
  }
  sprintf(cmd1, "%s;%c%c", cmd, YY_END_OF_BUFFER_CHAR, YY_END_OF_BUFFER_CHAR);

  psl_parsed_tree = Nil;
  parser_free_parsed_syntax_errors();

  psl_property_name = Nil;
  buf = psl_yy_scan_buffer(cmd1, len-1);
  nusmv_assert(buf != (YY_BUFFER_STATE) NULL);
  status = (psl_yyparse() != 0);
  psl_yy_delete_buffer(buf); /* frees the buffer */

  FREE(cmd);
  FREE(cmd1);

  /* We need to reset the input buffer */
  //psl_yyrestart(psl_yyin);
  *res = psl_parsed_tree;
  return status;
}



/**Function********************************************************************

  Synopsis           [Parses a PSL expression from the given file.]

  Description        [The PSL parser is directly called. The resulting
  parse tree is returned through res. 1 is returned if an error occurred.]

  SideEffects        []

  SeeAlso            []

*****************************************************************************/
int Parser_read_psl_from_file(const char* filename, node_ptr* res)
{
  int status = 1; /* by default there is a problem */
  char * old_input_file;

  psl_parsed_tree = Nil;
  parser_free_parsed_syntax_errors();

  psl_property_name = Nil;

  /* open the file */
  nusmv_assert(filename != (char*) NULL);

  psl_yyin = fopen(filename,"r");
  if (psl_yyin == (FILE*) NULL) rpterr("cannot open input file %s",filename);
  yylineno = 1;
  psl_yylineno = 1;

  old_input_file = get_input_file(OptsHandler_get_instance());
  if (NIL(char) != old_input_file) {
    old_input_file = util_strsav(old_input_file);
  }

  set_input_file(OptsHandler_get_instance(), (char*)filename);

  /* buffer has not been initialized before: */
  nusmv_assert(NULL == YY_CURRENT_BUFFER);
  YY_CURRENT_BUFFER = psl_yy_create_buffer(psl_yyin, YY_BUF_SIZE);
  /* Flushes the current input buffer */
  (void) psl_yy_switch_to_buffer(YY_CURRENT_BUFFER);

  (void) psl_yyrestart(psl_yyin);

  CATCH { /* catch errors in parsing */
    /* parse the file */
    status = psl_yyparse();

    /* parser does not check EndOfFile. check it here. Note: this
       solution may be not perfect because the buffer could have read one
       last token and not return it to the parser and we will not see it. */
    if (!status && !feof(psl_yyin)) {
      const size_t buf_size = 50;
      char buf[buf_size]; /* output just buf_size-1 bytes */
      size_t n;
      yylineno = psl_yylineno;
      n = fread(buf, 1, buf_size-1, psl_yyin);
      buf[n] = '\0';
      rpterr("unexpected text left after parsing done : \"%s\".", buf);
    }
  }
  FAIL {
    /* Do not exit now. Do cleaning at first */
    status = 1;
  }

  /* close the file */
  nusmv_assert(NULL != YY_CURRENT_BUFFER);/* buffer is to be initialized before */
  psl_yy_delete_buffer(YY_CURRENT_BUFFER);
  YY_CURRENT_BUFFER = NULL;

  fclose(psl_yyin);

  set_input_file(OptsHandler_get_instance(), old_input_file);

  if (NIL(char) != old_input_file) {
    FREE(old_input_file);
  }

  if (!status) *res = psl_parsed_tree;

  return status;
}


/**Function********************************************************************

  Synopsis           [Returns a list of SYNTAX_ERROR nodes]

  Description [Each node of the list can be passed to
  Parser_get_syntax_error to get information out of it. The
  returned lists must be NOT modified or freed by the caller.]

  SideEffects        []

  SeeAlso            [Parser_get_syntax_error]

******************************************************************************/
node_ptr Parser_get_syntax_errors_list()
{
  if (!errors_reversed) {
    parsed_errors = reverse(parsed_errors);
    errors_reversed = true;
  }
  return parsed_errors;
}


/**Function********************************************************************

  Synopsis [Returns information out of nodes contained in list
  returned by Parser_get_syntax_errors_list.]

  Description [Each node contains information which will be set in
  output params filename, lineno and message. Those information
  must be NOT modified or freed by the caller. If not interested in
  an information, pass NULL with the respective parameter.]

  SideEffects        []

  SeeAlso            [Parser_get_syntax_errors_list]

******************************************************************************/
void Parser_get_syntax_error(node_ptr node, 
                             const char** out_filename, 
                             int* out_lineno, 
                             const char** out_token,
                             const char** out_message)
{
  nusmv_assert(Nil != node);
  nusmv_assert(SYNTAX_ERROR == node_get_type(node));

  if ((const char**) NULL != out_filename) {
    nusmv_assert(COLON == node_get_type(car(node)));
    *out_filename = get_text((string_ptr) caar(node));
  }

  if ((int*) NULL != out_lineno) {
    nusmv_assert(COLON == node_get_type(car(node)));
    *out_lineno = PTR_TO_INT(cdar(node));
  }

  if ((const char**) NULL != out_token) {
    nusmv_assert(COLON == node_get_type(cdr(node)));
    *out_token = (const char*) cadr(node);
  }

  if ((const char**) NULL != out_message) {
    nusmv_assert(COLON == node_get_type(cdr(node)));
    *out_message = (const char*) cddr(node);
  }
}


/**Function********************************************************************

  Synopsis [Prints information contained in one node ot the list
  returned by Parser_get_syntax_errors_list.]

  Description [The syntax error information contained in the given
  node is printed to the given output file.]

  SideEffects        []

  SeeAlso            [Parser_get_syntax_errors_list]

******************************************************************************/
void Parser_print_syntax_error(node_ptr error, FILE* fout)
{
  const char* fname;
  const char* token;
  const char* msg;
  int lineno;

  Parser_get_syntax_error(error, &fname, &lineno, &token, &msg);
  if ((char*) NULL != fname) {
    fprintf(fout, "file %s: ", fname);
  }
  else {
    fprintf(fout, "file stdin: ");
  }

  fprintf(fout, "line %d: ", lineno);

  if ((const char*) NULL != token) {
    fprintf(fout, "at token \"%s\"", token);
  }
  fprintf(fout, ": %s\n", msg);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Add a new syntax error to the list]

  Description        [This is called by the parser when needed]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void parser_add_syntax_error(const char* fname, int lineno, 
                             const char* token, const char* err_msg)
{
  node_ptr err = new_node(SYNTAX_ERROR,
                          new_node(COLON, 
                                   (node_ptr) find_string((char*) fname),
                                   NODE_FROM_INT(lineno)),
                          new_node(COLON, 
                                   (node_ptr) util_strsav((char*) token),
                                   (node_ptr) util_strsav((char*) err_msg)));
  parsed_errors = cons(err, parsed_errors);
}


/**Function********************************************************************

  Synopsis [Frees the list of structures containing the syntax
  errors built by the parser. ]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void parser_free_parsed_syntax_errors() 
{
  node_ptr iter = parsed_errors;

  while (iter != Nil) {
    node_ptr se;
    char* str;

    nusmv_assert(CONS == node_get_type(iter));

    se = car(iter);
    nusmv_assert(SYNTAX_ERROR == node_get_type(se));
    nusmv_assert(COLON == node_get_type(car(se)));
    nusmv_assert(COLON == node_get_type(cdr(se)));

    free_node(car(se)); /* COLON fname and lineno */

    /*token string*/
    str = (char*) cadr(se); if ((char*) NULL != str) FREE(str); 

    /*message string*/
    str = (char*) cddr(se); if ((char*) NULL != str) FREE(str); 

    free_node(cdr(se)); /* COLON token and message */

    free_node(se); /* SYNTAX_ERROR */

    { /* frees the list node */
      node_ptr tmp = iter;
      iter=cdr(iter);
      free_node(tmp);
    }
  }
  parsed_errors = Nil;
  errors_reversed = false;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Open a file and inform the parser to read from it]

  Description        [Open a file, pre-process it, and inform the parser to
  start reading tokens from this file. The directory in which the original file
  resides is used to store the temporary files. This is so that any #includes
  statements used by the second or later pre-processor work properly.]

  SideEffects        [Creates temporary files which are subsequently deleted.]

  SeeAlso            []

******************************************************************************/
static void parser_open_input_pp(const char* filename)
{
#if NUSMV_HAVE_POPEN
  const char* STDIN_OPT_NAME = " -";
  char c;

  char* pp_curr;
  char* pp_next;
  char* pp_cmd;
  char* pp_exec;

  char* path;

  int tmp_len;

  FILE* tempstream1;
  FILE* tempstream2;

  char* pp_list = get_pp_list(OptsHandler_get_instance());
  char* pp_list_copy;

  pp_list_copy = ALLOC(char, strlen(pp_list) + 1);
  nusmv_assert(pp_list_copy != NULL);
  pp_list_copy[0] = '\0';

  /* Set tmpfname1 to initial file to use as input */
  if (tmpfname1 != (char*) NULL) FREE(tmpfname1);

  if (filename != (char*) NULL) {
    tmpfname1 = ALLOC(char, strlen(filename) + 1);
    nusmv_assert(tmpfname1 != (char*) NULL);
    strcpy(tmpfname1, filename);

    /* Get path to initial file */
    tmp_len = strlen(filename) - strlen(Utils_StripPath(filename));
    path = ALLOC(char, tmp_len + 1);
    strncpy(path, filename, tmp_len);
    path[tmp_len] = '\0';
  }
  else {
    tmpfname1 = (char*) NULL;

    path = ALLOC(char, 1);
    path[0] = '\0';
  }

  strcpy(pp_list_copy, pp_list);

  pp_curr = strtok(pp_list_copy, " ");
  pp_next = strtok(NULL, " ");

  /* Need to indicate that tmpfname1 refers to the original input file
     so that it does not get deleted when Parser_ClosePP() is called.
     This is done by making tmpfname2 not NULL */
  if (pp_next == NULL) {
    if (tmpfname2 != (char*) NULL) FREE(tmpfname2);
    tmpfname2 = ALLOC(char, 2);
    nusmv_assert(tmpfname2 != (char*) NULL);
    strcpy(tmpfname2, "x");
  }

  while (pp_next != NULL) {
    if (tmpfname1 != (char*) NULL) {
      /* Test whether it is possible to open the file */
      if(!(tempstream1 = fopen(tmpfname1, "r"))) {
        rpterr("cannot open input file %s",tmpfname1);
      }
      (void) fclose(tempstream1);
    }

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stdout, "Calling %s preprocessor\n", pp_curr);
    }
    pp_exec = get_preprocessor_call(pp_curr);
    if (pp_exec == (char*) NULL) {
      error_unknown_preprocessor(pp_curr);
    }

    if (tmpfname1 != (char*)NULL) {
      pp_cmd = ALLOC(char, strlen(pp_exec) + strlen(tmpfname1) + 2);
    }
    else {
      pp_cmd = ALLOC(char, strlen(pp_exec) + strlen(STDIN_OPT_NAME) + 1);
    }
    nusmv_assert(pp_cmd != NIL(char));

    strcpy(pp_cmd, pp_exec);
    if (tmpfname1 != (char*)NULL) {
      strcat(pp_cmd, " ");
      strcat(pp_cmd, tmpfname1);
    }
    else { strcat(pp_cmd, STDIN_OPT_NAME); }
    yylineno = 1;

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
      fprintf(nusmv_stderr, "\nInvoking the command: '%s'...\n", pp_cmd);
    }
    /* Invoke command */

    if ( !(tempstream1 = popen(pp_cmd, "r")) ) {
      rpterr("error executing command \"%s\"", pp_cmd);
    }

    /* Get unique temporary filename */
    if (tmpfname2 != (char*) NULL) FREE(tmpfname2);
    tmpfname2 = Utils_get_temp_filename_in_dir(path, "NuSMVXXXXXX");
    if (tmpfname2 == NULL) {
      rpterr("Unable to generate unique temporary file. (Previously generated temporary file: %s)\n", tmpfname1);
    }

    tempstream2 = fopen(tmpfname2, "w");
    if(!tempstream2) rpterr("cannot open input file %s",tmpfname2);

    while (feof(tempstream1) == 0) {
      c = getc(tempstream1);
      if (c != EOF) {putc(c, tempstream2);}
    }
    (void) pclose(tempstream1);
    (void) fclose(tempstream2);

    if (((char*)NULL == filename || strcmp(tmpfname1, filename) != 0) &&
        (tempstream1 = fopen(tmpfname1,"r"))) {
      (void) fclose(tempstream1);
      if (remove(tmpfname1) == -1) {
        rpterr("error deleting temporary file \"%s\"", tmpfname1);
      }
    }

    if (tmpfname1 != (char*) NULL) FREE(tmpfname1);
    tmpfname1 = ALLOC(char, strlen(tmpfname2) + 1);
    nusmv_assert(tmpfname1 != (char*) NULL);
    strcpy(tmpfname1, tmpfname2);

    FREE(tmpfname2); tmpfname2 = (char*) NULL;
    FREE(pp_cmd);

    pp_curr = pp_next;
    pp_next = strtok(NULL, " ");
  } /* End of while (pp_list != NULL) */

  pp_exec = get_preprocessor_call(pp_curr);
  if (pp_exec == (char*) NULL) {
    error_unknown_preprocessor(pp_curr);
  }

  /* Set yyin to result of running last pre-processor */
  if (tmpfname1 != (char*) NULL) {
    /* Test whether it is possible to open the file */
    if(!(tempstream1 = fopen(tmpfname1, "r"))) {
      rpterr("cannot open input file %s",tmpfname1);
    }
    (void) fclose(tempstream1);

    pp_cmd = ALLOC(char, strlen(pp_exec) + strlen(tmpfname1) + 2);
    strcpy(pp_cmd, pp_exec);
    strcat(pp_cmd, " ");
    strcat(pp_cmd, tmpfname1);
  }
  else {
    pp_cmd = ALLOC(char, strlen(STDIN_OPT_NAME) + 1);
    strcpy(pp_cmd, pp_exec);
    strcat(pp_cmd, STDIN_OPT_NAME);
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stdout, "Calling %s preprocessor\n", pp_curr);
  }
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "\nInvoking the command: '%s'...\n", pp_cmd);
  }
  if( !(yyin = popen(pp_cmd, "r")) ) {
    FREE(pp_list_copy);
    rpterr("error executing command \"%s\"", pp_cmd);
  }

  FREE(pp_cmd);
  FREE(pp_list_copy);

  psl_yyin = yyin;

  /* Flushes the current input buffer */
  (void) yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  (void) yyrestart(yyin);

#endif /* NUSMV_HAVE_POPEN */
}


/**Function********************************************************************

  Synopsis           [Close the input file]

  Description        [Closes the input file used from parser to read tokens.]

  SideEffects        [Deletes any temporary files created by
  parser_open_input_pp.]

  SeeAlso            []

******************************************************************************/
static void parser_close_input_pp()
{
#if NUSMV_HAVE_POPEN
  FILE* stream;

  /* tmpfname refers to original input file */
  if (tmpfname2 != NULL) FREE(tmpfname2);
  else {
    stream = fopen(tmpfname1,"r");
    if (stream != (FILE*) NULL) {
      (void) fclose(stream);
      if (remove(tmpfname1) == -1) {
        rpterr("error deleting file \"%s\"", tmpfname1);
      }
    }
  }

  FREE(tmpfname1); tmpfname1 = (char*) NULL;

  (void) pclose(yyin);
#endif
}


