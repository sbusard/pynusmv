/**CHeaderFile*****************************************************************

  FileName    [ParserProb.h]

  PackageName [parser.prob]

  Synopsis    [ The header file of ParserProb class.]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``parser.prob'' package of NuSMV version 2. 
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

  Revision    [$Id: ParserProb.h,v 1.1.2.1 2008-12-04 13:33:19 nusmv Exp $]

******************************************************************************/


#ifndef __PARSER_PROB_H__
#define __PARSER_PROB_H__

#include "utils/utils.h"
#include "utils/NodeList.h"


typedef struct ParserProb_TAG* ParserProb_ptr;

#define PARSER_PROB(x) \
        ((ParserProb_ptr) (x))

#define PARSER_PROB_CHECK_INSTANCE(x) \
        (nusmv_assert(PARSER_PROB(x) != PARSER_PROB(NULL)))


EXTERN ParserProb_ptr ParserProb_create ARGS((void));
EXTERN void ParserProb_destroy ARGS((ParserProb_ptr self));

EXTERN void ParserProb_parse_from_file ARGS((ParserProb_ptr self, FILE* f));

EXTERN void 
ParserProb_parse_from_string ARGS((ParserProb_ptr self, const char* str));

EXTERN NodeList_ptr ParserProb_get_prob_list ARGS((const ParserProb_ptr self));

EXTERN void ParserProb_reset ARGS((ParserProb_ptr self));


#endif /* __PARSER_PROB_H__ */
