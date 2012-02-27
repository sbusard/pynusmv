/**CFile***********************************************************************

  FileName    [BoolSexpFsm.c]

  PackageName [fsm.sexp]

  Synopsis    [Implementation of class 'BoolSexpFsm']

  Description [This module defines a class representing a boolean
               sexp-based FSM. The class BoolSexpFsm derives from
               SexpFsm.]

  SeeAlso     [BoolSexpFsm.h]

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

  Revision    [$Id: BoolSexpFsm.c,v 1.1.2.4 2010-01-24 16:19:37 nusmv Exp $]

******************************************************************************/

#include "BoolSexpFsm.h"
#include "BoolSexpFsm_private.h"

#include "sexpInt.h"

#include "parser/symbols.h"
#include "utils/utils.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: BoolSexpFsm.c,v 1.1.2.4 2010-01-24 16:19:37 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'BoolSexpFsm_private.h' for class 'BoolSexpFsm' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void bool_sexp_fsm_finalize ARGS((Object_ptr object, void* dummy));

static Object_ptr bool_sexp_fsm_copy ARGS((const Object_ptr object));

static Expr_ptr
bool_sexp_fsm_booleanize_expr ARGS((const BoolSexpFsm_ptr self,
                                    Expr_ptr expr));

static void
bool_sexp_fsm_build_input_state_mask ARGS((BoolSexpFsm_ptr self,
                                           Expr_ptr *input,
                                           Expr_ptr *state));
static boolean
bool_sexp_fsm_set_contains_infinite_variables ARGS((const SymbTable_ptr st,
                                                    const Set_t vars));
/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The BoolSexpFsm class constructor]

  Description        [The BoolSexpFsm class constructor]

  SideEffects        []

  SeeAlso            [BoolSexpFsm_destroy]

******************************************************************************/
BoolSexpFsm_ptr BoolSexpFsm_create(const FlatHierarchy_ptr hierarchy,
                                   const Set_t vars_set,
                                   BddEnc_ptr benc,
                                   SymbLayer_ptr det_layer)
{
  BoolSexpFsm_ptr self = ALLOC(BoolSexpFsm, 1);

  BOOL_SEXP_FSM_CHECK_INSTANCE(self);

  bool_sexp_fsm_init(self, hierarchy, vars_set, benc, det_layer);
  return self;
}



/**Function********************************************************************

  Synopsis           [The BoolSexpFsm class constructor from existing sexp
  fsm which is typically a scalar FSM]

  Description        [The BoolSexpFsm class constructor from existing
  fsm. If the given fsm is already boolean, a copy is returned. If it is
  a scalar FSM, its boolean version is created and returned.]

  SideEffects        []

  SeeAlso            [BoolSexpFsm_destroy]

******************************************************************************/
BoolSexpFsm_ptr
BoolSexpFsm_create_from_scalar_fsm(const SexpFsm_ptr scalar_fsm,
                                   BddEnc_ptr benc, SymbLayer_ptr det_layer)
{
  BoolSexpFsm_ptr self;

  if (SexpFsm_is_boolean(scalar_fsm)) {
    /* the instance is already a BoolSexpFsm */
    return BOOL_SEXP_FSM(SexpFsm_copy(scalar_fsm));
  }

  self = ALLOC(BoolSexpFsm, 1);
  BOOL_SEXP_FSM_CHECK_INSTANCE(self);

  bool_sexp_fsm_init(self, scalar_fsm->hierarchy, scalar_fsm->vars_set,
                     benc, det_layer);
  return self;
}

/**Function********************************************************************

  Synopsis           [The BoolSexpFsm copy constructor]

  Description        [The BoolSexpFsm copy constructor]

  SideEffects        []

  SeeAlso            [BoolSexpFsm_create]

******************************************************************************/
BoolSexpFsm_ptr BoolSexpFsm_copy(BoolSexpFsm_ptr self)
{
  BOOL_SEXP_FSM_CHECK_INSTANCE(self);
  return BOOL_SEXP_FSM(Object_copy(OBJECT(self)));
}



/**Function********************************************************************

  Synopsis           [The BoolSexpFsm class destructor]

  Description        [The BoolSexpFsm class destructor]

  SideEffects        []

  SeeAlso            [BoolSexpFsm_create]

******************************************************************************/
VIRTUAL void BoolSexpFsm_destroy(BoolSexpFsm_ptr self)
{
  BOOL_SEXP_FSM_CHECK_INSTANCE(self);
  Object_destroy(OBJECT(self), NULL);
}


/**Function********************************************************************

  Synopsis           [Returns the BoolEnc instance connected to self]

  Description [This method can be called only when a valid BddEnc was
  passed to the class constructor (not NULL). Returned instance do not
  belongs to the caller and must _not_ be destroyed]

  SideEffects        []

******************************************************************************/
BoolEnc_ptr BoolSexpFsm_get_bool_enc(const BoolSexpFsm_ptr self)
{
  BOOL_SEXP_FSM_CHECK_INSTANCE(self);
  return BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self->enc));
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The BoolSexpFsm class private initializer]

  Description        [The BoolSexpFsm class private initializer]

  SideEffects        []

  SeeAlso            [BoolSexpFsm_create]

******************************************************************************/
void bool_sexp_fsm_init(BoolSexpFsm_ptr self,
                        const FlatHierarchy_ptr hierarchy,
                        const Set_t vars_set,
                        BddEnc_ptr enc, SymbLayer_ptr det_layer)
{
  FlatHierarchy_ptr fh;
  int curr_verbosity;
  Expr_ptr inputs_mask = Expr_true();
  Expr_ptr states_mask = Expr_true();
  SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(enc));

  if (bool_sexp_fsm_set_contains_infinite_variables(st, vars_set)) {
    rpterr("Impossible to build a boolean FSM"
           " with infinite precision variables");
  }

  /* base class initialization. Here sele is made a copy of the
     scalar fsm is done. Later on the copy will be booleanized. */
  sexp_fsm_init(SEXP_FSM(self), hierarchy, vars_set);

  /* local members */
  self->enc = enc;
  self->det_layer = det_layer;

  /* here we get the hierarchy instead of using the one passed as
     formal parameter, as we need to use the local copy within the
     base class */
  fh = SexpFsm_get_hierarchy(SEXP_FSM(self));

  /* marks the sexp fsm to be a boolean fsm */
  SEXP_FSM(self)->is_boolean = true;

  /* We set the verbose level to 0 and then we restore the original
     value. This because booleanization uses eval */
  curr_verbosity = get_verbose_level(OptsHandler_get_instance());
  set_verbose_level(OptsHandler_get_instance(), 0);

  /*
     input and state mask for the FSM are computed and added to the
     model: input mask is added to the transition relation, while
     state mask is added to the invar.
  */
  bool_sexp_fsm_build_input_state_mask(self, &inputs_mask, &states_mask);

  /* here the flat hierarchy gets booleanized */

  /* init */
  FlatHierarchy_set_init(fh, bool_sexp_fsm_booleanize_expr(self,
                                         FlatHierarchy_get_init(fh)));

  /* invar */
  FlatHierarchy_set_invar(fh,
                          Expr_and(states_mask,
                                   bool_sexp_fsm_booleanize_expr(self,
                                         FlatHierarchy_get_invar(fh))));

  /* trans */
  FlatHierarchy_set_trans(fh,
                          Expr_and(inputs_mask,
                                   bool_sexp_fsm_booleanize_expr(self,
                                         FlatHierarchy_get_trans(fh))));

  /* justice */
  FlatHierarchy_set_justice(fh,
                            bool_sexp_fsm_booleanize_expr(self,
                                         FlatHierarchy_get_justice(fh)));

  /* compassion */
  FlatHierarchy_set_compassion(fh,
                               bool_sexp_fsm_booleanize_expr(self,
                                         FlatHierarchy_get_compassion(fh)));

  /* restores the verbosity level */
  set_verbose_level(OptsHandler_get_instance(), curr_verbosity);

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = bool_sexp_fsm_finalize;
  OVERRIDE(Object, copy)     = bool_sexp_fsm_copy;
}


/**Function********************************************************************

  Synopsis           [The BoolSexpFsm class private deinitializer]

  Description        [The BoolSexpFsm class private deinitializer]

  SideEffects        []

  SeeAlso            [BoolSexpFsm_destroy]

******************************************************************************/
void bool_sexp_fsm_deinit(BoolSexpFsm_ptr self)
{
  /* members deinitialization */

  /* base class deinitialization */
  sexp_fsm_deinit(SEXP_FSM(self));
}


/**Function********************************************************************

  Synopsis           [The BoolSexpFsm class private deinitializer]

  Description        [The BoolSexpFsm class private deinitializer]

  SideEffects        []

  SeeAlso            [BoolSexpFsm_destroy]

******************************************************************************/
void bool_sexp_fsm_copy_aux(const BoolSexpFsm_ptr self, BoolSexpFsm_ptr copy)
{
  /* copies the base class: */
  sexp_fsm_copy_aux(SEXP_FSM(self), SEXP_FSM(copy));

  /* copies private members */
  copy->enc = self->enc;
  copy->det_layer = self->det_layer;

  /* copies local virtual methods */
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis      [This is called by the virtual copy constructor]

  Description   []

  SideEffects   []

******************************************************************************/
static Object_ptr bool_sexp_fsm_copy(const Object_ptr object)
{
  BoolSexpFsm_ptr self = BOOL_SEXP_FSM(object);
  BoolSexpFsm_ptr copy;

  BOOL_SEXP_FSM_CHECK_INSTANCE(self);

  copy = ALLOC(BoolSexpFsm, 1);
  BOOL_SEXP_FSM_CHECK_INSTANCE(copy);

  bool_sexp_fsm_copy_aux(self, copy);
  return OBJECT(copy);
}


/**Function********************************************************************

  Synopsis    [The BoolSexpFsm class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void bool_sexp_fsm_finalize(Object_ptr object, void* dummy)
{
  BoolSexpFsm_ptr self = BOOL_SEXP_FSM(object);

  bool_sexp_fsm_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis [Booleanizes the given expression, keeping each top level
  part of a possible conjuction]

  Description [If the fsm is not boolean, the input expression is
  returned]

  SideEffects        []

******************************************************************************/
static Expr_ptr bool_sexp_fsm_booleanize_expr(BoolSexpFsm_ptr self,
                                              Expr_ptr expr)
{
  Expr_ptr result;

  if (expr == NODE_PTR(NULL)) return NODE_PTR(NULL);

  switch (node_get_type(NODE_PTR(expr))) {
  case AND:
    {
      Expr_ptr left  = bool_sexp_fsm_booleanize_expr(self, car(NODE_PTR(expr)));
      Expr_ptr right = bool_sexp_fsm_booleanize_expr(self, cdr(NODE_PTR(expr)));
      result = Expr_and(left, right);
      break;
    }

  default:
    result = EXPR(Compile_expr2bexpr(self->enc, self->det_layer,
                                     NODE_PTR(expr)));
  } /* switch */

  return result;
}


/**Function********************************************************************

  Synopsis [Computes the mask for the variables of the FSM.]

  Description [For the variables in the vars_set of the FSM computes the
  mask and accumulate them into input or state depending on the kind
  of the variable being respectively input or state.]

  SideEffects        []

******************************************************************************/
static void bool_sexp_fsm_build_input_state_mask(BoolSexpFsm_ptr self,
                                                 Expr_ptr *input,
                                                 Expr_ptr *state) {
  Set_t vars = SexpFsm_get_vars(SEXP_FSM(self));
  node_ptr var, mask;
  Set_Iterator_t iter;
  SymbTable_ptr st = SexpFsm_get_symb_table(SEXP_FSM(self));
  BoolEnc_ptr bool_enc = BoolSexpFsm_get_bool_enc(self);

  SET_FOREACH(vars, iter) {
    var = Set_GetMember(vars, iter);

    if (SymbTable_is_symbol_state_var(st, var)) {
      mask = BoolEnc_get_var_mask(bool_enc, var);
      *state = Expr_and(*state, mask);
    }
    else if (SymbTable_is_symbol_input_var(st, var)) {
      mask = BoolEnc_get_var_mask(bool_enc, var);
      *input = Expr_and(*input, mask);
    }
  }
#if BOOL_FSM_DEBUG_MASK
  fprintf(nusmv_stderr, "Input mask is: ");
  print_node(nusmv_stderr, *input);
  fprintf(nusmv_stderr, "\n");
  fprintf(nusmv_stderr, "State mask is: ");
  print_node(nusmv_stderr, *state);
  fprintf(nusmv_stderr, "\n");
#endif
}

/**Function********************************************************************

   Synopsis           [Checks if the given set of variables contains at least
                       one infinite precision variable]

   Description        [Checks if the given set of variables contains at least
                       one infinite precision variable]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean
bool_sexp_fsm_set_contains_infinite_variables(const SymbTable_ptr st,
                                              const Set_t vars)
{
  Set_Iterator_t iter;

  SET_FOREACH(vars, iter) {
    node_ptr var = Set_GetMember(vars, iter);
    SymbType_ptr type;

    nusmv_assert(SymbTable_is_symbol_var(st, var));

    type = SymbTable_get_var_type(st, var);

    if (SymbType_is_infinite_precision(type)) {
      return true;
    }
  }

  return false;
}

/**AutomaticEnd***************************************************************/

