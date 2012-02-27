/**CHeaderFile*****************************************************************

  FileName    [parserInt.h]

  PackageName [parser]

  Synopsis    [Internal header of the parser package.]

  Description [Internal header of the parser package.]

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

  Revision    [$Id: parserInt.h,v 1.4.6.2.4.6.4.3 2009-10-13 17:31:17 nusmv Exp $]

******************************************************************************/

#ifndef _PARSER_INT
#define _PARSER_INT

#include <stdlib.h>
#include <stdio.h>

#include "parser.h"
#include "util.h"

#include "utils/utils.h"
#include "node/node.h"
#include "dd/dd.h"
#include "set/set.h"
#include "rbc/rbc.h"
#include "compile/compile.h"
#include "opt/opt.h"
#include "cmd/cmd.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct yy_buffer_state* YY_BUFFER_STATE;


/*---------------------------------------------------------------------------*/
/* Constants declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN node_ptr parsed_tree;

enum PARSE_MODE { PARSE_MODULES, PARSE_COMMAND, PARSE_LTL_EXPR };
EXTERN enum PARSE_MODE parse_mode_flag;

EXTERN int yylineno;
EXTERN FILE *yyin;

EXTERN int psl_yylineno;
EXTERN FILE *psl_yyin;

EXTERN cmp_struct_ptr cmps;

EXTERN FILE *nusmv_stderr;
EXTERN FILE *nusmv_stdout;

EXTERN node_ptr psl_parsed_tree;
EXTERN node_ptr psl_property_name;

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN int yylex                        ARGS((void));
EXTERN int yyparse                      ARGS((void));
EXTERN void yyrestart                   ARGS((FILE *input_file));
EXTERN void yy_switch_to_buffer         ARGS((YY_BUFFER_STATE new_buffer));
EXTERN YY_BUFFER_STATE yy_scan_buffer   ARGS((char *base, size_t size));

EXTERN YY_BUFFER_STATE yy_create_buffer ARGS((FILE *file, int size));
EXTERN void yy_delete_buffer ARGS((YY_BUFFER_STATE b));

EXTERN YY_BUFFER_STATE yy_scan_string   ARGS((const char *yy_str));

EXTERN int psl_yyparse ARGS((void));
EXTERN void Parser_switch_to_psl ARGS((void));

EXTERN void parser_add_syntax_error ARGS((const char* fname, int lineno, 
                                          const char* token,
                                          const char* err_msg));

EXTERN void parser_free_parsed_syntax_errors ARGS((void));

#endif /* _PARSER_INT */
