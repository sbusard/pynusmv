/**CHeaderFile*****************************************************************

  FileName    [bmcConv.h]

  PackageName [bmc]

  Synopsis    [The conversion be<->bdd module interface]

  Description [This layer contains all functionalities to perform
  conversion from Boolean Expressions (be) to BDD-based boolean expressions,
  and vice-versa]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by FBK-irst and University of Trento.

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

  Revision    [$Id: bmcConv.h,v 1.2.4.2.2.1.2.2.6.4 2010-02-18 10:00:02 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_CONV__H
#define _BMC_CONV__H

#include "be/be.h"
#include "enc/be/BeEnc.h"

#include "utils/utils.h"
#include "node/node.h"

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN node_ptr Bmc_Conv_Be2Bexp ARGS((BeEnc_ptr be_enc, be_ptr be));
EXTERN be_ptr Bmc_Conv_Bexp2Be ARGS((BeEnc_ptr be_enc, node_ptr bexp));

EXTERN node_ptr
Bmc_Conv_BexpList2BeList ARGS((BeEnc_ptr be_enc, node_ptr bexp_list));

EXTERN void
Bmc_Conv_cleanup_cached_entries_about ARGS((BeEnc_ptr be_enc,
                                            NodeList_ptr symbs));

EXTERN
void Bmc_Conv_get_BeModel2SymbModel ARGS((const BeEnc_ptr be_enc,
                                          const Slist_ptr be_model,
                                          int k,
                                          boolean convert_to_scalars,
                                          node_ptr* frozen,
                                          array_t** states,
                                          array_t** inputs));


/**AutomaticEnd***************************************************************/

#endif /* _BMC_CONV__H */

