
/**CHeaderFile*****************************************************************

  FileName    [SexpFsm_private.h]

  PackageName [fsm.sexp]

  Synopsis    [Private and protected interface of class 'SexpFsm']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [SexpFsm.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm.sexp'' package of NuSMV version 2.
  Copyright (C) 2009 by FBK-irst.

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

  Revision    [$Id: SexpFsm_private.h,v 1.1.2.2 2009-06-19 08:38:33 nusmv Exp $]

******************************************************************************/


#ifndef __SEXP_FSM_PRIVATE_H__
#define __SEXP_FSM_PRIVATE_H__


#include "SexpFsm.h"

#include "compile/compile.h"
#include "compile/FlatHierarchy.h"
#include "set/set.h"

#include "utils/assoc.h"
#include "utils/NodeList.h"
#include "utils/object_private.h"
#include "utils/object.h"
#include "utils/utils.h"


/**Struct**********************************************************************

  Synopsis    [SexpFsm class definition derived from
               class Object]

  Description []

  SeeAlso     [Base class Object]

******************************************************************************/
typedef struct SexpFsm_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(Object);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  SymbTable_ptr st; /* the symbol table */
  FlatHierarchy_ptr hierarchy; /* contains fsm data */
  Set_t vars_set;
  NodeList_ptr symbols;

  hash_ptr hash_var_fsm;
  node_ptr const_var_fsm;

  int* family_counter; /* for reference counting */

  /* flag controlling inlining operations */
  boolean inlining;

  /* flag to recognize boolean fsm from scalar */
  boolean is_boolean;

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

} SexpFsm;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN void sexp_fsm_init ARGS((SexpFsm_ptr self,
                                const FlatHierarchy_ptr hierarchy,
                                const Set_t vars_set));

EXTERN void sexp_fsm_deinit ARGS((SexpFsm_ptr self));
EXTERN void sexp_fsm_copy_aux ARGS((const SexpFsm_ptr self, SexpFsm_ptr copy));


#endif /* __SEXP_FSM_PRIVATE_H__ */
