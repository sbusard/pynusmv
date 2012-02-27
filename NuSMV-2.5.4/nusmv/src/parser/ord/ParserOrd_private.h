/**CHeaderFile*****************************************************************

  FileName    [ParserOrd_private.h]

  PackageName [parser.ord]

  Synopsis    [ The private header file of ParserOrd class.]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``parser.ord'' package of NuSMV version 2. 
  Copyright (C) 2003 by FBK-irst. 

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

  Revision    [$Id: ParserOrd_private.h,v 1.1.2.2.4.2 2005-03-08 16:17:26 nusmv Exp $]

******************************************************************************/

#ifndef __PARSER_ORD_PRIVATE_H__
#define __PARSER_ORD_PRIVATE_H__

#include "ParserOrd.h"
#include "utils/utils.h"
#include "node/node.h"

EXTERN void 
parser_ord_add_var ARGS((ParserOrd_ptr self, node_ptr var));

EXTERN node_ptr 
parser_ord_mk_atom ARGS((ParserOrd_ptr self, const char* name));

EXTERN node_ptr 
parser_ord_mk_bit ARGS((ParserOrd_ptr self, node_ptr l, int suffix));

EXTERN node_ptr 
parser_ord_mk_array ARGS((ParserOrd_ptr self, node_ptr l, node_ptr r));

EXTERN node_ptr 
parser_ord_mk_dot ARGS((ParserOrd_ptr self, node_ptr l, node_ptr r));

EXTERN node_ptr parser_ord_mk_num ARGS((ParserOrd_ptr self, const int num));


#endif /* __PARSER_ORD_PRIVATE_H__ */
