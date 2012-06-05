/**CHeaderFile*****************************************************************

  FileName    [Triple.h]

  PackageName [utils]

  Synopsis    [Public interface of class 'Triple']

  Description []

  SeeAlso     [Triple.c]

  Author      [Alessandro Mariotti]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 2011 by FBK-irst. 

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


#ifndef __TRIPLE_H__
#define __TRIPLE_H__


#include "utils/utils.h" 


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class Triple]

  Description []

******************************************************************************/
typedef struct Triple_TAG*  Triple_ptr;


/**Struct**********************************************************************

  Synopsis    [Triple class definition]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct Triple_TAG
{
  void* first;
  void* second;
  void* third;
  boolean frozen;
} Triple;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class Triple]

  Description [These macros must be used respectively to cast and to check
  instances of class Triple]

******************************************************************************/
#define TRIPLE(self) \
         ((Triple_ptr) self)

#define TRIPLE_CHECK_INSTANCE(self) \
         (nusmv_assert(TRIPLE(self) != TRIPLE(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN Triple_ptr Triple_create ARGS((void* first,
                                      void* second,
                                      void* third));

EXTERN void Triple_freeze ARGS((Triple_ptr self));
EXTERN boolean Triple_is_freezed ARGS((const Triple_ptr self));

EXTERN void* Triple_get_first ARGS((const Triple_ptr self));
EXTERN void* Triple_get_second ARGS((const Triple_ptr self));
EXTERN void* Triple_get_third ARGS((const Triple_ptr self));

EXTERN void Triple_set_first ARGS((Triple_ptr self, void* first));
EXTERN void Triple_set_second ARGS((Triple_ptr self, void* second));
EXTERN void Triple_set_third ARGS((Triple_ptr self, void* third));

EXTERN void Triple_set_values ARGS((Triple_ptr self, void* first,
                                    void* second, void* third));

EXTERN void Triple_destroy ARGS((Triple_ptr self));

EXTERN int Triple_compare ARGS((const Triple_ptr a,
                                const Triple_ptr b));

EXTERN int Triple_hash ARGS((const Triple_ptr self, int size));

/**AutomaticEnd***************************************************************/



#endif /* __TRIPLE_H__ */
