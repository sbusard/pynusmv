/**CFile***********************************************************************

  FileName    [BeFsm.c]

  PackageName [fsm.be]

  Synopsis    [Implementation of class BeFsm]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm.be'' package of NuSMV version 2. 
  Copyright (C) 2005 by FBK-irst.

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

#include "BeFsm.h"

#include "bmc/bmcCheck.h"
#include "bmc/bmcConv.h"

#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: BeFsm.c,v 1.1.2.5.6.5 2009-09-04 09:22:47 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct*********************************************************************

  Synopsis           [Represents a Finite State Machine which 
  collects the whole model in Boolean Expression format]
  Description        [This class definition is private. Make reference to 
  the BeFsm_ptr type in order to handle a valid instance of this class.]
  SideEffects        []
  SeeAlso            []

******************************************************************************/
typedef struct BeFsm_TAG {
  BeEnc_ptr be_enc; 
  be_ptr init;
  be_ptr invar;
  be_ptr trans;
  node_ptr fairness_list;

} BeFsm;


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

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

static void be_fsm_init ARGS((BeFsm_ptr self, 
                               BeEnc_ptr be_enc, const be_ptr init, 
                               const be_ptr invar, const be_ptr trans, 
                               const node_ptr list_of_be_fairness));

static void be_fsm_deinit ARGS((BeFsm_ptr self));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Class BeFsm constructor]

  Description        [Creates a new instance of the BeFsm class, getting 
  information from an instance of a boolean Fsm_Sexp type.]

  SideEffects        []

  SeeAlso            [BeFsm_create, BeFsm_destroy]

******************************************************************************/
BeFsm_ptr BeFsm_create_from_sexp_fsm(BeEnc_ptr be_enc, 
                                     const BoolSexpFsm_ptr bfsm)
{
    BeFsm_ptr self;
    SexpFsm_ptr _bfsm = SEXP_FSM(bfsm);
    node_ptr list_of_valid_fairness;

    nusmv_assert(SexpFsm_is_boolean(_bfsm));

    list_of_valid_fairness = 
     Bmc_CheckFairnessListForPropositionalFormulae(SexpFsm_get_justice(_bfsm));
  
    self = BeFsm_create(be_enc, 
                 Bmc_Conv_Bexp2Be(be_enc, SexpFsm_get_init(_bfsm)),
                 Bmc_Conv_Bexp2Be(be_enc, SexpFsm_get_invar(_bfsm)),
                 Bmc_Conv_Bexp2Be(be_enc, SexpFsm_get_trans(_bfsm)),
                 Bmc_Conv_BexpList2BeList(be_enc, list_of_valid_fairness));
  
  free_list(list_of_valid_fairness);
  return self;
}


/**Function********************************************************************

  Synopsis           [Class BeFsm constructor]

  Description        [It gets init, invar, transition relation and the list
  of fairness in Boolean Expression format.]

  SideEffects        []

  SeeAlso            [BeFsm_destroy]

******************************************************************************/
BeFsm_ptr BeFsm_create(BeEnc_ptr be_enc, 
                       const be_ptr init, 
                       const be_ptr invar,
                       const be_ptr trans, 
                       const node_ptr list_of_be_fairness)
{
  BeFsm_ptr self = ALLOC(BeFsm, 1);
  BE_FSM_CHECK_INSTANCE(self);

  be_fsm_init(self, be_enc, init, invar, trans, list_of_be_fairness);

  return self;
}


/**Function********************************************************************

  Synopsis           [Class BeFsm destructor]

  Description        []

  SideEffects        [self will be invalidated]

  SeeAlso            [BeFsm_create, BeFsm_create_from_sexp_fsm]

******************************************************************************/
void BeFsm_destroy(BeFsm_ptr self)
{
  BE_FSM_CHECK_INSTANCE(self);

  be_fsm_deinit(self);
  FREE(self); 
}


/**Function********************************************************************

  Synopsis           [Copy constructor for class BeFsm]

  Description        [Creates a new independent copy of the given fsm instance.
  You must destroy the returned class instance by invoking the class 
  destructor when you no longer need it.]

  SideEffects        []

  SeeAlso            [BeFsm_destroy]

******************************************************************************/
BeFsm_ptr BeFsm_copy(BeFsm_ptr self)
{
  BeFsm_ptr copy;
  BE_FSM_CHECK_INSTANCE(self);

  /* Necessary since the master in BE is built only after be_setup */
  copy = BeFsm_create(self->be_enc, 
                      BeFsm_get_init(self), 
                      BeFsm_get_invar(self), 
                      BeFsm_get_trans(self),
                      BeFsm_get_fairness_list(self));
  return copy;
}


/**Function********************************************************************

  Synopsis [Returns the be encoding associated with the given fsm
  instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BeEnc_ptr BeFsm_get_be_encoding(const BeFsm_ptr self)
{
  BE_FSM_CHECK_INSTANCE(self);
  return self->be_enc;
}


/**Function********************************************************************

  Synopsis           [Returns the initial states stored in BE format into the
  given fsm instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr BeFsm_get_init(const BeFsm_ptr self)
{
  BE_FSM_CHECK_INSTANCE(self);
  return self->init;
}


/**Function********************************************************************

  Synopsis           [Returns the invariants stored in BE format into the
  given fsm instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr BeFsm_get_invar(const BeFsm_ptr self)
{
  BE_FSM_CHECK_INSTANCE(self);
  return self->invar;
}


/**Function********************************************************************

  Synopsis           [Returns the transition relation stored in BE format 
  into the given fsm instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr BeFsm_get_trans(const BeFsm_ptr self)
{
  BE_FSM_CHECK_INSTANCE(self);
  return self->trans;
}


/**Function********************************************************************

  Synopsis           [Returns the list of fairness stored in BE format 
  into the given fsm instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr BeFsm_get_fairness_list(const BeFsm_ptr self)
{
  BE_FSM_CHECK_INSTANCE(self);
  return self->fairness_list;
}

/**Function********************************************************************

  Synopsis           [Apply the synchronous product between self and other
                      modifying self]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BeFsm_apply_synchronous_product(BeFsm_ptr self, const BeFsm_ptr other) {
	node_ptr list;
	Be_Manager_ptr manager;

	BE_FSM_CHECK_INSTANCE(self);

	manager = BeEnc_get_be_manager(self->be_enc);

	list = other->fairness_list;

	while (Nil != list) {
		self->fairness_list = cons(car(list), self->fairness_list);

		list = cdr(list);
	}

	self->init = Be_And(manager, self->init, other->init);
	self->trans = Be_And(manager, self->trans, other->trans);
	self->invar = Be_And(manager, self->invar, other->invar);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Private service to initialize the internal members]

  Description        []

  SideEffects        [self will change internally]

  SeeAlso            []

******************************************************************************/
static void be_fsm_init(BeFsm_ptr self, 
                         BeEnc_ptr be_enc, const be_ptr init, 
                         const be_ptr invar, const be_ptr trans, 
                         const node_ptr list_of_be_fairness)
{
  self->be_enc = be_enc;
  self->init = init;
  self->invar = invar;
  self->trans = trans;
  self->fairness_list = list_of_be_fairness;
}


/**Function********************************************************************

  Synopsis           [Private service to deinitialize the internal members]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void be_fsm_deinit(BeFsm_ptr self)
{

}


