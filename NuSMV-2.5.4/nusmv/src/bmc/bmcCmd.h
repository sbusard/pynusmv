/**CHeaderFile*****************************************************************

  FileName    [bmcCmd.h]

  PackageName [bmc]

  Synopsis    [The header file for the <tt>cmd</tt> module, the user
  commands handling layer.]

  Description []

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

  Revision    [$Id: bmcCmd.h,v 1.2.4.1.2.2.2.4.6.6 2010-02-12 17:14:49 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_CMD_H
#define _BMC_CMD_H

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "prop/Prop.h" /* for Prop_type type */
#include "utils/utils.h"

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int Bmc_CommandBmcSetup ARGS((int argc, char** argv));

EXTERN int Bmc_CommandBmcSimulate ARGS((int argc, char** argv));

EXTERN int
Bmc_CommandBmcIncSimulate ARGS((int argc, char** argv));

EXTERN int Bmc_CommandBmcPickState ARGS((int argc, char **argv));
EXTERN int
Bmc_CommandBmcSimulateCheckFeasibleConstraints ARGS((int argc, char **argv));

EXTERN int Bmc_CommandGenLtlSpecBmc ARGS((int argc, char** argv));

EXTERN int
Bmc_CommandGenLtlSpecBmcOnePb ARGS((int argc, char** argv));

EXTERN int
Bmc_CommandCheckLtlSpecBmc ARGS((int argc, char** argv));

EXTERN int
Bmc_CommandCheckLtlSpecBmcOnePb ARGS((int argc, char** argv));

EXTERN int Bmc_CommandGenInvarBmc ARGS((int argc, char** argv));

EXTERN int Bmc_CommandCheckInvarBmc ARGS((int argc, char** argv));

EXTERN int Bmc_check_if_model_was_built ARGS((FILE* err, boolean forced));

EXTERN Outcome
Bmc_cmd_options_handling ARGS((int argc, char** argv,
                               Prop_Type prop_type,
                               Prop_ptr* res_prop,
                               int* res_k,
                               int* res_l,
                               char** res_a,
                               char** res_s,
                               char** res_o,
                               boolean* res_e));


#if NUSMV_HAVE_INCREMENTAL_SAT
EXTERN int
Bmc_CommandCheckLtlSpecBmcInc ARGS((int argc, char** argv));

EXTERN int
Bmc_CommandCheckInvarBmcInc ARGS((int argc, char** argv));
#endif


/**AutomaticEnd***************************************************************/

#endif /* _BMC_CMD_H */
