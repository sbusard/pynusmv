/**CHeaderFile*****************************************************************

  FileName    [PropDb_private.h]

  PackageName [prop]

  Synopsis    [Private and protected interface of class 'PropDb']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [PropDb.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``prop'' package of NuSMV version 2. 
  Copyright (C) 2010 by FBK-irst. 

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

  Revision    [$Id: $]

******************************************************************************/


#ifndef __PROP_DB_PRIVATE_H__
#define __PROP_DB_PRIVATE_H__

#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "PropDb.h"
#include "Prop.h"

#include "utils/object.h"
#include "utils/object_private.h"

#include "utils/utils.h"
#include "utils/array.h"


/**Struct**********************************************************************

  Synopsis    [PropDb class definition derived from
               class Object]

  Description []

  SeeAlso     [Base class Object]

******************************************************************************/

/* Those are the types of the virtual methods. They can be used for
   type casts in subclasses. */
typedef int (*PropDb_prop_create_and_add_method)(PropDb_ptr, \
                                                 SymbTable_ptr, \
                                                 node_ptr, \
                                                 Prop_Type);
typedef void (*PropDb_set_fsm_to_master_method)(PropDb_ptr, Prop_ptr);
typedef void (*PropDb_verify_all_method)(const PropDb_ptr);

/* The class itself. */
typedef struct PropDb_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(Object);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  array_t* prop_database; /* contained properties */

  Prop_ptr master; /* property master */

  PropDb_PrintFmt print_fmt; /* print format */

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */
  /* int (*)(PropDb_ptr, SymbTable_ptr, node_ptr, Prop_Type) */
  PropDb_prop_create_and_add_method prop_create_and_add;
  /* void (*)(PropDb_ptr, Prop_ptr) */
  PropDb_set_fsm_to_master_method set_fsm_to_master;
  /* void (*)(const PropDb_ptr) */
  PropDb_verify_all_method verify_all;

} PropDb;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN void prop_db_init ARGS((PropDb_ptr self));
EXTERN void prop_db_deinit ARGS((PropDb_ptr self));

int prop_db_prop_create_and_add(PropDb_ptr self,
                                SymbTable_ptr symb_table,
                                node_ptr spec,
                                Prop_Type type);
void prop_db_set_fsm_to_master(PropDb_ptr self, Prop_ptr prop);
void prop_db_verify_all(const PropDb_ptr self);

#endif /* __PROP_DB_PRIVATE_H__ */
