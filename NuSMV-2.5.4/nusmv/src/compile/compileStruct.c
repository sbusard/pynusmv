/**CFile***********************************************************************

  FileName    [compileStruct.c]

  PackageName [compile]

  Synopsis    [Structure used to store compilation results.]

  Description [Structure used to store compilation results.]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2. 
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

******************************************************************************/

#include "compileInt.h" 

static char rcsid[] UTIL_UNUSED = "$Id: compileStruct.c,v 1.8.4.1.4.6.4.2 2007-11-27 08:43:36 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Data structure used to store the current status of compilation.]

  Description []

******************************************************************************/
struct cmp_struct {
  int      read_model;
  int      hrc_built;
  int      flatten_hierarchy;
  int      encode_variables;
  int      process_selector;
  int      build_frames;
  int      build_model;
  int      build_flat_model;
  int      build_bool_model;
  int      bmc_init;
  int      bmc_setup;
  int      fairness_constraints;
  int      coi;
};

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable**********************************************************************

  Synopsis    [ This is a global variable responsable for 
  keeping track of performed phases.]

  Description [It is used in interactive mode,
  to distinguish which commands can or cannot be executed.]

******************************************************************************/
cmp_struct_ptr cmps = (cmp_struct_ptr) NULL;


/**Variable**********************************************************************

  Synopsis    [ This is a  global variable storing 
  the constructs of input modules.
  Such kinds of structures can be created only be Compile_FlattenHierarchy.
  ]

  Description []

******************************************************************************/
FlatHierarchy_ptr mainFlatHierarchy = FLAT_HIERARCHY(NULL);


/**Variable**********************************************************************

  Synopsis    [ This is a global variable keeping the instance to the 
  system fsm builder, used to help building FSMs]

  Description []

******************************************************************************/
FsmBuilder_ptr global_fsm_builder = FSM_BUILDER(NULL);


/**Variable********************************************************************

  Synopsis           [The global predicate normaliser]

  Description [An instance of a predicate normaliser is created and
  destroyed during initialisation and de-initialisation of the class
  package. During its lifetime a predicate normaliser can deal with
  only one type checker.  As result, an instance of a predicate
  normaliser can deal with only symbol table and one input SMV model
  only.]

******************************************************************************/
PredicateNormaliser_ptr 
global_predication_normaliser = PREDICATE_NORMALISER(NULL);



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Returns a global predicate normaliser]

  Description [See PredicateNormaliser.h for more info 
  on predication normaliser.]

  SideEffects        []

******************************************************************************/
PredicateNormaliser_ptr Compile_get_global_predicate_normaliser()
{
  return global_predication_normaliser;
}


/**Function********************************************************************

  Synopsis           [Returns the global fsm builder]

  Description [See fsm/FsmBuilder.h for more info]

  SideEffects        []

******************************************************************************/
FsmBuilder_ptr Compile_get_global_fsm_builder()
{
  return global_fsm_builder;
}



/**Function********************************************************************

  Synopsis           [Initializes the cmp structure]

  Description        []

  SideEffects        []

******************************************************************************/
cmp_struct_ptr cmp_struct_init()
{
  cmp_struct_ptr cmp;
  cmp = ALLOC(struct cmp_struct, 1);
  cmp->read_model           = 0;
  cmp->hrc_built            = 0;
  cmp->flatten_hierarchy    = 0;
  cmp->encode_variables     = 0;
  cmp->process_selector     = 0;
  cmp->build_frames         = 0;
  cmp->build_model          = 0;
  cmp->build_flat_model     = 0;
  cmp->build_bool_model     = 0;
  cmp->bmc_init             = 0;
  cmp->bmc_setup            = 0;
  cmp->fairness_constraints = 0;
  cmp->coi                  = 0;
  return(cmp);
}

/**Function********************************************************************

  Synopsis           [Free the cmp structure]

  Description        []

  SideEffects        []

******************************************************************************/
void cmp_struct_quit(cmp_struct_ptr cmp)
{
  nusmv_assert((cmp_struct_ptr) NULL != cmp);
  FREE(cmp);
}

int cmp_struct_get_read_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->read_model);
}

void cmp_struct_set_read_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->read_model = 1;
}

void cmp_struct_unset_read_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->read_model = 0;
}

int cmp_struct_get_hrc_built(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->hrc_built);
}

void cmp_struct_set_hrc_built(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->hrc_built = 1;
}

int cmp_struct_get_flatten_hrc(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->flatten_hierarchy);
}

void cmp_struct_set_flatten_hrc(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->flatten_hierarchy = 1;
}

int cmp_struct_get_encode_variables(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->encode_variables);
}

void cmp_struct_set_encode_variables(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->encode_variables = 1;
}

int cmp_struct_get_process_selector(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->process_selector);
}

void cmp_struct_set_process_selector(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->process_selector = 1;
}

int cmp_struct_get_build_frames(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->build_frames);
}

void cmp_struct_set_build_frames(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->build_frames = 1;
}

int cmp_struct_get_build_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->build_model);
}

void cmp_struct_set_build_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->build_model = 1;
}

int cmp_struct_get_build_flat_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->build_flat_model);
}

void cmp_struct_set_build_flat_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->build_flat_model = 1;
}

int cmp_struct_get_build_bool_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->build_bool_model);
}

void cmp_struct_set_build_bool_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->build_bool_model = 1;
}

int cmp_struct_get_bmc_init(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->bmc_init);
}
void cmp_struct_set_bmc_init(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->bmc_init = 1;
}
void cmp_struct_unset_bmc_init(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->bmc_init = 0;
}

int cmp_struct_get_bmc_setup(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->bmc_setup);
}

void cmp_struct_set_bmc_setup(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->bmc_setup = 1;
}

void cmp_struct_unset_bmc_setup(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->bmc_setup = 0;
}

int cmp_struct_get_fairness(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->fairness_constraints);
}

void cmp_struct_set_fairness(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->fairness_constraints = 1;
}

int cmp_struct_get_coi(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->coi);
}

void cmp_struct_set_coi(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->coi = 1;
}



