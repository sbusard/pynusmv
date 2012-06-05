/**CHeaderFile*****************************************************************

  FileName    [PropDb.h]

  PackageName [prop]

  Synopsis    [Public interface of class 'PropDb']

  Description []

  SeeAlso     [PropDb.c]

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


#ifndef __PROP_DB_H__
#define __PROP_DB_H__

#include "prop/Prop.h"

#include "utils/object.h"
#include "utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class PropDb]

  Description []

******************************************************************************/
typedef struct PropDb_TAG*  PropDb_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class PropDb]

  Description [These macros must be used respectively to cast and to check
  instances of class PropDb]

******************************************************************************/
#define PROP_DB(self) \
         ((PropDb_ptr) self)

#define PROP_DB_CHECK_INSTANCE(self) \
         (nusmv_assert(PROP_DB(self) != PROP_DB(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN PropDb_ptr PropDb_create ARGS((void));
EXTERN void PropDb_destroy ARGS((PropDb_ptr self));
EXTERN void PropDb_clean ARGS((PropDb_ptr self));

EXTERN int
PropDb_fill ARGS((PropDb_ptr self, SymbTable_ptr symb_table,
                  node_ptr, node_ptr, node_ptr,
                  node_ptr, node_ptr));

EXTERN boolean PropDb_add ARGS((PropDb_ptr self, Prop_ptr));

EXTERN int
PropDb_prop_create_and_add ARGS((PropDb_ptr self, SymbTable_ptr symb_table,
                                 node_ptr, Prop_Type));

EXTERN int PropDb_get_size ARGS((const PropDb_ptr self));

/* Master property */
EXTERN Prop_ptr PropDb_set_master ARGS((PropDb_ptr self, Prop_ptr prop));
EXTERN Prop_ptr PropDb_get_master ARGS((const PropDb_ptr self));

EXTERN void PropDb_set_fsm_to_master ARGS((PropDb_ptr self, Prop_ptr prop));

/* Master's FSMs getters: */
EXTERN SexpFsm_ptr PropDb_master_get_scalar_sexp_fsm ARGS((const PropDb_ptr self));
EXTERN BoolSexpFsm_ptr PropDb_master_get_bool_sexp_fsm ARGS((const PropDb_ptr self));
EXTERN BddFsm_ptr  PropDb_master_get_bdd_fsm ARGS((const PropDb_ptr self));
EXTERN BeFsm_ptr PropDb_master_get_be_fsm ARGS((const PropDb_ptr self));

/* Master's FSMs setters: */
EXTERN void PropDb_master_set_scalar_sexp_fsm ARGS((PropDb_ptr self, SexpFsm_ptr fsm));
EXTERN void PropDb_master_set_bool_sexp_fsm ARGS((PropDb_ptr self, BoolSexpFsm_ptr fsm));
EXTERN void PropDb_master_set_bdd_fsm ARGS((PropDb_ptr self, BddFsm_ptr fsm));
EXTERN void PropDb_master_set_be_fsm ARGS((PropDb_ptr self, BeFsm_ptr fsm));

EXTERN Prop_ptr PropDb_get_prop_at_index ARGS((const PropDb_ptr self,
                                               int num));

EXTERN int PropDb_get_prop_name_index ARGS((const PropDb_ptr self,
                                            const node_ptr name));

EXTERN int PropDb_prop_parse_name ARGS((const PropDb_ptr self,
                                        const char* str));

EXTERN Prop_ptr PropDb_get_last ARGS((const PropDb_ptr self));

EXTERN PropDb_PrintFmt PropDb_get_print_fmt ARGS((const PropDb_ptr self));

EXTERN PropDb_PrintFmt PropDb_set_print_fmt ARGS((const PropDb_ptr self,
                                                  PropDb_PrintFmt new_fmt));

EXTERN void PropDb_print_list_header ARGS((const PropDb_ptr self,
                                           FILE* file));
EXTERN void PropDb_print_list_footer ARGS((const PropDb_ptr self,
                                           FILE* file));

int PropDb_print_prop_at_index ARGS((const PropDb_ptr self,
                                     FILE* file, const int index));

EXTERN void PropDb_print_all ARGS((const PropDb_ptr self,
                                   FILE* file));

EXTERN void PropDb_print_all_type ARGS((const PropDb_ptr self,
                                        FILE* file, Prop_Type type));

EXTERN void PropDb_print_all_status ARGS((const PropDb_ptr self,
                                          FILE* file, Prop_Status status));

EXTERN void PropDb_print_all_status_type ARGS((const PropDb_ptr self,
                                               FILE* file, Prop_Status status,
                                               Prop_Type type));

EXTERN lsList PropDb_get_props_of_type ARGS((const PropDb_ptr self,
                                             const Prop_Type type));

EXTERN lsList PropDb_get_ordered_props_of_type ARGS((const PropDb_ptr self,
                                                     const FlatHierarchy_ptr hierarchy,
                                                     const Prop_Type type));

EXTERN int PropDb_prop_parse_and_add ARGS((const PropDb_ptr self,
                                           SymbTable_ptr symb_table,
                                           const char* str,
                                           const Prop_Type type));

EXTERN int PropDb_get_prop_index_from_string ARGS((const PropDb_ptr self,
                                                   const char* idx));

EXTERN int PropDb_get_prop_index_from_trace_index ARGS((const PropDb_ptr self,
                                                        const int trace_idx));

EXTERN void PropDb_verify_all ARGS((const PropDb_ptr self));

EXTERN void PropDb_verify_all_type ARGS((const PropDb_ptr self, Prop_Type));

EXTERN void PropDb_ordered_verify_all ARGS((const PropDb_ptr self,
                                            const FlatHierarchy_ptr hierarchy));

EXTERN void
PropDb_ordered_verify_all_type ARGS((const PropDb_ptr self,
                                     const FlatHierarchy_ptr hierarchy,
                                     const Prop_Type type));

EXTERN void PropDb_verify_prop_at_index ARGS((const PropDb_ptr self,
                                              const int index));

EXTERN NodeList_ptr
PropDb_get_ordered_properties ARGS((const PropDb_ptr self,
                                    const FlatHierarchy_ptr hierarchy));

EXTERN NodeList_ptr
PropDb_get_coi_grouped_properties ARGS((const PropDb_ptr self,
                                        const FlatHierarchy_ptr hierarchy));


/**AutomaticEnd***************************************************************/

#endif /* __PROP_DB_H__ */
