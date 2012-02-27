/**CFile*****************************************************************

  FileName    [compassCmd.c]

  PackageName [compass commands]

  Synopsis    []

  Description []

  SeeAlso     [compass.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compass'' package of NuSMV version 2. 
  Copyright (C) 2008 by FBK-irst.

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

#include "compass.h"
#include "compassInt.h"
#include "parser/prob/ParserProb.h"
#include "parser/ap/ParserAp.h"
#include "compile/ProbAssign.h"

#include "opt/opt.h"
#include "cmd/cmd.h"
#include "enc/enc.h"
#include "prop/propPkg.h"
#include "utils/NodeList.h"
#include "parser/symbols.h"
#include "parser/parser.h"


static char rcsid[] UTIL_UNUSED = "$Id: compassCmd.c,v 1.1.2.12 2010-02-02 10:09:32 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int UsageCompassGenSigref ARGS((void));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

int CommandCompassGenSigref(int argc, char **argv);

/**Function********************************************************************

  Synopsis           [Initializes the commands provided by this package]

  Description        []

  SideEffects        []

******************************************************************************/
void Compass_init_cmd()
{
  Cmd_CommandAdd("compass_gen_sigref", CommandCompassGenSigref, 0, true);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/



/**Function********************************************************************

  Synopsis           []

  CommandName        []            

  CommandSynopsis    []  

  CommandArguments   []  

  CommandDescription []  

  SideEffects        []

******************************************************************************/
int CommandCompassGenSigref(int argc, char **argv)
{
  int c = 0;
  const char* prob_fname = (char*) NULL;
  const char* sigref_fname = (char*) NULL;
  const char* tau_expr_str = (char*) NULL;
  const char* ap_fname = (char*) NULL;
  boolean do_indent = false;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hbi:t:o:a:")) != EOF) {
    switch (c) {
    case 'b':
      do_indent = true;
      break;
    case 'h': goto __compass_gen_sigref_fail_help;
    case 'i': 
      if (prob_fname != (char*) NULL) { FREE(prob_fname); }
      prob_fname = util_strsav(util_optarg);
      break;

    case 't': 
      if (tau_expr_str != (char*) NULL) { FREE(tau_expr_str); }
      tau_expr_str = util_strsav(util_optarg);
      break;

    case 'o': 
      if (sigref_fname != (char*) NULL) FREE(sigref_fname);
      sigref_fname = util_strsav(util_optarg);
      break;

    case 'a':
      if (ap_fname != (char*) NULL) FREE(ap_fname);
      ap_fname = util_strsav(util_optarg);
      break;
      
    default: goto __compass_gen_sigref_fail_help;
    }
  }

  if (argc != util_optind) goto __compass_gen_sigref_fail_help;

  /* preconditions */
  if (Compile_check_if_flat_model_was_built(nusmv_stderr, false) ||
      Compile_check_if_encoding_was_built(nusmv_stderr)) goto __compass_gen_sigref_fail;

  {   
    /* Input checking */
    Expr_ptr tau_expr = (Expr_ptr) NULL;
    FILE* prob_file = (FILE*) NULL;
    FILE* sigref_file;
    FILE* ap_file;

    ParserProb_ptr pl_parser = PARSER_PROB(NULL);
    ParserAp_ptr ap_parser = PARSER_AP(NULL);
    BddEnc_ptr enc = Enc_get_bdd_encoding();
    TypeChecker_ptr tc = BaseEnc_get_type_checker(BASE_ENC(enc));
    NodeList_ptr prob_list = NODE_LIST(NULL);
    NodeList_ptr ap_list = NODE_LIST(NULL);

    if (tau_expr_str != (char*) NULL) {


      if (Parser_ReadSimpExprFromString(tau_expr_str, &tau_expr) != 0) {
        goto __compass_gen_sigref_fail;
      }

      tau_expr = Compile_FlattenSexp(BaseEnc_get_symb_table(BASE_ENC(enc)), 
                                     car(tau_expr) /*gets rid of SIMPWFF*/,
                                     Nil);
    }

    if (prob_fname != (char*) NULL) {
      prob_file = fopen(prob_fname, "r");
      if (prob_file == (FILE*) NULL) {
        fprintf(nusmv_stderr, "Unable to open probability list file \"%s\".\n", 
                prob_fname);
        goto __compass_gen_sigref_fail;
      }

      { /* checks the input probabilistic file */
        pl_parser = ParserProb_create();
        ParserProb_parse_from_file(pl_parser, prob_file);    
        fclose(prob_file);
        
        prob_list = ParserProb_get_prob_list(pl_parser);
        Compass_check_prob_list(tc, prob_list);
      }
    }

    if (ap_fname != (char*) NULL) {
      ap_file = fopen(ap_fname, "r");
      if (ap_file == (FILE*) NULL) {
        fprintf(nusmv_stderr, "Unable to atomic proposition list file \"%s\".\n", 
                prob_fname);
        goto __compass_gen_sigref_fail;
      }

      { /* checks the input probabilistic file */
        ap_parser = ParserAp_create();
        ParserAp_parse_from_file(ap_parser, ap_file);    
        fclose(ap_file);
        
        ap_list = ParserAp_get_ap_list(ap_parser);
        Compass_check_ap_list(tc, ap_list);
      }
    }
    
    /* Was file name specified for sigref output? */
    if (sigref_fname != (char*) NULL) {
      sigref_file = fopen(sigref_fname, "w");
      if (sigref_file == (FILE*) NULL) {
        fprintf(nusmv_stderr, "Unable to open output sigref file \"%s\".\n", 
                sigref_fname);
        fclose(prob_file);
        goto __compass_gen_sigref_fail;
      }      
    }
    else sigref_file = nusmv_stdout;
    /* end of input checkings */
    
    /* does the actual work */
    Compass_write_sigref(sigref_file, 
                         PropDb_master_get_bdd_fsm(PropPkg_get_prop_database()),
                         prob_list, tau_expr,
                         ap_list, do_indent);
        
    if (PARSER_PROB(NULL) != pl_parser) ParserProb_destroy(pl_parser);
    if (PARSER_AP(NULL) != ap_parser) ParserAp_destroy(ap_parser);
  }

  /* success here */
  if (tau_expr_str != (char*) NULL) FREE(tau_expr_str);
  if (sigref_fname != (char*) NULL) FREE(sigref_fname);
  if (prob_fname != (char*) NULL) FREE(prob_fname);
  if (ap_fname  != (char*) NULL) FREE(ap_fname);
  return 0;

  /* failure handlers */
 __compass_gen_sigref_fail_help:
  if (tau_expr_str != (char*) NULL) FREE(tau_expr_str);
  if (sigref_fname != (char*) NULL) FREE(sigref_fname);
  if (prob_fname != (char*) NULL) FREE(prob_fname);
  if (ap_fname  != (char*) NULL) FREE(ap_fname);
  return UsageCompassGenSigref();

 __compass_gen_sigref_fail:
  if (tau_expr_str != (char*) NULL) FREE(tau_expr_str);
  if (sigref_fname != (char*) NULL) FREE(sigref_fname);
  if (prob_fname != (char*) NULL) FREE(prob_fname);
  if (ap_fname  != (char*) NULL) FREE(ap_fname);
  return 1;
}

static int UsageCompassGenSigref()
{
  fprintf(nusmv_stderr, "usage: compass_gen_sigref [-h] [-b] [-i <prob-fname>] [-t \"<tau>\"] [-o <sigref-fname>]\n");
  fprintf(nusmv_stderr, "  -h \t\t Prints the command usage.\n");
  fprintf(nusmv_stderr, "  -i <fname>\t Read probabilistic info from fname.\n");
  fprintf(nusmv_stderr, "  -t \"<tau>\"\t Read tau location from given assignment expression.\n");
  fprintf(nusmv_stderr, "  -a <fname>\t Read atomic proposition list from file fname.\n");
  fprintf(nusmv_stderr, "  -o <fname>\t Write result into file fname.\n");
  fprintf(nusmv_stderr, "  -b \t\t Beautify the XML output (default disabled).\n");

  return 1;
}
