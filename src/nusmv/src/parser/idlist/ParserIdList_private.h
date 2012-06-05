/**CHeaderFile*****************************************************************

  FileName    [ParserIdList_private.h]

  PackageName [parser.idlist]

  Synopsis    [ The private header file of ParserIdList class.]

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

  Revision    [$Id: ParserIdList_private.h,v 1.1.2.2 2006-12-05 14:14:52 nusmv Exp $]

******************************************************************************/

#ifndef __PARSER_ID_LIST_PRIVATE_H__
#define __PARSER_ID_LIST_PRIVATE_H__

#include "ParserIdList.h"
#include "utils/utils.h"
#include "node/node.h"

EXTERN void 
parser_id_list_add_id ARGS((ParserIdList_ptr self, node_ptr name));

EXTERN node_ptr 
parser_id_list_mk_atom ARGS((ParserIdList_ptr self, const char* name));

EXTERN node_ptr 
parser_id_list_mk_array ARGS((ParserIdList_ptr self, node_ptr l, node_ptr r));

EXTERN node_ptr 
parser_id_list_mk_dot ARGS((ParserIdList_ptr self, node_ptr l, node_ptr r));

EXTERN node_ptr 
parser_id_list_mk_num ARGS((ParserIdList_ptr self, const int num));


#endif /* __PARSER_ID_LIST_PRIVATE_H__ */
