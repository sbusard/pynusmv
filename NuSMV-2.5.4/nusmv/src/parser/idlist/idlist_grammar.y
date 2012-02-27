%{
/**CFile***********************************************************************

  FileName    [grammar.y]

  PackageName [parser.idlist]

  Synopsis    [Yacc for id list parsing]

  SeeAlso     [input.l]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``parser.idlist'' package of NuSMV version 2. 
  Copyright (C) 2006 by FBK-irst. 

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

#include <setjmp.h>

#if NUSMV_HAVE_MALLOC_H
# if NUSMV_HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif  
# include <malloc.h>
#elif NUSMV_HAVE_SYS_MALLOC_H
# if NUSMV_HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif  
# include <sys/malloc.h>
#elif NUSMV_HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include "ParserIdList.h"
#include "ParserIdList_private.h"
#include "idlist_int.h"

#include "node/node.h"
#include "utils/error.h"
#include "utils/utils.h"

static char rcsid[] UTIL_UNUSED = "$Id: ";

extern FILE* nusmv_stderr;

   
void parser_idlist_error(char *s);

%}

%union {
  node_ptr node;
}

/*
  All of the terminal grammar symbols (tokens recognized by the lexical analyzer) 
  Note: all binary operators associate from left to right,  operators are
        listed from lowest to highest priority 

  Note: The following token are not used inside the grammar, but are
  used by other modules inside the system (i.e. the compiler, mc).
  CONTEXT EU AU EBU ABU MINU MAXU
  CONS OVER BIT
*/

%left LB RB
%left <node> ATOM NUMBER 
%left DOT

/* all nonterminals return a parse tree node */
%type <node> id  id_list_item  id_list


%start begin
%%
begin         : id_list { }
              ;

/* Repetition of variables */
id_list     : {}
| id_list_item id_list 
{
  parser_id_list_add_id(parser_idlist_get_global_parser(), $1);
}
;

id_list_item : id
;


id      : ATOM
| id DOT ATOM
{
  $$ = parser_id_list_mk_dot(parser_idlist_get_global_parser(), 
			     $1, $3);
}
| id LB NUMBER RB 
{
  $$ = parser_id_list_mk_array(parser_idlist_get_global_parser(), 
			       $1, $3);
}
| id DOT NUMBER
{
  $$ = parser_id_list_mk_dot(parser_idlist_get_global_parser(), 
			     $1, $3);
}
;

%%


/* Additional source code */
void parser_idlist_error(char *s) 
{
  extern char parser_idlist_text[];
    
  fprintf(nusmv_stderr,"\nIn id list file ");
  
  if (parser_idlist_lineno) {
    fprintf(nusmv_stderr, "line %d: ", parser_idlist_lineno);
  }
    
  fprintf(nusmv_stderr, "at token \"%s\": %s\n", parser_idlist_text, s);
  nusmv_exit(1);
}

int parser_idlist_wrap()  { return 1; }


