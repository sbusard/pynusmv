/**CFile*****************************************************************

   FileName    [SymbType.c]

   PackageName [compile.symb_table]

   Synopsis    [Implementation of functions dealing with the type of
   variables in a symbol table.]

   Description []

   SeeAlso     [SymbType.h]

   Author      [Andrei Tchaltsev]

   Copyright   [
   This file is part of the ``compile.symb_table'' package of NuSMV
   version 2.  Copyright (C) 2005 by FBK-irst.

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

#include <stdarg.h>

#include "SymbType.h"
#include "symb_table.h"
#include "symb_table_int.h"

#include "compile/compileInt.h"
#include "parser/symbols.h"
#include "utils/WordNumber.h"
#include "utils/error.h"
#include "utils/ustring.h"

static char rcsid[] UTIL_UNUSED = "$Id: SymbType.c,v 1.1.2.26.4.25 2010-02-13 10:52:03 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct SymbType_TAG {
  SymbTypeTag tag;
  node_ptr body;
  boolean isMemorySharingInstance;
} SymbType;


/**Struct**********************************************************************

   Synopsis     [Data structure used to store information needed for printout.]

   Description  []

   SeeAlso      [SymbType_print, SymbType_sprint]

******************************************************************************/
typedef struct SymbTypePrinterInfo_TAG
{
  /* The file stream used in case of file printout */
  FILE* file_stream;

  /* The string buffer (initilally NULL) to be used in case of string printout */
  char* string_buffer;

  /* The string buffer length */
  unsigned buffer_size;

  /* The string buffer remaining space */
  unsigned remaining_space;

  /* Enumeration used to switch between file and string printout*/
  enum {STRING_OUTPUT, FILE_OUTPUT} output_type;
} SymbTypePrinterInfo;


typedef SymbTypePrinterInfo* SymbTypePrinterInfo_ptr;

/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

   Synopsis     [The initial buffer size used when printing on a string]

   Description  []

   SideEffects  []

   SeeAlso      [SymbType_print, SymbType_sprint]

******************************************************************************/
#define INITIAL_PRINT_BUFFER_SIZE 32


/**Macro***********************************************************************

   Synopsis     [ Initialize the given SymbTypePrinterInfo to print on a
                  string ]

   Description  []

   SideEffects  []

   SeeAlso      [SymbType_print, SymbType_sprint]

******************************************************************************/
#define INITIALIZE_PRINTER_INFO_FOR_STRING(pinfo)                   \
  pinfo.output_type = STRING_OUTPUT;                                \
  pinfo.buffer_size = INITIAL_PRINT_BUFFER_SIZE;                    \
  pinfo.remaining_space = INITIAL_PRINT_BUFFER_SIZE;                \
  pinfo.string_buffer = ALLOC(char, INITIAL_PRINT_BUFFER_SIZE + 1); \
  *(pinfo.string_buffer) = '\0'


/**Macro***********************************************************************

   Synopsis     [ Initialize the given SymbTypePrinterInfo to print on a
                  file ]

   Description  []

   SideEffects  []

   SeeAlso      [SymbType_print, SymbType_sprint]

******************************************************************************/
#define INITIALIZE_PRINTER_INFO_FOR_FILE(pinfo, file_ptr)               \
  pinfo.output_type = FILE_OUTPUT;                                      \
  pinfo.file_stream = file_ptr


/**Macro***********************************************************************

   Synopsis     [ Clear the given SymbTypePrinterInfo ]

   Description  []

   SideEffects  []

   SeeAlso      [SymbType_print, SymbType_sprint]

******************************************************************************/
#define CLEAR_PRINTER_INFO(pinfo)                                       \
  if(STRING_OUTPUT == pinfo.output_type) FREE(pinfo.string_buffer)


/*---------------------------------------------------------------------------*/
/* Constants declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static boolean
node_equal ARGS((node_ptr n1, node_ptr n2));

static void symb_type_sprint_aux ARGS((const SymbType_ptr self,
                                       SymbTypePrinterInfo_ptr printer_info));

static void str_print ARGS((SymbTypePrinterInfo_ptr printer_info,
                            const char *fmt, ...));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis    [Class SymbType constructor]

   Description [The tag must be a correct tag. The 'body' is the
   additional info corresponding to a particular kind of the type:
   * for a enum type the body is the list of values;
   * for "BOOL", "INT" or "REAL" the body is unused, and set to Nil;
   * for signed and unsigned "WORD" it is the NUMBER node defining the
      width of the type;
   * for "WORDARRAY", the body is a pair of NUMBER nodes, defining
      the width of the address, and the width of the value.
   * for everything else body is Nil;

   Note that array types have to be created with
   SymbType_create_array, not with this constructor.

   Set-types are used with expressions which represent a set values.
   "NO-TYPE" is used with expressions which normally do not
        have any type such as assignments.
   "ERROR" type indicates an error (not an actual type).

   No-type, error-type and all set-types (boolean-set, integer-set,
   symbolic-set, symbolic-integer-set) should not be created with this
   constructor, but only with memory-shared function
   SymbTablePkg_..._type.  The reason behind this constrain is that
   only expressions (not variables) can have these types, therefore
   only memory-shared versions of these types are required.

   The constructor does not create a copy of the body, but just remember
   the pointer.

   NB: system "reset" command destroys all node_ptr objects, including those
   used in SymbType_ptr. So destroy all symbolic types before the destruction
   of node_ptr objects, i.e. before or during "reset"]

   SideEffects        [allocate memory]

   SeeAlso            [SymbType_create_array, SymbType_destroy]

******************************************************************************/
SymbType_ptr SymbType_create(SymbTypeTag tag, node_ptr body)
{
  SymbType_ptr self = ALLOC(SymbType, 1);
  SYMB_TYPE_CHECK_INSTANCE(self);

  nusmv_assert(SYMB_TYPE_NONE <= tag && tag <= SYMB_TYPE_ERROR);
  /* array type have to be create with SymbType_create_array */
  nusmv_assert(SYMB_TYPE_ARRAY != tag);

  self->tag = tag;
  self->isMemorySharingInstance = false;

  if (SYMB_TYPE_ENUM == tag) {
    /* these is an enum type. Find out which kind of enum this is.
       It can be a pure integer enum, a pure symbolic enum
       or integer-symbolic enum.
    */
    enum Enum_types enumType = -1; /* for sure incorrect value */
    node_ptr iter = body;
    int thereIsInt = 0;
    int thereIsSymbolic = 0;
    while (iter != Nil) {
      switch (node_get_type(car(iter))) {
      case ATOM: /* symbolic constant */
      case BIT:
      case DOT:
      case ARRAY:
        thereIsSymbolic = 1;
        break;
      case NUMBER: /* integer numbers */
        thereIsInt = 1;
        break;

      case TRUEEXP:
      case FALSEEXP:
        /* TRUE and FALSE cannot be in a enum
        */
        error_unreachable_code();

      default: /* only numbers, true, false, and
                  symbolic constants can be in the constant-list (enum-type).
               */
        print_node(nusmv_stderr, body); fprintf(nusmv_stderr,"\n\n");
        error_unreachable_code();
      }
      iter = cdr(iter);
    }/* while */
    /* there must be constant or symbolic constant in the enum */
    nusmv_assert(thereIsInt | thereIsSymbolic);

    enumType = (thereIsInt && thereIsSymbolic)
      ?  ENUM_TYPE_INT_SYMBOLIC
      : thereIsInt
      ? ENUM_TYPE_PURE_INT
      : ENUM_TYPE_PURE_SYMBOLIC;
    /* we add additional node to remember the kind of the enum-type */
    self->body = new_node(enumType, body, Nil);
  }
  else {

    /* except ENUM type only Word and WordArray can can have body */
    nusmv_assert((Nil != body) == (SYMB_TYPE_SIGNED_WORD == tag ||
                                   SYMB_TYPE_UNSIGNED_WORD == tag ||
                                   SYMB_TYPE_WORDARRAY == tag));
    /* usual type => just remember the body */
    self->body = body;
  }
  return self;
}


/**Function********************************************************************

   Synopsis    [Class SymbType constructor for array types only]

   Description [This is specialized version of SymbType_create
   which is designed for array types only.
   It is implemented as a special construtor because array types are quite
   different from all the others.

   Parameter subtype is the subtype of the array type. This type has
   to be not-memory-shared and its ownership is passed to created
   type. I.e. subtype will be destroyed when returned type is destroyed.

   lower_bound, upper-bound are the lower and upper bounds,resp, of
   the array.

   All the constrains about memory, lifetype, etc are the same as for
   SymbType_create.
   ]

   SideEffects        [allocate memory]

   SeeAlso            [SymbType_destroy]

******************************************************************************/
SymbType_ptr SymbType_create_array(SymbType_ptr subtype,
                                   int lower_bound, int upper_bound)
{
  SymbType_ptr self = ALLOC(SymbType, 1);
  SYMB_TYPE_CHECK_INSTANCE(self);

  nusmv_assert(NULL != subtype);
  nusmv_assert(upper_bound >= lower_bound);

  self->tag = SYMB_TYPE_ARRAY;
  self->isMemorySharingInstance = false;
  /* the body of the array type is a list
     CONS(sub_type, CONS(lower bound, upper bound)) */
  self->body = cons(NODE_PTR(subtype),
                    cons(NODE_FROM_INT(lower_bound),
                                  NODE_FROM_INT(upper_bound)));
  return self;
}

/**Function********************************************************************

   Synopsis           [Class SymbType destructor]

   Description        [Deallocate the memory. The destructor
   does not deallocate memory from the type's body (since the
   constructor did not created the body).

   NOTE: If self is a memory sharing type instance, i.e. a type returned by
   SymbTablePkg_..._type functions then the destructor will not delete
   the type.]

   SideEffects        []

   SeeAlso            [SymbType_create]

******************************************************************************/
void SymbType_destroy(SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  if (!self->isMemorySharingInstance) { /* this is a usual type instance */

    /* debugging code to detect double freeing */
    nusmv_assert(self->body != NODE_FROM_INT(-1));

    if (SYMB_TYPE_ENUM == self->tag) {
      /* for the enum-type we added one addional node => remove it now */
      node_node_setcar(self->body, Nil);
      free_node(self->body);
    }
    else if (SYMB_TYPE_ARRAY == self->tag) {
      /* for array_type 2 nodes are to be deleted and the subtype.

         NOTE FOR DEVELOPERS: keep implementation of this function andq
         SymbType_destroy the same.
      */
      SymbType_ptr subtype = SYMB_TYPE(car(self->body));
      SymbType_destroy(subtype);
      free_node(cdr(self->body));
      free_node(self->body);
    }

    /* debugging code to detect double freeing */
    self->body = NODE_FROM_INT(-1);

    FREE(self);
  }
}


/**Function********************************************************************

   Synopsis           [Class SymbType copy-constructor]

   Description [This function takes one type and returns its copy.

   Note: the body of the type is not copied, i.e. just pointer is remembered.
   See SymbType_create for more info about body.

   Note: the input type should not be a memory-shared type (since there is no
   meaning in coping a memory sharing type).
   ]

   SideEffects        [allocate memory]

   SeeAlso            [SymbType_destroy]

******************************************************************************/
SymbType_ptr SymbType_copy(const SymbType_ptr self)
{

  if (SYMB_TYPE_ARRAY == self->tag) {
    /* array type has a special constructor */
   return SymbType_create_array(SymbType_copy(SymbType_get_array_subtype(self)),
                                SymbType_get_array_lower_bound(self),
                                SymbType_get_array_upper_bound(self));
  }

  return SymbType_create(self->tag,
                         self->tag == SYMB_TYPE_ENUM
                         ? car(self->body) : self->body);
}


/**Function********************************************************************

   Synopsis           [Private class SymbType constructor
   for memory sharing type instances]

   Description        [The difference from the public constructor is that this
   constructor marks the created type as a memory sharing type. As
   result the public constructor will not be able to destroy memory
   sharing instance of a type. Use the private constructor
   SymbType_destroy_memory_sharing_type to destroy such instances.
   ]

   SideEffects        [allocate memory]

   SeeAlso            [SymbType_create, SymbType_destroy_memory_sharing_type]

******************************************************************************/
SymbType_ptr
SymbType_create_memory_sharing_type(SymbTypeTag tag, node_ptr body)
{
  SymbType_ptr self = SymbType_create(tag, body);
  self->isMemorySharingInstance = true;
  return self;
}


/**Function********************************************************************

   Synopsis           [Private class SymbType constructor
   for memory sharing array type instances]

   Description        [The same as SymbType_create_memory_sharing_type
   but can be used to create array types.
   subtype has to be memory shared.]

   SideEffects        [allocate memory]

   SeeAlso            [SymbType_create, SymbType_destroy_memory_sharing_type]

******************************************************************************/
SymbType_ptr
SymbType_create_memory_sharing_array_type(SymbType_ptr subtype,
                                          int lower_bound, int upper_bound)
{
  nusmv_assert(subtype->isMemorySharingInstance);

  SymbType_ptr self = SymbType_create_array(subtype, lower_bound, upper_bound);
  self->isMemorySharingInstance = true;
  return self;
}


/**Function********************************************************************

   Synopsis           [Private Class SymbType destructor
   for memory sharing instances of types.]

   Description        [The same as the public destructor SymbType_destroy
   but 'self' has to be created by private constructor
   SymbType_create_memory_sharing_type only.]

   SideEffects        []

   SeeAlso            [SymbType_create_memory_sharing_type, SymbType_create]

******************************************************************************/
void SymbType_destroy_memory_sharing_type(SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  /* this is a memory sharing instance */
  nusmv_assert(self->isMemorySharingInstance);

  /* array type is destroyed separately because we should not allow
     SymbType_destroy to go recursively down to subtype which
     may be already destroyed.

     NOTE FOR DEVELOPERS: keep implementation of this function and
     SymbType_destroy the same.
  */
  if (SYMB_TYPE_ARRAY == self->tag) {
    /* debugging code to detect double freeing */
    nusmv_assert(self->body != NODE_FROM_INT(-1));

    /* for array_type 2 nodes are to be deleted */
    free_node(cdr(self->body));
    free_node(self->body);

    /* debugging code to detect double freeing */
    self->body = NODE_FROM_INT(-1);

    FREE(self);
    return;
  }

  self->isMemorySharingInstance = false;
  SymbType_destroy(self);
}


/**Function********************************************************************

   Synopsis           [Returns the tag (the kind) of the type]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
SymbTypeTag SymbType_get_tag(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  return self->tag;
}


/**Function********************************************************************

   Synopsis [Returns true if the type is a enum-type, or else returns false]

   Description []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
boolean SymbType_is_enum(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  return SYMB_TYPE_ENUM == self->tag;
}


/**Function********************************************************************

   Synopsis [Returns true, if the type is boolean. Otherwise - returns false.]

   Description [The kind of enum-type is analysed in the constructor.]

   SideEffects        []

   SeeAlso            [SymbType_create]

******************************************************************************/
boolean SymbType_is_boolean(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  return SYMB_TYPE_BOOLEAN == self->tag;
}


/**Function********************************************************************

   Synopsis [Returns true if the type is a integer-type, or else returns false]

   Description []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
boolean SymbType_is_integer(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  return SYMB_TYPE_INTEGER == self->tag;
}


/**Function********************************************************************

   Synopsis [Returns true if the type is a real-type, or else returns false]

   Description []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
boolean SymbType_is_real(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  return SYMB_TYPE_REAL == self->tag;
}


/**Function********************************************************************

   Synopsis    [Returns true, if the type is a enum-type and its value
   are integers only. Otherwise - returns false.]

   Description [The kind of enum-type is analysed in the constructor.]

   SideEffects        []

   SeeAlso            [SymbType_create]

******************************************************************************/
boolean SymbType_is_pure_int_enum(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  return SYMB_TYPE_ENUM == self->tag &&
    ENUM_TYPE_PURE_INT == node_get_type(self->body);
}


/**Function********************************************************************

   Synopsis    [Returns true, if the type is a enum-type and its value
   are symbolic constants only. Otherwise - returns false.]

   Description [The kind of enum-type is analysed in the constructor.]

   SideEffects        []

   SeeAlso            [SymbType_create]

******************************************************************************/
boolean SymbType_is_pure_symbolic_enum(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  return SYMB_TYPE_ENUM == self->tag &&
    ENUM_TYPE_PURE_SYMBOLIC == node_get_type(self->body);
}


/**Function********************************************************************

   Synopsis    [Returns true, if the type is a enum-type and its value
   are symbolic AND integer constants. Otherwise - returns false.]

   Description [The kind of enum-type is analysed in the constructor.]

   SideEffects        []

   SeeAlso            [SymbType_create]

******************************************************************************/
boolean SymbType_is_int_symbolic_enum(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  return SYMB_TYPE_ENUM == self->tag &&
    ENUM_TYPE_INT_SYMBOLIC == node_get_type(self->body);
}


/**Function********************************************************************

   Synopsis    [Returns true, if the type is a Unsigned Word type and the width of
   the word is 1. Otherwise - returns false.]

   Description []

   SideEffects []

   SeeAlso     [SymbType_create]

******************************************************************************/
boolean SymbType_is_word_1(const SymbType_ptr self)
{
  /* expects the body of Word type to be NUMBER */
  return SYMB_TYPE_UNSIGNED_WORD == self->tag && 1 == node_get_int(self->body);
}


/**Function********************************************************************

   Synopsis    [Returns true, if the type is an unsigned Word type]

   Description []

   SideEffects []

   SeeAlso     [SymbType_create]

******************************************************************************/
boolean SymbType_is_unsigned_word(const SymbType_ptr self)
{
  /* expects the body of Word type to be NUMBER */
  return SYMB_TYPE_UNSIGNED_WORD == self->tag;
}


/**Function********************************************************************

   Synopsis    [Returns true, if the type is a signed Word type]

   Description []

   SideEffects []

   SeeAlso     [SymbType_create]

******************************************************************************/
boolean SymbType_is_signed_word(const SymbType_ptr self)
{
  /* expects the body of Word type to be NUMBER */
  return SYMB_TYPE_SIGNED_WORD == self->tag;
}


/**Function********************************************************************

   Synopsis    [Returns true, if the type is a Word type (signed or unsigned)]

   Description []

   SideEffects        []

   SeeAlso            [SymbType_create]

******************************************************************************/
boolean SymbType_is_word(const SymbType_ptr self)
{
  /* expects the body of Word type to be NUMBER */
  return SYMB_TYPE_UNSIGNED_WORD == self->tag ||
    SYMB_TYPE_SIGNED_WORD == self->tag;
}


/**Function********************************************************************

   Synopsis    [Returns true, if the type is one of the set-types, i.e.
   boolean-set, integer-set, symbolic-set, integer-symbolic-set, and
   false otherwise.]

   Description []

   SideEffects        []

   SeeAlso            [SymbType_create]

******************************************************************************/
boolean SymbType_is_set(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  return SYMB_TYPE_SET_BOOL == self->tag ||
    SYMB_TYPE_SET_INT == self->tag ||
    SYMB_TYPE_SET_SYMB == self->tag ||
    SYMB_TYPE_SET_INT_SYMB == self->tag;
}


/**Function********************************************************************

   Synopsis           [Returns true, if the type is a String type]

   Description        []

   SideEffects        []

   SeeAlso            [SymbType_create]

******************************************************************************/
boolean SymbType_is_string(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  return SYMB_TYPE_STRING == self->tag;
}


/**Function********************************************************************

   Synopsis    [Returns true, if the type is a error-type, and false otherwise.]

   Description [Error type is used to indicate an error]

   SideEffects        []

   SeeAlso            [SymbType_create]

******************************************************************************/
boolean SymbType_is_error(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  return SYMB_TYPE_ERROR == self->tag;
}


/**Function********************************************************************

   Synopsis    [Returns true, if the type is a statement-type,
   and false otherwise.]

   Description []

   SideEffects []

   SeeAlso     [SymbType_create]

******************************************************************************/
boolean SymbType_is_statement(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);

  return SYMB_TYPE_STATEMENT == self->tag;
}


/**Function********************************************************************

   Synopsis    [Returns true, if the type is one of infinite-precision types]

   Description [Inifinite-precision types are such as integer and real.]

   SideEffects []

   SeeAlso     [SymbType_create]

******************************************************************************/
boolean SymbType_is_infinite_precision(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);

  return SYMB_TYPE_INTEGER == self->tag || SYMB_TYPE_REAL == self->tag;
}


/**Function********************************************************************

   Synopsis [Returns true if the type is an array-type, or else returns false]

   Description []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
boolean SymbType_is_array(const SymbType_ptr self)
{
  return SYMB_TYPE_ARRAY == self->tag;
}


/**Function********************************************************************

   Synopsis           [Returns true if the given type is a wordarray]

   Description        [.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
boolean SymbType_is_wordarray(const SymbType_ptr self)
{
  return SYMB_TYPE_WORDARRAY == self->tag;
}


/**Function********************************************************************

   Synopsis    [The function calculate how many bits is required to
   store a value of a given type]

   Description [This function can be invoked only on finite-precision
   valid type of variables. This means that such types as no-type or error-type
   or real or any memory-shared ones should not be given to this function.
   ]

   SideEffects []

   SeeAlso     [SymbType_create]

******************************************************************************/
int SymbType_calculate_type_size(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  nusmv_assert(!self->isMemorySharingInstance); /* must not be memory-shared */
  int size = -1; /* for sure wrong value */

  if (SYMB_TYPE_BOOLEAN == self->tag) {
    return 1;
  }
  else if (SYMB_TYPE_ENUM == self->tag) {
    int num = llength(SymbType_get_enum_type_values(self));
    nusmv_assert(0 != num); /* enum type cannot be without values */

    /* special case: enumeration of 1 value. Inside NuSMV this never happens 
       but with external tools it is possible */
    if (num == 1) return 1;

    /* N values are encoded with numbers 0..N-1, i.e. it is necessary
       to have bits enough to represent N-1 as unsigned
       integer */
    num -= 1;

    size = 0;
    while (num != 0) { /* calculate logarithm */
      num = num >>1;
      ++size;
    }
  }
  else if (SYMB_TYPE_UNSIGNED_WORD == self->tag ||
           SYMB_TYPE_SIGNED_WORD == self->tag) {
    size = SymbType_get_word_width(self);
  }
  else {
    error_unreachable_code();/* only enum and word can be valid variable type */
  }

  return size;
}


/**Function********************************************************************

   Synopsis           [Generates and returns a list of all possible values
   of a particular Unsigned Word type]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr SymbType_generate_all_word_values(const SymbType_ptr self)
{
  /* the function is implemented only for unsigned words */
  nusmv_assert(SYMB_TYPE_UNSIGNED_WORD == self->tag);

  /* create all the constants for the Word type */
  int width;
  unsigned long long value;
  node_ptr list;

  /* All assertions are checked in the function SymbType_get_word_width */

  width = SymbType_get_word_width(self);
  nusmv_assert(width <= WordNumber_max_width());

  list = Nil;
  /* create the max value. Two left shifts are used because in C
     shifting by a full width is not allowed
  */
  value = ~((~0ull) << (width - 1) << 1);
  ++value;
  do {
    --value;
    list = cons(find_node(NUMBER_UNSIGNED_WORD,
                          (node_ptr) WordNumber_from_integer(value, width),
                          Nil),
                list);
  }
  while ( value > 0);
  return list;
}


/**Function********************************************************************

   Synopsis           [Returns the width of a Word type]

   Description        [The given type should be Word and the
   body of the type (given to the constructor) should be NUMBER node.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
int SymbType_get_word_width(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  nusmv_assert((SYMB_TYPE_UNSIGNED_WORD == self->tag ||
                SYMB_TYPE_SIGNED_WORD == self->tag
                ) &&
               node_get_type(self->body) == NUMBER);
  return node_get_int(self->body); /* the width */
}


/**Function********************************************************************

   Synopsis           [Returns the width of the address in a WordArray type]

   Description        [.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
int SymbType_get_wordarray_awidth(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);

  nusmv_assert(SYMB_TYPE_WORDARRAY == self->tag &&
               node_get_type(self->body) == CONS &&
               node_get_type(car(self->body)) == NUMBER &&
               node_get_type(cdr(self->body)) == NUMBER);

  return node_get_int(car(self->body)); /* the address width */
}


/**Function********************************************************************

   Synopsis           [Returns the width of the value in a WordArray type]

   Description        [.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
int SymbType_get_wordarray_vwidth(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);

  nusmv_assert(SYMB_TYPE_WORDARRAY == self->tag &&
               node_get_type(self->body) == CONS &&
               node_get_type(car(self->body)) == NUMBER &&
               node_get_type(car(self->body)) == NUMBER);

  return node_get_int(cdr(self->body)); /* the value width */
}

/**Function********************************************************************

   Synopsis    [Returns the line number where the type was declared.]

   Description [The body of the type, provided during construction, is
   a node NUMBER specifying the width of the Word or a node CONS
   specifying the address-value widths or WordArray.  This node was
   create during parsing and contains the line number of the type
   declaration.
   NB: The type should not be memory-sharing.
   NB: Virtually this function is used only in TypeChecker_is_type_wellformed
   ]

   SideEffects        []

   SeeAlso            [SymbType_create]

******************************************************************************/
int SymbType_get_word_line_number(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  nusmv_assert(( ( SYMB_TYPE_UNSIGNED_WORD == self->tag ||
                   SYMB_TYPE_SIGNED_WORD == self->tag
                   ) &&
                 node_get_type(self->body) == NUMBER
                 ) ||
               ( SYMB_TYPE_WORDARRAY == self->tag &&
                 node_get_type(self->body) == CONS));
  nusmv_assert(!self->isMemorySharingInstance); /* type is not memory-shared */
  return node_get_lineno(self->body);
}


/**Function********************************************************************

   Synopsis           [Returns the list of values of an enum type]

   Description        [The given type has to be a ENUM type.
   The return list is a list of all possible values of a enum type. This list
   was provided during construction.

   NB: Memory sharing types do not have particular values, since they
   are "simplified".]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr SymbType_get_enum_type_values(const SymbType_ptr self)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  nusmv_assert(SYMB_TYPE_ENUM == self->tag);
  nusmv_assert(!self->isMemorySharingInstance); /* type is not memory-shared */
  /* for the enum-type we added one addional node => skip it now */
  return car(self->body);
}


/**Function********************************************************************

   Synopsis            [Get inner type of an array]

   Description         [The returned pointer belongs to the ginven SymbType_ptr
                        and must not be freed]

   SideEffects         []

   SeeAlso             []

******************************************************************************/
SymbType_ptr SymbType_get_array_subtype(const SymbType_ptr self) {
  nusmv_assert(SymbType_is_array(self));
  /* subtype is the first element in body list : see the array constructor */
  return SYMB_TYPE(car(self->body));
}


/**Function********************************************************************

   Synopsis            [Get array lower bound]

   Description         []

   SideEffects         []

   SeeAlso             []

******************************************************************************/
int SymbType_get_array_lower_bound(const SymbType_ptr self) {
  nusmv_assert(SymbType_is_array(self));
  /* low bound is the second element in body list : see the array constructor */
  return NODE_TO_INT(car(cdr(self->body)));
}


/**Function********************************************************************

   Synopsis            [Get array upper bound]

   Description         []

   SideEffects         []

   SeeAlso             []

******************************************************************************/
int SymbType_get_array_upper_bound(const SymbType_ptr self) {
  nusmv_assert(SymbType_is_array(self));
  /* up bound is the third element in body list : see the array constructor */
  return NODE_TO_INT(cdr(cdr(self->body)));
}


/**Function********************************************************************

   Synopsis            [Prints the type structure to the output stream.]

   Description         [This function is made very similar to print_node.
   If a Enum type was created with SymbType_create then all its values will be
   printed, otherwise the type was created with SymbTablePkg_..._type
   and simplified type name (instead of actual type values) is printed.
   ]

   SideEffects        []

   SeeAlso            [ SymbType_sprint ]

******************************************************************************/
void SymbType_print(const SymbType_ptr self, FILE* output_stream)
{
  SymbTypePrinterInfo printer_info;

  INITIALIZE_PRINTER_INFO_FOR_FILE(printer_info, output_stream);
  symb_type_sprint_aux(self, &printer_info);
  CLEAR_PRINTER_INFO(printer_info);
}


/**Function********************************************************************

   Synopsis            [ Return a string representation of the given type. ]

   Description         [ This function is made very similar to sprint_node.
                         If an Enum type was created with
                         SymbType_create then all its values will be
                         printed, otherwise the type was created with
                         SymbTablePkg_..._type and simplified type
                         name (instead of actual type values) is
                         printed.

                         The returned string must be released by the caller. ]

   SideEffects        [ The returned string is allocated and has to be released
                        by the caller]

   SeeAlso            [ SymbType_print ]

******************************************************************************/
char* SymbType_sprint(const SymbType_ptr self)
{
  SymbTypePrinterInfo printer_info;

  INITIALIZE_PRINTER_INFO_FOR_STRING(printer_info);

  symb_type_sprint_aux(self, &printer_info);
  char* res = util_strsav(printer_info.string_buffer);

  CLEAR_PRINTER_INFO(printer_info);

  return res;
}


/**Function********************************************************************

   Synopsis           [returns true if the given type is "backward compatible",
   i.e. a enum or integer type.]

   Description        [We distinguish "old" types because we may want to turn
   off the type checking on these types for backward
   compatibility. Integer is also considered as "old", because an enum
   of integer values is always casted to Integer.]

   SideEffects        []

   SeeAlso            []
******************************************************************************/
boolean SymbType_is_back_comp(const SymbType_ptr type)
{
  return SYMB_TYPE_BOOLEAN == SymbType_get_tag(type) ||
    SYMB_TYPE_ENUM == SymbType_get_tag(type) ||
    SYMB_TYPE_INTEGER == SymbType_get_tag(type);
}



/**Function********************************************************************

   Synopsis           [Returns one of the given types, if the other one
   can be implicitly converted to the former one. Otherwise - Nil.]

   Description        [The implicit conversion is performed
   in accordance to the type order.
   NOTE: only memory-shared types can be given to this function.]

   SideEffects        []

   SeeAlso            []
******************************************************************************/
SymbType_ptr SymbType_get_greater(const SymbType_ptr type1,
                                  const SymbType_ptr type2)
{
  SymbType_ptr returnType;

  returnType = SymbType_convert_right_to_left(type1, type2);
  if (returnType) return returnType; /* conversion was successful */
  /* return with swapped types and return whatever is obtained */
  return SymbType_convert_right_to_left(type2, type1);
}


/**Function********************************************************************

   Synopsis           [Returns the left type, if the right
   one can be implicitly converted to the left one. NULL - otherwise]

   Description        [The implicit conversion is performed
   in accordance to the type order.
   NOTE: only memory-shared types can be given to this function.]

   SideEffects        []

   SeeAlso            []
******************************************************************************/
SymbType_ptr
SymbType_convert_right_to_left(SymbType_ptr leftType,
                               SymbType_ptr rightType)
{
  /* types have to be shared. So no worry about memory */
  nusmv_assert(leftType->isMemorySharingInstance &&
               rightType->isMemorySharingInstance);

  /* the same pointers, so, the same types, so, just return */
  if (leftType == rightType) return leftType;

  /* pointer are different => the types are different.
     Try to perform implicit convertion of the right operand to the left one.
     No need to care about word, word-array and no-type since
     they cannot be implicitly converted to any other type.
     NB: No-type is created for assignment, and symbolic (predicate-normalised)
     FSM may push assignments into case-expressions, for example.
  */

  /* real * int => real */
  if (SymbType_get_tag(leftType) == SYMB_TYPE_REAL) {
    return (SymbType_get_tag(rightType) == SYMB_TYPE_INTEGER)
      ? leftType : nullType;
  }

  /* int_symb_enum * (int or symb_enum) => int_symb_enum */
  if (SymbType_is_int_symbolic_enum(leftType)) {
    return (SymbType_get_tag(rightType) == SYMB_TYPE_INTEGER ||
            SymbType_is_pure_symbolic_enum(rightType))
      ? leftType : nullType;
  }

  /* arrays are compatible only with array of the same size and
     compatible subtypes */
  if (SymbType_is_array(leftType) || SymbType_is_array(rightType)) {
    if (!SymbType_is_array(leftType) ||
        !SymbType_is_array(rightType) ||
        SymbType_get_array_lower_bound(leftType)
          != SymbType_get_array_lower_bound(rightType) ||
        SymbType_get_array_upper_bound(leftType)
          != SymbType_get_array_upper_bound(rightType)) {
      return nullType;
    }
    SymbType_ptr subLeft = SymbType_get_array_subtype(leftType);
    SymbType_ptr subRight = SymbType_get_array_subtype(rightType);
    SymbType_ptr subtype = SymbType_convert_right_to_left(subLeft, subRight);
    if (subtype == nullType) return nullType;
    /* create a proper memory shared array type */
    return SymbTablePkg_array_type(subtype,
                                   SymbType_get_array_lower_bound(leftType),
                                   SymbType_get_array_upper_bound(leftType));
  }

  /* only set-types remain */

  /* int_set * bool_set=> int_set */
  /* if (SymbType_get_tag(leftType) == SYMB_TYPE_SET_INT) { */
  /*   return SymbType_get_tag(rightType) == SYMB_TYPE_SET_BOOL */
  /*     ? leftType : SYMB_TYPE(NULL); */
  /* } */

  /* int_symb_set * any set-type => int_symb_set  */
  if (SymbType_get_tag(leftType) == SYMB_TYPE_SET_INT_SYMB) {
    return SymbType_is_set(rightType) ? leftType : SYMB_TYPE(NULL);
  }

  /* all other combinations are illegal */
  return nullType;
}


/**Function********************************************************************

   Synopsis           [Returns the minimal type to which the both given types
   can be converted, or Nil if there is none.]

   Description [The implicit conversion is performed in accordance to
   the type order.  NOTE: only memory-shared types can be given to this
   function except for SYMB_TYPE_ARRAY which can be non-memory shared]

   SideEffects        []

   SeeAlso            []
******************************************************************************/
SymbType_ptr
SymbType_get_minimal_common(SymbType_ptr type1,
                            SymbType_ptr type2)
{
  /* types have to be shared. So no worry about memory */
  nusmv_assert(type1->isMemorySharingInstance &&
               type2->isMemorySharingInstance);

  /* the same pointers, so, the same types, so, just return the type */
  if (type1 == type2) return type1;

  /* pointer are different => the types are different.
     Try to perform implicit convertion of both operand to
     a minimal type.
     No need to care about word, word-array and no-type since
     they cannot be implicitly converted to any other type.
     NB: No-type is created for assignment, and (predicate-normalised)
     symbolic FSM may push assignments into case-expressions, for example.
  */

  /* both are [Int or Real] -> Real */
  if ( ( SymbType_get_tag(type1) == SYMB_TYPE_INTEGER ||
         SymbType_get_tag(type1) == SYMB_TYPE_REAL ) &&
       ( SymbType_get_tag(type2) == SYMB_TYPE_INTEGER ||
         SymbType_get_tag(type2) == SYMB_TYPE_REAL ) ) {
    return SymbTablePkg_real_type();
  }

  /* both are [Int or Symbolic or SymbolicInt] -> SymbolicInt */
  if ( ( SymbType_get_tag(type1) == SYMB_TYPE_INTEGER ||
         SymbType_is_pure_symbolic_enum(type1) ||
         SymbType_is_int_symbolic_enum(type1) ) &&
       ( SymbType_get_tag(type2) == SYMB_TYPE_INTEGER ||
         SymbType_is_pure_symbolic_enum(type2) ||
         SymbType_is_int_symbolic_enum(type2) ) ) {
    return SymbTablePkg_int_symbolic_enum_type();
  }

  /* arrays are compatible only with array of the same size and
     compatible subtypes */
  if (SymbType_is_array(type1) || SymbType_is_array(type2)) {
    if (!SymbType_is_array(type1) ||
        !SymbType_is_array(type2) ||
        SymbType_get_array_lower_bound(type1)
          != SymbType_get_array_lower_bound(type2) ||
        SymbType_get_array_upper_bound(type1)
          != SymbType_get_array_upper_bound(type2)) {
      return SYMB_TYPE(NULL);
    }

    SymbType_ptr sub1 = SymbType_get_array_subtype(type1);
    SymbType_ptr sub2 = SymbType_get_array_subtype(type2);
    SymbType_ptr subtype = SymbType_get_minimal_common(sub1, sub2);
    if (subtype == NULL) return SYMB_TYPE(NULL);
    /* create a proper memory shared array type */
    return SymbTablePkg_array_type(subtype,
                                   SymbType_get_array_lower_bound(type1),
                                   SymbType_get_array_upper_bound(type1));
  }

  /* only set types remain */

  /* both are [int_set or bool_set] => int_set */
  /* if ( (SymbType_get_tag(type1) == SYMB_TYPE_SET_BOOL || */
  /*       SymbType_get_tag(type1) == SYMB_TYPE_SET_INT) && */
  /*      (SymbType_get_tag(type2) == SYMB_TYPE_SET_BOOL || */
  /*       SymbType_get_tag(type2) == SYMB_TYPE_SET_INT)) { */
  /*   return SymbTablePkg_integer_set_type(); */
  /* } */

  /* both are any set type => int_symb_set */
  if (SymbType_is_set(type1) || SymbType_is_set(type2)) {
    return SymbTablePkg_integer_symbolic_set_type();
  }

  /* the given types are incompatible => error */
  return SYMB_TYPE(NULL);
}


/**Function********************************************************************

   Synopsis           [Returns a minimal set-type which the given type
   can be implicitly converted to, or NULL if this is impossible.]

   Description        [
   The implicit conversion is performed in accordance to the type order.
   NOTE: only memory-shared types can be given to this function.]

   SideEffects        []

   SeeAlso            [SymbType_make_type_from_set_type]
******************************************************************************/
SymbType_ptr SymbType_make_set_type(const SymbType_ptr self)
{
  /* converts to set types if it is required:
     bool -> bool_set
     int -> int_set
     symb -> symb_set
     int_symb_enum -> int_symb_set
     any set type -> the same set type
     other type -> NULL
  */
  if (SymbType_is_boolean(self)) {
    return SymbTablePkg_boolean_set_type();
  }
  if (SymbType_get_tag(self) == SYMB_TYPE_INTEGER) {
    return SymbTablePkg_integer_set_type();
  }
  else if (SymbType_is_pure_symbolic_enum(self)) {
    return SymbTablePkg_symbolic_set_type();
  }
  else if (SymbType_is_int_symbolic_enum(self)) {
    return SymbTablePkg_integer_symbolic_set_type();
  }
  else if (SymbType_is_set(self)) {
    return self;
  }
  else return nullType;
}


/**Function********************************************************************

   Synopsis           [This function is opposite to
   SymbType_make_set_type, i.e. if the given type is one of
   the set-types, then the type without "set" suffix is
   returned. Otherwise the type is returned without change.]

   Description        [
   More precisely the following conversion takes place:
   boolean-set -> boolean
   integer-set ->integer
   symbolic-set -> symbolic-enum
   integer-symbolic-set -> integer-symbolic-set
   another type -> the same type

   The implicit conversion is performed in accordance to the type order.
   NOTE: only memory-shared types can be given to this function.]

   SideEffects        []

   SeeAlso            [SymbType_make_set_type]
******************************************************************************/
SymbType_ptr SymbType_make_from_set_type(const SymbType_ptr self)
{
  if (SymbType_get_tag(self) == SYMB_TYPE_SET_BOOL) {
    return SymbTablePkg_boolean_type();
  }
  if (SymbType_get_tag(self) == SYMB_TYPE_SET_INT) {
    return SymbTablePkg_integer_type();
  }
  else if (SymbType_get_tag(self) == SYMB_TYPE_SET_SYMB) {
    return SymbTablePkg_pure_symbolic_enum_type();
  }
  else if (SymbType_get_tag(self) == SYMB_TYPE_SET_INT_SYMB) {
    return SymbTablePkg_int_symbolic_enum_type();
  }
  else return self;
}


/**Function********************************************************************

   Synopsis           [This function takes a NOT memory shared type
   and returns a memory shared one. ]

   Description        [The input type should have
   a corresponding memory shared type. For example, function type
   and error type do not have memory shared instances.]

   SideEffects        []

   SeeAlso            []
******************************************************************************/
SymbType_ptr SymbType_make_memory_shared(const SymbType_ptr self)
{
  SymbType_ptr res = self;

  switch (SymbType_get_tag(self)) {
  case SYMB_TYPE_BOOLEAN:
    res = SymbTablePkg_boolean_type();
    break;

  case SYMB_TYPE_INTEGER:
    res = SymbTablePkg_integer_type();
    break;

  case SYMB_TYPE_REAL:
    res = SymbTablePkg_real_type();
    break;

  case SYMB_TYPE_UNSIGNED_WORD:
    res = SymbTablePkg_unsigned_word_type(SymbType_get_word_width(self));
    break;

  case SYMB_TYPE_SIGNED_WORD:
    res = SymbTablePkg_signed_word_type(SymbType_get_word_width(self));
    break;

  case SYMB_TYPE_WORDARRAY:
    res = SymbTablePkg_wordarray_type(SymbType_get_wordarray_awidth(self),
                                      SymbType_get_wordarray_vwidth(self));
    break;

  case SYMB_TYPE_ENUM:
    if (SymbType_is_pure_int_enum(self)) {
      /* RC: This was a tentative bug fix see 1435. It does not
         work as Integer type is largely used in the TC. In
         particular this fixes the mentioned issue 1435, but is
         breaks the whole type system, so that any expression like
         "scalar + 1" got broken. */
      /* res = SymbTablePkg_pure_int_enum_type(); */
      res = SymbTablePkg_integer_type(); /* convert to Integer */
    }
    else if (SymbType_is_pure_symbolic_enum(self)) {
      res = SymbTablePkg_pure_symbolic_enum_type();
    }
    else {
      nusmv_assert(SymbType_is_int_symbolic_enum(self));
      res = SymbTablePkg_int_symbolic_enum_type();
    }
    break;

  case SYMB_TYPE_STRING:
    res = SymbTablePkg_string_type();
    break;

  case SYMB_TYPE_ARRAY:
    res = SymbTablePkg_array_type(
             SymbType_make_memory_shared(SymbType_get_array_subtype(self)),
             SymbType_get_array_lower_bound(self),
             SymbType_get_array_upper_bound(self));
    break;

  default:
    error_unreachable_code(); /* unknown type */
  }

  return res;
}

/**Function********************************************************************

   Synopsis           [True if and only if the given type is memory shared ]

   Description        []

   SideEffects        []

   SeeAlso            []
******************************************************************************/
boolean SymbType_is_memory_shared(SymbType_ptr self) {
  return self->isMemorySharingInstance;
}


/**Function********************************************************************

   Synopsis [ True if and only if the given types are equal, the given
   types can be memory-sharing or not. ]

   Description        []

   SideEffects        []

   SeeAlso            []
******************************************************************************/
boolean
SymbType_equals(SymbType_ptr self, SymbType_ptr oth) {

  if (self == oth) return true;

  if (self->tag == oth->tag) {
    if (SYMB_TYPE_ENUM == self->tag) {
      /*
         If the two types are exacly the same, i.e. the two body are
         exacly the same in term of list element. Order of elements is
         important.
      */
      return node_equal(car(self->body), car(oth->body));
    }
    else if ((SYMB_TYPE_SIGNED_WORD == self->tag) ||
             (SYMB_TYPE_UNSIGNED_WORD == self->tag)) {
      /* Sizes should be the same */
      return (SymbType_get_word_width(self) == SymbType_get_word_width(oth));
    }
    if (SYMB_TYPE_WORDARRAY == self->tag) {
      return ((SymbType_get_wordarray_awidth(self) ==
               SymbType_get_wordarray_awidth(oth)) &&
              (SymbType_get_wordarray_vwidth(self) ==
               SymbType_get_wordarray_vwidth(oth)));
    }
    else if ( (SYMB_TYPE_INTEGER == self->tag) ||
              (SYMB_TYPE_REAL == self->tag) ||
              (SYMB_TYPE_BOOLEAN == self->tag) ||
              (SYMB_TYPE_STRING == self->tag) ) {
      return true;
    }
    else if (SYMB_TYPE_ARRAY == self->tag) {
      if ((SymbType_get_array_lower_bound(self) ==
           SymbType_get_array_lower_bound(oth)) &&
          (SymbType_get_array_upper_bound(self) ==
           SymbType_get_array_upper_bound(oth))) {
        return SymbType_equals(SymbType_get_array_subtype(self),
                               SymbType_get_array_subtype(oth));
      }
    }
    else {
      internal_error("SymbType_equals: Unknown type combination.");
      error_unreachable_code();
      return true;
    }
  }

  return false;
}


/**Function********************************************************************

  Synopsis [ Equality function for node_ptr, used to compare bodies of
  types in SymbType_equal.]

  Description        []

  SideEffects        []

  SeeAlso            [SymbType_equal]

******************************************************************************/
static boolean node_equal(node_ptr n1, node_ptr n2) 
{
  
  if (n1 == n2) return true;

  if ((Nil == n1) || (Nil == n2)) {
    return false;
  }

  if (node_get_type(n1) == node_get_type(n2)) {
    switch(node_get_type(n1)) {
    case NUMBER:
      return car(n1) == car(n2);

    case ATOM:
      if ((Nil != car(n1)) && (Nil != car(n2))) {
        /* If the atom is not Nil then compare strings */
        return !strcmp(get_text(node_get_lstring(n1)),
                       get_text(node_get_lstring(n2)));
      }
      else return false;
    default:
      return (node_equal(car(n1), car(n2)) &&
              node_equal(cdr(n1), cdr(n2)));

    }
  }

  return false;
}


/**Function********************************************************************

  Synopsis           [ Utility of SymbType_sprint and SymbType_print to print
                       in an fprintf-like fashion using information in the given
                       SymbTypePrinterInfo ]

  Description        [ This function prints the specified formatted string in
                       the pinfo ]

  SideEffects        []

  SeeAlso            [SymbType_print, SymbType_sprint]

******************************************************************************/
static void str_print(SymbTypePrinterInfo_ptr pinfo,
                      const char *fmt, ...)
{
  va_list ap;

  if (pinfo->output_type == FILE_OUTPUT) {
    va_start(ap, fmt);
    vfprintf(pinfo->file_stream, fmt, ap);
    va_end(ap);
  }
  else if (pinfo->output_type == STRING_OUTPUT) {
    unsigned size;
    int n;
    char *np, *p;

    /* Guess we need no more than INITIAL_PRINT_BUFFER_SIZE bytes. */
    size = INITIAL_PRINT_BUFFER_SIZE;

    p = ALLOC(char, size);

    if(NIL(char) == p) {
      internal_error("Out of memory");
    }

    while (true) {
      /* Try to print in the allocated space. */
      va_start(ap, fmt);
      n = vsnprintf (p, size, fmt, ap);
      va_end(ap);

      /* If that worked, append it to the buffer res */
      if (n > -1 && n < size) {
        char* tmp;

        if (NIL(char) == pinfo->string_buffer) {
          pinfo->string_buffer = p;
          pinfo->buffer_size = n;
          return;
        }
        else {
          if (pinfo->remaining_space <= n) {
            /* We do not have enough memory, we have to REALLOC */
            tmp = REALLOC(char,
                          pinfo->string_buffer,
                          (pinfo->buffer_size * 2) + n + 1);

            if (NIL(char) == tmp) {
              internal_error("Out of memory");
            }

            pinfo->buffer_size += pinfo->buffer_size + n;
            pinfo->remaining_space += pinfo->buffer_size + n;
            pinfo->string_buffer = tmp;
          }

          /* Concatenate the buffer with p */
          strncat(pinfo->string_buffer, p, n);
          pinfo->remaining_space -= n;

          FREE(p);

          return;
        }
      }

      /* Else try again with more space. */
      if (n > -1) {
        /* glibc 2.1 */
        size = n+1; /* precisely what is needed */
      }
      else {
        /* glibc 2.0 */
        size *= 2;  /* twice the old size */
      }

      /* Resize p */
      np = REALLOC(char, p, size);

      if (NIL(char) == np) {
        FREE(p);
        internal_error("Out of memory");
      }
      else {
        p = np;
      }
    }
  }
  else {
    internal_error("Unknown output type");
    error_unreachable_code();
  }
}


/**Function********************************************************************

  Synopsis           [ Utility of SymbType_sprint and SymbType_print to
                       actually print the type ]

  Description        [ This is the function that prints the SymbType using the
                       file stream or the string buffer provided by pinfo ]

  SideEffects        []

  SeeAlso            [SymbType_print, SymbType_sprint]

******************************************************************************/
static void symb_type_sprint_aux(const SymbType_ptr self,
                                 SymbTypePrinterInfo_ptr pinfo)
{
  SYMB_TYPE_CHECK_INSTANCE(self);
  switch (self->tag) {

  case SYMB_TYPE_NONE:
    str_print(pinfo, "no-type");
    break;

  case SYMB_TYPE_STATEMENT:
    str_print(pinfo, "statement");
    break;

  case SYMB_TYPE_BOOLEAN:
    str_print(pinfo, "boolean");
    break;

  case SYMB_TYPE_ENUM:
    if (!self->isMemorySharingInstance) {
      node_ptr l = SymbType_get_enum_type_values(self);
      str_print(pinfo, "{");
      while (l != Nil) {
        char* tmp;

        tmp = sprint_node(car(l));
        str_print(pinfo, "%s", tmp);
        FREE(tmp);

        l = cdr(l);
        if (l != Nil) { str_print(pinfo, ", "); }
      }
      str_print(pinfo, "}");
    }
    else { /* this is memory sharing types (therefore, are "simplified") */
      switch (node_get_type(self->body)) {
      case ENUM_TYPE_PURE_INT:
        str_print(pinfo, "integer-enum");
        break;
      case ENUM_TYPE_PURE_SYMBOLIC:
        str_print(pinfo, "symbolic-enum");
        break;
      case ENUM_TYPE_INT_SYMBOLIC:
        str_print(pinfo, "integer-and-symbolic-enum");
        break;
      default: error_unreachable_code();
      }
    } /* else */
    break;

  case SYMB_TYPE_INTEGER:
    str_print(pinfo, "integer");
    break;

  case SYMB_TYPE_REAL:
    str_print(pinfo, "real");
    break;

  case SYMB_TYPE_SIGNED_WORD:
    str_print(pinfo, "signed word[");
    str_print(pinfo, "%d", SymbType_get_word_width(self));
    str_print(pinfo, "]");
    break;

  case SYMB_TYPE_UNSIGNED_WORD:
    str_print(pinfo, "unsigned word[");
    str_print(pinfo, "%d", SymbType_get_word_width(self));
    str_print(pinfo, "]");
    break;

  case SYMB_TYPE_WORDARRAY:
    str_print(pinfo, "array ");

    str_print(pinfo, "word[");
    str_print(pinfo, "%d", SymbType_get_wordarray_awidth(self));
    str_print(pinfo, "]");

    str_print(pinfo, " of ");

    str_print(pinfo, "word[");
    str_print(pinfo, "%d", SymbType_get_wordarray_vwidth(self));
    str_print(pinfo, "]");
    break;

  case SYMB_TYPE_STRING:
    str_print(pinfo, "string");
    break;

  case SYMB_TYPE_SET_BOOL:  str_print(pinfo, "boolean-set"); break;
  case SYMB_TYPE_SET_INT:  str_print(pinfo, "integer-set"); break;
  case SYMB_TYPE_SET_SYMB: str_print(pinfo, "symbolic-set"); break;
  case SYMB_TYPE_SET_INT_SYMB: str_print(pinfo, "integer-symbolic-set");
    break;

  case SYMB_TYPE_ARRAY:
    {
      str_print(pinfo, "array %d..%d of ",
              SymbType_get_array_lower_bound(self),
              SymbType_get_array_upper_bound(self));
      symb_type_sprint_aux(SymbType_get_array_subtype(self), pinfo);
      break;
    }

  case SYMB_TYPE_ERROR:
    str_print(pinfo, "error-type");
    break;

  default: error_unreachable_code();
  }

  return;
}
