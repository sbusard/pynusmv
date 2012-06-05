/**CHeaderFile*****************************************************************

  FileName    [operators.h]

  PackageName [enc]

  Synopsis    [Interface for operators are used by dd package]

  Description [Functions like add_plus, add_equal, etc., call these operators]

  SeeAlso     [operators.c]

  Author      [Marco Roveri, Roberto Cavada]

  Copyright   [
  This file is part of the ``enc'' package of NuSMV version 2. 
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

#ifndef __ENC_OPERATORS_H__
#define __ENC_OPERATORS_H__


#include "utils/utils.h"
#include "utils/WordNumber.h"
#include "utils/array.h"
#include "node/node.h"


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN node_ptr one_number;
EXTERN node_ptr zero_number;


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN node_ptr node_and ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_or ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_not ARGS((node_ptr n, node_ptr this_node_not_used));
EXTERN node_ptr node_iff ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_xor ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_implies ARGS((node_ptr n1, node_ptr n2));

EXTERN node_ptr node_equal ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_not_equal ARGS((node_ptr n1, node_ptr n2));

EXTERN node_ptr node_lt ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_gt ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_le ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_ge ARGS((node_ptr n1, node_ptr n2));

EXTERN node_ptr node_unary_minus ARGS((node_ptr n1, node_ptr n2));

EXTERN node_ptr node_plus ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_minus ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_times ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_divide ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_mod ARGS((node_ptr n1, node_ptr n2));

EXTERN node_ptr node_bit_range ARGS((node_ptr n1, node_ptr n2));

EXTERN node_ptr node_union ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_setin ARGS((node_ptr n1, node_ptr n2));

/* ---------------------------------------------------------------------- */
/* WORD related                                                           */
/* ---------------------------------------------------------------------- */
/* RC: some function may be moved away */
EXTERN size_t node_word_get_width ARGS((node_ptr w));

#define _CHECK_WORD(w)                                      \
  nusmv_assert(((node_get_type(w) == UNSIGNED_WORD ||       \
                 node_get_type(w) == SIGNED_WORD) &&        \
                 node_word_get_width(w) > 0) ||             \
               (node_get_type(w) == NUMBER_UNSIGNED_WORD || \
                node_get_type(w) == NUMBER_SIGNED_WORD))


#define _CHECK_WORDS(w1, w2)                                          \
  _CHECK_WORD(w1); _CHECK_WORD(w2);                                   \
  if ((node_get_type(w1) == UNSIGNED_WORD ||                          \
       node_get_type(w1) == SIGNED_WORD)  &&                          \
      (node_get_type(w2) == UNSIGNED_WORD ||                          \
       node_get_type(w2) == SIGNED_WORD)) /* all words */             \
    nusmv_assert(node_word_get_width(w1) == node_word_get_width(w2)); \
  else if ((node_get_type(w1) == UNSIGNED_WORD ||                       \
            node_get_type(w1) == SIGNED_WORD)  &&                       \
           (node_get_type(w2) == NUMBER_UNSIGNED_WORD ||                \
            node_get_type(w2) == NUMBER_SIGNED_WORD)) /* word and const */ \
    nusmv_assert(node_word_get_width(w1) ==                             \
                 WordNumber_get_width(WORD_NUMBER(car(w2))));           \
  else if ((node_get_type(w2) == UNSIGNED_WORD ||                       \
            node_get_type(w2) == SIGNED_WORD)  &&                       \
           (node_get_type(w1) == NUMBER_UNSIGNED_WORD ||                \
            node_get_type(w1) == NUMBER_SIGNED_WORD)) /* const and word */ \
    nusmv_assert(node_word_get_width(w2) ==                             \
                 WordNumber_get_width(WORD_NUMBER(car(w1))));           \
  else if ((node_get_type(w2) == NUMBER_UNSIGNED_WORD ||                \
            node_get_type(w2) == NUMBER_SIGNED_WORD)  &&                \
           (node_get_type(w1) == NUMBER_UNSIGNED_WORD ||                \
            node_get_type(w1) == NUMBER_SIGNED_WORD)) /* const and const */ \
    nusmv_assert(WordNumber_get_width(WORD_NUMBER(car(w2))) ==          \
                 WordNumber_get_width(WORD_NUMBER(car(w1))));           \
  else nusmv_assert(false /*no other possible cases*/)


EXTERN node_ptr node_word_create ARGS((node_ptr bitval, size_t w));
EXTERN node_ptr node_word_create_from_list ARGS((node_ptr l, size_t w));
EXTERN node_ptr node_word_create_from_wordnumber ARGS((WordNumber_ptr wn));

EXTERN node_ptr 
node_word_create_from_integer ARGS((unsigned long long value, size_t width));

EXTERN node_ptr node_word_create_from_array ARGS((array_t* arr));
EXTERN array_t* node_word_to_array ARGS((node_ptr w));

/* operations: */

EXTERN node_ptr node_word_apply_unary ARGS((node_ptr wenc, int op));
EXTERN node_ptr node_word_apply_attime ARGS((node_ptr wenc, int time));
EXTERN node_ptr 
node_word_apply_binary ARGS((node_ptr wenc1, node_ptr wenc2, int op));

EXTERN node_ptr node_word_make_conjuction ARGS((node_ptr w));
EXTERN node_ptr node_word_make_disjunction ARGS((node_ptr w));
EXTERN node_ptr node_word_cast_bool ARGS((node_ptr w));
EXTERN node_ptr node_word_not ARGS((node_ptr w));
EXTERN node_ptr node_word_and ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_or ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_xor ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_xnor ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_implies ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_iff ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_equal ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_notequal ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_concat ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_selection ARGS((node_ptr word, node_ptr range));

EXTERN node_ptr node_word_extend ARGS((node_ptr a, node_ptr b,
                                       boolean isSigned));
EXTERN node_ptr 
node_word_adder ARGS((node_ptr a, node_ptr b, node_ptr carry_in, 
                      node_ptr* carry_out));

EXTERN node_ptr node_word_plus ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_minus ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_uminus ARGS((node_ptr a));
EXTERN node_ptr node_word_times ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_unsigned_divide ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_unsigned_mod ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_signed_divide ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_signed_mod ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_unsigned_less ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_unsigned_less_equal ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_unsigned_greater ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_unsigned_greater_equal ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_signed_less ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_signed_less_equal ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_signed_greater ARGS((node_ptr a, node_ptr b));
EXTERN node_ptr node_word_signed_greater_equal ARGS((node_ptr a, node_ptr b));

#endif /* __ENC_OPERATORS_H__ */
