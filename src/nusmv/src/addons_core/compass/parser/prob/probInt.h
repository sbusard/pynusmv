/**CHeaderFile*****************************************************************

  FileName    [probInt.h]

  PackageName [parser.ord]

  Synopsis    [ The internal header file the package]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``parser.ord'' package of NuSMV version 2. 
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

  Revision    [$Id: probInt.h,v 1.1.2.1 2008-12-04 13:33:19 nusmv Exp $]

******************************************************************************/

#ifndef __PROB_INT_H__
#define __PROB_INT_H__

#include "ParserProb.h"
#include "utils/utils.h"
#include "opt/opt.h"


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct yy_buffer_state* YY_BUFFER_STATE;


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;
extern FILE* nusmv_stdin;

EXTERN int parser_prob_lineno;
EXTERN FILE* parser_prob_in;


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void parser_prob_set_global_parser ARGS((ParserProb_ptr parser));
EXTERN void parser_prob_reset_global_parser ARGS((ParserProb_ptr parser));
EXTERN ParserProb_ptr parser_prob_get_global_parser ARGS((void));


/* from generated code: */
EXTERN int parser_prob_lex ARGS((void));
EXTERN int parser_prob_parse ARGS((void));
EXTERN void parser_prob_restart ARGS((FILE* input_file));

EXTERN void 
parser_prob__switch_to_buffer ARGS((YY_BUFFER_STATE new_buffer));

EXTERN YY_BUFFER_STATE 
parser_prob__create_buffer ARGS((FILE* file, int size));

EXTERN void parser_prob__delete_buffer ARGS((YY_BUFFER_STATE buf));

EXTERN YY_BUFFER_STATE 
parser_prob__scan_string ARGS((const char* str));


#endif /* __PROB_INT_H__ */
