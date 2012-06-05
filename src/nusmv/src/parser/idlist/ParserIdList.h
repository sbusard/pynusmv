/**CHeaderFile*****************************************************************

  FileName    [ParserIdList.h]

  PackageName [parser.idlist]

  Synopsis    [ The header file of ParserIdList class.]

  Description []

  SeeAlso     []

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

  Revision    [$Id: ParserIdList.h,v 1.1.2.1 2006-09-26 14:27:20 nusmv Exp $]

******************************************************************************/


#ifndef __PARSER_ID_LIST_H__
#define __PARSER_ID_LIST_H__

#include "utils/utils.h"
#include "utils/NodeList.h"


typedef struct ParserIdList_TAG* ParserIdList_ptr;

#define PARSER_ID_LIST(x) \
        ((ParserIdList_ptr) (x))

#define PARSER_ID_LIST_CHECK_INSTANCE(x) \
        (nusmv_assert(PARSER_ID_LIST(x) != PARSER_ID_LIST(NULL)))


/* ---------------------------------------------------------------------- */

EXTERN ParserIdList_ptr ParserIdList_create ARGS((void));
EXTERN void ParserIdList_destroy ARGS((ParserIdList_ptr self));

EXTERN void ParserIdList_parse_from_file ARGS((ParserIdList_ptr self, FILE* f));

EXTERN void 
ParserIdList_parse_from_string ARGS((ParserIdList_ptr self, const char* str));

EXTERN NodeList_ptr ParserIdList_get_id_list ARGS((const ParserIdList_ptr self));

EXTERN void ParserIdList_reset ARGS((ParserIdList_ptr self));


#endif /* __PARSER_ID_LIST_H__ */
