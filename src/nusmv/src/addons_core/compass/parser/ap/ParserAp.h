/**CHeaderFile*****************************************************************

  FileName    [ParserAp.h]

  PackageName [compaass.parser.ap]

  Synopsis    [ The header file of ParserAp class.]

  Description []

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``parser.ap'' package of NuSMV version 2. 
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
  Please report bugs to <nusmv-users@fbk.eu>.]

  Revision    [$Id: ParserAp.h,v 1.1.2.1 2008-12-29 14:57:15 nusmv Exp $]

******************************************************************************/


#ifndef __PARSER_AP_H__
#define __PARSER_AP_H__

#include "utils/utils.h"
#include "utils/NodeList.h"


typedef struct ParserAp_TAG* ParserAp_ptr;

#define PARSER_AP(x) \
        ((ParserAp_ptr) (x))

#define PARSER_AP_CHECK_INSTANCE(x) \
        (nusmv_assert(PARSER_AP(x) != PARSER_AP(NULL)))


EXTERN ParserAp_ptr ParserAp_create ARGS((void));
EXTERN void ParserAp_destroy ARGS((ParserAp_ptr self));

EXTERN void ParserAp_parse_from_file ARGS((ParserAp_ptr self, FILE* f));

EXTERN void 
ParserAp_parse_from_string ARGS((ParserAp_ptr self, const char* str));

EXTERN NodeList_ptr ParserAp_get_ap_list ARGS((const ParserAp_ptr self));

EXTERN void ParserAp_reset ARGS((ParserAp_ptr self));


#endif /* __PARSER_AP_H__ */
