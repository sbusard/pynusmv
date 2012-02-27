/**CHeaderFile*****************************************************************

  FileName    [sbmcBmc.h]

  PackageName [bmc.sbmc]

  Synopsis    [High-level functionalities interface file for SBMC]

  Description [High level functionalities to perform Simple Bounded Model 
  Checking for LTL properties.

  For further information about this implementation see:
  T. Latvala, A. Biere, K. Heljanko, and T. Junttila. Simple is
  Better: Efficient Bounded Model Checking for Past LTL. In: R. Cousot
  (ed.), Verification, Model Checking, and Abstract Interpretation,
  6th International Conference VMCAI 2005, Paris, France, Volume 3385
  of LNCS, pp. 380-395, Springer, 2005.  Copyright ©
  Springer-Verlag. 
  ]

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

  For more information of NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/
#ifndef _SBMC_BMC_H
#define _SBMC_BMC_H

#include "bmc/bmcDump.h"
#include "utils/utils.h"
#include "prop/Prop.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int
Bmc_SBMCGenSolveLtl ARGS((Prop_ptr ltlprop, 
                       const int k, const int relative_loop, 
                       const boolean must_inc_length, 
                       const boolean must_solve, 
                       const Bmc_DumpType dump_type, 
                       const char* dump_fname_template));

#endif /* _SBMC_BMC_H */

