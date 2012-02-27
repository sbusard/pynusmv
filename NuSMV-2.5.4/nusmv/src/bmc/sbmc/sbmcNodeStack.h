/**CHeaderFile*****************************************************************

  FileName    [sbmcNodeStack.h]

  PackageName [bmc.sbmc]

  Synopsis    [Public interface for the stack of node_ptr.]

  Description [A stack of node_ptr]

  SeeAlso     []

  Author      [Timo Latvala]

  Copyright   [
  This file is part of the ``bmc.sbmc'' package of NuSMV version 2.
  Copyright (C) 2004 by Timo Latvala <timo.latvala@tkk.fi>.

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
#ifndef sbmcNodeStack_h_
#define sbmcNodeStack_h_

#include "node/node.h" /*For node_ptr*/
#include "utils/utils.h" /* for ARGS and EXTERN */

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct nodeStack *Bmc_Stack_ptr;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

struct nodeStack {
  /**Number of slots allocated*/
  unsigned alloc;
  /**Number of slots occupied*/
  unsigned first_free;
  /**The table*/
  node_ptr *table; 
};

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

EXTERN Bmc_Stack_ptr Bmc_Stack_new_stack ARGS((void));
EXTERN void Bmc_Stack_push ARGS((Bmc_Stack_ptr, node_ptr));
EXTERN node_ptr Bmc_Stack_pop ARGS((Bmc_Stack_ptr));
EXTERN void Bmc_Stack_delete ARGS((Bmc_Stack_ptr stack));
EXTERN unsigned Bmc_Stack_size ARGS((Bmc_Stack_ptr stack));
EXTERN node_ptr Bmc_Stack_top ARGS((Bmc_Stack_ptr stack));

/**AutomaticEnd***************************************************************/

#endif /* sbmcNodeStack_h_*/
