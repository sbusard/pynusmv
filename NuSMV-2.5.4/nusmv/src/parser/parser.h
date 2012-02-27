/**CHeaderFile*****************************************************************

  FileName    [parser.h]

  PackageName [parser]

  Synopsis    [Interface with the parser]

  Description [This file describe the interface with the parser. The
  result of the parsing is stored in a global variable called
  <code>parsed_tree</code>.]

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

  Revision    [$Id: parser.h,v 1.3.6.2.4.5.4.5 2010-02-02 10:09:34 nusmv Exp $]

******************************************************************************/

#ifndef _PARSER_H
#define _PARSER_H

#include "utils/utils.h"
#include "node/node.h"


/*---------------------------------------------------------------------------*/
/* Macros definitions                                                        */
/*---------------------------------------------------------------------------*/
#define OPT_PARSER_IS_LAX  "parser_is_lax"

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void Parser_Init ARGS((void));
EXTERN void Parser_Quit ARGS((void));

EXTERN int Parser_ReadCmdFromString ARGS((int argc, const char** argv, 
                                          const char* head, const char* tail, 
                                          node_ptr* pc));

EXTERN int Parser_ReadSimpExprFromString ARGS((const char* str_expr, 
                                               node_ptr* res));

EXTERN int Parser_ReadCmdFromFile ARGS((const char *filename, 
                                        node_ptr* res));


EXTERN int Parser_ReadSMVFromFile ARGS((const char* filename));
EXTERN int Parser_ReadLtlExprFromFile ARGS((const char* filename));

EXTERN int Parser_read_psl_from_string ARGS((int argc, const char** argv, 
                                             node_ptr* res));

EXTERN int Parser_read_psl_from_file ARGS((const char* filename, node_ptr* res));

EXTERN int
Parser_ReadNextExprFromString ARGS((const char* str_expr, node_ptr* res));

EXTERN int
Parser_ReadIdentifierExprFromString ARGS((const char* str_expr, node_ptr* res));

EXTERN int
Parser_ReadNextExprFromFile ARGS((const char *filename, node_ptr* res));

EXTERN node_ptr Parser_get_syntax_errors_list ARGS((void));

EXTERN void Parser_get_syntax_error ARGS((node_ptr node, 
                                          const char** out_filename, 
                                          int* out_lineno, 
                                          const char** out_token,
                                          const char** out_message));

EXTERN void Parser_print_syntax_error ARGS((node_ptr error, FILE* fout));


#endif /* _PARSER_H */
