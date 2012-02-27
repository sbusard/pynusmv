/**CHeaderFile*****************************************************************

  FileName    [compileInt.h]

  PackageName [compile]

  Synopsis    [Internal declaration needed for the compilation.]

  Description [This file provides the user routines to perform
  compilation of the model]

  Author      [Marco Roveri, Roberto Cavada]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2. 
  Copyright (C) 1998-2004 by CMU and FBK-irst. 

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

  Revision    [$Id: compileInt.h,v 1.29.4.13.4.12.6.10 2010-02-03 16:48:53 nusmv Exp $]

******************************************************************************/

#ifndef __COMPILE_INT_H__
#define __COMPILE_INT_H__


#include "compile/compile.h"

#include "symb_table/symb_table_int.h"
#include "symb_table/SymbTable.h"
#include "symb_table/SymbLayer.h"

#include "fsm/FsmBuilder.h"

#include "utils/utils.h"
#include "utils/NodeList.h"
#include "utils/assoc.h"

#include "opt/opt.h"
#include "dd/dd.h"
#include "node/node.h"
#include "set/set.h"

#include "hrc/HrcNode.h"

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN FsmBuilder_ptr global_fsm_builder; 
EXTERN FlatHierarchy_ptr mainFlatHierarchy;
EXTERN PredicateNormaliser_ptr global_predication_normaliser;


EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;

EXTERN int yylineno;

EXTERN DdManager* dd_manager;

EXTERN cmp_struct_ptr cmps;

EXTERN node_ptr zero_number;
EXTERN node_ptr one_number;
EXTERN node_ptr boolean_range; 

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int CommandIwls95PrintOption ARGS((int argc, char ** argv));
EXTERN int CommandCPPrintClusterInfo ARGS((int argc, char ** argv));

EXTERN cmp_struct_ptr cmp_struct_init ARGS((void));
EXTERN void cmp_struct_quit ARGS((cmp_struct_ptr));

EXTERN void init_coi_hash ARGS((void));
EXTERN void clear_coi_hash ARGS((void));
EXTERN void insert_coi_hash ARGS((node_ptr, Set_t));
EXTERN Set_t lookup_coi_hash ARGS((node_ptr));

EXTERN void init_coi0_hash ARGS((void));
EXTERN void clear_coi0_hash ARGS((void));
EXTERN void insert_coi0_hash ARGS((node_ptr, Set_t));
EXTERN Set_t lookup_coi0_hash ARGS((node_ptr));

EXTERN void  init_define_dep_hash ARGS((void));
EXTERN void  clear_define_dep_hash ARGS((void));
EXTERN void  insert_define_dep_hash ARGS((node_ptr, Set_t));
EXTERN Set_t lookup_define_dep_hash ARGS((node_ptr));

EXTERN void init_dependencies_hash ARGS((void));
EXTERN void clear_dependencies_hash ARGS((void));
EXTERN void insert_dependencies_hash ARGS((node_ptr, Set_t));
EXTERN Set_t lookup_dependencies_hash ARGS((node_ptr));

EXTERN void init_check_constant_hash ARGS((void));
EXTERN void deinit_check_constant_hash ARGS((void));
EXTERN void clear_check_constant_hash ARGS((void));

EXTERN void init_check_next_hash ARGS((void));
EXTERN void deinit_check_next_hash ARGS((void));
EXTERN void clear_check_next_hash ARGS((void));

EXTERN void init_expr2bexp_hash ARGS((void));
EXTERN void clear_expr2bexp_hash ARGS((void));

void init_consts_hash ARGS((void));
void insert_consts_hash ARGS((node_ptr key, Set_t value));
Set_t lookup_consts_hash ARGS((node_ptr key));
void clear_consts_hash ARGS((void));

EXTERN void Compile_ConstructHierarchy 
ARGS((SymbTable_ptr symb_table,
      SymbLayer_ptr, node_ptr, node_ptr,
      node_ptr, FlatHierarchy_ptr, HrcNode_ptr, hash_ptr));

EXTERN void Compile_ProcessHierarchy ARGS((SymbTable_ptr symb_table,
                                           SymbLayer_ptr layer,
                                           FlatHierarchy_ptr hierachy,
                                           node_ptr name,
                                           boolean create_process_variables, 
                                           boolean calc_vars_constr));

#endif /* __COMPILE_INT_H__ */
