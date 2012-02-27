#include "opt/opt.h"
#include "node/node.h"
#include "utils/ustring.h"
#include "parser/psl/pslNode.h"
#include "compile/compile.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

extern DdManager* dd_manager;
extern FILE* nusmv_stderr;
extern FILE* nusmv_stdout;
extern cmp_struct_ptr cmps;

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

static void psl_init()
{
  init_memory();
    
  nusmv_stdout     = stdout;
  nusmv_stderr     = stderr;

  init_string();
  init_options();
  node_pkg_init(); 

  dd_manager = init_dd_package();
  init_the_node();

  Compile_init();
}


static void psl_quit()
{

  Compile_quit();
  quit_dd_package(dd_manager);  
  node_pkg_quit();
  quit_string();

  if (nusmv_stdout != stdout) fclose(nusmv_stdout);  
  if (nusmv_stderr != stderr) fclose(nusmv_stderr);
}

extern PslNode_ptr psl_parsed_tree;

#define DOEXPAN 1
#define DOCONV  1

int main() 
{
  int res; 

  psl_init();
  res = psl_yyparse();

  if (res == 0) {

    fprintf(nusmv_stdout, "Parsed expression\n");

    print_node(nusmv_stdout, psl_parsed_tree);
    
    fprintf(nusmv_stdout, "\n");

    if (PslNode_is_obe(psl_parsed_tree)) {
      printf("Parsed an OBE (CTL) expression\n");
#if DOEXPAN
      printf("The expanded OBE expression is\n");
      print_node(nusmv_stdout, 
		 PslNode_pslobe2ctl(psl_parsed_tree, PSL2PSL));
      printf("\n\n");    
#endif

#if DOCONV
      printf("The SMV compatible CTL expression is\n");
      print_node(nusmv_stdout, 
		 PslNode_pslobe2ctl(psl_parsed_tree, PSL2SMV));
      printf("\n\n");
#endif 
    }
    else if (PslNode_is_handled_psl(psl_parsed_tree)) {
      PslNode_ptr expr=psl_parsed_tree;

#if 1
      if (!PslNode_is_ltl(expr)) {
        PslNode_ptr m;
        
	/* handled sere */
	m = PslNode_remove_sere(expr);
	expr=m;

	printf("The translated SERE expression is\n");
	print_node(nusmv_stdout, expr);
	printf("\n\n");

      }
#endif
      fprintf(nusmv_stdout, "Parsed a LTL expression\n");
#if DOEXPAN
      printf("The expanded LTL expression is\n");
      print_node(nusmv_stdout, 
		 PslNode_pslltl2ltl(expr, PSL2PSL));
      printf("\n\n");
#endif
      
#if DOCONV
      printf("The SMV compatible LTL expression is\n");
      print_node(nusmv_stdout, 
		 PslNode_pslltl2ltl(expr, PSL2SMV));
      printf("\n\n");
#endif 
    }
    else {
      printf("Parsed a NON LTL/CTL expression\n");
    }
  }
  else {
    printf("Parsing error\n");
  }

  printf("\n");
  
  psl_quit();
  return res;
}
