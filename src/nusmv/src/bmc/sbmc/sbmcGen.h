/**CHeaderFile*****************************************************************

  FileName    [sbmcGen.h]

  PackageName [bmc.sbmcGen]

  Synopsis    [Public interface of the SBMC Generation module]

  Description []

  SeeAlso     []

  Author      [Timo Latvala, Marco Roveri]

  Copyright   [
  This file is part of the ``bmc.sbmc'' package of NuSMV version 2.
  Copyright (C) 2004 Timo Latvala <timo.latvala@tkk.fi>

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

  Revision    [$Id: sbmcGen.h,v 1.1.2.3.4.1 2007-05-14 16:05:42 nusmv Exp $]

******************************************************************************/

#ifndef _SBMC_GEN__H
#define _SBMC_GEN__H

#include "be/be.h"
#include "fsm/be/BeFsm.h"

#include "utils/utils.h"
#include "node/node.h"


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN be_ptr 
Bmc_Gen_SBMCProblem ARGS((const BeFsm_ptr be_fsm,
                          const node_ptr ltl_wff,
                          const int k, const int l));

/**AutomaticEnd***************************************************************/

#endif /* _SBMC_GEN__H */
