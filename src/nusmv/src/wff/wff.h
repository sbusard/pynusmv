/**CHeaderFile*****************************************************************

  FileName    [wff.h]

  PackageName [wff]

  Synopsis    [Public interface for Well-Formed-Formula manipulation]

  Description []

  Author      [Alessandro Mariotti]

  Copyright   [
  This file is part of the ``wff'' package of NuSMV version 2.
  Copyright (C) 2011 by FBK-irst.

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


#ifndef __WFF_H__
#define __WFF_H__

#include "utils/utils.h"
#include "node/node.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/* Package initialization / deinitialization */
EXTERN void wff_pkg_init ARGS((void));
EXTERN void wff_pkg_quit ARGS((void));


/* Package top-level exported functions */
EXTERN int Wff_get_depth ARGS((node_ptr ltl_wff));

EXTERN node_ptr Wff_make_truth ARGS((void));

EXTERN node_ptr Wff_make_falsity ARGS((void));

EXTERN node_ptr Wff_make_not ARGS((node_ptr arg));

EXTERN node_ptr Wff_make_and ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Wff_make_or ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Wff_make_implies ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Wff_make_iff ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Wff_make_next ARGS((node_ptr arg));

EXTERN node_ptr Wff_make_opnext_times ARGS((node_ptr arg, int x));

EXTERN node_ptr Wff_make_opnext ARGS((node_ptr arg));

EXTERN node_ptr Wff_make_opprec ARGS((node_ptr arg));

EXTERN node_ptr Wff_make_opnotprecnot ARGS((node_ptr arg));

EXTERN node_ptr Wff_make_globally ARGS((node_ptr arg));

EXTERN node_ptr Wff_make_historically ARGS((node_ptr arg));

EXTERN node_ptr Wff_make_eventually ARGS((node_ptr arg));

EXTERN node_ptr Wff_make_once ARGS((node_ptr arg));

EXTERN node_ptr Wff_make_until ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Wff_make_since ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Wff_make_releases ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Wff_make_triggered ARGS((node_ptr arg1, node_ptr arg2));

#endif /* __WFF_H__ */
