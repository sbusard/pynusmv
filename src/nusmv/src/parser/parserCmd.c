/**CFile***********************************************************************

  FileName    [parserCmd.c]

  PackageName [parser]

  Synopsis    [Interface of the parser package with the shell.]

  Description [Provides command for reading the NuSMV input file and
  build an internal representation of it.]

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

#include "parserInt.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: parserCmd.c,v 1.4.6.2.4.4 2006-07-27 06:43:05 nusmv Exp $";

int CommandReadModel(int argc, char **argv);

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int UsageReadModel ARGS((void));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the parser]

  Description        [Initializes the parser]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Parser_Init(void)
{
  /* lib init */
  parser_free_parsed_syntax_errors();

  /* commands */
  Cmd_CommandAdd("read_model", CommandReadModel, 0, true);

  /* options */
  OptsHandler_register_bool_option(OptsHandler_get_instance(),
                                   OPT_PARSER_IS_LAX,
                                   false,
                                   true /*public*/);
}


/**Function********************************************************************

  Synopsis           [Deinitializes the parser]

  Description        [Deinitializes the parser]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Parser_Quit(void)
{
  /* lib quit */
  parser_free_parsed_syntax_errors();
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Reads a NuSMV file into NuSMV.]

  CommandName        [read_model]

  CommandSynopsis    [Reads a NuSMV file into NuSMV.]

  CommandArguments   [\[-h\] \[-i model-file\]]

  CommandDescription [Reads a NuSMV file. If the <tt>-i</tt> option is
  not specified, it reads from the file specified in the environment
  variable <tt>input_file</tt>.<p>
  Command options:<p>
  <dl>
    <dt> <tt>-i model-file</tt>
       <dd> Sets the environment variable <tt>input_file</tt> to
           <tt>model-file</tt>, and reads the model from the specified file.
  </dl>]

  SideEffects        []

******************************************************************************/
int CommandReadModel(int argc, char ** argv)
{
  int c;
  char* i_file = (char*)NULL;
  int res = 1;

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"hi:")) != EOF){
    switch(c){
    case 'i': {
      /* -i already specified */
      if ((char*)NULL != i_file) { goto read_model_usage; }

      i_file = util_strsav(util_optarg);
      break;
    }
    case 'h': goto read_model_usage;
    default: goto read_model_usage;
    }
  }
  if (argc != util_optind) { goto read_model_usage; }

  if (cmp_struct_get_read_model(cmps)) {
    fprintf(nusmv_stderr,
            "A model appears to be already read from file: %s.\n",
            get_input_file(OptsHandler_get_instance()));
    goto read_model_free;
  }

  /* Set the input file if specified using the -i option */
  if ((char*)NULL != i_file) {
    set_input_file(OptsHandler_get_instance(), i_file);
  }

  /* NULL input files are allowed in batch mode (that calls this
     command) when reading from stdin */
  if (get_input_file(OptsHandler_get_instance()) == (char*) NULL &&
      !opt_batch(OptsHandler_get_instance())) {
    fprintf(nusmv_stderr,
            "Input file is (null). You must set the input file before.\n");
    goto read_model_free;
  }

  /* Parse the input file */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Parsing file \"%s\" ..... ",
            get_input_file(OptsHandler_get_instance()));
    fflush(nusmv_stderr);
  }

  if (Parser_ReadSMVFromFile(get_input_file(OptsHandler_get_instance()))) {
    nusmv_exit(1);
  }


  { /* dumps erros if there are any */
    node_ptr errors = Parser_get_syntax_errors_list();
    if (Nil != errors) {
      fprintf(nusmv_stderr, "\n");
      fflush(NULL); /* to flush all existing messages before outputting */

      while (Nil != errors) {
        Parser_print_syntax_error(car(errors), nusmv_stderr);
        errors = cdr(errors);
      }
    }
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "done.\n");
    fflush(nusmv_stderr);
  }

  cmp_struct_set_read_model(cmps);

  /* Everything went smooth */
  res = 0;
  goto read_model_free;

 read_model_usage:
  res = UsageReadModel();

 read_model_free:
  if ((char*)NULL != i_file) {
    FREE(i_file);
  }

  return res;
}

static int UsageReadModel()
{
  fprintf(nusmv_stderr, "usage: read_model [-h] [-i <file>]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -i <file> \tReads the model from the specified <file>.\n");
  return(1);
}
