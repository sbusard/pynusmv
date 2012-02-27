/**CHeaderFile*****************************************************************

  FileName    [bmcDump.h]

  PackageName [bmc]

  Synopsis    [Public interface for dumping functionalities, like dimacs]

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

******************************************************************************/

#ifndef _BMC_DIMACS_H
#define _BMC_DIMACS_H

#include <stdio.h>

#include "enc/be/BeEnc.h"

#include "utils/utils.h"
#include "prop/Prop.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Enum***********************************************************************

  Synopsis    []

  Description []

  SeeAlso     []

******************************************************************************/
typedef enum { 
  BMC_DUMP_NONE, 
  BMC_DUMP_DIMACS, 
  BMC_DUMP_DA_VINCI, 
  BMC_DUMP_GDL 
} Bmc_DumpType;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void 
Bmc_Dump_WriteProblem ARGS((const BeEnc_ptr be_enc, 
			    const Be_Cnf_ptr cnf, 
			    Prop_ptr prop, 
			    const int k, const int loop, 
			    const Bmc_DumpType dump_type,
			    const char* dump_fname_template)); 

EXTERN int 
Bmc_Dump_DimacsInvarProblemFilename ARGS((const BeEnc_ptr be_enc, 
					  const Be_Cnf_ptr cnf, 
					  const char* filename));

EXTERN int 
Bmc_Dump_DimacsProblemFilename ARGS((const BeEnc_ptr be_enc, 
				     const Be_Cnf_ptr cnf,
				     const char* filename,  
				     const int k));


EXTERN void 
Bmc_Dump_DimacsInvarProblem ARGS((const BeEnc_ptr be_enc, 
				  const Be_Cnf_ptr cnf,
				  FILE* dimacsfile)); 

EXTERN void 
Bmc_Dump_DimacsProblem ARGS((const BeEnc_ptr be_enc, 
			     const Be_Cnf_ptr cnf,
			     const int k, FILE* dimacsfile)); 

/**AutomaticEnd***************************************************************/

#endif /* _BMC_DIMACS_H */

