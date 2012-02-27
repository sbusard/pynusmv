/**CHeaderFile*****************************************************************

  FileName    [WordNumber.h]

  PackageName [enc.utils]

  Synopsis    [The header file of WordNumber class.]

  Description [This class represent Word arithmetics (which includes
  bitwise and integer arithmetic). This class is created to hide the
  implementation of Word numbers.

  In the sence of memory this class is similar to node_ptr, i.e. no
  need to warry about memory, but class deinitialiser frees memory
  from all Word numbers.
  The class should be initialized and deinitialised after and before node_ptr,
  repspectively.
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

  Revision    [$Id: WordNumber.h,v 1.1.2.1.4.9 2010-01-21 19:54:29 nusmv Exp $]

******************************************************************************/

#ifndef __WORD_NUMBER_H__
#define __WORD_NUMBER_H__

#include "utils/utils.h"


/**Type************************************************************************

  Synopsis           [WordNumber type]

  Description        []

  Notes              []

******************************************************************************/
typedef struct WordNumber_TAG* WordNumber_ptr;


/**Type************************************************************************

  Synopsis           [WordNumber integer value]

  Description        []

  Notes              [It has to be unsigned type.
                      This is used in implementation.]

******************************************************************************/
typedef unsigned long long  WordNumberValue;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define WORD_NUMBER(x)  \
        ((WordNumber_ptr) (x))

#define WORD_NUMBER_CHECK_INSTANCE(x)  \
        (nusmv_assert(WORD_NUMBER(x) != WORD_NUMBER(NULL)))

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
/* implementation limit */
EXTERN int WordNumber_max_width ARGS((void));

/* constructor-converters */
EXTERN WordNumber_ptr WordNumber_from_string ARGS((char* str, int base));

EXTERN WordNumber_ptr WordNumber_from_sized_string ARGS((char* str,
                                                         int base,
                                                         int width));

EXTERN WordNumber_ptr WordNumber_from_parsed_string ARGS((char* str,
                                                          char** errorString));

EXTERN WordNumber_ptr WordNumber_from_integer ARGS((WordNumberValue value,
                                                    int width));

EXTERN WordNumber_ptr WordNumber_from_signed_integer ARGS((WordNumberValue val,
                                                           int width));

EXTERN WordNumber_ptr WordNumber_normalize ARGS((const WordNumber_ptr number));

/* access function */
EXTERN WordNumberValue WordNumber_get_unsigned_value ARGS((WordNumber_ptr self));
EXTERN WordNumberValue WordNumber_get_signed_value ARGS((WordNumber_ptr self));

EXTERN int WordNumber_get_width ARGS((WordNumber_ptr self));
EXTERN boolean WordNumber_get_bit ARGS((WordNumber_ptr self, int n));
EXTERN boolean WordNumber_get_sign ARGS((WordNumber_ptr self));
EXTERN char* WordNumber_get_parsed_string ARGS((WordNumber_ptr self));

EXTERN WordNumberValue WordNumber_max_unsigned_value ARGS((int width));
EXTERN WordNumberValue WordNumber_max_signed_value ARGS((int width));
EXTERN WordNumberValue WordNumber_min_signed_value ARGS((int width));

/* output functions */
EXTERN int WordNumber_print ARGS((FILE* output_stream, WordNumber_ptr self,
                                  boolean isSigned));
EXTERN int WordNumber_based_print ARGS((FILE* output_stream,
                                        WordNumber_ptr self,
                                        int base,
                                        boolean isSigned));
EXTERN char* WordNumber_to_string ARGS((WordNumber_ptr self,
                                        boolean isSigned));
EXTERN char* WordNumber_to_based_string ARGS((WordNumber_ptr self, int base,
                                              boolean isSigned));

/* integer arithmetic */
EXTERN WordNumber_ptr WordNumber_unary_minus ARGS((WordNumber_ptr v1));

EXTERN WordNumber_ptr WordNumber_plus ARGS((WordNumber_ptr v1,
                                            WordNumber_ptr v2));
EXTERN WordNumber_ptr WordNumber_minus ARGS((WordNumber_ptr v1,
                                             WordNumber_ptr v2));
EXTERN WordNumber_ptr WordNumber_times ARGS((WordNumber_ptr v1,
                                             WordNumber_ptr v2));
EXTERN WordNumber_ptr 
WordNumber_unsigned_divide ARGS((WordNumber_ptr v1, WordNumber_ptr v2));

EXTERN WordNumber_ptr 
WordNumber_signed_divide ARGS((WordNumber_ptr v1, WordNumber_ptr v2));

EXTERN WordNumber_ptr 
WordNumber_unsigned_mod ARGS((WordNumber_ptr v1, WordNumber_ptr v2));

EXTERN WordNumber_ptr 
WordNumber_signed_mod ARGS((WordNumber_ptr v1, WordNumber_ptr v2));

/* relational operations */
EXTERN boolean WordNumber_is_zero ARGS((WordNumber_ptr v));
EXTERN boolean WordNumber_equal ARGS((WordNumber_ptr v1, WordNumber_ptr v2));
EXTERN boolean WordNumber_not_equal ARGS((WordNumber_ptr v1,
                                          WordNumber_ptr v2));
EXTERN boolean WordNumber_unsigned_less ARGS((WordNumber_ptr v1, 
                                              WordNumber_ptr v2));
EXTERN boolean WordNumber_unsigned_less_or_equal ARGS((WordNumber_ptr v1,
                                                       WordNumber_ptr v2));
EXTERN boolean WordNumber_unsigned_greater ARGS((WordNumber_ptr v1, 
                                                 WordNumber_ptr v2));
EXTERN boolean WordNumber_unsigned_greater_or_equal ARGS((WordNumber_ptr v1,
                                                          WordNumber_ptr v2));

EXTERN boolean WordNumber_signed_less ARGS((WordNumber_ptr v1,
                                            WordNumber_ptr v2));
EXTERN boolean WordNumber_signed_less_or_equal ARGS((WordNumber_ptr v1,
                                                     WordNumber_ptr v2));
EXTERN boolean WordNumber_signed_greater ARGS((WordNumber_ptr v1,
                                               WordNumber_ptr v2));
EXTERN boolean WordNumber_signed_greater_or_equal ARGS((WordNumber_ptr v1,
                                                 WordNumber_ptr v2));
/* bitwise(boolean) */
EXTERN WordNumber_ptr WordNumber_not ARGS((WordNumber_ptr v));
EXTERN WordNumber_ptr WordNumber_and ARGS((WordNumber_ptr v1,
                                           WordNumber_ptr v2));
EXTERN WordNumber_ptr WordNumber_or ARGS((WordNumber_ptr v1,
                                          WordNumber_ptr v2));
EXTERN WordNumber_ptr WordNumber_xor ARGS((WordNumber_ptr v1,
                                           WordNumber_ptr v2));
EXTERN WordNumber_ptr WordNumber_xnor ARGS((WordNumber_ptr v1,
                                            WordNumber_ptr v2));
EXTERN WordNumber_ptr WordNumber_implies ARGS((WordNumber_ptr v1,
                                               WordNumber_ptr v2));
EXTERN WordNumber_ptr WordNumber_iff ARGS((WordNumber_ptr v1,
                                           WordNumber_ptr v2));

/* pure bitwise */

EXTERN WordNumber_ptr WordNumber_concatenate ARGS((WordNumber_ptr v1,
                                                   WordNumber_ptr v2));
EXTERN WordNumber_ptr WordNumber_bit_select ARGS((WordNumber_ptr v,
                                                  int highBit, int lowBit));
EXTERN WordNumber_ptr 
WordNumber_unsigned_right_shift ARGS((WordNumber_ptr v,
                                      int numberOfBits));
EXTERN WordNumber_ptr 
WordNumber_signed_right_shift ARGS((WordNumber_ptr v, int numberOfBits));

EXTERN WordNumber_ptr WordNumber_left_shift ARGS((WordNumber_ptr v,
                                                  int numberOfBits));
EXTERN WordNumber_ptr WordNumber_right_rotate ARGS((WordNumber_ptr v,
                                                    int numberOfBits));
EXTERN WordNumber_ptr WordNumber_left_rotate ARGS((WordNumber_ptr v,
                                                   int numberOfBits));
EXTERN WordNumber_ptr 
WordNumber_unsigned_extend ARGS((WordNumber_ptr v, int numberOfTimes));

EXTERN WordNumber_ptr 
WordNumber_signed_extend ARGS((WordNumber_ptr v, int numberOfTimes));

#endif /* __WORD_NUMBER_H__ */
