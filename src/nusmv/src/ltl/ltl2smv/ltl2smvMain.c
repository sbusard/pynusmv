/**CFile***********************************************************************

  FileName    [main.c]

  PackageName [ltl2smv]

  Synopsis    [The LTL to SMV Translator]

  Description [This file contains the main function
  which invokes a routine for reducing LTL model
  checking to CTL model checking. see file ltl2smv.h]

  SeeAlso     [ltl2smv.h]

  Author      [Adapted to NuSMV by Marco Roveri. 
               Extended to the past operators by Ariel Fuxman.
	       Addopted to use NuSMV library by Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``ltl2smv'' package of NuSMV version 2. 
  Copyright (C) 1998-2005 by CMU and FBK-irst. 

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
#include "ltl2smv.h"

#include "utils/error.h"
#include "cinit/cinit.h"
#include "compile/compile.h"
#include "parser/parser.h"
#include "parser/symbols.h"
#include "utils/utils.h"
#include "utils/ustring.h"
#include <stdio.h>

static void ltl2smv_print_module ARGS((FILE* ostream, node_ptr));

int main(int argc, char **argv)
{
  unsigned int uniqueId;
  node_ptr in_ltl_expr;
  FILE* output_file;

  /* check the number of arguments */
  if ((argc > 4) || (argc < 3)) {
    fprintf(stderr, "%s: Converts an LTL formula to a fragment of an "
	    "SMV program.\n", argv[0]);
    fprintf(stderr, "%s: %s # <ifile> [<ofile>]\n", argv[0], argv[0]);
    fprintf(stderr, "Where:\n\t#\t is a positive number, "
            "which is converted to a string\n"
	    "\t\t and then used as a part of the generated SMV model\n"
	    "\t\t name _LTL#_SPECF_N_.\n");
    fprintf(stderr, "\t<ifile>\t is the file from which the LTL Formula "
	    "to be translated\n\t\t is read.\n");
    fprintf(stderr, "\t<ofile>\t is the file in which the SMV code "
	    "corresponding to the\n\t\t tableau of LTL Formula is "
	    "written in.\n\t\t If not specified than stdout is used.\n");
    exit(1);
  }

  /* initialise all the packages even though they are not required */
  NuSMVCore_init_data();
  NuSMVCore_init(NULL, 0);
  
  /* check the first argument -- a zero or positive number */
  {
    char* err_occ = "";
    long int r = strtol(argv[1], &err_occ, 10);
    if (strcmp(err_occ, "") != 0) {
      fprintf(stderr, "Error: \"%s\" is not a natural number.\n", err_occ);
      exit(1);
    }
    if (r < 0) {
      fprintf(stderr, "Error: \"%ld\" is not a positive number.\n", r);
      exit(1);
    }
    if (r != (unsigned int)r) {
      fprintf(stderr, "Error: \"%ld\" is a too large number.\n", r);
      exit(1);
    }
    uniqueId = (unsigned int)r;
  }

  /* open and parse from the input file */
  {
    extern node_ptr parsed_tree; /* the result of parsing */

    if ((char*)NULL == argv[2]) {
      fprintf(stderr, "Error: ltl2smv : the input file is not specified.\n");
      exit(1);
    }
    
    if (Parser_ReadLtlExprFromFile(argv[2])) {
      fprintf(stderr, "Paring Error in LTL expression.\n");
      exit(1);
    }
    in_ltl_expr = parsed_tree; /* the result of parsing */
  }

  /* open the output file */
  {
    if (NULL != argv[3]) {
      output_file = fopen(argv[3], "w");
      if (output_file == (FILE *)NULL) {
	fprintf(stderr, "Error: Unable to open file \"%s\" for writing.\n",
		argv[3]);
	exit(1);
      }
    } else {
      output_file = stdout;
    }
  }

  /* transform the expression and print it out */
  {
    node_ptr out_smv_module = ltl2smv(uniqueId, 
				      Compile_pop_distrib_ops(in_ltl_expr));
    
    ltl2smv_print_module(output_file, out_smv_module);

    if (output_file != stdout) fclose(output_file);
  }

  /* do not deinitilise the packages. What for? */
  return 0;
}
    

/* static function which prints a module */
static void ltl2smv_print_module(FILE* ostream, node_ptr module)
{
  node_ptr iter;
  
  nusmv_assert(Nil != module);
  nusmv_assert(MODULE == node_get_type(module));
  /* print the name */
  nusmv_assert(MODTYPE == node_get_type(car(module)));
  nusmv_assert(ATOM == node_get_type(car(car(module))));
  nusmv_assert(Nil == cdr(car(module)));

  fprintf(ostream, "MODULE %s\n", get_text((string_ptr)car(car(car(module)))));

  iter = cdr(module);
  while (Nil != iter) {
    nusmv_assert(CONS == node_get_type(iter));
    switch (node_get_type(car(iter))) {
     
    case VAR: { /* variable declarations */
      node_ptr var;
      var = car(car(iter)); 
      if ( Nil != var) {
	fprintf(ostream, "VAR\n");
	while (Nil != var) { /* iterate over variable declarations */
	  
	  nusmv_assert(CONS == node_get_type(var));
	  nusmv_assert(COLON == node_get_type(car(var)));
	  nusmv_assert(ATOM == node_get_type(car(car(var))));
	  nusmv_assert(BOOLEAN == node_get_type(cdr(car(var))));
	  
	  fprintf(ostream, "   %s : boolean;\n",
		  get_text((string_ptr)car(car(car(var)))));
	  
	  var = cdr(var);
	}
      }
      break;
    }
      
    case DEFINE: { /* define declarations */
      node_ptr def;
      def = car(car(iter)); 
      if ( Nil != def) {
	fprintf(ostream, "DEFINE\n");
	while (Nil != def) { /* iterate over define declarations */
	  
	  nusmv_assert(CONS == node_get_type(def));
	  nusmv_assert(EQDEF == node_get_type(car(def)));
	  
	  fprintf(ostream, "   ");
	  print_node(ostream, car(def));
	  fprintf(ostream, ";\n");
	  
	  def = cdr(def);
	}
      } /* if */
      break;
    }

    case INIT: /* INIT  declarations */
      fprintf(ostream, "INIT\n   ");
      print_node(ostream, car(car(iter)));
      fprintf(ostream, "\n");
      break;

    case TRANS: /* TRANS  declarations */
      fprintf(ostream, "TRANS\n   ");
      print_node(ostream, car(car(iter)));
      fprintf(ostream, "\n");
      break;

    case JUSTICE: /* JUSTICE  declarations */
      fprintf(ostream, "JUSTICE\n   ");
      print_node(ostream, car(car(iter)));
      fprintf(ostream, "\n");
      break;
    default: error_unreachable_code(); /* unexpected node */
    } /*switch */
    
    iter = cdr(iter);
  } /* while */

  return;
}
