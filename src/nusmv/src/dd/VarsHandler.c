/**CFile***********************************************************************

  FileName    [VarsHandler.c]

  PackageName [dd]

  Synopsis    [Implementation of class 'VarsHandler']

  Description [VarsHandler handles the allocation of new variables,
  and their organization within 'groups' (blocks in dd
  terminology). This is done to allow multiple BddEnc instances to
  share the same dd space. For all details see the structure
  description below.]

  SeeAlso     [VarsHandler.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``dd'' package of NuSMV version 2. 
  Copyright (C) 2010 by FBK-irst. 

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

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif


#include "VarsHandler.h" 

#include "set/set.h"
#include "dd/dd.h"
#include "dd/ddInt.h"
#include "utils/Olist.h"
#include "utils/error.h" 
#include "utils/utils.h" 
#include "utils/defs.h" 


static char rcsid[] UTIL_UNUSED = "$Id: $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
enum GroupSearchRes {
  GS_PERF_MATCH_FOUND, /* found group matches perfectly */
  GS_ROOT_FOUND, /* no perfect match, only root found */

  GS_NOT_FOUND, /* the group was not found */
  GS_OVERLAP_FOUND, /* overlap found */
  GS_WRONG_CHUNK, /* not compatible chunk size */
};


/**Macro***********************************************************************

   Synopsis    [Minimal block size]

   Description [Minimal block size depends on the version of
   CUDD. Cudd-2.4 requires that groups are created also for single
   variables, whereas 2.3 does not allow groups for single
   variables. ]

******************************************************************************/
#if NUSMV_HAVE_CUDD_24
# define BDD_ENC_MIN_BLOCK_SIZE 1
#else
# define BDD_ENC_MIN_BLOCK_SIZE 2
#endif


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [VarsHandler class definition]

  Description [ VarsHandler is responsible for allocating dd
  variables and dd blocks, and one shared instance can be used by
  multiple BddEnc encoder instances which need to share the same dd
  space (DdManager). VarsHandler handles the reservation and the
  release of groups of variables. Since groups can be shared among
  several encoders, a mechanism like reference counting is
  implemented to handle the release.

  VarsHandler contains a forest of tree of groups. In
  the forest, groups are separated in logical and physical
  groups. Physical groups correspond to actual dd blocks, and
  logical groups are sub-groups which share the variables locked by
  the physical groups they belongs to. 
  
  Each tree in the forest correspond to one dd block (the root of
  the tree), and possibly to a forest of logical groups (which
  correspond to the set of children of the root, with their
  sub-trees).

  Each node in the trees is a VarsGroup, which contain the low
  level, the size and the chunk size. Low level is referenced at
  group construction time, as levels can later change due to dd
  reordering. size is the number of dd variables the group has to
  block. The chunk size identifies the type of the group. Groups
  with different chunk size are not compatible, and will never
  share their variables. Currently there are two group types: for
  state variables (chunk is 2), and for input/frozen variables
  (chunk is 1).

  Each VarsGroup contains also a list of GroupInfo structures. An
  instance of GroupInfo is returned when a group is reserved. The
  same GroupInfo structure has later to be used to release the
  group.

  A final note about visibility: the only publicly visible
  structure is the VarsHandler. VarsGroup is used only internally,
  and GroupInfo are returned as opaque structures, to be used only
  as handlers.]

  SeeAlso     []   
  
******************************************************************************/
typedef struct VarsHandler_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  DdManager* dd;
  Olist_ptr forest; /* the set of VarsGroup roots */

  size_t id_counter; /* group id number counter */
} VarsHandler;


typedef struct VarsGroup_TAG {
  int lev_low;  /* low level */
  int lev_high; /* high level */
  int idx_low;  /* index of the lowest level (for traceability) */
  int chunk;    /* minimal group size */ 
  dd_block* block; /* dd block */

  Olist_ptr gis; /* the set of users of this group (GroupInfo structures) */
  
  /* tree support structures */
  struct VarsGroup_TAG* parent;
  Olist_ptr children;
} VarsGroup;


typedef struct GroupInfo_TAG {
  /* respect this order! */
  size_t id;    /* id number */
  int lev_low;  /* original group low level (used when searching) */
  int lev_high; /* original group high level (used when searching) */
} GroupInfo;


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/* VarsHandler related */
static void vars_handler_init ARGS((VarsHandler_ptr self, DdManager* dd));
static void vars_handler_deinit ARGS((VarsHandler_ptr self));

static void vars_handler_add_group ARGS((VarsHandler_ptr self, 
                                         VarsGroup* parent, VarsGroup* group));
                                 
static int 
vars_handler_remove_group ARGS((VarsHandler_ptr self, 
                                Olist_ptr list, const GroupInfo_ptr ginfo));

static int vars_handler_update_levels ARGS((VarsHandler_ptr self, 
                                            VarsGroup* root));

static Oiter vars_handler_promote_group ARGS((VarsHandler_ptr self, 
                                              Olist_ptr list, 
                                              const GroupInfo_ptr ginfo));

static int vars_handler_get_first_free_level ARGS((VarsHandler_ptr self,
                                                   int from_lev, int size));


/* VarsGroup related */
static VarsGroup* 
VarsGroup_create ARGS((int lev_low, int lev_high, int idx_low, 
                       int chunk));

static void VarsGroup_destroy ARGS((VarsGroup* self, DdManager* dd));

static void VarsGroup_add_ginfo ARGS((VarsGroup* group, GroupInfo_ptr gi));
static boolean 
VarsGroup_remove_ginfo ARGS((VarsGroup* self, GroupInfo_ptr gi));

static Oiter 
VarsGroup_find_ginfo ARGS((const VarsGroup* self, GroupInfo_ptr gi));

static VarsGroup* 
vars_handler_search_group ARGS((const VarsHandler_ptr self,
                                int lev_low, size_t size, int chunk, 
                                enum GroupSearchRes* res));

static VarsGroup* 
vars_group_search_group_aux ARGS((const VarsGroup* in_group,
                                  int lev_low, size_t size, 
                                  int chunk,
                                  enum GroupSearchRes* res));

static int vars_group_sort ARGS((void* g1, void* g2));

static void vars_group_print(VarsGroup* group, FILE* _file, int indent);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The VarsHandler class constructor]

  Description        [The VarsHandler class constructor]

  SideEffects        []

  SeeAlso            [VarsHandler_destroy]   
  
******************************************************************************/
VarsHandler_ptr VarsHandler_create(DdManager* dd)
{
  VarsHandler_ptr self = ALLOC(VarsHandler, 1);
  VARS_HANDLER_CHECK_INSTANCE(self);

  vars_handler_init(self, dd);
  return self;
}


/**Function********************************************************************

  Synopsis           [The VarsHandler class destructor]

  Description        [The VarsHandler class destructor]

  SideEffects        []

  SeeAlso            [VarsHandler_create]   
  
******************************************************************************/
void VarsHandler_destroy(VarsHandler_ptr self)
{
  VARS_HANDLER_CHECK_INSTANCE(self);

  vars_handler_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Returns the contained dd manager]

  Description        []

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
DdManager* VarsHandler_get_dd_manager(const VarsHandler_ptr self)
{
  VARS_HANDLER_CHECK_INSTANCE(self);
  return self->dd;
}


/**Function********************************************************************

  Synopsis [Returns true if currently it is possible to
  create/reuse the given group of levels]

  Description [This method can be used to check if a group can be
  created at given level.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean VarsHandler_can_group(const VarsHandler_ptr self,
                              int level, int size, int chunk)
{
  VarsGroup* group;
  enum GroupSearchRes res;

  group = vars_handler_search_group(self, level, size, chunk, &res);
  return (GS_OVERLAP_FOUND != res && GS_WRONG_CHUNK != res);
}


/**Function********************************************************************

  Synopsis [Constructs a group, with minimal level from_level, size
  and chunk size. Returns a structure which has to be used to
  release the group later.]

  Description [The reservation does not necessarily create a group
  at given level, but may allocate it at a greater level (this is
  why the parameter is called from_level). Returns the group ID,
  and the actual minimal level allocated. When done with it, the
  caller has to release the returned groupinfo with
  VarsHandler_release_group or VarsHandler_dissolve_group]

  SideEffects        []

  SeeAlso [VarsHandler_release_group,
  VarsHandler_dissolve_group]
  
******************************************************************************/
GroupInfo_ptr VarsHandler_reserve_group(VarsHandler_ptr self,
                                        int from_lev, int size, int chunk, 
                                        boolean can_share, int* lev_low)
{
  VarsGroup* group = (VarsGroup*) NULL;
  enum GroupSearchRes res = GS_NOT_FOUND;

  VARS_HANDLER_CHECK_INSTANCE(self);

  if (can_share && from_lev >= 0) {
    group = vars_handler_search_group(self, from_lev, size, chunk, &res);
  }

  if (GS_PERF_MATCH_FOUND != res) { /* group has to be created here */
    VarsGroup* child;

    if ((VarsGroup*) NULL == group) {
      /* at root level: creates variables and groups */
      dd_reorderingtype reord_type;
      int reord_status = dd_reordering_status(self->dd, &reord_type);
      int new_lev, new_idx;

      dd_autodyn_disable(self->dd);
      new_lev = vars_handler_get_first_free_level(self, from_lev, size);
      new_idx = dd_get_index_at_level(self->dd, new_lev);

      child = VarsGroup_create(new_lev, new_lev+size-1, new_idx, chunk);
      if (size >= BDD_ENC_MIN_BLOCK_SIZE) {
        if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
          fprintf(nusmv_stderr, "VarsHandler: creating physical var block "\
                  "at index %d, size %d\n", new_idx, size);
        }
        CATCH {
          child->block = dd_new_var_block(self->dd, new_idx, size);
        }
        FAIL {
          if (1 == reord_status) {
            dd_autodyn_enable(self->dd, reord_type);
          }
          internal_error("vars_handler: Failure during variable group construction\n");
        }
      }
      if (1 == reord_status) {
        dd_autodyn_enable(self->dd, reord_type);
      }
    }
    else {
      /* a containing group already exists, creates a logical 
         sub-group of the found root */
      child = VarsGroup_create(from_lev, from_lev+size-1, 
                               dd_get_index_at_level(self->dd, from_lev), 
                               chunk);
    }

    vars_handler_add_group(self, group, child);
    group = child;
  }

  { /* result group info constrution */
    GroupInfo_ptr ginfo = ALLOC(GroupInfo, 1);
    nusmv_assert((GroupInfo_ptr) NULL != ginfo);
    ginfo->id = self->id_counter++;
    ginfo->lev_low = group->lev_low;
    ginfo->lev_high = group->lev_low+size-1;

    VarsGroup_add_ginfo(group, ginfo);

    *lev_low = group->lev_low;
    return ginfo;
  }
}


/**Function********************************************************************

  Synopsis [Releases the group (previously created with
  reserve_group)]

  Description [The group is not necessarily released, at it (or
  part of it) may be shared with other created groups. After this
  method has been called, gid cannot be used anymore. Returns true
  iff the group is actually removed.]

  SideEffects        []

  SeeAlso            [VarsHandler_reserve_group]
  
******************************************************************************/
boolean VarsHandler_release_group(VarsHandler_ptr self, GroupInfo_ptr gid)
{
  int num = 0;
  VARS_HANDLER_CHECK_INSTANCE(self);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stderr, "VarsHandler: freeing group: id=%" PRIuPTR \
            " low=%d, len=%d\n",
            gid->id, gid->lev_low, gid->lev_high-gid->lev_low+1);
  }

  /* checks if this groups is still valid */
  if (0 <= gid->lev_low && 0 <= gid->lev_high) {
    num = vars_handler_remove_group(self, self->forest, gid);     
  }

  FREE(gid);
  return num != 0;
}


/**Function********************************************************************

  Synopsis [Releases the given block (previously created with
  reserve_group). Differently from release_group, this method
  actually dissolves the group, all its children (contained groups)
  and all groups containing it (i.e. all parents).]

  Description [After this method has been called, gid cannot be
  used anymore. Also, all other GroupInfo instances possibly
  pointing to any of the removed groups will be invalidated, so
  later removals will be handled correctly.]

  SideEffects        []

  SeeAlso            [VarsHandler_reserve_group]
  
******************************************************************************/
void VarsHandler_dissolve_group(VarsHandler_ptr self, GroupInfo_ptr gid)
{
  VARS_HANDLER_CHECK_INSTANCE(self);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stderr, 
            "VarsHandler: dissolving group: id=%" PRIuPTR \
            " low=%d, len=%d\n",
            gid->id, gid->lev_low, gid->lev_high-gid->lev_low+1);
  }

  VarsHandler_update_levels(self);

  /* checks if this groups is still valid */
  if (0 <= gid->lev_low && 0 <= gid->lev_high) {
    VarsGroup* group;
    Oiter iter = vars_handler_promote_group(self, self->forest, gid);

    Olist_delete(self->forest, iter, (void**) &group);
    VarsGroup_destroy(group, self->dd);
  }

  FREE(gid);
}


/**Function********************************************************************

  Synopsis [After a reordering, levels in the dd package may do not
  correspond to the levels in the vars handler. This method re-align the 
  vars handler wrt the current levels.]

  Description [Realigns the whole internal groups structure, and
  all currently existing GroupInfo instances.]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
void VarsHandler_update_levels(VarsHandler_ptr self)
{
  Oiter iter;
  int prev_delta = INT_MAX;
  boolean order_may_change = false;

  VARS_HANDLER_CHECK_INSTANCE(self);
  

  OLIST_FOREACH(self->forest, iter) {
    VarsGroup* child = Oiter_element(iter);
    int delta = vars_handler_update_levels(self, child);
      
    order_may_change |= (prev_delta != INT_MAX && prev_delta != delta);
    prev_delta = delta;
  }

  if (order_may_change) Olist_sort(self->forest, vars_group_sort);    
}


/**Function********************************************************************

  Synopsis           [Prints the content of the VarsHandler]

  Description        [This is used for debugging/verosity purposes]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
void VarsHandler_print(const VarsHandler_ptr self, FILE* _file)
{
  Oiter iter;  
  VARS_HANDLER_CHECK_INSTANCE(self);

  OLIST_FOREACH(self->forest, iter) {
    VarsGroup* child = Oiter_element(iter);
    vars_group_print(child, _file, 0);
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Internal service for VarsHandler_release_group]

  Description        []

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
static int vars_handler_remove_group(VarsHandler_ptr self, 
                                     Olist_ptr list, 
                                     const GroupInfo_ptr ginfo)
{
  int removed = 0;
  Oiter iter = Olist_first(list);

  while (!Oiter_is_end(iter)) {
    
    VarsGroup* group = (VarsGroup*) Oiter_element(iter);
    if (ginfo->lev_high < group->lev_low) break; /* search is over */
    
    if (ginfo->lev_low <= group->lev_low && 
        ginfo->lev_high >= group->lev_high) {
      /* group id can be here and in children */
      boolean res = VarsGroup_remove_ginfo(group, ginfo);

      if (res && Olist_is_empty(group->gis)) {
        /* this group can be destroyed, and children moved up */
        if ((Olist_ptr) NULL != group->children) {   
          Oiter insert_iter = iter;
          Oiter iter2;
          OLIST_FOREACH(group->children, iter2) {
            VarsGroup* child = (VarsGroup*) Oiter_element(iter2);
            child->parent = group->parent;          
            insert_iter = Olist_insert_after(list, insert_iter, child);
          }
        }

        if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) { 
          fprintf(nusmv_stderr, 
                  "VarsHandler: removing %s group at level %d, size %d\n",
                  ((dd_block*) NULL != group->block) ? "physical" : "logical",
                  group->lev_low, group->lev_high-group->lev_low+1);
        }               

        /* if physical, removes the dd block as well */
        if ((dd_block*) NULL != group->block) {
          int res = dd_free_var_block(self->dd, group->block);
          nusmv_assert(0 == res);
          group->block = (dd_block*) NULL;
        }
        
        ++removed;
        iter = Olist_delete(list, iter, NULL);
        continue;
      }
      else if ((Olist_ptr) NULL != group->children) {  
        /* not find in the parent, search within the children */        
        removed += vars_handler_remove_group(self, group->children, ginfo);
      }
    }
    
    iter = Oiter_next(iter);
  } /* loop on input list */

  return removed;
}


/**Function********************************************************************

  Synopsis [Brings the given group at top level (along with all its
  children), splitting all the parents accordingly.]

  Description [Internal service of
  VarsHandler_dissolve_group, as before dissolving
  groups, they has to be brough to the top-level. Returns an
  iterator to the top level which contains the group.]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
static Oiter vars_handler_promote_group(VarsHandler_ptr self, 
                                        Olist_ptr list, 
                                        const GroupInfo_ptr ginfo)
{
  Oiter iter = Olist_first(list);
  while (!Oiter_is_end(iter)) {
    
    VarsGroup* group = (VarsGroup*) Oiter_element(iter);
    if (ginfo->lev_high < group->lev_low) { /* search is over */
      Oiter_make_end(&iter);
      break; 
    }
    
    if (ginfo->lev_low <= group->lev_low && 
        ginfo->lev_high >= group->lev_high) {
      /* group id can be here and in children */
      if (!Oiter_is_end(VarsGroup_find_ginfo(group, ginfo))) {
        /* found the group, returns it up, after  */
        return iter;
      }
      /* may be in children */
      else if ((Olist_ptr) NULL != group->children) { 
        Oiter iter_child = vars_handler_promote_group(self, group->children, 
                                                      ginfo);
        /* if found, substitutes at the current iteration point */
        if (!Oiter_is_end(iter_child)) {
          Oiter iter2, next;
          
          if ((dd_block*) NULL != group->block) {
            /* this is a physical block, dissolve it and creates
               new blocks out of its children */            
            int res = dd_free_var_block(self->dd, group->block);
            nusmv_assert(0 == res);
            group->block = (dd_block*) NULL;

            /* moves up the children */
            OLIST_FOREACH(group->children, iter2) {
              VarsGroup* child = (VarsGroup*) Oiter_element(iter2);
              if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
                fprintf(nusmv_stderr, "VarsHandler: promoting physical var block " \
                        "at index %d, size %d\n", child->idx_low, 
                        child->lev_high - child->lev_low + 1);
              }

              CATCH {
                nusmv_assert(child->lev_low == ginfo->lev_low && 
                             child->lev_high == ginfo->lev_high);
                child->block = 
                  dd_new_var_block(self->dd, child->idx_low, 
                                   child->lev_high - child->lev_low + 1);
              }
              FAIL {
                internal_error("vars_handler: Failure during variable group construction\n");
              }
            }
          }

          /* the set of pointing groupinfo has to be set as eliminated */
          OLIST_FOREACH(group->gis, iter2) {
            GroupInfo_ptr ginfo = Oiter_element(iter2);
            ginfo->lev_low = -1;
            ginfo->lev_high = -1;              
          }

          next = Olist_delete(list, iter, NULL);
          Olist_move(group->children, list, next);
        }
        return iter;
      }
    }

    iter = Oiter_next(iter);
  }

  return iter;
}


/**Function********************************************************************

  Synopsis    [Searches a good place where the group can be inserted.]

  Description [This method is used for searching a given group, or
  for searching a good place where a new group can be
  inserted. 

  There are four input parameters: The VarsHandler instances, the
  minimal level at which a group shall be searched from, the size
  of the searched group, and the chunk size (2 for state variables,
  1 for frozen and input).

  There are three output parameters: a group, the actual found
  minimal level, and a value identifying the result of the search.

  If a perfect match is found (the searched group is found), the
  group is returned, and lev_low will be kept untouched.

  If a root is found, it means that the searched group does not
  exist, but a group was found which can contain it. In this case
  the parent is returned.

  If not found, NULL is returned as VarsGroup, and lev_low will
  contain the first usable level which can be used to create a new
  top-level group (see how this case is used in
  VarsHandler_reserve_group)]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
static VarsGroup* vars_handler_search_group(const VarsHandler_ptr self,
                                            int lev_low, size_t size,
                                            int chunk,
                                            enum GroupSearchRes* res)
{
  Oiter iter; 
  OLIST_FOREACH(self->forest, iter) {
    VarsGroup* root = (VarsGroup*) Oiter_element(iter);
    VarsGroup* group = vars_group_search_group_aux(root, lev_low, size,
                                                   chunk, res);
    if ((VarsGroup*) NULL != group) return group;
    if (GS_NOT_FOUND != *res) break;
  }

  return (VarsGroup*) NULL;
}


/**Function********************************************************************

  Synopsis           [Adds the given group to the forest]

  Description        [If parent is NULL, group has to be a physical ]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
static void vars_handler_add_group(VarsHandler_ptr self, 
                                   VarsGroup* parent, VarsGroup* group)
{
  Olist_ptr list;
  Oiter iter; 
  
  if ((VarsGroup*) NULL == parent) {
    /* it has to be a physical group */
    nusmv_assert((dd_block*) NULL != group->block);
    list = self->forest;
  }
  else {
    /* it has to be a logical group */
    nusmv_assert((dd_block*) NULL == group->block);
    if ((Olist_ptr) NULL == parent->children) {
      parent->children = Olist_create();
    }
    list = parent->children;
    group->parent = parent;
  }

  OLIST_FOREACH(list, iter) {
    VarsGroup* el = (VarsGroup*) Oiter_element(iter);
    if (group->lev_low < el->lev_low) { /* insert point */
      nusmv_assert(group->lev_high < el->lev_low); /* no overlap */
      Olist_insert_before(list, iter, group);            
      return;
    }
  }

  /* append */
  Olist_append(list, group);
}


/**Function********************************************************************

  Synopsis           [Updates the levels of the root and all its children]

  Description        [Internal service of VarsHandler_update_levels]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
static int vars_handler_update_levels(VarsHandler_ptr self, VarsGroup* root)
{
  const int delta = (dd_get_level_at_index(self->dd, root->idx_low) - 
                     root->lev_low);
  Oiter iter; 

  if (0 != delta) {
    root->lev_low += delta;
    root->lev_high += delta;

    OLIST_FOREACH(root->gis, iter) {
      GroupInfo_ptr ginfo = Oiter_element(iter);
      if (0 <= ginfo->lev_low && 0 <= ginfo->lev_high) {
        ginfo->lev_low += delta;
        ginfo->lev_high += delta;
      }
    }
    /* ordering of gis is kept */
  }

  if ((Olist_ptr) NULL != root->children) { /* update children */    
    int prev_child_delta = INT_MAX;
    boolean children_order_may_change = false;
    OLIST_FOREACH(root->children, iter) {
      VarsGroup* child = Oiter_element(iter);
      int child_delta = vars_handler_update_levels(self, child);
      
      children_order_may_change |= (prev_child_delta != INT_MAX && 
                                    prev_child_delta != child_delta);
      prev_child_delta = child_delta;
    }

    if (children_order_may_change) Olist_sort(root->children, vars_group_sort);    
  }

  return delta;
}


/**Function********************************************************************

  Synopsis           [Searches the closest group]

  Description        [Internal service of vars_group_search_group]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static VarsGroup* vars_group_search_group_aux(const VarsGroup* in_group,
                                              int lev_low, size_t size, 
                                              int chunk,
                                              enum GroupSearchRes* res)
{
  int lev_high = lev_low + size - 1;

  /* perfect match */
  if (lev_low == in_group->lev_low &&
      lev_high == in_group->lev_high &&
      chunk == in_group->chunk) {
    *res = GS_PERF_MATCH_FOUND;
    return (VarsGroup*) in_group;
  }

  /* match fail: outside */
  if (lev_low > in_group->lev_high || lev_high < in_group->lev_low) {
    *res = GS_NOT_FOUND;
    return (VarsGroup*) NULL;
  }

  /* match fail: overlap */
  if ((lev_low < in_group->lev_low && lev_high >= in_group->lev_low) ||
      (lev_high > in_group->lev_high && lev_low <= in_group->lev_high)) {
    *res = GS_OVERLAP_FOUND;
    return (VarsGroup*) NULL;
  }

  /* wrong chunk size */
  if (chunk != in_group->chunk) {
    *res = GS_WRONG_CHUNK;
    return (VarsGroup*) NULL;
  }

  /* here is inside, search the most precise sub-group */
  nusmv_assert(lev_low >= in_group->lev_low && lev_high <= in_group->lev_high);

  /* searches among the children */
  if ((Olist_ptr) NULL != in_group->children ) {
    Oiter iter;
    OLIST_FOREACH(in_group->children, iter) {
      VarsGroup* child = Oiter_element(iter);

      if (lev_low >= child->lev_low && lev_high <= child->lev_high) {
        /* the group has to be in child */
        return vars_group_search_group_aux(child, lev_low, size, chunk, res);
      }

      if (lev_low < child->lev_low) break; /* it is over */
    } /* for in_group's children */
  }

  *res = GS_ROOT_FOUND;
  return (VarsGroup*) in_group;
}


/**Function********************************************************************

  Synopsis           [The VarsHandler class private initializer]

  Description        [The VarsHandler class private initializer]

  SideEffects        []

  SeeAlso            [VarsHandler_create]   
  
******************************************************************************/
static void vars_handler_init(VarsHandler_ptr self, DdManager* dd)
{
  /* members initialization */
  nusmv_assert((DdManager*) NULL != dd);
  self->dd = dd;

  self->forest = Olist_create();
  self->id_counter = 0;
}


/**Function********************************************************************

  Synopsis           [The VarsHandler class private deinitializer]

  Description        [The VarsHandler class private deinitializer]

  SideEffects        []

  SeeAlso            [VarsHandler_destroy]   
  
******************************************************************************/
static void vars_handler_deinit(VarsHandler_ptr self)
{
  /* members deinitialization */
  Oiter iter;
  OLIST_FOREACH(self->forest, iter) {
    VarsGroup* group = Oiter_element(iter);
    VarsGroup_destroy(group, self->dd);    
  }
  Olist_destroy(self->forest);
}


/**Function********************************************************************

  Synopsis           [VarsGroups cosntructor]

  Description        []

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
static VarsGroup* VarsGroup_create(int lev_low, int lev_high, int idx_low, 
                                   int chunk)
{
  VarsGroup* self = ALLOC(VarsGroup, 1);
  nusmv_assert((VarsGroup*) NULL != self);

  nusmv_assert(lev_low <= lev_high);
  nusmv_assert(chunk > 0);

  self->lev_low = lev_low;
  self->lev_high = lev_high;
  self->chunk = chunk;
  self->idx_low = idx_low;
  self->block = (dd_block*) NULL;
  
  self->gis = Olist_create();
  self->parent = (VarsGroup*) NULL;
  self->children = (Olist_ptr) NULL;

  return self;
}


/**Function********************************************************************

  Synopsis           [VarsGroups destroyer]

  Description        [Traverses all the tree, and frees it]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
static void VarsGroup_destroy(VarsGroup* self, DdManager* dd)
{
  Oiter iter;

  if ((dd_block*) NULL != self->block) {
    int res = dd_free_var_block(dd, self->block);
    nusmv_assert(0 == res);
    self->block = (dd_block*) NULL;
  }

  /* invalidates all pointing group info  */
  OLIST_FOREACH(self->gis, iter) {
    GroupInfo_ptr ginfo = Oiter_element(iter);
    ginfo->lev_low = -1;
    ginfo->lev_high = -1;              
  }
  Olist_destroy(self->gis);

  if ((Olist_ptr) NULL != self->children) {   
    Oiter iter; 
    OLIST_FOREACH(self->children, iter) {
      VarsGroup* child = Oiter_element(iter);
      VarsGroup_destroy(child, dd);
    }
    Olist_destroy(self->children);
  }
   
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Adds the given GroupInfo to the given VarsGroup]

  Description [Since internal list of group info is sorted for the
  sake of good performances, this method inserts in order.]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
static void VarsGroup_add_ginfo(VarsGroup* self, GroupInfo_ptr gi)
{
  Oiter iter;

  OLIST_FOREACH(self->gis, iter) {
    GroupInfo_ptr _gi = (GroupInfo_ptr) Oiter_element(iter);
    nusmv_assert(gi->id != _gi->id); /* not already created id */

    if (_gi->id > gi->id) { /* found insertion point */
      Olist_insert_before(self->gis, iter, (void*) gi);
      return;
    }
  }
  
  /* append */
  Olist_append(self->gis, (void*) gi);
}


/**Function********************************************************************

  Synopsis    [Removes the id if found.]

  Description [Returns: true if found, false otherwise.]

  SideEffects [Changes the gis internal list. Do not call when
  iterating on it.]

  SeeAlso     []
  
******************************************************************************/
static boolean VarsGroup_remove_ginfo(VarsGroup* self, GroupInfo_ptr gi)
{
  Oiter iter = VarsGroup_find_ginfo(self, gi);
  if (!Oiter_is_end(iter)) {
    Olist_delete(self->gis, iter, (void**) NULL);
    return true;
  }
  return false;
}


/**Function********************************************************************

  Synopsis    [Searches the given group information.]

  Description [Returns the iterator pointing to it in the internal list]

  SideEffects []

  SeeAlso     []
  
******************************************************************************/
static Oiter VarsGroup_find_ginfo(const VarsGroup* self, GroupInfo_ptr gi)
{
  Oiter iter;

  OLIST_FOREACH(self->gis, iter) {    
    GroupInfo_ptr _gi = (GroupInfo_ptr) Oiter_element(iter);
    if (_gi->id == gi->id) break;
    if (_gi->id > gi->id) {
      Oiter_make_end(&iter);
      break; /* limit found */
    }
  }
    
  return iter;
}


/**Function********************************************************************

  Synopsis           [Internal service used when sorting VarsGroup lists]

  Description        []

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
static int vars_group_sort(void* _g1, void* _g2)
{ return (((VarsGroup*)_g1)->lev_low - ((VarsGroup*)_g2)->lev_low); }


/**Function********************************************************************

  Synopsis           [Internal service used when printing groups]

  Description        []

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
static void vars_group_print(VarsGroup* group, FILE* _file, int indent)
{
  int i;
  for (i=indent; i>0; --i) fprintf(_file, "  ");
  fprintf(_file, "%s %d-%d idx:%d chunk:%d (%d users:", 
          (NULL != group->block) ? "P" : "L", 
          group->lev_low, group->lev_high, group->idx_low, group->chunk, 
          Olist_get_size(group->gis));

  {
    Oiter iter;
    OLIST_FOREACH(group->gis, iter) {
      GroupInfo_ptr ginfo = Oiter_element(iter);
      fprintf(_file, " %" PRIuPTR, ginfo->id);
    }
    fprintf(_file, ")\n");
  }

  if ((Olist_ptr) NULL != group->children) {
    Oiter iter;
    OLIST_FOREACH(group->children, iter) {
      VarsGroup* child = Oiter_element(iter);
      vars_group_print(child, _file, indent + 1);
    }
  }
}


/**Function********************************************************************

  Synopsis [Searches among the available groups for holes, and
  return the first free level starting from given level.]

  Description [If needed, creates size new variables from the
  frontier, in particular, if from_lev is < 0.]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
static int vars_handler_get_first_free_level(VarsHandler_ptr self,
                                             int from_lev, int size)
{    
  if (from_lev >= 0) {
    int _from_lev = from_lev;
    Oiter iter;    

  
    /* searches for a gap in already created levels */
    OLIST_FOREACH(self->forest, iter) {
      VarsGroup* curr = Oiter_element(iter);
      
      /* sees if there is a usable gap between requested level and curr */
      if (curr->lev_low - _from_lev >= size) {
        return _from_lev;
      }

      /* found the searched threshold: now searches above it */
      if (curr->lev_low >= from_lev) {
        _from_lev = curr->lev_high+1;
      }
    }

    /* there is space after the last group and the frontier */
    if (_from_lev+size-1 < dd_get_size(self->dd)) return _from_lev; 
  }

  { /* no previously created level is available: create new
       variables from the frontier */
    dd_reorderingtype reord_type;
    int reord_status = dd_reordering_status(self->dd, &reord_type);
    int new_idx = dd_get_size(self->dd);
    int i;
      
    /* avoid index 0 */
    if (0 == new_idx) new_idx += 1; 

    if (new_idx >= MAX_VAR_INDEX) error_too_many_vars();

    dd_autodyn_disable(self->dd);
    for (i=0; i<size; ++i) {      
      add_ptr add = add_new_var_with_index(self->dd, i+new_idx);
      nusmv_assert(dd_get_level_at_index(self->dd, i+new_idx) == i+new_idx);
      add_free(self->dd, add);
    }
      
    if (1 == reord_status) {
      dd_autodyn_enable(self->dd, reord_type);
    }
    return new_idx; /* index and level coincide in this case */
  }
}



/**AutomaticEnd***************************************************************/

