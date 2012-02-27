/**CFile***********************************************************************

  FileName    [bePkg.c]

  PackageName [be]

  Synopsis    [Contains initialization and deinitialization code for this
  module]

  Description [Contains code to be called when entering and exiting the module]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``be'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by FBK-irst and University of Trento. 

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

#include "be.h"
#include "beInt.h"
#include "rbc/rbc.h"

/*---------------------------------------------------------------------------*/
/* Variable definitions                                                      */
/*---------------------------------------------------------------------------*/
st_table* htShift_ptr = NULL; /* shift hash table */

static void be_shiftHashInit ARGS((void)); 
static void be_shiftHash_Quit ARGS((void)); 

static int 
be_shiftHash_key_cmp ARGS((const char* _key1, const char* _key2));

static int be_shiftHash_key_hash ARGS((char* _key, const int size));

static enum st_retval be_shiftHash_CallbackDeleteEntryAndKey
ARGS((char* key, char* record, char* dummy));

/**Function********************************************************************

  Synopsis [Initializes the module]
  Description [Call before any other function contained in this module]

  SideEffects [Any module structure is allocated and initialized if required]

  SeeAlso     [Be_Quit]

******************************************************************************/
void Be_Init()
{
  Rbc_pkg_init();
  be_shiftHashInit();
}

/**Function********************************************************************

  Synopsis [De-initializes the module]
  Description [Call as soon as you finished to use this module services]

  SideEffects [Any module structure is deleted if required]

  SeeAlso     [Be_Init]

******************************************************************************/
void Be_Quit()
{
  be_shiftHash_Quit();
  Rbc_pkg_quit();
}

/**Function********************************************************************

  Synopsis           [Initializes private hast table member for shifting
  operations]

  Description        [Call be_shiftHash_Quit() before quit from the be  
  module]

  SideEffects        [Private global vars htShift_ptr will change]

  SeeAlso            [Be_LogicalShiftVar, Hash_Quit]

******************************************************************************/
static void be_shiftHashInit() 
{
  /* Initializes the shifting hash tables */
  nusmv_assert(htShift_ptr == NULL); /* not already initialized */

  htShift_ptr = st_init_table( &be_shiftHash_key_cmp, 
			       &be_shiftHash_key_hash );

  nusmv_assert(htShift_ptr != NULL); /* was no able to init */
}


/**Function********************************************************************

  Synopsis           [Deletes private hast table member for shifting
  operations]

  Description        [Call be_shiftHash_Quit() before quit from BMC module]

  SideEffects        [Private global vars htShift_ptr will be put to NULL]

  SeeAlso            [be_shiftHashInit]

******************************************************************************/
static void be_shiftHash_Quit() 
{
  /* Destroys the shifting hash tables */
  if (htShift_ptr != NULL) {
    st_foreach(htShift_ptr, be_shiftHash_CallbackDeleteEntryAndKey, 
	       NULL /*dummy*/);

    st_free_table(htShift_ptr);
    htShift_ptr = NULL;
  }
}


/* ------------------------------------------------------------------------  */
/* Shift memoizing internal functions:                                       */
 
static int be_shiftHash_key_cmp(const char* _key1, const char* _key2) 
{
  const shift_memoize_key* key1 = (shift_memoize_key*) _key1;
  const shift_memoize_key* key2 = (shift_memoize_key*) _key2;
  
  return (key1->be != key2->be) || (key1->shift != key2->shift);
}

static int be_shiftHash_key_hash(char* _key, const int size) 
{
  shift_memoize_key* key = (shift_memoize_key*) _key;
  return (int) ((((nusmv_ptruint) key->be >> 2) ^ (key->shift) ) % size);
}  


static enum st_retval 
be_shiftHash_CallbackDeleteEntryAndKey(char* key, char* record, char* dummy)
{
  FREE(key); /* removes allocated key for this entry */
  return ST_DELETE; /* removes associated element */
}
    
