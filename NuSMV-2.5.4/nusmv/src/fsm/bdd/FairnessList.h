/**CHeaderFile*****************************************************************

  FileName    [FairnessList.h]

  PackageName [fsm.bdd]

  Synopsis    [Declares the interface for the classes that contains fairness
  conditions]

  Description [This interface exports three objects: 
  - a generic FairnessList base class
  - a class for justice list (list of BDDs), derived from FairnessList
  - a class for compassion list (couple of BDDs), derived from
    FairnessList]
  
  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm.bdd'' package of NuSMV version 2. 
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

******************************************************************************/


#ifndef __FSM_BDD_FAIRNESS_LIST_H__
#define __FSM_BDD_FAIRNESS_LIST_H__

#include "bdd.h"
#include "utils/object.h" /* for object inheritance support */
#include "utils/utils.h"
#include "node/node.h"
#include "dd/dd.h"

/* ---------------------------------------------------------------------- */
/* Base type, derives from Object                                         */
typedef struct FairnessList_TAG* FairnessList_ptr;

#define FAIRNESS_LIST(x) \
       ((FairnessList_ptr) x)

#define FAIRNESS_LIST_CHECK_INSTANCE(self) \
       (nusmv_assert( FAIRNESS_LIST(self) != FAIRNESS_LIST(NULL) ))
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- */
/* Iterator for the list                                                  */
typedef node_ptr FairnessListIterator_ptr; 
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- */
/* Derives from FairnessList */
typedef struct JusticeList_TAG* JusticeList_ptr;

#define JUSTICE_LIST(x) \
       ((JusticeList_ptr) x)

#define JUSTICE_LIST_CHECK_INSTANCE(self) \
       (nusmv_assert( JUSTICE_LIST(self) != JUSTICE_LIST(NULL) ))
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- */
/* Derives from FairnessList */
typedef struct CompassionList_TAG* CompassionList_ptr;

#define COMPASSION_LIST(x) \
       ((CompassionList_ptr) x)

#define COMPASSION_LIST_CHECK_INSTANCE(self) \
       (nusmv_assert( COMPASSION_LIST(self) != COMPASSION_LIST(NULL) ))
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- */
/*  Public methods:                                                       */
/* ---------------------------------------------------------------------- */
EXTERN FairnessList_ptr FairnessList_create ARGS((DdManager* dd_manager));

EXTERN boolean FairnessList_is_empty ARGS((const FairnessList_ptr self));

EXTERN FairnessListIterator_ptr 
FairnessList_begin ARGS((const FairnessList_ptr self));

EXTERN boolean 
FairnessListIterator_is_end ARGS((const FairnessListIterator_ptr self));

EXTERN FairnessListIterator_ptr 
FairnessListIterator_next ARGS((const FairnessListIterator_ptr self));


/* Justice */
EXTERN JusticeList_ptr JusticeList_create ARGS((DdManager* dd_manager));

EXTERN BddStates 
JusticeList_get_p ARGS((const JusticeList_ptr self, 
                        const FairnessListIterator_ptr iter));

EXTERN void JusticeList_append_p ARGS((JusticeList_ptr self, BddStates p));

EXTERN void 
JusticeList_apply_synchronous_product ARGS((JusticeList_ptr self, 
                                            const JusticeList_ptr other));

/* Compassion */
EXTERN CompassionList_ptr CompassionList_create ARGS((DdManager* dd_manager));

EXTERN BddStates 
CompassionList_get_p ARGS((const CompassionList_ptr self, 
                           const FairnessListIterator_ptr iter));

EXTERN BddStates 
CompassionList_get_q ARGS((const CompassionList_ptr self, 
                           const FairnessListIterator_ptr iter));

EXTERN void CompassionList_append_p_q ARGS((CompassionList_ptr self, 
                                            BddStates p, BddStates q));

EXTERN void 
CompassionList_apply_synchronous_product ARGS((CompassionList_ptr self, 
                                               const CompassionList_ptr other));

#endif /* __FSM_BDD_FAIRNESS_LIST_H__ */
