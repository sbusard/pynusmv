/**CHeaderFile*****************************************************************

  FileName    [ParserAp_private.h]

  PackageName [compass.parser.ap]

  Synopsis    [ The private header file of ParserAp class.]

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

  Revision    [$Id: ParserAp_private.h,v 1.1.2.1 2008-12-29 14:57:15 nusmv Exp $]

******************************************************************************/

#ifndef __PARSER_AP_PRIVATE_H__
#define __PARSER_AP_PRIVATE_H__

#include "ParserAp.h"
#include "utils/utils.h"
#include "node/node.h"

void parser_ap_add ARGS((ParserAp_ptr self, node_ptr ap));
node_ptr parser_ap_mk_ap ARGS((ParserAp_ptr self, 
                               node_ptr label, node_ptr ap));

#endif /* __PARSER_AP_PRIVATE_H__ */
