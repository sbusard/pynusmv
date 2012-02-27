/**CHeaderFile*****************************************************************

  FileName    [ParserProb_private.h]

  PackageName [parser.prob]

  Synopsis    [ The private header file of ParserProb class.]

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

  Revision    [$Id: ParserProb_private.h,v 1.1.2.2 2009-02-04 20:02:12 nusmv Exp $]

******************************************************************************/

#ifndef __PARSER_PROB_PRIVATE_H__
#define __PARSER_PROB_PRIVATE_H__

#include "ParserProb.h"
#include "utils/utils.h"
#include "node/node.h"

void parser_prob_add ARGS((ParserProb_ptr self, node_ptr prob));
node_ptr parser_prob_mk_prob ARGS((ParserProb_ptr self, 
                                   node_ptr assigns, node_ptr prob));

node_ptr parser_prob_mk_var_assign ARGS((ParserProb_ptr self, 
                                        node_ptr var, node_ptr val));

node_ptr 
parser_prob_mk_var_assigns ARGS((ParserProb_ptr self, 
                                 node_ptr left, node_ptr right));

node_ptr 
parser_prob_mk_dot ARGS((ParserProb_ptr self, node_ptr left, node_ptr right));

node_ptr 
parser_prob_mk_array ARGS((ParserProb_ptr self, node_ptr left, node_ptr right));

node_ptr parser_prob_mk_atom ARGS((ParserProb_ptr self, const char* name));

node_ptr parser_prob_mk_num ARGS((ParserProb_ptr self, const int num));

node_ptr parser_prob_mk_real ARGS((ParserProb_ptr self, const char* real_text));

node_ptr parser_prob_mk_true ARGS((ParserProb_ptr self));
node_ptr parser_prob_mk_false ARGS((ParserProb_ptr self));


#endif /* __PARSER_PROB_PRIVATE_H__ */
