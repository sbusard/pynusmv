/**CHeaderFile*****************************************************************

  FileName    [set.h]

  PackageName [set]

  Synopsis    [Generic Set Data Structure]

  Description [This package provides an implementation of sets.
  It is possible to perform the test of equality among two sets in
  constant time by simply comparing the two sets. Thus it is possible
  to check if a union has increased the cardinality of a set inserting
  elements in one of the two operands by simply comparing the
  result of the union among the operands.]

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``set'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by FBK-irst. 

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

  Revision    [$Id: set.h,v 1.3.4.1.4.3.4.6 2010-02-05 20:23:03 nusmv Exp $]

******************************************************************************/

#ifndef _set
#define _set

#include "utils/utils.h"
#include "utils/NodeList.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct Set_TAG* Set_t;
typedef node_ptr Set_Element_t;
typedef ListIter_ptr Set_Iterator_t;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis           [use this to iterate over a set]

  Description        []

  Notes              []

  SideEffects        []
  
******************************************************************************/
#define SET_FOREACH(set, iter)                               \
   for (iter=Set_GetFirstIter(set); !Set_IsEndIter(iter);    \
        iter=Set_GetNextIter(iter))


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void set_pkg_init ARGS((void));
EXTERN void set_pkg_quit ARGS((void));

EXTERN Set_t Set_MakeEmpty ARGS((void));
EXTERN Set_t Set_Make ARGS((node_ptr list));
EXTERN Set_t Set_MakeFromUnion ARGS((node_ptr _union));
EXTERN Set_t Set_MakeSingleton ARGS((Set_Element_t elem));

EXTERN Set_t Set_Copy ARGS((const Set_t set));
EXTERN Set_t Set_Freeze ARGS((Set_t set));

EXTERN void Set_ReleaseSet ARGS((Set_t set));
EXTERN void Set_ReleaseSetOfSet ARGS((Set_t set));

EXTERN boolean Set_IsEmpty ARGS((const Set_t set));
EXTERN boolean Set_IsMember ARGS((const Set_t set, Set_Element_t elem));

EXTERN int Set_GiveCardinality ARGS((const Set_t set));

EXTERN Set_t Set_AddMember ARGS((Set_t set, Set_Element_t el)); 
EXTERN Set_t Set_RemoveMember ARGS((Set_t set, Set_Element_t el));

EXTERN Set_t Set_AddMembersFromList ARGS((Set_t set, const NodeList_ptr list));

EXTERN boolean Set_Contains ARGS((const Set_t set1, const Set_t set2));
EXTERN boolean Set_Equals ARGS((const Set_t set1, const Set_t set2));
EXTERN boolean Set_Intersects ARGS((const Set_t set1, const Set_t set2));

EXTERN Set_t Set_Union ARGS((Set_t set1, const Set_t set2));
EXTERN Set_t Set_Intersection ARGS((Set_t set1, const Set_t set2));
EXTERN Set_t Set_Difference ARGS((Set_t set1, const Set_t set2));

EXTERN Set_t Set_GetRest ARGS((const Set_t set, Set_Iterator_t from));

EXTERN Set_Iterator_t Set_GetFirstIter ARGS((Set_t set1));
EXTERN Set_Iterator_t Set_GetNextIter ARGS((Set_Iterator_t iter));
EXTERN boolean Set_IsEndIter ARGS((Set_Iterator_t iter));
EXTERN Set_Element_t Set_GetMember ARGS((const Set_t set, Set_Iterator_t iter));

EXTERN NodeList_ptr Set_Set2List ARGS((const Set_t set));

EXTERN void Set_PrintSet ARGS((FILE *, const Set_t set, 
                               void (*printer)(FILE* file, 
                                               Set_Element_t el, void* arg), 
                               void* printer_arg));




/**AutomaticEnd***************************************************************/

#endif /* _set */
