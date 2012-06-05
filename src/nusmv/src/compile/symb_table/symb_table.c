/**CFile*****************************************************************

  FileName    [symb_table.c]

  PackageName [compile.symb_table]

  Synopsis    [Implementation of the compile.symb_table package]

  Description [This package contains just a few public functions.
  Most functionality lays in the classes this package contains.
  So these classes for more info.]

  SeeAlso     []

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``compile.symb_table'' package of NuSMV version 2. 
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

#include "symb_table.h"
#include "SymbTable.h"
#include "SymbType_private.h"

#include "symb_table_int.h"
#include "utils/assoc.h" 
#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: symb_table.c,v 1.1.2.10.6.8 2007-11-08 19:06:28 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Constants declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

  Synopsis    [Contains the global symbol table instance]

  Description [This instance is (de)inititialized by package (de)init
  functions]

******************************************************************************/
static SymbTable_ptr global_symb_table = SYMB_TABLE(NULL);

SymbTable_ptr Compile_get_global_symb_table()
{
  SYMB_TABLE_CHECK_INSTANCE(global_symb_table);
  return global_symb_table;
}

static void symb_table_init_global_symb_table() 
{
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
    fprintf(nusmv_stderr, "Instantiating the global SymbTable instance...\n");
  } 

  nusmv_assert(global_symb_table == SYMB_TABLE(NULL));
  global_symb_table = SymbTable_create();
}

static void symb_table_deinit_global_symb_table()
{
  if (global_symb_table != SYMB_TABLE(NULL)) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
      fprintf(nusmv_stderr, "Clearing the global SymbTable instance...\n");
    } 
    SymbTable_destroy(global_symb_table);
    global_symb_table = SYMB_TABLE(NULL);
  }
}


/* all the possible "simplified" symbolic types SymbType. "Simplified" means
   that an enum type's body contains a list of some arbitrary
   (not real) values.
   The important feature is that the memory is shared,
   so you can compare pointers to compare types.
   De-initialisation of the package destroys these types.
   The only access to these types is through functions
   "SymbTablePkg_..._type" (such as SymbTablePkg_integer_type).
*/
static SymbType_ptr symb_table_no_type = SYMB_TYPE(NULL);
static SymbType_ptr symb_table_statement_type = SYMB_TYPE(NULL);
static SymbType_ptr symb_table_boolean_type = SYMB_TYPE(NULL);
static SymbType_ptr symb_table_pure_symbolic_enum_type = SYMB_TYPE(NULL);
static SymbType_ptr symb_table_int_symbolic_enum_type = SYMB_TYPE(NULL);
static SymbType_ptr symb_table_pure_int_enum_type = SYMB_TYPE(NULL);
static SymbType_ptr symb_table_integer_type = SYMB_TYPE(NULL);
static SymbType_ptr symb_table_real_type = SYMB_TYPE(NULL);
/* Word width -> Word type (for signed: Word -width -> Word type) */
static hash_ptr symb_table_width_to_word_type_hash = (hash_ptr)NULL;
/* cons(AddressWidth, ValueWidth) -> WordArray type */
static hash_ptr symb_table_widths_to_wordarray_type_hash = (hash_ptr)NULL;
/* cons(subtype, cons(lower_bound, upper_bound)) -> Array type */
static hash_ptr symb_table_subtype_bound_to_array_type_hash = (hash_ptr)NULL;
static SymbType_ptr symb_table_string_type = SYMB_TYPE(NULL);
static SymbType_ptr symb_table_boolean_set_type = SYMB_TYPE(NULL);
static SymbType_ptr symb_table_integer_set_type = SYMB_TYPE(NULL);
static SymbType_ptr symb_table_symbolic_set_type = SYMB_TYPE(NULL);
static SymbType_ptr symb_table_integer_symbolic_set_type = SYMB_TYPE(NULL);
static SymbType_ptr symb_table_error_type = SYMB_TYPE(NULL);

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static enum st_retval
symb_table_types_hash_cleaner ARGS((char * c1, char * c2, char *c3));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initialises the class package.]

  Description        [This initialisation can be performed only after
  the Node package and the variable boolean_range have been initialised.]

  SideEffects        []

******************************************************************************/
void SymbTablePkg_init()
{
  node_ptr atom;

  /* initializes the global symbol table */
  symb_table_init_global_symb_table();

  /* initialise the symbolic types with shared memory */
  nusmv_assert(SYMB_TYPE(NULL) == symb_table_no_type &&
               SYMB_TYPE(NULL) == symb_table_statement_type &&
               SYMB_TYPE(NULL) == symb_table_boolean_type &&
               SYMB_TYPE(NULL) == symb_table_pure_symbolic_enum_type &&
               SYMB_TYPE(NULL) == symb_table_int_symbolic_enum_type &&
               SYMB_TYPE(NULL) == symb_table_pure_int_enum_type &&
               SYMB_TYPE(NULL) == symb_table_integer_type &&
               SYMB_TYPE(NULL) == symb_table_real_type &&
               (hash_ptr)(NULL) == symb_table_width_to_word_type_hash &&
               (hash_ptr)(NULL) == symb_table_widths_to_wordarray_type_hash &&
               (hash_ptr)(NULL) == symb_table_subtype_bound_to_array_type_hash &&
               SYMB_TYPE(NULL) == symb_table_string_type &&
               SYMB_TYPE(NULL) == symb_table_boolean_set_type &&
               SYMB_TYPE(NULL) == symb_table_integer_set_type &&
               SYMB_TYPE(NULL) == symb_table_symbolic_set_type &&
               SYMB_TYPE(NULL) == symb_table_integer_symbolic_set_type &&
               SYMB_TYPE(NULL) == symb_table_error_type);

  atom = new_node(ATOM,Nil,Nil);

  symb_table_no_type
    = SymbType_create_memory_sharing_type(SYMB_TYPE_NONE, Nil);
  symb_table_statement_type  
    = SymbType_create_memory_sharing_type(SYMB_TYPE_STATEMENT, Nil);
  /* the Enum type will contain the list of some some artificial values */
  symb_table_boolean_type
    = SymbType_create_memory_sharing_type(SYMB_TYPE_BOOLEAN, Nil);
  symb_table_pure_symbolic_enum_type 
    = SymbType_create_memory_sharing_type(SYMB_TYPE_ENUM, cons(atom, Nil));
  symb_table_int_symbolic_enum_type 
    = SymbType_create_memory_sharing_type(SYMB_TYPE_ENUM,
                                          cons(atom, cons(zero_number,Nil)));
  symb_table_pure_int_enum_type 
    = SymbType_create_memory_sharing_type(SYMB_TYPE_ENUM,
                                          cons(zero_number, Nil));
  symb_table_integer_type
    = SymbType_create_memory_sharing_type(SYMB_TYPE_INTEGER, Nil);
  symb_table_real_type
    = SymbType_create_memory_sharing_type(SYMB_TYPE_REAL, Nil);
  symb_table_width_to_word_type_hash = new_assoc();
  symb_table_widths_to_wordarray_type_hash = new_assoc();
  symb_table_subtype_bound_to_array_type_hash = new_assoc();
  symb_table_string_type
    = SymbType_create_memory_sharing_type(SYMB_TYPE_STRING, Nil);
  symb_table_boolean_set_type
    = SymbType_create_memory_sharing_type(SYMB_TYPE_SET_BOOL, Nil);
  symb_table_integer_set_type
    = SymbType_create_memory_sharing_type(SYMB_TYPE_SET_INT, Nil);
  symb_table_symbolic_set_type
    = SymbType_create_memory_sharing_type(SYMB_TYPE_SET_SYMB, Nil);
  symb_table_integer_symbolic_set_type
    = SymbType_create_memory_sharing_type(SYMB_TYPE_SET_INT_SYMB, Nil);
  symb_table_error_type
    = SymbType_create_memory_sharing_type(SYMB_TYPE_ERROR, Nil);
}


/**Function********************************************************************

  Synopsis           [Shut down the package.]

  Description        [WARNING: the package de-initialisation
  destroys types created with the function SymbTablePkg_..._type.

  NB: The reason behind this constrain is the following: these functions
  exploit memory sharing, and this memory is freed during
  de-initialisation.

  In any case, the de-initialisation is performed by
  system "reset" command, and this command also frees all node_ptr,
  so in any case the symbolic types will be unusable, because they 
  use node_ptr inside. 
  SO, DESTROY ALL SYMBOLIC TYPES CREATED SO FAR BEFORE THE symb_table
  PACKAGE DE-INITIALISATION!  
  ]
  
  SideEffects        []

******************************************************************************/
void SymbTablePkg_quit()
{
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stderr, "Clearing the symbol table package...\n");
  } 

  /* deinits the global symbol table: */
  symb_table_deinit_global_symb_table();

  /* has the package ever been initialised? */
  if (SYMB_TYPE(NULL) != symb_table_boolean_type) {
  
    /* finally clean the types with shared memory */
    nusmv_assert(SYMB_TYPE(NULL) != symb_table_no_type &&
                 SYMB_TYPE(NULL) != symb_table_statement_type &&
                 SYMB_TYPE(NULL) != symb_table_boolean_type &&
                 SYMB_TYPE(NULL) != symb_table_pure_symbolic_enum_type &&
                 SYMB_TYPE(NULL) != symb_table_int_symbolic_enum_type &&
                 SYMB_TYPE(NULL) != symb_table_pure_int_enum_type &&
                 SYMB_TYPE(NULL) != symb_table_integer_type &&
                 SYMB_TYPE(NULL) != symb_table_real_type &&
                (hash_ptr)(NULL) != symb_table_width_to_word_type_hash &&
                (hash_ptr)(NULL) != symb_table_widths_to_wordarray_type_hash &&
                (hash_ptr)(NULL) != symb_table_subtype_bound_to_array_type_hash &&
                 SYMB_TYPE(NULL) != symb_table_string_type &&
                 SYMB_TYPE(NULL) != symb_table_boolean_set_type &&
                 SYMB_TYPE(NULL) != symb_table_integer_set_type &&
                 SYMB_TYPE(NULL) != symb_table_symbolic_set_type &&
                 SYMB_TYPE(NULL) != symb_table_integer_symbolic_set_type &&
                 SYMB_TYPE(NULL) != symb_table_error_type);
  
    SymbType_destroy_memory_sharing_type(symb_table_no_type);
    SymbType_destroy_memory_sharing_type(symb_table_statement_type);
    SymbType_destroy_memory_sharing_type(symb_table_boolean_type);
    SymbType_destroy_memory_sharing_type(symb_table_pure_symbolic_enum_type);
    SymbType_destroy_memory_sharing_type(symb_table_int_symbolic_enum_type);
    SymbType_destroy_memory_sharing_type(symb_table_pure_int_enum_type);
    SymbType_destroy_memory_sharing_type(symb_table_integer_type);
    SymbType_destroy_memory_sharing_type(symb_table_real_type);
    clear_assoc_and_free_entries(symb_table_width_to_word_type_hash,
                                 symb_table_types_hash_cleaner);
    free_assoc(symb_table_width_to_word_type_hash);
    clear_assoc_and_free_entries(symb_table_widths_to_wordarray_type_hash,
                                 symb_table_types_hash_cleaner);
    free_assoc(symb_table_widths_to_wordarray_type_hash);
    clear_assoc_and_free_entries(symb_table_subtype_bound_to_array_type_hash,
                                 symb_table_types_hash_cleaner);
    free_assoc(symb_table_subtype_bound_to_array_type_hash);
    SymbType_destroy_memory_sharing_type(symb_table_string_type);
    SymbType_destroy_memory_sharing_type(symb_table_boolean_set_type);
    SymbType_destroy_memory_sharing_type(symb_table_integer_set_type);
    SymbType_destroy_memory_sharing_type(symb_table_symbolic_set_type);
    SymbType_destroy_memory_sharing_type(symb_table_integer_symbolic_set_type);
    SymbType_destroy_memory_sharing_type(symb_table_error_type);
    
    symb_table_no_type = SYMB_TYPE(NULL);
    symb_table_statement_type = SYMB_TYPE(NULL);
    symb_table_boolean_type = SYMB_TYPE(NULL);
    symb_table_pure_symbolic_enum_type = SYMB_TYPE(NULL);
    symb_table_int_symbolic_enum_type = SYMB_TYPE(NULL);
    symb_table_pure_int_enum_type = SYMB_TYPE(NULL);
    symb_table_integer_type = SYMB_TYPE(NULL);
    symb_table_real_type = SYMB_TYPE(NULL);
    symb_table_width_to_word_type_hash = (hash_ptr)NULL;
    symb_table_widths_to_wordarray_type_hash = (hash_ptr)NULL;
    symb_table_subtype_bound_to_array_type_hash = (hash_ptr)NULL;
    symb_table_string_type = SYMB_TYPE(NULL);
    symb_table_boolean_set_type = SYMB_TYPE(NULL);
    symb_table_integer_set_type = SYMB_TYPE(NULL);
    symb_table_symbolic_set_type = SYMB_TYPE(NULL);
    symb_table_integer_symbolic_set_type = SYMB_TYPE(NULL);
    symb_table_error_type = SYMB_TYPE(NULL);
  }

}


/**Function********************************************************************

  Synopsis           [returns a no-type]

  Description        [This type is a type of correct expressions
  which normally do not have any time. 
  The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_no_type()
{
  return symb_table_no_type;
}


/**Function********************************************************************

  Synopsis           [returns a no-type]

  Description [This type is a type of correct expressions which are
  statements, like assignments, or high-level nodes like TRANS, INIT,
  etc. The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_statement_type()
{
  return symb_table_statement_type;
}



/**Function********************************************************************

  Synopsis           [returns a boolean enum type]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_boolean_type()
{
  return symb_table_boolean_type;
}


/**Function********************************************************************

  Synopsis           [returns a pure symbolic enum type.]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.
  Do not access the values contained in the type's body.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_pure_symbolic_enum_type()
{
  return symb_table_pure_symbolic_enum_type;
}


/**Function********************************************************************

  Synopsis           [returns a enum type containing integers AND symbolic 
  constants]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.
  Do not access the values contained in the type's body.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_int_symbolic_enum_type()
{
  return symb_table_int_symbolic_enum_type;
}


/**Function********************************************************************

  Synopsis           [returns a pure integer enum type]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.
  Do not access the values contained in the type's body.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_pure_int_enum_type()
{
  return symb_table_pure_int_enum_type;
}


/**Function********************************************************************

  Synopsis           [returns an Integer type.]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_integer_type()
{
  return symb_table_integer_type;
}


/**Function********************************************************************

  Synopsis           [returns a Real type.]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_real_type()
{
  return symb_table_real_type;
}


/**Function********************************************************************

  Synopsis           [returns an unsigned Word type (with a given width)]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_unsigned_word_type(int width)
{
  SymbType_ptr type;
  
  nusmv_assert(width > 0); /* debugging check */
  
  type = SYMB_TYPE(find_assoc(symb_table_width_to_word_type_hash,
                              NODE_FROM_INT(width)));
  if (SYMB_TYPE(NULL) == type) {
    type = SymbType_create_memory_sharing_type(SYMB_TYPE_UNSIGNED_WORD, 
                             find_node(NUMBER, NODE_FROM_INT(width), Nil));
    insert_assoc(symb_table_width_to_word_type_hash,
                 NODE_FROM_INT(width), (node_ptr) type);
  }

  return type;
}


/**Function********************************************************************

  Synopsis           [returns a signed Word type (with a given width)]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_signed_word_type(int width)
{
  SymbType_ptr type;
  
  nusmv_assert(width > 0); /* debugging check */

  type = SYMB_TYPE(find_assoc(symb_table_width_to_word_type_hash,
                              NODE_FROM_INT(-width)));
  if (SYMB_TYPE(NULL) == type) {
    type = SymbType_create_memory_sharing_type(SYMB_TYPE_SIGNED_WORD, 
                             find_node(NUMBER, NODE_FROM_INT(width), Nil));
    insert_assoc(symb_table_width_to_word_type_hash,
                 NODE_FROM_INT(-width), (node_ptr) type);
  }

  return type;
}


/**Function********************************************************************

  Synopsis           [Returns a WordArray type (given array width and value width)]

  Description        [The memory is shared, so you can compare pointers to
  compare types. The association is done based on the cons of awidth and vwidth.
  De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_wordarray_type(int awidth, int vwidth)
{
  node_ptr a; 
  node_ptr v; 
  node_ptr av;
  SymbType_ptr type;
 
  a = find_node(NUMBER, NODE_FROM_INT(awidth), Nil);
  v = find_node(NUMBER, NODE_FROM_INT(vwidth), Nil);
  av = find_node(CONS, a, v);

  type = SYMB_TYPE(find_assoc(symb_table_widths_to_wordarray_type_hash, av));
  if (SYMB_TYPE(NULL) == type) {
    type = SymbType_create_memory_sharing_type(SYMB_TYPE_WORDARRAY, av);
    insert_assoc(symb_table_widths_to_wordarray_type_hash, av, (node_ptr)type);
  }

  return type;
}


/**Function********************************************************************

  Synopsis           [returns an array type.]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.
  PRECONDITION: subtype has to be created with one of SymbTypePkg_.._type 
  function.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_array_type(SymbType_ptr subtype,
                                     int lower_bound,
                                     int upper_bound)
{
  node_ptr key = find_node(CONS, NODE_PTR(subtype), 
                           find_node(CONS,
                                     NODE_FROM_INT(lower_bound), 
                                     NODE_FROM_INT(upper_bound)));
  
  SymbType_ptr type =
    SYMB_TYPE(find_assoc(symb_table_subtype_bound_to_array_type_hash, key));

  if (SYMB_TYPE(NULL) == type) {
    type = SymbType_create_memory_sharing_array_type(subtype,
                                                     lower_bound,
                                                     upper_bound);
    insert_assoc(symb_table_subtype_bound_to_array_type_hash, key,
                 NODE_PTR(type));
  }

  return type;
}


/**Function********************************************************************

  Synopsis           [returns a String type.]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_string_type()
{
  return symb_table_string_type;
}


/**Function********************************************************************

  Synopsis           [returns a boolean-set type.]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_boolean_set_type()
{
  return symb_table_boolean_set_type;
}


/**Function********************************************************************

  Synopsis           [returns a integer-set type.]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_integer_set_type()
{
  return symb_table_integer_set_type;
}


/**Function********************************************************************

  Synopsis           [returns a symbolic-set type.]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_symbolic_set_type()
{
  return symb_table_symbolic_set_type;
}


/**Function********************************************************************

  Synopsis           [returns a integer-symbolic-set type.]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_integer_symbolic_set_type()
{
  return symb_table_integer_symbolic_set_type;
}


/**Function********************************************************************

  Synopsis           [returns an Error-type.]

  Description        [The memory is shared, so you can compare pointers to
  compare types. De-initialisation of the package destroys this type.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr SymbTablePkg_error_type()
{
  return symb_table_error_type;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function*******************************************************************

  Synopsis           [The function is used to free the memory from
  memory-sharing Word/WordArray/Array types stored in the hash table
  symb_table_..._type_hash. Used in the SymbTablePkg_quit only]

******************************************************************************/
static enum st_retval
symb_table_types_hash_cleaner(char * c1, char * c2, char *c3)
{
  SymbType_ptr type;

  type = SYMB_TYPE(c2);
  /* only Word type can be in the hash table */
  nusmv_assert(SymbType_is_word(type) || 
               SYMB_TYPE_WORDARRAY == SymbType_get_tag(type) ||
               SYMB_TYPE_ARRAY == SymbType_get_tag(type));
  SymbType_destroy_memory_sharing_type(type);
  return ST_DELETE;
}


