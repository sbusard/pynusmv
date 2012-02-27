/**CFile***********************************************************************

  FileName    [Prop.c]

  PackageName [prop]

  Synopsis    [Implementation of class 'Prop']

  Description []

  SeeAlso     [Prop.h]

  Author      [Marco Roveri, Roberto Cavada]

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

#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "Prop.h"
#include "Prop_private.h"
#include "propInt.h"
#include "PropDb.h"
#include "propPkg.h"

#include "mc/mc.h"
#include "ltl/ltl.h"
#include "parser/symbols.h"
#include "parser/psl/pslNode.h"
#include "compile/compile.h"

#include "utils/utils.h"
#include "utils/utils_io.h"

#include "trace/pkg_trace.h"
#include "trace/exec/PartialTraceExecutor.h"
#include "trace/exec/BDDPartialTraceExecutor.h"
#include "trace/exec/SATPartialTraceExecutor.h"
#include "trace/exec/CompleteTraceExecutor.h"
#include "trace/exec/BDDCompleteTraceExecutor.h"
#include "trace/exec/SATCompleteTraceExecutor.h"

#include <string.h>


static char rcsid[] UTIL_UNUSED = "$Id: $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'Prop_private.h' for class 'Prop' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/* Used to encode that a property MIN/MAX has not yet been checked. */
#define PROP_UNCHECKED -2

/* Used to encode the infinite distanca between two set of states in
   MIN/MAX properties */
#define PROP_INFINITE -1
#define PROP_UNDEFINED -3


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void prop_finalize ARGS((Object_ptr object, void* dummy));
static Expr_ptr prop_get_expr_core_for_coi ARGS((const Prop_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The Prop class constructor]

  Description        [Allocate a property. If no more room is available
                      then a call to <tt>numsv_exit</tt> is
                      performed. All the fields of the prop
                      structure are initialized to either NULL or
                      the corresponding default type
                      (e.g. Prop_NoType for property type).]

  SideEffects        []

  SeeAlso            [Prop_destroy]

******************************************************************************/
Prop_ptr Prop_create()
{
  Prop_ptr self = ALLOC(Prop, 1);
  PROP_CHECK_INSTANCE(self);

  prop_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [Creates a property, but does not insert it within the
                      database, so the property can be used on the
                      fly.]

  Description        [Creates a property structure filling only the
                      property and property type fields. The
                      property index within the db is not set.]

  SideEffects        []

******************************************************************************/
Prop_ptr Prop_create_partial(Expr_ptr expr, Prop_Type type)
{
  Prop_ptr self = Prop_create();
  PROP_CHECK_INSTANCE(self);

  self->index = -1;
  self->status = Prop_Unchecked;
  self->prop = expr;
  self->type = type;

  return self;
}


/**Function********************************************************************

  Synopsis           [The Prop class destructor]

  Description        [Free a property. Notice that before freeing the
                      property all the elements of the property
                      that needs to be freed will be automatically
                      freed.]

  SideEffects        []

  SeeAlso            [Prop_create]

******************************************************************************/
void Prop_destroy(Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  Object_destroy(OBJECT(self), NULL);
}


/**Function********************************************************************

  Synopsis [Returns the property as it has been parsed and created]

  Description [Returns the property stored in the prop. If the
  property is PSL, the result should be converted to core symbols
  before model checking (see Prop_get_expr_core or
  PslNode_convert_psl_to_core).]

  SideEffects        []

  SeeAlso            [Prop_get_expr_core]

******************************************************************************/
VIRTUAL Expr_ptr Prop_get_expr(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->get_expr(self);
}


/**Function********************************************************************

  Synopsis           [Returns the property, but it is converted before in
                      terms of core symbols.]

  Description        [Returns the property in a form that it can be
                      handled by the system (model checking,
                      dependency finder, etc.).  This may imply a
                      conversion and a different structure of the
                      resulting formula. For example in PSL FORALLs
                      are expanded, SERE are removed, global
                      operators G and AG are simplified, etc.

                      Use this function at system-level, and
                      Prop_get_expr to get the original formula instead]

  SideEffects        []

  SeeAlso            [Prop_get_expr]

******************************************************************************/
Expr_ptr Prop_get_expr_core(const Prop_ptr self)
{
  Expr_ptr res;

  PROP_CHECK_INSTANCE(self);

  /* PSL formulae are converted to SMV LTL or CTL: */
  if (Prop_get_type(self) == Prop_Psl) {
    res = PslNode_convert_psl_to_core(Prop_get_expr(self));
  }
  else res = Prop_get_expr(self); /* usual expression */

  return Compile_pop_distrib_ops(res);
}


/**Function********************************************************************

  Synopsis           []

  Description        [Derived from Prop_get_expr_core, but for PSL only
                      removes forall replicators rather than
                      converting the whole expression into LTL. ]

  SideEffects        [prop_get_expr_core_for_coi]

******************************************************************************/
Expr_ptr Prop_get_expr_core_for_coi(const Prop_ptr self)
{
  return prop_get_expr_core_for_coi(self);
}


/**Function********************************************************************

  Synopsis           [Returns the cone of a property]

  Description        [If the cone of influence of a property has been
                      computed, this function returns it.]

  SideEffects        []

******************************************************************************/
Set_t Prop_get_cone(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);

  return self->cone;
}


/**Function********************************************************************

  Synopsis           [Sets the cone of a property]

  Description        [Stores the cone of influence of the property]

  SideEffects        []

******************************************************************************/
void Prop_set_cone(Prop_ptr self, Set_t cone)
{
  PROP_CHECK_INSTANCE(self);

  self->cone = cone;
}


/**Function********************************************************************

  Synopsis           [Returns the property type]

  Description        [Returns the property kind of the stroed
  property, i.e. CTL, LTL, ...]

  SideEffects        []

******************************************************************************/
Prop_Type Prop_get_type(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);

  return self->type ;
}


/**Function********************************************************************

  Synopsis           [Returns the status of the property]

  Description        [Returns the status of the property]

  SideEffects        []

******************************************************************************/
Prop_Status Prop_get_status(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);

  return self->status;
}

/**Function********************************************************************

  Synopsis           [Sets the status of the property]

  Description        [Sets the status of the property]

  SideEffects        []

******************************************************************************/
void Prop_set_status(Prop_ptr self, Prop_Status s)
{
  PROP_CHECK_INSTANCE(self);
  self->status = s;
}


/**Function********************************************************************

  Synopsis           [Returns the number of the property]

  Description        [For COMPUTE properties returns the number resulting
                      from the evaluation of the property.]

  SideEffects        []

******************************************************************************/
int Prop_get_number(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->number;
}


/**Function********************************************************************

  Synopsis           [Sets the number of the property]

  Description        [Sets the number resulting from the
                      evaluation of the property.]

  SideEffects        []

******************************************************************************/
void Prop_set_number(Prop_ptr self, int n)
{
  PROP_CHECK_INSTANCE(self);
  self->number = n;
}


/**Function********************************************************************

  Synopsis           [Sets the number of the property to INFINITE]

  Description        [Sets the to INFINITE the number resulting from the
                      evaluation of the property.]

  SideEffects        []

******************************************************************************/
void Prop_set_number_infinite(Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  self->number = PROP_INFINITE;
}


/**Function********************************************************************

  Synopsis           [Sets the number of the property to UNDEFINED]

  Description        [Sets the to UNDEFINED the number resulting from the
                      evaluation of the property.]

  SideEffects        []

******************************************************************************/
void Prop_set_number_undefined(Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  self->number = PROP_UNDEFINED;
}


/**Function********************************************************************

  Synopsis           [Returns the trace number associated to a property]

  Description        [For unsatisfied properties, the trace number of the
                      asscociated counterexample is returned. 0 is
                      returned if no trace is available]

  SideEffects        []

******************************************************************************/
int Prop_get_trace(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->trace;
}


/**Function********************************************************************

  Synopsis           [Sets the trace number]

  Description        [Sets the trace number for an unsatisfied property.]

  SideEffects        []

******************************************************************************/
void Prop_set_trace(Prop_ptr self, int t)
{
  PROP_CHECK_INSTANCE(self);
  self->trace = t;
}


/**Function********************************************************************

  Synopsis           [Returns the index of a property]

  Description        [Returns the unique identifier of a property]

  SideEffects        []

******************************************************************************/
int Prop_get_index(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->index;
}


/**Function********************************************************************

  Synopsis           [Sets the index of a property]

  Description        [Sets the unique identifier of a property]

  SideEffects        []

******************************************************************************/
void Prop_set_index(Prop_ptr self, const int index)
{
  PROP_CHECK_INSTANCE(self);
  self->index = index;
}


/**Function********************************************************************

  Synopsis           [Gets the name of a property]

  Description        [Get the property name]

  SideEffects        []

******************************************************************************/
node_ptr Prop_get_name(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return (self->name);
}


/**Function********************************************************************

  Synopsis           [Gets the name of a property as a string]

  Description        [Get the property name as a string, must be freed]

  SideEffects        []

******************************************************************************/
char* Prop_get_name_as_string(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);

  return sprint_node(self->name);
}



/**Function********************************************************************

  Synopsis           [Sets the name of a property]

  Description        [Sets the name of a property]

  SideEffects        []

******************************************************************************/
void Prop_set_name(const Prop_ptr self, const node_ptr name)
{
  PROP_CHECK_INSTANCE(self);
  self->name = name;
}


/**Function********************************************************************

  Synopsis    [Computes ground scalar sexp fsm for property \"self\"]

  Description []

  SideEffects [Ground sexp fsm is computed (taking COI into account if
  needed) and registered into self.]

******************************************************************************/
SexpFsm_ptr Prop_compute_ground_sexp_fsm (const Prop_ptr self,
                                          const FsmBuilder_ptr builder,
                                          const SymbTable_ptr symb_table)
{
  SexpFsm_ptr res = SEXP_FSM(NULL);

  /* setup ground FSM taking COI into account */
  if (opt_cone_of_influence(OptsHandler_get_instance())) {
    Prop_apply_coi_for_scalar(self, builder, mainFlatHierarchy, symb_table);
  }

  if (SEXP_FSM(NULL) == Prop_get_scalar_sexp_fsm(self)) {
    PropDb_set_fsm_to_master(PropPkg_get_prop_database(), self);
  }

  /* at this point a private ground FSM exists */
  res = Prop_get_scalar_sexp_fsm(self);
  SEXP_FSM_CHECK_INSTANCE(res);

  return res;
}


/**Function********************************************************************

  Synopsis    [Computes ground bdd fsm for property \"self\"]

  Description []

  SideEffects [Ground bdd fsm is computed (taking COI into account if
  needed) and registered into self.]

******************************************************************************/
BddFsm_ptr Prop_compute_ground_bdd_fsm (const Prop_ptr self,
                                        const FsmBuilder_ptr builder)
{
  BddFsm_ptr res = BDD_FSM(NULL);

  /* setup ground FSM taking COI into account */
  if (opt_cone_of_influence(OptsHandler_get_instance()) == true) {
    Prop_apply_coi_for_bdd(self, builder);
  }

  if (BDD_FSM(NULL) == Prop_get_bdd_fsm(self)) {
    PropDb_set_fsm_to_master(PropPkg_get_prop_database(), self);
  }

  /* at this point a private ground FSM exists */
  res = Prop_get_bdd_fsm(self);
  BDD_FSM_CHECK_INSTANCE(res);

  return res;
}


/**Function********************************************************************

  Synopsis    [Computes ground be fsm for property \"self\"]

  Description [Ground be fsm is computed (taking COI into account if
  needed) and registered into self.]

  SideEffects   []

******************************************************************************/
BeFsm_ptr Prop_compute_ground_be_fsm (const Prop_ptr self,
                                      const FsmBuilder_ptr builder)
{
  BeFsm_ptr res = BE_FSM(NULL);

  /* setup ground FSM taking COI into account */
  if (opt_cone_of_influence(OptsHandler_get_instance()) == true) {
    Prop_apply_coi_for_bmc(self, builder);
  }

  if (BE_FSM(NULL) == Prop_get_be_fsm(self)) {
    PropDb_set_fsm_to_master(PropPkg_get_prop_database(), self);
  }

  /* at this point a private ground FSM exists */
  res = Prop_get_be_fsm(self);
  BE_FSM_CHECK_INSTANCE(res);

  return res;
}


/**Function********************************************************************

  Synopsis           [Returns the scalar FSM of a property]

  Description        [Returns the scalar FSM associated to the
                      property. Self keeps the ownership of the
                      given fsm]

  SideEffects        []

******************************************************************************/
SexpFsm_ptr Prop_get_scalar_sexp_fsm(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->scalar_fsm;
}


/**Function********************************************************************

  Synopsis           [Sets the scalar FSM of a property]

  Description        [The given fsm will be duplicated, so the caller keeps
                      the ownership of fsm]

  SideEffects        []

******************************************************************************/
void Prop_set_scalar_sexp_fsm(Prop_ptr self, SexpFsm_ptr fsm)
{
  PROP_CHECK_INSTANCE(self);
  prop_set_scalar_sexp_fsm(self, fsm, true);
}


/**Function********************************************************************

  Synopsis           [Returns the boolean FSM of a property]

  Description        [Returns the boolean FSM associated to the
                      property. Self keeps the ownership of the
                      given fsm]

  SideEffects        []

******************************************************************************/
BoolSexpFsm_ptr Prop_get_bool_sexp_fsm(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->bool_fsm;
}


/**Function********************************************************************

  Synopsis           [Sets the boolean FSM of a property]

  Description        [The given fsm will be duplicated, so the caller
                      keeps the ownership of fsm]

  SideEffects        []

******************************************************************************/
void Prop_set_bool_sexp_fsm(Prop_ptr self, BoolSexpFsm_ptr fsm)
{
  PROP_CHECK_INSTANCE(self);
  prop_set_bool_sexp_fsm(self, fsm, true);
}


/**Function********************************************************************

  Synopsis           [Returns the BDD FSM of a property]

  Description        [Returns the BDD FSM associated to the property. Self
                      keeps the ownership of the given fsm]

  SideEffects        []

******************************************************************************/
BddFsm_ptr Prop_get_bdd_fsm(Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->bdd_fsm;
}


/**Function********************************************************************

  Synopsis           [Sets the boolean FSM in BDD of a property]

  Description        [The given fsm will be duplicated, so the caller
                      keeps the ownership of fsm]

  SideEffects        []

******************************************************************************/
void Prop_set_bdd_fsm(Prop_ptr self, BddFsm_ptr fsm)
{
  PROP_CHECK_INSTANCE(self);
  prop_set_bdd_fsm(self, fsm, true);
}


/**Function********************************************************************

  Synopsis           [Returns the BE FSM  of a property]

  Description        [Returns the boolean BE FSM associated to the
                      property. Self keeps the ownership of the
                      given fsm]

  SideEffects        []

******************************************************************************/
BeFsm_ptr Prop_get_be_fsm(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->be_fsm;
}


/**Function********************************************************************

  Synopsis           [Sets the boolean BE FSM of a property]

  Description        [The given fsm will be duplicated, so the caller keeps
                      the ownership of fsm]

  SideEffects        []

******************************************************************************/
void Prop_set_be_fsm(Prop_ptr self, BeFsm_ptr fsm)
{
  PROP_CHECK_INSTANCE(self);
  prop_set_be_fsm(self, fsm, true);
}


/**Function********************************************************************

  Synopsis           [ Check if the given property needs rewriting to be
                       checked ]

  Description        [ Returns true if the property needs rewriting,
                       false otherwise]

  SideEffects        []

******************************************************************************/
boolean Prop_needs_rewriting(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);

  if (Prop_Invar == Prop_get_type(self)) {
    Set_t cone;
    boolean result;
    SymbTable_ptr st;
    node_ptr expression;

    st = Compile_get_global_symb_table();
    expression = Compile_FlattenSexpExpandDefine(st, Prop_get_expr(self), Nil);

    cone = Formula_GetDependenciesByType(st,
                                         expression,
                                         Nil,
                                         VFT_NEXT | VFT_INPUT,
                                         true);

    /* If there are next or input then return true */
    result = !Set_IsEmpty(cone);
    Set_ReleaseSet(cone);

    return result;
  }

  return false;
}


/**Function********************************************************************

  Synopsis           [Computes the COI for the given property]

  Description        [Computes the COI for the given property.
                      The caller should free the returned set]

  SideEffects        []

******************************************************************************/
Set_t Prop_compute_cone(Prop_ptr self,
                        FlatHierarchy_ptr hierarchy,
                        SymbTable_ptr symb_table)
{
  Set_t cone;
  PROP_CHECK_INSTANCE(self);

  /* The point here is not to apply this to game properties. Hence, in
     general, this may be too restrictive and might have to be
     removed. */
  nusmv_assert(Prop_Prop_Type_First < Prop_get_type(self) &&
               Prop_Prop_Type_Last > Prop_get_type(self));

  {
    /* Here it is sufficient to call the prop_get_expr_core_for_coi
       that just remopved forall replicator instead of calling the
       Prop_get_expr that also try to convert the expression in LTL if
       possible and rises an error if the conversion is not
       possible */
    node_ptr spec = prop_get_expr_core_for_coi(self);

    Set_t spec_dep = Formulae_GetDependencies(symb_table, spec,
                              FlatHierarchy_get_justice(hierarchy),
                              FlatHierarchy_get_compassion(hierarchy));

    cone = ComputeCOI(symb_table, spec_dep);
  }

  return cone;
}


/**Function********************************************************************

  Synopsis           [Applies cone of influence to the given property]

  Description        [The COI is applied only on the scalar FSM]

  SideEffects        [Internal Scalar FSM is computed]

******************************************************************************/
void Prop_apply_coi_for_scalar(Prop_ptr self, FsmBuilder_ptr helper,
                               FlatHierarchy_ptr hierarchy,
                               SymbTable_ptr symb_table)
{
  PROP_CHECK_INSTANCE(self);
  SexpFsm_ptr scalar_fsm;

  /* The point here is not to apply this to game properties. Hence, in
     general, this may be too restrictive and might have to be
     removed. */
  nusmv_assert(Prop_Prop_Type_First < Prop_get_type(self) &&
               Prop_Prop_Type_Last > Prop_get_type(self));

  /* scalar sexp fsm */
  {
    Set_t cone = Prop_compute_cone(self, hierarchy, symb_table);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr, "Using cone of influence\n");
    }

    scalar_fsm = FsmBuilder_create_scalar_sexp_fsm(helper, hierarchy, cone);

    Prop_set_cone(self, cone);
    prop_set_scalar_sexp_fsm(self, scalar_fsm, false); /* does not dup */

  }
}


/**Function********************************************************************

  Synopsis           [Applies cone of influence to the given property]

  Description        [The COI is applied only for BDD-based model
                      checking.  To apply for BMC, use
                      Prop_apply_coi_for_bmc. If psl2core is false,
                      then the PSL property is only expanded to
                      remove forall, otherwise it is converted into LTL.]

  SideEffects        [Internal FSMs are computed]

******************************************************************************/
void Prop_apply_coi_for_bdd(Prop_ptr self, FsmBuilder_ptr helper)
{
  SexpFsm_ptr scalar_fsm;
  BddFsm_ptr  bdd_fsm;
  SymbTable_ptr symb_table;
  boolean applied = false;


  PROP_CHECK_INSTANCE(self);

  /* The point here is not to apply this to game properties. Hence, in
     general, this may be too restrictive and might have to be
     removed. */
  nusmv_assert(Prop_Prop_Type_First < Prop_get_type(self) &&
               Prop_Prop_Type_Last > Prop_get_type(self));

  symb_table = Compile_get_global_symb_table();
  scalar_fsm = Prop_get_scalar_sexp_fsm(self);
  bdd_fsm    = Prop_get_bdd_fsm(self);

  /* scalar sexp fsm */
  if (scalar_fsm == SEXP_FSM(NULL)) {

    Prop_apply_coi_for_scalar(self, helper, mainFlatHierarchy, symb_table);
    scalar_fsm = Prop_get_scalar_sexp_fsm(self);

    applied = true;
  }

  /* bdd fsm */
  if (bdd_fsm == BDD_FSM(NULL)) {
    bdd_fsm = FsmBuilder_create_bdd_fsm(helper, Enc_get_bdd_encoding(),
                                        scalar_fsm,
                                        get_partition_method(OptsHandler_get_instance()));
    prop_set_bdd_fsm(self, bdd_fsm, false); /* does not dup */
    applied = true;
  }

  if (! applied) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr, "Using previously built model for COI...\n");
    }
  }
}


/**Function********************************************************************

  Synopsis           [Applies cone of influence to the given property]

  Description        [The COI is applied only for BMC-based model
                      checking.  To apply for BDD, use
                      Prop_apply_coi_for_bdd. This method creates a
                      new layer for those determinization vars that
                      derives from the booleanization of the fsm
                      deriving from the property cone. That layer
                      will be committed to the BoolEnc and BeEnc
                      encodings only, not to the BddEnc. The newly
                      created layer will be assigned to a name that
                      depends on the property number within the
                      database DbProp. If psl2core is false, then
                      the PSL property is only expanded to remove
                      forall, otherwise it is converted into LTL.]

  SideEffects        [Internal FSMs are computed]

******************************************************************************/
void Prop_apply_coi_for_bmc(Prop_ptr self, FsmBuilder_ptr helper)
{
  BeEnc_ptr be_enc;
  BoolSexpFsm_ptr bool_fsm;
  BeFsm_ptr be_fsm;
  SymbTable_ptr symb_table;

  PROP_CHECK_INSTANCE(self);

  /* The point here is not to apply this to game properties. Hence, in
     general, this may be too restrictive and might have to be
     removed. */
  nusmv_assert(Prop_Prop_Type_First < Prop_get_type(self) &&
               Prop_Prop_Type_Last > Prop_get_type(self));

  be_enc = Enc_get_be_encoding();
  bool_fsm   = Prop_get_bool_sexp_fsm(self);
  be_fsm    = Prop_get_be_fsm(self);
  symb_table = Compile_get_global_symb_table();

  /* boolean sexp fsm */
  if (BOOL_SEXP_FSM(NULL) == bool_fsm) {
    SymbLayer_ptr det_layer;
    int layer_name_dim;
    char* layer_name = (char*) NULL;
    int c;

    Set_t cone = Prop_compute_cone(self, mainFlatHierarchy, symb_table);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr, "Using cone of influence\n");
    }

    layer_name_dim = strlen(DETERM_LAYER_NAME) + 6;
    layer_name = ALLOC(char, layer_name_dim);
    nusmv_assert(layer_name != (char*) NULL);

    c = snprintf(layer_name, layer_name_dim, "%s_%03d",
                 DETERM_LAYER_NAME, Prop_get_index(self));
    SNPRINTF_CHECK(c, layer_name_dim);

    det_layer = SymbTable_create_layer(symb_table, layer_name,
                                       SYMB_LAYER_POS_BOTTOM);

    {  /* commits the layer: */

      /* BddEnc is required as bdds are used when producing
         counter-examples: */
      BddEnc_ptr bdd_enc;
      BoolEnc_ptr bool_enc;

      bool_enc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(be_enc));
      bdd_enc = Enc_get_bdd_encoding();

      /* creates the boolean FSM */
      bool_fsm = FsmBuilder_create_boolean_sexp_fsm(helper,
                                                    mainFlatHierarchy,
                                                    cone,
                                                    Enc_get_bdd_encoding(),
                                                    det_layer);

      BaseEnc_commit_layer(BASE_ENC(bool_enc), layer_name);
      BaseEnc_commit_layer(BASE_ENC(be_enc), layer_name);
      BaseEnc_commit_layer(BASE_ENC(bdd_enc), layer_name);
    }

    Prop_set_cone(self, cone);
    prop_set_bool_sexp_fsm(self, bool_fsm, false); /* does not dup */

    /* creates the BE FSM */
    nusmv_assert(be_fsm == BE_FSM(NULL));

    /* Notice that currently a single variable manager instance
       exists, and it is handled by the BMC package as a public global
       variable. Current implementation is temporary kept in this
       format. */
    be_fsm = BeFsm_create_from_sexp_fsm(be_enc, bool_fsm);
    prop_set_be_fsm(self, be_fsm, false); /* does not dup */

    FREE(layer_name);
  }
  else {
    if (be_fsm == BE_FSM(NULL)) {
      if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
        fprintf(nusmv_stderr, "BeFsm was found unexpectedly to be constructed\n");
      }

      /* For some reason bool fsm is existing, but befsm is not. Make it */
      be_fsm = BeFsm_create_from_sexp_fsm(be_enc, bool_fsm);
      prop_set_be_fsm(self, be_fsm, false); /* does not dup */
    }
    else {
      if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
        fprintf(nusmv_stderr, "Using previously built model for COI...\n");
      }
    }
  }
}


/**Function********************************************************************

  Synopsis           [Cleans up part of the stuff generated by
                      Prop_apply_coi_for_bmc]

  Description        [Removes the layer created by Prop_apply_coi_for_bmc
                      from be_enc, bdd_enc, and bool_enc and
                      destroys layer. Fsms are assumed to be
                      destroyed upon destroying the property.]

  SideEffects        [Prop_apply_coi_for_bmc]

******************************************************************************/
void Prop_destroy_coi_for_bmc(Prop_ptr self)
{
  int layer_name_dim;
  char* layer_name;
  BoolEnc_ptr bool_enc;
  BddEnc_ptr bdd_enc;
  BeEnc_ptr be_enc;
  SymbTable_ptr symb_table;
  int c;

  PROP_CHECK_INSTANCE(self);
  nusmv_assert(Prop_get_bool_sexp_fsm(self) != BOOL_SEXP_FSM(NULL));
  nusmv_assert(Prop_get_be_fsm(self) != BE_FSM(NULL));

  /* The point here is not to apply this to game properties. Hence, in
     general, this may be too restrictive and might have to be
     removed. */
  nusmv_assert(Prop_Prop_Type_First < Prop_get_type(self) &&
               Prop_Prop_Type_Last > Prop_get_type(self));

  layer_name_dim = strlen(DETERM_LAYER_NAME) + 6;
  layer_name = ALLOC(char, layer_name_dim);
  nusmv_assert(layer_name != (char*) NULL);

  c = snprintf(layer_name, layer_name_dim, "%s_%03d",
               DETERM_LAYER_NAME, Prop_get_index(self));
  SNPRINTF_CHECK(c, layer_name_dim);

  be_enc = Enc_get_be_encoding();
  bdd_enc = Enc_get_bdd_encoding();
  bool_enc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(be_enc));
  symb_table = Compile_get_global_symb_table();

  nusmv_assert(BaseEnc_layer_occurs(BASE_ENC(be_enc), layer_name));
  BaseEnc_remove_layer(BASE_ENC(be_enc), layer_name);
  nusmv_assert(BaseEnc_layer_occurs(BASE_ENC(bdd_enc), layer_name));
  BaseEnc_remove_layer(BASE_ENC(bdd_enc), layer_name);
  nusmv_assert(BaseEnc_layer_occurs(BASE_ENC(bool_enc), layer_name));
  BaseEnc_remove_layer(BASE_ENC(bool_enc), layer_name);
  SymbTable_remove_layer(symb_table,
                         SymbTable_get_layer(symb_table, layer_name));

  FREE(layer_name);
}


/**Function********************************************************************

  Synopsis           [Returns the number value as a string (only for compute
  types)]

  Description        [Returns a number, 'Inifinite' or 'Unchecked'. The
                      returned string is dynamically created, and
                      caller must free it.]

  SideEffects        []

******************************************************************************/
char* Prop_get_number_as_string(const Prop_ptr self)
{
  char buf[16];
  char* ret = NULL;
  int n, c = 0;

  PROP_CHECK_INSTANCE(self);

  nusmv_assert(Prop_get_type(self) == Prop_Compute); /* compute type only */

  n = Prop_get_number(self);
  if (n == PROP_UNCHECKED) c = snprintf(buf, 16, "Unchecked");
  else if (n == PROP_INFINITE) c = snprintf(buf, 16, "Infinite");
  else if (n == PROP_UNDEFINED) c = snprintf(buf, 16, "Undefined");
  else c = snprintf(buf, 16, "%d", n);

  SNPRINTF_CHECK(c, 16);

  ret = ALLOC(char, strlen(buf)+sizeof(char));
  nusmv_assert(ret != NULL);

  strcpy(ret, buf);
  return ret;
}


/**Function********************************************************************

  Synopsis           [Returns the context name of a property]

  Description        [If the property has no explicit context, 'Main' will
                      be returned. The returned string must be
                      deleted by the caller.]

  SideEffects        []

******************************************************************************/
char* Prop_get_context_text(const Prop_ptr self)
{
  char* cntx = (char *)NULL;
  char* EMTPY_CONTEXT_STR = "Main";
  node_ptr context;

  PROP_CHECK_INSTANCE(self);

  context = (node_ptr) self->prop;

  if (node_get_type(context) == CONTEXT) {
    context = car(context);
    if (context != Nil) {
      cntx = sprint_node(context);
    }
    else {
      cntx = ALLOC(char, strlen(EMTPY_CONTEXT_STR)+1);
      nusmv_assert(cntx != NULL);
      strcpy(cntx, EMTPY_CONTEXT_STR);
    }
  }
  else {
    cntx = ALLOC(char, strlen(EMTPY_CONTEXT_STR)+1);
    nusmv_assert(cntx != NULL);
    strcpy(cntx, EMTPY_CONTEXT_STR);
  }

  return cntx;
}


/**Function********************************************************************

  Synopsis           [Returns the property text, with no explicit context]

  Description        [The returned string must be deleted by the caller.]

  SideEffects        []

******************************************************************************/
char* Prop_get_text(const Prop_ptr self)
{
  node_ptr p;

  PROP_CHECK_INSTANCE(self);

  p = (node_ptr) Prop_get_expr(self);
  if (node_get_type(p) == CONTEXT) p = cdr(p);  /* skip context */

  return sprint_node(p);
}


/**Function********************************************************************

  Synopsis           [Returns the a string associated to a property type]

  Description        [Returns the string corresponding to a property type
                      for printing it. Returned string must NOT be
                      deleted]

  SideEffects        []

******************************************************************************/
VIRTUAL const char* Prop_get_type_as_string(Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->get_type_as_string(self);
}


/**Function********************************************************************

  Synopsis           [Returns the a string associated to a property status]

  Description        [Returns the string corresponding to a property
                      status for printing it. The caller must NOT
                      free the returned string, dince it is a
                      constant.]

  SideEffects        []

******************************************************************************/
const char* Prop_get_status_as_string(const Prop_ptr self)
{
  char* res = (char*) NULL;
  Prop_Status t;

  PROP_CHECK_INSTANCE(self);

  t = Prop_get_status(self);

  switch (t) {
  case Prop_NoStatus:    res = PROP_NOSTATUS_STRING; break;
  case Prop_Unchecked:   res = PROP_UNCHECKED_STRING; break;
  case Prop_True:        res = PROP_TRUE_STRING; break;
  case Prop_False:       res = PROP_FALSE_STRING; break;
  case Prop_Number:      res = PROP_NUMBER_STRING; break;

  default:  error_unreachable_code(); /* invalid status */
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Check if a property in the database is of a given type]

  Description        [Checks if a property in the database is of a given
                      type.  If the type is correct, value 0 is
                      returned, otherwise an error message is
                      emitted and value 1 is returned.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Prop_check_type(const Prop_ptr self, Prop_Type type)
{
  PROP_CHECK_INSTANCE(self);

  if (Prop_get_type(self) != type) {
    if (Prop_Prop_Type_First < type && Prop_Prop_Type_Last > type) {
      fprintf(nusmv_stderr,
              "Error: specified property type is %s, "
              "but type %s was expected.\n",
              Prop_get_type_as_string(self), PropType_to_string(type));
    } else {
      fprintf(nusmv_stderr,
              "Error: specified property type is %s, "
              "but a different type (%d) was expected.\n",
              Prop_get_type_as_string(self), type);
    }
    return 1;
  }

  return 0;
}


/**Function********************************************************************

  Synopsis           [Verifies a given property]

  Description        [Depending the property, different model checking
                      algorithms are called. The status of the
                      property is updated accordingly to the result
                      of the verification process.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
VIRTUAL void Prop_verify(Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  self->verify(self);
}


/**Function********************************************************************

  Synopsis           [Prints a property]

  Description        [Prints a property.  PSL properties are specially
  handled.]

  SideEffects        []

******************************************************************************/
VIRTUAL void Prop_print(const Prop_ptr self, FILE* file, Prop_PrintFmt fmt)
{
  PROP_CHECK_INSTANCE(self);

  switch (fmt) {
  case PROP_PRINT_FMT_NAME:
    if (Nil != self->name) {
      fprintf(file, "'");
      print_node(file, self->name);
      fprintf(file, "' ");
      break;
    }
    /* Else default on next */

  case PROP_PRINT_FMT_INDEX:
    if (-1 != self->index) {
      fprintf(file, "[%d] ", self->index);
      break;
    }
    /* Else default on next */

  case PROP_PRINT_FMT_FORMULA_TRUNC:
    self->print_truncated(self, file);
    break;

  case PROP_PRINT_FMT_FORMULA:
    self->print(self, file);
    break;

  default: error_unreachable_code();
  }
}


/**Function********************************************************************

  Synopsis           [Prints a property with info or its position and status
                      within the database]

  Description        [Prints a property on the specified FILE stream. Some
                      of the information stored in the property
                      structure are printed out (e.g. property,
                      property kind, property status, ...).

                      The property is printed in the given format. Use
                      PROPDB_PRINT_FMT_DEFAULT for a default format.]

  SideEffects        []

******************************************************************************/
VIRTUAL void Prop_print_db(const Prop_ptr self, FILE* file,
                           PropDb_PrintFmt fmt)
{
  PROP_CHECK_INSTANCE(self);

  switch (fmt) {
  case PROPDB_PRINT_FMT_TABULAR:
    self->print_db_tabular(self, file);
    break;
  case PROPDB_PRINT_FMT_XML:
    self->print_db_xml(self, file);
    break;
  default:
    internal_error("Unsupported print format");
  }
}


/**Function********************************************************************

  Synopsis           [Returns true if the property is PSL property and it
  is LTL compatible]

  Description        []

  SideEffects        []

******************************************************************************/
boolean Prop_is_psl_ltl(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return (Prop_get_type(self) == Prop_Psl) &&
    PslNode_is_handled_psl(PslNode_remove_forall_replicators(self->prop));
}

/**Function********************************************************************

  Synopsis           [Returns true if the property is PSL property and it
  is CTL compatible]

  Description        []

  SideEffects        []

******************************************************************************/
boolean Prop_is_psl_obe(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return (Prop_get_type(self) == Prop_Psl) && PslNode_is_obe(self->prop);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The Prop class private initializer]

  Description        [The Prop class private initializer]

  SideEffects        []

  SeeAlso            [Prop_create]

******************************************************************************/
void prop_init(Prop_ptr self)
{
  /* base class initialization */
  object_init(OBJECT(self));


  /* members initialization */
  self->index = 0;
  self->prop = EXPR(NULL);
  self->cone = (Set_t) Set_MakeEmpty();
  self->type = Prop_NoType;
  self->status = Prop_NoStatus;
  self->number = PROP_UNCHECKED;
  self->trace = 0;
  self->scalar_fsm = SEXP_FSM(NULL);
  self->bool_fsm = BOOL_SEXP_FSM(NULL);
  self->bdd_fsm = BDD_FSM(NULL);
  self->be_fsm = (BeFsm_ptr) NULL;
  self->name = Nil;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = prop_finalize;

  OVERRIDE(Prop, get_expr) = prop_get_expr;
  OVERRIDE(Prop, get_type_as_string) = prop_get_type_as_string;
  OVERRIDE(Prop, print) = prop_print;
  OVERRIDE(Prop, print_truncated) = prop_print_truncated;
  OVERRIDE(Prop, print_db_tabular) = prop_print_db_tabular;
  OVERRIDE(Prop, print_db_xml) = prop_print_db_xml;
  OVERRIDE(Prop, verify) = prop_verify;
}


/**Function********************************************************************

  Synopsis           [The Prop class private deinitializer]

  Description        [The Prop class private deinitializer]

  SideEffects        []

  SeeAlso            [Prop_destroy]

******************************************************************************/
void prop_deinit(Prop_ptr self)
{
  /* members deinitialization */
  {
    if (self->be_fsm != NULL) BeFsm_destroy(self->be_fsm);
    if (self->bdd_fsm != BDD_FSM(NULL)) BddFsm_destroy(self->bdd_fsm);
    if (self->bool_fsm != BOOL_SEXP_FSM(NULL)) BoolSexpFsm_destroy(self->bool_fsm);
    if (self->scalar_fsm != SEXP_FSM(NULL)) SexpFsm_destroy(self->scalar_fsm);
  }

  /* base class deinitialization */
  object_deinit(OBJECT(self));
}


/**Function********************************************************************

  Synopsis [Returns the property as it has been parsed and created]

  Description [Returns the property stored in the prop. If the
  property is PSL, the result should be converted to core symbols
  before model checking (see Prop_get_expr_core or
  PslNode_convert_psl_to_core).]

  SideEffects        []

  SeeAlso            [Prop_get_expr_core]

******************************************************************************/
Expr_ptr prop_get_expr(const Prop_ptr self)
{
  return self->prop;
}


/**Function********************************************************************

  Synopsis    [ Returns the a string associated to the propertys type. ]

  Description [ Returns the string corresponding to the propertys type
                for printing it. Returned string must NOT be
                deleted. ]

  SideEffects [ ]

  SeeAlso     [ ]

******************************************************************************/
const char* prop_get_type_as_string(const Prop_ptr self)
{
  return PropType_to_string(Prop_get_type(self));
}


/**Function********************************************************************

  Synopsis           [Prints a property]

  Description        [Prints a property. PSL properties are specially
  handled.]

  SideEffects        []

******************************************************************************/
void prop_print(const Prop_ptr self, FILE* file)
{
  node_ptr p;
  node_ptr context;

  p = Prop_get_expr(self);
  context = Nil;

  if (p != Nil && node_get_type(p) == CONTEXT) {
    context = car(p);
    p = cdr(p);
  }

  indent_node(file, "", p, " ");

  if (context != Nil) {
    fprintf(file, "IN ");
    print_node(file, context);
  }
}

/**Function********************************************************************

  Synopsis           [Prints a property]

  Description        [Prints a property. PSL properties are specially
  handled. The formula is truncated after the first 40 characters]

  SideEffects        []

******************************************************************************/
void prop_print_truncated(const Prop_ptr self, FILE* file)
{
  node_ptr p;
  node_ptr context;
  char* prop;
  int len;

  p = Prop_get_expr(self);
  context = Nil;

  if (p != Nil && node_get_type(p) == CONTEXT) {
    context = car(p);
    p = cdr(p);
  }

  prop = sprint_node(p);
  len = strlen(prop);

  if (len > 40) {
    prop[40] = '\0';
  }

  indent(file);
  fprintf(file, prop);

  if (len > 40) {
    fprintf(file, " [...]");
  }

  FREE(prop);

  if (context != Nil) {
    fprintf(file, "IN ");
    print_node(file, context);
  }
}


/**Function********************************************************************

  Synopsis           [Prints a property with info or its position and status
  within the database]

  Description        [Prints a property on the specified FILE
  stream. Some of the information stored in the property structure are
  printed out (e.g. property, property kind, property status, ...).]

  SideEffects        []

******************************************************************************/
void prop_print_db_tabular(const Prop_ptr self, FILE* file)
{
  PROP_CHECK_INSTANCE(self);

  fprintf(file, "%.3d : ", self->index);
  prop_print(self, file);
  fprintf(file, "\n");

  fprintf(file, "  [%-15s", Prop_get_type_as_string(self));
  if (self->type == Prop_Compute) {
    char* str_number = Prop_get_number_as_string(self);
    fprintf(file, "%-15s", str_number);
    FREE(str_number);
  }
  else fprintf(file, "%-15s", Prop_get_status_as_string(self));

  if (self->trace == 0) fprintf(file, "N/A    ");
  else fprintf(file, "%-7d", self->trace);

  if (Nil != self->name) {
    print_node(file, self->name);
  }
  else {
    fprintf(file, "N/A");
  }
  fprintf(file, "]\n");
}


/**Function********************************************************************

  Synopsis    [Prints a property with info or its position and status
  within the database, in XML format]

  Description [Prints a property on the specified FILE stream, in XML
  format. Some of the information stored in the property structure are
  printed out (e.g. property, property kind, property status, ...).]

  SideEffects        []

******************************************************************************/
void prop_print_db_xml(const Prop_ptr self, FILE* file)
{
  char* str;

  PROP_CHECK_INSTANCE(self);
  fprintf(file, "  <property>\n");

  fprintf(file, "    <name>");
  if (Nil != self->name) {
    str = sprint_node(self->name);
    Utils_str_escape_xml_file(str, file);
    FREE(str);
  }
  fprintf(file, "</name>\n");

  fprintf(file, "    <index>%d</index>\n", self->index);

  fprintf(file, "    <formula><![CDATA[\n");
  Prop_print(self, file, PROP_PRINT_FMT_FORMULA);
  fprintf(file, "\n]]>\n");
  fprintf(file, "</formula>\n");

  fprintf(file, "    <type>");
  Utils_str_escape_xml_file(Prop_get_type_as_string(self), file);
  fprintf(file, "</type>\n");

  fprintf(file, "    <status>");
  switch (Prop_get_status(self)) {
  case Prop_NoStatus:  fprintf(file, "UNKNOWN"); break;
  case Prop_Unchecked: fprintf(file, "UNCHECKED"); break;
  case Prop_True:      fprintf(file, "TRUE"); break;
  case Prop_False:     fprintf(file, "FALSE"); break;
  case Prop_Number:    fprintf(file, "NUMBER"); break;
  default:             error_unreachable_code(); /* invalid status */
  }
  fprintf(file, "</status>\n");

  fprintf(file, "    <bound>-1</bound>\n");

  fprintf(file, "    <trace>%d</trace>\n", self->trace);

  fprintf(file, " </property>\n\n");
}


/**Function********************************************************************

  Synopsis           [Verifies a given property]

  Description        [Depending the property, different model checking
                      algorithms are called. The status of the
                      property is updated accordingly to the result
                      of the verification process.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void prop_verify(Prop_ptr self)
{
  if (Prop_get_status(self) == Prop_Unchecked)  {
    switch (Prop_get_type(self)) {

    case Prop_Ctl:
      if (opt_ag_only(OptsHandler_get_instance())) {
        if ( opt_forward_search(OptsHandler_get_instance())) { Mc_CheckAGOnlySpec(self); }
        else {
          /* Cannot use AG-only since reachables must be calculated before */
          warning_ag_only_without_reachables();
          Mc_CheckCTLSpec(self);
        }
      }
      else { Mc_CheckCTLSpec(self); }
      break;

    case Prop_Compute:  Mc_CheckCompute(self); break;

    case Prop_Invar:    Mc_CheckInvar(self); break;

    case Prop_Ltl:      Ltl_CheckLtlSpec(self); break;

    case Prop_Psl:
      if (Prop_is_psl_ltl(self)) { Ltl_CheckLtlSpec(self); }
      else {
        if (Prop_is_psl_obe(self)) {
          if (opt_ag_only(OptsHandler_get_instance())) {
            if ( opt_forward_search(OptsHandler_get_instance())) { Mc_CheckAGOnlySpec(self); }
            else {
              /* Cannot use AG-only since reachables must be calculated before */
              warning_ag_only_without_reachables();
              Mc_CheckCTLSpec(self);
            }
          }
          else { Mc_CheckCTLSpec(self); }
        }
        else { error_psl_not_supported_feature(); }
      }
      break;

    default:  error_unreachable_code(); /* invalid type */
    }
  }
}

/**Function********************************************************************

  Synopsis           [Returns the a string associated to a property type]

  Description        [Returns the string corresponding to a property type
                      for printing it. Returned string must NOT be
                      deleted]

  SideEffects        []

******************************************************************************/
const char* PropType_to_string(const Prop_Type type)
{
  char* res = (char*) NULL;

  switch (type) {
  case Prop_NoType:  res = PROP_NOTYPE_STRING; break;
  case Prop_Ctl:     res = PROP_CTL_STRING; break;
  case Prop_Ltl:     res = PROP_LTL_STRING; break;
  case Prop_Psl:     res = PROP_PSL_STRING; break;
  case Prop_Invar:   res = PROP_INVAR_STRING; break;
  case Prop_Compute: res = PROP_COMPUTE_STRING; break;

  default: error_unreachable_code(); /* unknown type! */
  }

  return res;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
void prop_set_scalar_sexp_fsm(Prop_ptr self, SexpFsm_ptr fsm,
                              const boolean duplicate)
{
  if (self->scalar_fsm != SEXP_FSM(NULL)) SexpFsm_destroy(self->scalar_fsm);
  if (duplicate && (fsm != SEXP_FSM(NULL))) {
    self->scalar_fsm = SexpFsm_copy(fsm);
  }
  else self->scalar_fsm = fsm;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
void prop_set_bool_sexp_fsm(Prop_ptr self, BoolSexpFsm_ptr fsm,
                            const boolean duplicate)
{
  if (self->bool_fsm != BOOL_SEXP_FSM(NULL)) {
    BoolSexpFsm_destroy(self->bool_fsm);
  }
  if (duplicate && (fsm != BOOL_SEXP_FSM(NULL))) {
    self->bool_fsm = BOOL_SEXP_FSM(SexpFsm_copy(SEXP_FSM(fsm)));
  }
  else self->bool_fsm = fsm;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
void prop_set_bdd_fsm(Prop_ptr self, BddFsm_ptr fsm, const boolean duplicate)
{
  if (self->bdd_fsm != BDD_FSM(NULL)) BddFsm_destroy(self->bdd_fsm);
  if (duplicate && (fsm != BDD_FSM(NULL))) {
    self->bdd_fsm = BddFsm_copy(fsm);
  }
  else self->bdd_fsm = fsm;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
void prop_set_be_fsm(Prop_ptr self, BeFsm_ptr fsm, const boolean duplicate)
{
  if (self->be_fsm != (BeFsm_ptr) NULL) BeFsm_destroy(self->be_fsm);
  if (duplicate && (fsm != (BeFsm_ptr) NULL)) {
    self->be_fsm = BeFsm_copy(fsm);
  }
  else self->be_fsm = fsm;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The Prop class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void prop_finalize(Object_ptr object, void* dummy)
{
  Prop_ptr self = PROP(object);

  prop_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           []

  Description        [Derived from Prop_get_expr_core, but for PSL only
                      removes forall replicators rather than
                      converting the whole expression into
                      LTL. Written to be used with
                      Prop_apply_coi_for_{bdd,bmc}.]

  SideEffects        []

******************************************************************************/
static Expr_ptr prop_get_expr_core_for_coi(const Prop_ptr self)
{
  Expr_ptr res;

  if (Prop_get_type(self) == Prop_Psl) {
    res = PslNode_remove_forall_replicators(Prop_get_expr(self));
  }
  else res = Prop_get_expr(self); /* usual expression */

  return Compile_pop_distrib_ops(res);
}


/**AutomaticEnd***************************************************************/

