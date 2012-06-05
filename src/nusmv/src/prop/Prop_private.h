/**CHeaderFile*****************************************************************

  FileName    [Prop_private.h]

  PackageName [prop]

  Synopsis    [Private and protected interface of class 'Prop']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [Prop.h]

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

#ifndef __PROP_PRIVATE_H__
#define __PROP_PRIVATE_H__


#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "Prop.h"

#include "utils/object.h"
#include "utils/object_private.h"
#include "utils/utils.h"


/**Struct**********************************************************************

  Synopsis    [Prop class definition derived from
               class Object]

  Description [This structure contains informations about a given
  property:<br>
  <dl>
  <dt><code>index</code>
     <dd>is the progressive number identifying the specification.</dd>
  <dt><code>prop</code>
      <dd>is the specification formula (s-expression).
  <dt><code>type</code>
      <dd>is the type of the specification (CTL, LTL, INVAR, COMPUTE).
  <dt><code>cone</code>
      <dd>is the cone of influence of the formula.
  <dt><code>status</code>
      <dd>is the actual checking status of the specification.
  <dt><code>number</code>
      <dd>Result of a COMPUTE property.
  <dt><code>trace</code>
      <dd>is the index of the counterexample produced when the
          formula is found to be false, otherwise is zero.
  <dt><code>scalar_fsm</code>
      <dd>The FSM associated to the property in scalar format.
  <dt><code>bool_fsm</code>
      <dd>The FSM associated to the property in boolean format.
  <dt><code>bdd_fsm</code>
      <dd>The FSM associated to the property in BDD format.
  <dt><code>be_fsm</code>
      <dd>The FSM associated to the property in BE format.
  </dl>]

  SeeAlso     [Base class Object]

******************************************************************************/

/* Those are the types of the virtual methods. They can be used for
   type casts in subclasses. */
typedef Expr_ptr (*Prop_get_expr_method)(const Prop_ptr);
typedef const char* (*Prop_get_type_as_string_method)(const Prop_ptr);
typedef void (*Prop_print_method)(const Prop_ptr, FILE*);
typedef void (*Prop_print_db_method)(const Prop_ptr, FILE*);
typedef void (*Prop_verify_method)(Prop_ptr);

/* The class itself. */
typedef struct Prop_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(Object);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  unsigned int index;  /* Progressive number */
  Expr_ptr     prop;   /* property formula (s-expression) */

  /* AM -> Support for property name */
  node_ptr name;

  Set_t        cone;   /* The cone of influence */
  Prop_Type    type;   /* type of specification */
  Prop_Status  status; /* verification status */
  int          number; /* The result of a quantitative spec */
  int          trace;  /* the counterexample number (if any) */

  FsmBuilder_ptr fsm_mgr;  /* Used to produce FSMs from cone */

  SexpFsm_ptr     scalar_fsm; /* the scalar FSM */
  BoolSexpFsm_ptr bool_fsm;   /* The scalar FSM converted in Boolean */
  BddFsm_ptr      bdd_fsm;    /* The BDD FSM */
  BeFsm_ptr       be_fsm;     /* The BE FSM */

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */
  /* Expr_ptr (*)(const Prop_ptr) */
  Prop_get_expr_method get_expr;
  /* const char* (*)(const Prop_ptr) */
  Prop_get_type_as_string_method get_type_as_string;
  /* void (*)(const Prop_ptr, FILE*) */
  Prop_print_method print;
  /* void (*)(const Prop_ptr, FILE*) */
  Prop_print_method print_truncated;
  /* void (*)(const Prop_ptr, FILE*) */
  Prop_print_db_method print_db_tabular;
  /* void (*)(const Prop_ptr, FILE*) */
  Prop_print_db_method print_db_xml;
  /* void (*)(Prop_ptr) */
  Prop_verify_method verify;

} Prop;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only        */
/* ---------------------------------------------------------------------- */
EXTERN void prop_init ARGS((Prop_ptr self));
EXTERN void prop_deinit ARGS((Prop_ptr self));

Expr_ptr prop_get_expr(const Prop_ptr self);
const char* prop_get_type_as_string(const Prop_ptr self);

void prop_print(const Prop_ptr self, FILE* file);
void prop_print_truncated(const Prop_ptr self, FILE* file);

void prop_print_db_tabular(const Prop_ptr self, FILE* file);
void prop_print_db_xml(const Prop_ptr self, FILE* file);

void prop_verify(Prop_ptr self);

void prop_set_scalar_sexp_fsm ARGS((Prop_ptr self, SexpFsm_ptr fsm,
                                    const boolean duplicate));
void prop_set_bool_sexp_fsm ARGS((Prop_ptr self, BoolSexpFsm_ptr fsm,
                                  const boolean duplicate));
void prop_set_bdd_fsm ARGS((Prop_ptr self, BddFsm_ptr fsm,
                            const boolean duplicate));
void prop_set_be_fsm ARGS((Prop_ptr self, BeFsm_ptr fsm,
                           const boolean duplicate));


#endif /* __PROP_PRIVATE_H__ */
