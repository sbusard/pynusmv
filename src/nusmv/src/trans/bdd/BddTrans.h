/**CHeaderFile*****************************************************************

  FileName    [BddTrans.h]

  PackageName [trans.bdd]

  Synopsis    [The header file of the class BddTrans]

  Description [ The package <tt> trans.bdd </tt> implements 
  classes to store and maipulate transition relation in bdd form]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``trans.bdd'' package of NuSMV version 2. 
  Copyright (C) 2003 by FBK-irst. 

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

  Revision    [$Id: BddTrans.h,v 1.1.2.3.4.2.6.1 2007-04-30 16:41:18 nusmv Exp $]

******************************************************************************/

#ifndef __TRANS_BDD_BDD_TRANS_H__
#define __TRANS_BDD_BDD_TRANS_H__

#include "trans/generic/GenericTrans.h"

#include "ClusterList.h"
#include "ClusterOptions.h"

#include "utils/utils.h" /* for EXTERN and ARGS */
#include "dd/dd.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/**Type***********************************************************************

  Synopsis    [The structure used to represent the transition relation.]

  Description []

******************************************************************************/
typedef struct BddTrans_TAG* BddTrans_ptr;

#define BDD_TRANS(x)  \
        ((BddTrans_ptr) x)

#define BDD_TRANS_CHECK_INSTANCE(x)  \
        (nusmv_assert(BDD_TRANS(x) != BDD_TRANS(NULL)))


/**Type***********************************************************************

  Synopsis    [This is enumeration of possible kinds of image computations]

  Description [Image computation can be done forward or backward. In
  both cases it is possible to leave only state or state-input
  variables. For example, TRANS_IMAGE_FORWARD_STATE is the kind
  referring to forward image which returns state variables only,
  i.e. with input variables abstracted away.

  Use macros TRANS_IMAGE_IS_FORWARD and TRANS_IMAGE_IS_STATE_ONLY
  to detect the class of kinds.]

******************************************************************************/
typedef enum TransImageKind_TAG {
  TRANS_IMAGE_FORWARD_STATE = 0,
  TRANS_IMAGE_FORWARD_STATE_INPUT = 1,
  TRANS_IMAGE_BACKWARD_STATE = 2,
  TRANS_IMAGE_BACKWARD_STATE_INPUT = 3
} TransImageKind;
 


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
/**Macro***********************************************************************

  Synopsis    [Takes TransImageKind and returns 1 iff the kind
  is one of forward ones.]

  Description [Thus 0 is returned iff the kind is backward one.]

******************************************************************************/
#define TRANS_IMAGE_IS_FORWARD(kind) (!((kind) >> 1))

/**Macro***********************************************************************

  Synopsis    [Takes TransImageKind and returns 1 iff the kind
  is one of image returning states only without inputs.]

  Description [Thus 0 is returned iff the image is to return
  both state and input vars]

******************************************************************************/
#define TRANS_IMAGE_IS_STATE_ONLY(kind) (!((kind) & 1))

/* ---------------------------------------------------------------------- */
/* Enable this to auto-check trans after creation                         */
#if 0
# define TRANS_DEBUG_THRESHOLD  
#endif
/* ---------------------------------------------------------------------- */

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/


EXTERN BddTrans_ptr 
BddTrans_create ARGS((DdManager* dd_manager, 
                   const ClusterList_ptr clusters_bdd, 
                   bdd_ptr state_vars_cube, 
                   bdd_ptr input_vars_cube, 
                   bdd_ptr next_state_vars_cube,                               
                   const TransType trans_type, 
                   const ClusterOptions_ptr cl_options));

EXTERN BddTrans_ptr 
BddTrans_generic_create ARGS((
                   void* transition,
                   void* (*copy)(void* tranistion),
                   void  (*destroy)(void* tranistion),
                   bdd_ptr (*compute_image)(void* tranistion,
                                             bdd_ptr bdd, TransImageKind kind),
                   bdd_ptr (*compute_k_image)(void* tranistion,
                                              bdd_ptr bdd, int k,
                                              TransImageKind kind),
                   bdd_ptr (*get_monolithic_bdd)(void* tranistion),
                   void (*synchronous_product)(void* tranistion1,
                                               void* const transition2),
                   void (*print_short_info)(void* tranistion, FILE* file)
                              ));

EXTERN void 
BddTrans_apply_synchronous_product ARGS((BddTrans_ptr self, 
                                         const BddTrans_ptr other));

EXTERN bdd_ptr 
BddTrans_get_monolithic_bdd ARGS((const BddTrans_ptr self));


EXTERN bdd_ptr 
BddTrans_get_forward_image_state ARGS((const BddTrans_ptr self, bdd_ptr s));

EXTERN bdd_ptr 
BddTrans_get_forward_image_state_input ARGS((const BddTrans_ptr self, 
                                             bdd_ptr s));

EXTERN bdd_ptr 
BddTrans_get_backward_image_state ARGS((const BddTrans_ptr self, bdd_ptr s));

EXTERN bdd_ptr 
BddTrans_get_backward_image_state_input ARGS((const BddTrans_ptr self, 
                                              bdd_ptr s));

EXTERN bdd_ptr 
BddTrans_get_k_forward_image_state ARGS((const BddTrans_ptr self, 
                                         bdd_ptr s, int k));

EXTERN bdd_ptr 
BddTrans_get_k_forward_image_state_input ARGS((const BddTrans_ptr self, 
                                               bdd_ptr s, int k));

EXTERN bdd_ptr 
BddTrans_get_k_backward_image_state ARGS((const BddTrans_ptr self, 
                                          bdd_ptr s, int k));

EXTERN bdd_ptr 
BddTrans_get_k_backward_image_state_input ARGS((const BddTrans_ptr self, 
                                                  bdd_ptr s, int k));

EXTERN void BddTrans_print_short_info ARGS((const BddTrans_ptr self, 
                                            FILE* file));



#endif /* __TRANS_BDD_BDD_TRANS_H__ */
