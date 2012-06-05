/**CHeaderFile*****************************************************************

  FileName    [ltl.h]

  PackageName [ltl]

  Synopsis    [Routines to handle with LTL model checking.]

  Description [Here we perform the reduction of LTL model checking to
  CTL model checking. The technique adopted has been taken from \[1\].
  <ol>
    <li>O. Grumberg E. Clarke and K. Hamaguchi. "Another Look at LTL Model Checking".
       <em>Formal Methods in System Design</em>, 10(1):57--71, February 1997.</li>
  </ol>
  ]

  SeeAlso     [mc]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``ltl'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and FBK-irst. 

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

  Revision    [$Id: ltl.h,v 1.4.6.2.4.3.6.3 2009-04-06 15:16:19 nusmv Exp $]

******************************************************************************/

#ifndef __LTL_H__
#define  __LTL_H__

#include "utils/utils.h"
#include "node/node.h"
#include "prop/Prop.h"
#include "fsm/sexp/Expr.h"
#include "trace/Trace.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct Ltl_StructCheckLtlSpec_TAG* Ltl_StructCheckLtlSpec_ptr;


typedef node_ptr (*Ltl_StructCheckLtlSpec_oreg2smv) ARGS((unsigned int, node_ptr));
typedef node_ptr (*Ltl_StructCheckLtlSpec_ltl2smv) ARGS((unsigned int, node_ptr));

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define LTL_STRUCTCHECKLTLSPEC(self) \
         ((Ltl_StructCheckLtlSpec_ptr) self)

#define LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self) \
         (nusmv_assert(LTL_STRUCTCHECKLTLSPEC(self) != \
          LTL_STRUCTCHECKLTLSPEC(NULL)))


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void print_ltlspec ARGS((FILE*, Prop_ptr));
EXTERN void Ltl_Init ARGS((void));
EXTERN void Ltl_CheckLtlSpec ARGS((Prop_ptr prop));

EXTERN void 
Ltl_spec_to_hierarchy ARGS((Expr_ptr spec, node_ptr context, 
                            SymbTable_ptr st,
                            node_ptr (*what2smv)(unsigned int id, node_ptr expr),
                            SymbLayer_ptr layer, 
                            FlatHierarchy_ptr outfh));

EXTERN Expr_ptr 
Ltl_apply_input_vars_rewriting ARGS((Expr_ptr spec, SymbTable_ptr st, 
                                     SymbLayer_ptr layer, 
                                     FlatHierarchy_ptr outfh));
  
EXTERN Ltl_StructCheckLtlSpec_ptr Ltl_StructCheckLtlSpec_create ARGS((Prop_ptr prop));
EXTERN void Ltl_StructCheckLtlSpec_destroy ARGS((Ltl_StructCheckLtlSpec_ptr self));

EXTERN void Ltl_StructCheckLtlSpec_set_oreg2smv ARGS((Ltl_StructCheckLtlSpec_ptr self,
                                                      Ltl_StructCheckLtlSpec_oreg2smv oreg2smv));
EXTERN void Ltl_StructCheckLtlSpec_set_ltl2smv ARGS((Ltl_StructCheckLtlSpec_ptr self,
                                                     Ltl_StructCheckLtlSpec_ltl2smv ltl2smv));
EXTERN void Ltl_StructCheckLtlSpec_set_negate_formula ARGS((Ltl_StructCheckLtlSpec_ptr self,
                                                            boolean negate_formula));
EXTERN void Ltl_StructCheckLtlSpec_set_do_rewriting ARGS((Ltl_StructCheckLtlSpec_ptr self,
                                                         boolean do_rewriting));
EXTERN bdd_ptr Ltl_StructCheckLtlSpec_get_s0 ARGS((Ltl_StructCheckLtlSpec_ptr self));
EXTERN bdd_ptr Ltl_StructCheckLtlSpec_get_clean_s0 ARGS((Ltl_StructCheckLtlSpec_ptr self));
EXTERN void Ltl_StructCheckLtlSpec_build ARGS((Ltl_StructCheckLtlSpec_ptr self));
EXTERN void Ltl_StructCheckLtlSpec_check ARGS((Ltl_StructCheckLtlSpec_ptr self));
EXTERN void Ltl_StructCheckLtlSpec_print_result ARGS((Ltl_StructCheckLtlSpec_ptr self));
EXTERN Trace_ptr
Ltl_StructCheckLtlSpec_build_counter_example ARGS((Ltl_StructCheckLtlSpec_ptr self,
                                                   NodeList_ptr symbols));
EXTERN void
Ltl_StructCheckLtlSpec_explain ARGS((Ltl_StructCheckLtlSpec_ptr self,
                                     NodeList_ptr symbols));

#endif /*  __LTL_H__ */
