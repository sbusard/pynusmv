/**CHeaderFile*****************************************************************

  FileName    [AddArray.h]

  PackageName [enc.utils]

  Synopsis    [The header file of AddArray class.]

  Description [This class represent an array of ADD. 
  The class is used to internally represent Word exressions during 
  encoding in enc/bdd/BddEnc module.
  Actually, all other the expressions are also represented with AddArray,
  but such array always have only one element.

  NB: some construction and destruction of AddArray can reference or
  de-reference its ADDs. Take care about it.
  ]
  

  SeeAlso     []

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``enc.utils'' package of NuSMV version 2. 
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

  Revision    [$Id: AddArray.h,v 1.1.2.2.6.6 2007-11-08 19:06:31 nusmv Exp $]

******************************************************************************/

#ifndef __ADD_ARRAY_H__
#define __ADD_ARRAY_H__

#include "dd/dd.h"
#include "utils/array.h"
#include "utils/WordNumber.h"


/*---------------------------------------------------------------------------*/
/* Types                                                                     */
/*---------------------------------------------------------------------------*/

/**Type************************************************************************

  Synopsis           [AddArray type]

  Description        []

  Notes              []

******************************************************************************/
typedef struct AddArray_TAG* AddArray_ptr;

/*---------------------------------------------------------------------------*/
/* Macros                                                                    */
/*---------------------------------------------------------------------------*/

#define ADD_ARRAY(x)  \
        ((AddArray_ptr) (x))

#define ADD_ARRAY_CHECK_INSTANCE(x)  \
        (nusmv_assert(ADD_ARRAY(x) != ADD_ARRAY(NULL)))

/*---------------------------------------------------------------------------*/
/* Definition of external funcions                                           */
/*---------------------------------------------------------------------------*/

/* constructor-converters */
EXTERN AddArray_ptr AddArray_create ARGS((int number));
EXTERN void AddArray_destroy ARGS((DdManager* dd, AddArray_ptr self));

EXTERN AddArray_ptr 
AddArray_from_word_number ARGS((DdManager* dd, WordNumber_ptr wn));

EXTERN AddArray_ptr AddArray_from_add ARGS((add_ptr add));
EXTERN AddArray_ptr AddArray_duplicate ARGS((AddArray_ptr self));

/* access function */
EXTERN int AddArray_get_size ARGS((AddArray_ptr self));

EXTERN size_t AddArray_get_add_size ARGS((const AddArray_ptr self, 
                                          DdManager* dd));


EXTERN add_ptr AddArray_get_add ARGS((AddArray_ptr self));
EXTERN add_ptr AddArray_get_n ARGS((AddArray_ptr self, int number));
EXTERN void AddArray_set_n ARGS((AddArray_ptr self, int number, 
                                 add_ptr add));
EXTERN array_t* AddArray_get_array ARGS((AddArray_ptr self));

/* --------------------------------------------------------- */
/* auxiliary arithmetic functions */
/* --------------------------------------------------------- */

/* arithemtic Word operations */
EXTERN AddArray_ptr 
AddArray_word_apply_unary ARGS((DdManager* dd,
                                AddArray_ptr arg1,
                                FP_A_DA op));

EXTERN AddArray_ptr 
AddArray_word_apply_binary ARGS((DdManager* dd,
                                 AddArray_ptr arg1,
                                 AddArray_ptr arg2, 
                                 FP_A_DAA op));

EXTERN add_ptr 
AddArray_make_disjunction ARGS((DdManager* dd,
                                AddArray_ptr arg));
EXTERN add_ptr 
AddArray_make_conjunction ARGS((DdManager* dd,
                                AddArray_ptr arg));


EXTERN AddArray_ptr AddArray_word_plus ARGS((DdManager* dd,
                                             AddArray_ptr arg1,
                                             AddArray_ptr arg2));
EXTERN AddArray_ptr AddArray_word_minus ARGS((DdManager* dd,
                                              AddArray_ptr arg1,
                                              AddArray_ptr arg2));
EXTERN AddArray_ptr AddArray_word_unary_minus ARGS((DdManager* dd, 
                                                    AddArray_ptr arg));

EXTERN AddArray_ptr AddArray_word_times ARGS((DdManager* dd,
                                              AddArray_ptr arg1,
                                              AddArray_ptr arg2));
EXTERN AddArray_ptr AddArray_word_unsigned_divide ARGS((DdManager* dd,
                                               AddArray_ptr arg1,
                                               AddArray_ptr arg2));
EXTERN AddArray_ptr AddArray_word_unsigned_mod ARGS((DdManager* dd,
                                            AddArray_ptr arg1,
                                            AddArray_ptr arg2));
EXTERN AddArray_ptr AddArray_word_signed_divide ARGS((DdManager* dd,
                                                      AddArray_ptr arg1,
                                                      AddArray_ptr arg2));
EXTERN AddArray_ptr AddArray_word_signed_mod ARGS((DdManager* dd,
                                                   AddArray_ptr arg1,
                                                   AddArray_ptr arg2));

EXTERN AddArray_ptr AddArray_word_left_shift ARGS((DdManager* dd,
                                                   AddArray_ptr arg,
                                                   AddArray_ptr number));
EXTERN AddArray_ptr AddArray_word_unsigned_right_shift ARGS((DdManager* dd,
                                                    AddArray_ptr arg,
                                                    AddArray_ptr number));
EXTERN AddArray_ptr AddArray_word_signed_right_shift ARGS((DdManager* dd,
                                                    AddArray_ptr arg,
                                                    AddArray_ptr number));
EXTERN AddArray_ptr AddArray_word_left_rotate ARGS((DdManager* dd,
                                                    AddArray_ptr arg,
                                                    AddArray_ptr number));
EXTERN AddArray_ptr AddArray_word_right_rotate ARGS((DdManager* dd,
                                                     AddArray_ptr arg,
                                                     AddArray_ptr number));

EXTERN AddArray_ptr AddArray_word_unsigned_less ARGS((DdManager* dd,
                                                     AddArray_ptr arg1,
                                                     AddArray_ptr arg2));
EXTERN AddArray_ptr AddArray_word_unsigned_less_equal ARGS((DdManager* dd,
                                                           AddArray_ptr arg1,
                                                           AddArray_ptr arg2));
EXTERN AddArray_ptr AddArray_word_unsigned_greater ARGS((DdManager* dd,
                                                        AddArray_ptr arg1,
                                                        AddArray_ptr arg2));
EXTERN AddArray_ptr AddArray_word_unsigned_greater_equal ARGS((DdManager* dd,
                                                              AddArray_ptr arg1,
                                                              AddArray_ptr arg2));

EXTERN AddArray_ptr AddArray_word_signed_less ARGS((DdManager* dd, 
                                                    AddArray_ptr arg1,
                                                    AddArray_ptr arg2));

EXTERN AddArray_ptr AddArray_word_signed_less_equal ARGS((DdManager* dd, 
                                                          AddArray_ptr arg1,
                                                          AddArray_ptr arg2));

EXTERN AddArray_ptr AddArray_word_signed_greater ARGS((DdManager* dd, 
                                                       AddArray_ptr arg1,
                                                       AddArray_ptr arg2));
EXTERN AddArray_ptr AddArray_word_signed_greater_equal ARGS((DdManager* dd, 
                                                             AddArray_ptr arg1, 
                                                             AddArray_ptr arg2));
EXTERN AddArray_ptr 
AddArray_word_signed_extend ARGS((DdManager* dd,
                                AddArray_ptr arg, AddArray_ptr repeat));
EXTERN AddArray_ptr 
AddArray_word_unsigned_extend ARGS((DdManager* dd,
                                    AddArray_ptr arg, AddArray_ptr repeat));
EXTERN AddArray_ptr
AddArray_word_signed_resize ARGS((DdManager* dd,
                                AddArray_ptr arg, AddArray_ptr new_size));
EXTERN AddArray_ptr
AddArray_word_unsigned_resize ARGS((DdManager* dd,
                                    AddArray_ptr arg, AddArray_ptr new_size));

EXTERN AddArray_ptr AddArray_word_equal ARGS((DdManager* dd,
                                              AddArray_ptr arg1,
                                              AddArray_ptr arg2));
EXTERN AddArray_ptr AddArray_word_not_equal ARGS((DdManager* dd,
                                                  AddArray_ptr arg1,
                                                  AddArray_ptr arg2));

EXTERN AddArray_ptr AddArray_word_ite ARGS((DdManager* dd,
                                            AddArray_ptr _if,
                                            AddArray_ptr _then, 
                                            AddArray_ptr _else));

EXTERN AddArray_ptr AddArray_word_bit_selection ARGS((DdManager* dd,
                                                      AddArray_ptr word,
                                                      AddArray_ptr range));
EXTERN AddArray_ptr AddArray_word_concatenation ARGS((DdManager* dd,
                                                      AddArray_ptr arg1,
                                                      AddArray_ptr arg2));

#endif /* __ADD_ARRAY_H__ */
