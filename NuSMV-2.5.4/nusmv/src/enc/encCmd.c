/**CFile***********************************************************************

  FileName    [encCmd.c]

  PackageName [enc]

  Synopsis    [The shell interface of the ENC package]

  Description [Shell interface of the ENC package.]

  SeeAlso     []

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``enc'' package of NuSMV version 2. 
  Copyright (C) 2009 by FBK-IRST. 

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

#include "encInt.h"
#include "prop/Prop.h"
#include "prop/propPkg.h"

#include "cmd/cmd.h" /* for Cmd_CommandAdd */
#include "compile/compile.h" /* for cmp_struct_get_encode_variables */
#include "parser/parser.h"
#include "utils/error.h" /* internal error */ 

#include <math.h> /* for log10 */
#include <stdio.h> /* for fopen */

static char rcsid[] UTIL_UNUSED = "$Id nusmv Exp$";

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int CommandCleanSexp2BDDCache ARGS((int argc, char **argv));
static int UsageCleanSexp2BDDCache ARGS((void));

static int CommandPrintFormula ARGS((int argc, char** argv));
static int UsagePrintFormula ARGS((void));

static int CommandDumpExpr ARGS((int argc, char **argv));
static int UsageDumpExpr ARGS((void));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

void enc_add_commands()
{
  Cmd_CommandAdd("clean_sexp2bdd_cache", CommandCleanSexp2BDDCache, 0, true);
  Cmd_CommandAdd("print_formula", CommandPrintFormula, 0, true);
  Cmd_CommandAdd("dump_expr", CommandDumpExpr, 0, true);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Implements the cleaning BDD cache]

  CommandName        [clean_sexp2bdd_cache]

  CommandSynopsis    [Cleans the cache used during evaluation of 
  expressions to ADD and BDD representations.]  

  CommandArguments   [\[-h\] ]  

  CommandDescription [ During conversion of symbolic (node_ptr)
  expressions to ADD and BDD representations the results of
  evaluations are normally cached (see additionally NuSMV option
  enable_sexp2bdd_caching). This allows to save time by avoid the
  construction of BDD for the same expression several time.
  
  In some situations it  may be preferable to clean the cache and
  free collected ADD and BDD. This operation can be done, for example,
  to free some memory. Another possible reason is that dynamic
  reordering may modify all existing BDDs, and cleaning the cache
  thereby freeing the BDD may speed up the reordering.

  This command is designed specifically to free the internal cache of
  evaluated expressions and their ADDs and BDDs.
  
  Note that only the cache of exp-to-bdd evaluator is freed.  BDDs of
  variables, constants and expressions collected in BDD FSM or
  anywhere else are not touched.]

  SideEffects        []

******************************************************************************/
static int CommandCleanSexp2BDDCache(int argc, char **argv)
{
  BddEnc_ptr enc;
  int c;
  extern cmp_struct_ptr cmps;

  /* Parse the options. */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch (c) {
    case 'h': return(UsageCleanSexp2BDDCache());
    default:
      return(UsageCleanSexp2BDDCache());
    }
  }

  if (argc != util_optind) return(UsageCleanSexp2BDDCache());

  /* pre-conditions: */
  if (!cmp_struct_get_encode_variables(cmps)) {
    fprintf(nusmv_stderr, "ERROR: BDD encoding has to be created before. "
            "Use \"encode_variables\" command.\n\n");
    return UsageCleanSexp2BDDCache();
  }

  enc = Enc_get_bdd_encoding();
  nusmv_assert(enc != NULL);

  BddEnc_clean_evaluation_cache(enc);

  return 0;
}


static int UsageCleanSexp2BDDCache(void)
{
  fprintf(nusmv_stderr, "usage: clean_sexp2bdd_cache [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage\n"
          "The command cleans the cache and frees intermediate BDDs constructed\n"
          "during evaluation of symbolic expressions into ADD and BDD form\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Prints a formula in canonical format.]

  CommandName        [print_formula]

  CommandSynopsis    [Prints a formula]

  CommandArguments   [\[-h\] | \[-v\] | \[-f \] <expression>]

  CommandDescription [In formula mode, the formula as the canonical
                      formula is printed.  In verbose mode, the
                      explicit assignments satisfying the formula are
                      printed. Prints the number of satsfying
                      assignments for the given formula.<p>

  Command Options:
  <dl>
  <dt> <tt>-v</tt>
  <dd> Verbosely prints the list of assignments satisfying the formula.
  <dt> <tt>-f</tt>
  <dd> Prints a canonical representation of input.
  </dl>]
]

  SideEffects        []

******************************************************************************/
static int CommandPrintFormula(int argc, char **argv)
{
  int c;
  boolean verbose = false;
  boolean formula = false;
  char* str_constr = (char*) NULL;
  int parse_result;
  node_ptr constr = Nil;

  /* Parse the command line */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hvf")) != EOF) {
    switch (c) {
    case 'h': return UsagePrintFormula();
    case 'v': verbose = true; break;
    case 'f': formula = true; break;
    default:
      return UsagePrintFormula();
    }
  }

  if (util_optind < argc) {
    str_constr = util_strsav(argv[util_optind]);
  }

  if (verbose && formula) {
    UsagePrintFormula();
    goto CommandPrintFormula_fail;
  }

  if ((char *)NULL == str_constr || 0 == strlen(str_constr)) {
    fprintf(nusmv_stderr, "No expression given.\n\n");
    UsagePrintFormula();
    goto CommandPrintFormula_fail;
  }

  if (Compile_check_if_model_was_built(nusmv_stderr, true)) {
    goto CommandPrintFormula_fail;
  }

  { /* parsing given formula */
    const char* arv[2];
    const int arc = 2;

    arv[0] = (char*) NULL;
    arv[1] = (char*) str_constr;
    parse_result = Parser_ReadCmdFromString(arc, arv, "SIMPWFF ",
                                            ";", &constr);
  }

  if (parse_result != 0 || constr == Nil) {
    fprintf(nusmv_stderr, "Parsing error: expected an expression.\n" );
    goto CommandPrintFormula_fail;
  }

  /* check that the expression is a predicate */
  {
    BddEnc_ptr bdd_enc = Enc_get_bdd_encoding();

    SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(bdd_enc));
    TypeChecker_ptr tc = SymbTable_get_type_checker(st);

    if (SymbType_is_boolean(
            TypeChecker_get_expression_type(tc, car(constr), Nil))) {

    fprintf(nusmv_stdout,
    "######################################################################\n");

    BddEnc_print_formula_info(bdd_enc, car(constr),
                              verbose, formula, nusmv_stdout);
    fprintf(nusmv_stdout,
    "######################################################################\n");

    } /* if SymbType_is_boolean */
    else  {
      fprintf(nusmv_stderr, "expression is not a predicate.\n");
    }
  }

 CommandPrintFormula_fail:
  if ((char *)NULL != str_constr) FREE(str_constr);

  return 0;
}

static int UsagePrintFormula()
{
  fprintf(nusmv_stderr, "usage: print_formula [-h] | [-v] | [-f] <expr>\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -v \t\tPrints explicit models of the formula.\n");
  fprintf(nusmv_stderr,
          "   -f \t\tPrints the simplified and canonical formula.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Dumps an expression in the specified output format.]

  CommandName        [dump_expr]

  CommandSynopsis [Dumps a given expression (which can be
  contextualized) in the specified format. Example of supported format
  is 'dot'. ]

  CommandArguments   [\[-h\] | -e <expression> -f <format> \[-o <fname>\]]

  CommandDescription [Dumps an expression in the specified output format.

  Command Options:
  <dl>
  <dt> <tt>-e expression</tt>
  <dd> The required expression to be dumped.
  <dt> <tt>-f format</tt>
  <dd> The format to be used when dumping. Examples are dot, davinci.
  <dt> <tt>-o filename</tt>
  <dd> The name of the output file (default: standard output)
  </dl>]

  SideEffects        []

******************************************************************************/
static int CommandDumpExpr(int argc, char **argv)
{
  int c;
  int res = 0;
  node_ptr parsed_expr;
  char* str_constr = (char*) NULL;
  char* str_format = (char*) NULL;
  char* fname = (char*) NULL;
  FILE* outfile = (FILE*) NULL;
  FILE* const default_outfile = nusmv_stdout;
 
  typedef enum { DUMP_FORMAT_INVALID, 
                 DUMP_FORMAT_DOT,
                 DUMP_FORMAT_DAVINCI,
  } t_format;


  const struct {
    const char* name;
    t_format format; 
  } supported_formats[] = { 
    {"dot", DUMP_FORMAT_DOT},
    {"davinci", DUMP_FORMAT_DAVINCI},
  };
  t_format format = DUMP_FORMAT_INVALID;
  
  /* Parse the command line */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "he:o:f:")) != EOF) {
    switch (c) {
    case 'h': 
      res = UsageDumpExpr();
      goto dump_expr_quit;       
      
    case 'f': 
      if ((char*) NULL != str_format) FREE(str_format);
      str_format = util_strsav(util_optarg);
      break;

    case 'e': 
      if ((char*) NULL != str_constr) FREE(str_constr);
      str_constr = util_strsav(util_optarg);
      break;

    case 'o':
      if ((char*) NULL != fname) FREE(fname);
      fname = util_strsav(util_optarg);
      break;
      
    default:
      res = 1;
      goto dump_expr_quit;       
    }
  }

  /* preconditions */
  if (Compile_check_if_encoding_was_built(nusmv_stderr)) {
    res = 1;
    goto dump_expr_quit;
  }

  /* checks arguments */
  if ((char*) NULL == str_constr) {
    fprintf(nusmv_stderr, "No expression given.\n");
    res = 1;
    goto dump_expr_quit;
  }

  if ((char*) NULL == str_format) {
    fprintf(nusmv_stderr, "No format was specified.\n");
    res = 1;
    goto dump_expr_quit;    
  }

  { /* checks format */
    int i;
    for (i=0; i<sizeof(supported_formats)/sizeof(supported_formats[0]); ++i) {
      if (Utils_strcasecmp(supported_formats[i].name, str_format) == 0) {
        format = supported_formats[i].format;
        break;
      }
    }
    if (DUMP_FORMAT_INVALID == format) {
      fprintf(nusmv_stderr, "Invalid format. Valid formats are: ");
      for (i=0; i<sizeof(supported_formats)/sizeof(supported_formats[0]); ++i) {
        fprintf(nusmv_stderr, "'%s' ", supported_formats[i].name);
      }
      fprintf(nusmv_stderr, "\n");
      res = 1;
      goto dump_expr_quit;    
    }
  }

  if (Parser_ReadNextExprFromString(str_constr, &parsed_expr)) {
    res = 1;
    goto dump_expr_quit;
  }

  if ((char*) NULL != fname) {
    outfile = fopen(fname, "w");
    if ((FILE*) NULL == outfile) {
      fprintf(nusmv_stderr, "Problems opening output file '%s'.\n", fname);
      res = 1;
      goto dump_expr_quit;     
    }
  }
  else {
    /* file name was not specified: use default output descriptor */
    outfile = default_outfile;
  }

      
  /* type check the expression, then convers to bdd and dumps */
  {
    BddEnc_ptr bdd_enc = Enc_get_bdd_encoding();
    SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(bdd_enc));
    TypeChecker_ptr tc = SymbTable_get_type_checker(st);
    node_ptr node_expr =  Compile_FlattenSexp(st, car(parsed_expr), Nil);   
    SymbType_ptr tp = TypeChecker_get_expression_type(tc, node_expr, Nil);

    /* checks type compatibility */
    if (SymbType_is_error(tp)) {
      fprintf(nusmv_stderr, "Type of expression is not correct\n");
      res = 1;
      goto dump_expr_quit;
    }
    if (SymbType_is_real(tp) || 
        SymbType_is_string(tp) || 
        SymbType_is_statement(tp)) {
      fprintf(nusmv_stderr, "Type of expression is not supported\n");
      res = 1;
      goto dump_expr_quit;
    }
   
    { /* the actual job */
      AddArray_ptr addarray = BddEnc_expr_to_addarray(bdd_enc, node_expr, Nil);
      const int adr_size = AddArray_get_size(addarray);
      const char** onames;
      int i;

      onames = ALLOC(const char*, adr_size);
      nusmv_assert((const char**) NULL != onames);

      /* fills onames */
      if (1 == adr_size) {
        onames[0] = util_strsav(str_constr);
      }
      else {
        const char* oname_fmt = "%s[%0*d]";
        const int digits = (int) log10(adr_size);
        const int oname_len = (strlen(str_constr) + strlen(oname_fmt) + digits + 1);
        
        for (i=0; i<adr_size; ++i) {          
          char* oname = ALLOC(char, oname_len);
          int c;
          nusmv_assert((char*) NULL != oname);
          c = snprintf(oname, oname_len, oname_fmt, str_constr, digits, i);
          SNPRINTF_CHECK(c, oname_len);
          onames[i] = oname;
        }
      }
      
      switch (format) {
      case DUMP_FORMAT_DOT:
        res = BddEnc_dump_addarray_dot(bdd_enc, addarray, onames, outfile);
        break;
      
      case DUMP_FORMAT_DAVINCI:
        res = BddEnc_dump_addarray_davinci(bdd_enc, addarray, onames, outfile);
        break;

      default:
        internal_error("Unknown format");
      }

      /* cleanup */
      for (i=0; i<adr_size; ++i) { FREE(onames[i]); }
      FREE(onames);
      AddArray_destroy(BddEnc_get_dd_manager(bdd_enc), addarray);
    }
  }
 
 dump_expr_quit:
  if ((char*) NULL != str_constr) { FREE(str_constr); }
  if ((char*) NULL != str_format) { FREE(str_format); }
  if ((char*) NULL != fname) { FREE(fname); }
  if ((FILE*) NULL != outfile && outfile != default_outfile) { 
    fclose(outfile);
  }
  return res;  
}

static int UsageDumpExpr()
{
  fprintf(nusmv_stderr, "usage: dump_expr [-h] | -e <expr> -f <format> [-o fname]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -e <expr> \tExpression to be dumped.\n");
  fprintf(nusmv_stderr, "   -f <format> \tThe format of dumping (e.g. 'dot').\n");
  fprintf(nusmv_stderr, "   -o <fname> \tThe output file name.\n");
  return 1;
}
