/**CHeaderFile*****************************************************************

  FileName    [ParserOrd.h]

  PackageName [parser.ord]

  Synopsis    [ The header file of ParserOrd class.]

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

  Revision    [$Id: ParserOrd.h,v 1.1.2.2.4.2 2005-03-08 16:17:26 nusmv Exp $]

******************************************************************************/


#ifndef __PARSER_ORD_H__
#define __PARSER_ORD_H__

#include "utils/utils.h"
#include "utils/NodeList.h"


typedef struct ParserOrd_TAG* ParserOrd_ptr;

#define PARSER_ORD(x) \
        ((ParserOrd_ptr) (x))

#define PARSER_ORD_CHECK_INSTANCE(x) \
        (nusmv_assert(PARSER_ORD(x) != PARSER_ORD(NULL)))


EXTERN ParserOrd_ptr ParserOrd_create ARGS((void));
EXTERN void ParserOrd_destroy ARGS((ParserOrd_ptr self));

EXTERN void ParserOrd_parse_from_file ARGS((ParserOrd_ptr self, FILE* f));

EXTERN void 
ParserOrd_parse_from_string ARGS((ParserOrd_ptr self, const char* str));

EXTERN NodeList_ptr ParserOrd_get_vars_list ARGS((const ParserOrd_ptr self));

EXTERN void ParserOrd_reset ARGS((ParserOrd_ptr self));


#endif /* __PARSER_ORD_H__ */
