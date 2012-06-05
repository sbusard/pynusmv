/**CFile***********************************************************************

  FileName    [rbcInline.c]

  PackageName [rbc]

  Synopsis    [Implementaion of RBC inlining]

  Description []

  SeeAlso     [rbc.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2. 
  Copyright (C) 2007 by FBK-irst. 

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

  Revision    [$Id: rbcInline.c,v 1.1.2.5 2007-04-05 21:55:46 nusmv Exp $]

******************************************************************************/

#include "rbc/rbcInt.h"
#include "InlineResult.h"

#include "dag/dag.h"
#include "node/node.h"
#include "parser/symbols.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
hash_ptr inlining_cache = (hash_ptr) NULL;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Calculates the inlining of the given formula]

  Description [Returned InlineResult instance is cached and must be _NOT_ 
	destroyed by the caller]

  SideEffects [None]

  SeeAlso     [InlineResult]

******************************************************************************/
InlineResult_ptr RbcInline_apply_inlining(Rbc_Manager_t* rbcm, Rbc_t* f)
{
  InlineResult_ptr ir;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Rbc: starting inlining ... \n");
  }

  ir = InlineResult_create(rbcm, f);

  
  /*        MR: completely disregarded the caching since it is too expensive
            MR: if a large number of calls to inliner are performes
                (e.g. incremental SBMC) */
  // rbc_inlining_cache_add_result(f, ir); /* caches result */

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "RBC: end of inlining\n");
  }

  return ir;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Inline caching private service]

  Description        []

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
void rbc_inlining_cache_init()
{
  nusmv_assert(inlining_cache == (hash_ptr) NULL);
  inlining_cache = new_assoc();
}

static assoc_retval _destroy_cache_entry(char* key, char* _elem, char* arg)
{ 
  if (_elem != (char*) NULL) InlineResult_destroy(INLINE_RESULT(_elem)); 
  return ST_DELETE;
}

/**Function********************************************************************

  Synopsis           [Inline caching private service]

  Description        []

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
void rbc_inlining_cache_quit()
{
  if (inlining_cache != (hash_ptr) NULL) {
    clear_assoc_and_free_entries(inlining_cache, _destroy_cache_entry);
    free_assoc(inlining_cache);
    inlining_cache = (hash_ptr) NULL;
  }  
}

/**Function********************************************************************

  Synopsis           [Inline caching private service]

  Description        []

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
void rbc_inlining_cache_add_result(Rbc_t* f, InlineResult_ptr res)
{  
  InlineResult_ptr old = rbc_inlining_cache_lookup_result(f);
  if (res == old) return;
  if (old != INLINE_RESULT(NULL)) InlineResult_destroy(old);
  insert_assoc(inlining_cache, (node_ptr) f, (node_ptr) res);
}

/**Function********************************************************************

  Synopsis           [Inline caching private service]

  Description        []

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
InlineResult_ptr rbc_inlining_cache_lookup_result(Rbc_t* f)
{ return INLINE_RESULT(find_assoc(inlining_cache, (node_ptr) f)); }
