/**CHeaderFile*****************************************************************

  FileName    [sbmcCmd.h]

  PackageName [bmc.sbmc]

  Synopsis    [The header file for the <tt>cmd</tt> module, the user 
  commands handling layer.]

  Description []

  SeeAlso     []

  Author      [Timo Latvala, Marco Roveri]

  Copyright   [
  This file is part of the ``bmc.sbmc'' package of NuSMV version 2.
  Copyright (C) 2004 Timo Latvala <timo.latvala@tkk.fi>
  Copyright (C) 2006 Tommi Junttila

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

#ifndef _SBMC_CMD_H
#define _SBMC_CMD_H

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "utils/utils.h"
#include "prop/Prop.h"

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int Sbmc_CommandCheckLtlSpecSBmc  ARGS((int argc, char** argv)); 
EXTERN int Sbmc_CommandGenLtlSpecSBmc    ARGS((int argc, char** argv)); 
EXTERN int Sbmc_CommandLTLCheckZigzagInc ARGS((int argc, char** argv));

EXTERN int Sbmc_check_psl_property ARGS((Prop_ptr prop, 
                                         boolean dump_prob, 
                                         boolean inc_sat, 
                                         boolean do_completeness_check, 
                                         boolean do_virtual_unrolling, 
                                         boolean single_prob, 
                                         int k, int rel_loop));

/**AutomaticEnd***************************************************************/

#endif /* _BMC_CMD_H */
