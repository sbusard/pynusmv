/**CFile***********************************************************************

  FileName    [Cluster.c]

  PackageName [trans.bdd]

  Synopsis    [Routines related to clusters of transition relation. ]
 
  Description [ This file conains the definition of Cluster and all derived 
  classes]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``trans.bdd'' package of NuSMV version 2. 
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

#include "Cluster.h"
#include "utils/object_private.h"

static char rcsid[] UTIL_UNUSED = "$Id: Cluster.c,v 1.1.2.2.4.1.6.2 2007-06-18 16:51:00 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [Cluster Class]

  Description [ This class contains informations about a cluster:<br>
          <dl> 
            <dt><code>curr_cluster</code>
                <dd> The clusterized transition relation.  
            <dt><code>ex_state_input</code>
                <dd>  List of variables (state and input vars) that can be 
                existentially quantified when curr_cluster is multiplied in
                the product.
            <dt><code>ex_state</code>
                <dd> List of variables (only state vars) that can be 
                existentially quantified when curr_cluster is multiplied in the
                product. 
        </dl>
        <br>
        Note that frozen variables are not taken into account because
        they are never abstracted away (or used some other way) in
        pre- or post-image computation.
        <br>
        In addition, this class inherits from the Object class and contains
        a virtual copy constructor. ]

  SeeAlso     []   
  
******************************************************************************/
typedef struct Cluster_TAG
{
  INHERITS_FROM(Object);
  
  /* The current Cluster */
  bdd_ptr curr_cluster; /* was Ti */            
  
  /* Variables that can be existentially quantified when curr_cluster is
     multiplied in the product (state and input vars) */ 
  bdd_ptr ex_state_input; /* was Ei_StateInput */
  
  /* Variables that can be existentially quantified when curr_cluster is
     multiplied in the product (only state vars) */ 
  bdd_ptr ex_state; /* was Ei_State */

/* ---------------------------------------------------------------------- */ 
/*     Virtual Methods                                                    */
/* ---------------------------------------------------------------------- */   
  
} Cluster; 


/**Struct**********************************************************************

  Synopsis    [Iwls'95 Cluster Class ]

  Description [ This class inherits from the "Cluster" class and also contains
  a field "benifit" to be used while ordering clusters.]

  SeeAlso     []   
  
******************************************************************************/
typedef struct ClusterIwls95_TAG
{
  INHERITS_FROM(Cluster);
  
  double benefit;

} ClusterIwls95; 

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void cluster_init ARGS((Cluster_ptr self, DdManager* dd));
static void cluster_deinit ARGS((Cluster_ptr self, DdManager* dd));
static void cluster_finalize ARGS((Object_ptr object, void* arg));
static Object_ptr cluster_copy ARGS((const Object_ptr object));

static void cluster_copy_aux ARGS((const Cluster_ptr object, 
                                   Cluster_ptr copy));

static void cluster_iwls95_init ARGS((ClusterIwls95_ptr self, 
                                      DdManager* dd,  
                                      const ClusterOptions_ptr cl_options, 
                                      const double v_c, 
                                      const double w_c, 
                                      const double x_c, 
                                      const double y_c, 
                                      const double z_c, 
                                      const double min_c, 
                                      const double max_c));

static void cluster_iwls95_deinit ARGS((ClusterIwls95_ptr self, DdManager* dd));
static void cluster_iwls95_finalize ARGS((Object_ptr object, void* arg));

static Object_ptr cluster_iwls95_copy ARGS((const Object_ptr object));

static void 
cluster_iwls95_copy_aux ARGS((const ClusterIwls95_ptr object, 
                              ClusterIwls95_ptr copy));

/**Function********************************************************************

  Synopsis           [The "Cluster" class constructor.]

  Description        [Allocates and initializes a cluster.]

  SideEffects        []

  SeeAlso            [Object_destroy]   
  
******************************************************************************/
Cluster_ptr Cluster_create(DdManager* dd)
{
  Cluster_ptr self = ALLOC(Cluster, 1);
  CLUSTER_CHECK_INSTANCE(self);

  cluster_init(self, dd);
  return self;
}


/**Function********************************************************************

  Synopsis           [Checks if two clusters are equal]

  Description [Notice that the check is performed only using the
  \"curr_cluster\" field of the Cluster class.]

  SideEffects        []

  SeeAlso     []   
  
******************************************************************************/
boolean Cluster_is_equal(const Cluster_ptr self, const Cluster_ptr other)
{
  CLUSTER_CHECK_INSTANCE(self);
  CLUSTER_CHECK_INSTANCE(other);

  return (self->curr_cluster == other->curr_cluster);
}


/**Function********************************************************************

  Synopsis           [Retrives the clusterized transition relation of the self
  .]

  Description        [Returned bdd will be referenced]

  SideEffects        []

  SeeAlso     []   
  
******************************************************************************/
bdd_ptr Cluster_get_trans(const Cluster_ptr self)
{
  bdd_ptr tmp = (bdd_ptr)NULL;
  
  CLUSTER_CHECK_INSTANCE(self);
  if (self->curr_cluster != (bdd_ptr)NULL) {
    tmp = bdd_dup(self->curr_cluster);
  }

  return tmp;  
}


/**Function********************************************************************

  Synopsis           [Sets the transition relation inside the cluster]

  Description        [The given bdd will be referenced. Previously stored bdd 
  will be released]

  SideEffects        []

  SeeAlso     []   
  
******************************************************************************/
void Cluster_set_trans(Cluster_ptr self, DdManager* dd, bdd_ptr current)
{
  CLUSTER_CHECK_INSTANCE(self);

  if (self->curr_cluster != (bdd_ptr) NULL) {
    bdd_free(dd, self->curr_cluster);
    self->curr_cluster = (bdd_ptr) NULL;
  }
  if (current != (bdd_ptr) NULL) {
    self->curr_cluster = bdd_dup(current);
  }
}

/**Function********************************************************************

  Synopsis           [Returns a pointer to the list of variables (both state 
  and input vars) to be quantified.]

  Description        [Returns a pointer to the list of variables to be
  quantified respect to the transition relation inside the cluster. Returned
  bdd is referenced.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
bdd_ptr Cluster_get_quantification_state_input(const Cluster_ptr self)
{
  bdd_ptr tmp = (bdd_ptr)NULL;
  
  CLUSTER_CHECK_INSTANCE(self);
  if (self->ex_state_input != (bdd_ptr) NULL) {
    tmp = bdd_dup(self->ex_state_input);
  }

  return tmp;  
}


/**Function********************************************************************

  Synopsis           [Sets the list of variables (both state and input vars) to
  be quantified inside the cluster.]

  Description        [Given value will be referenced]

  SideEffects        []

  SeeAlso     []   
  
******************************************************************************/
void Cluster_set_quantification_state_input(Cluster_ptr self, 
                                            DdManager* dd, bdd_ptr new)
{
  CLUSTER_CHECK_INSTANCE(self);

  if (self->ex_state_input != (bdd_ptr) NULL) {
    bdd_free(dd, self->ex_state_input);
    self->ex_state_input = (bdd_ptr) NULL;
  }
  if (new != (bdd_ptr) NULL) {
    self->ex_state_input = bdd_dup(new);
  }
}


/**Function********************************************************************

  Synopsis           [Returns a pointer to the list of variables (state vars
  only) to be quantified]

  Description        [Returned value is referenced]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
bdd_ptr Cluster_get_quantification_state(const Cluster_ptr self)
{
  bdd_ptr tmp = (bdd_ptr)NULL;
  
  CLUSTER_CHECK_INSTANCE(self);
  if (self->ex_state != (bdd_ptr) NULL) {
    tmp = bdd_dup(self->ex_state);
  }

  return tmp;  
}

/**Function********************************************************************

  Synopsis           [Sets the list of variables (state vars only) to be
  quantified inside the cluster]

  Description        [Given value will be referenced]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
void Cluster_set_quantification_state(Cluster_ptr self, 
                                      DdManager* dd, bdd_ptr new)
{
  CLUSTER_CHECK_INSTANCE(self);

  if (self->ex_state != (bdd_ptr) NULL) {
    bdd_free(dd, self->ex_state);
    self->ex_state = (bdd_ptr) NULL;
  }

  if (new != (bdd_ptr) NULL) {
    self->ex_state = bdd_dup(new);
  }
}


/* ====================================================================== */


/**Function********************************************************************

  Synopsis           [ "ClusterIwls95" Class constructor.]
  
  Description        [Allocates and initializes a cluster for IWLS95 alg.
  Please note that returned object can be casted to a cluster class instance.
  Use Cluster_destroy to destroy returned instance. The parameters passed to
  the constructor correspond to cluster options and 7 different factors (v_c,
  w_c, x_c, y_c, z_c, min_c and max_c) as explained in IWLS95 paper.]

  SideEffects        []

  SeeAlso            [Cluster_destroy Cluster_create]
  
******************************************************************************/
ClusterIwls95_ptr ClusterIwls95_create(DdManager* dd, 
                                       const ClusterOptions_ptr cl_options, 
                                       const double v_c, 
                                       const double w_c, 
                                       const double x_c, 
                                       const double y_c, 
                                       const double z_c, 
                                       const double min_c, 
                                       const double max_c)
{
  ClusterIwls95_ptr self =  ALLOC(ClusterIwls95, 1);

  CLUSTER_IWLS95_CHECK_INSTANCE(self); 

  cluster_iwls95_init(self, dd, cl_options, v_c, w_c, x_c, 
                      y_c, z_c, min_c, max_c);
  return self;
}


/**Function********************************************************************

  Synopsis           [Returns the value of the "benifit" variable. ]

  Description        []

  SideEffects        []

  SeeAlso     []   
  
******************************************************************************/

double ClusterIwls95_get_benefit(const ClusterIwls95_ptr self)
{
  CLUSTER_IWLS95_CHECK_INSTANCE(self); 
  return self->benefit;
}





/**Function********************************************************************

  Synopsis           [Initializes the cluster with default values.]

  Description        []

  SideEffects        []

  SeeAlso     []   
  
******************************************************************************/
static void cluster_init(Cluster_ptr self, DdManager* dd)
{
  object_init(OBJECT(self));

  self->curr_cluster    = bdd_true(dd);
  self->ex_state_input  = bdd_true(dd);
  self->ex_state        = bdd_true(dd);

  OVERRIDE(Object, finalize) = cluster_finalize;
  OVERRIDE(Object, copy) = cluster_copy; 
}


/**Function********************************************************************

  Synopsis           [Deinitializes the cluster. ]

  Description        [Releases the contained bdds.]

  SideEffects        []

  SeeAlso     []   
  
******************************************************************************/
static void cluster_deinit(Cluster_ptr self, DdManager* dd)
{
  object_deinit(OBJECT(self));

  /* Releases contained bdds: */
  if (self->curr_cluster != (bdd_ptr) NULL) bdd_free(dd, self->curr_cluster);
  if (self->ex_state_input != (bdd_ptr) NULL) bdd_free(dd, self->ex_state_input);
  if (self->ex_state != (bdd_ptr) NULL) bdd_free(dd, self->ex_state);
}


/**Function********************************************************************

  Synopsis           [ Finalize a cluster.]

  Description        []

  SideEffects        []

  SeeAlso     []   
  
******************************************************************************/
static void cluster_finalize(Object_ptr object, void* arg)
{
  Cluster_ptr self = CLUSTER(object); 

  cluster_deinit(self, (DdManager*) arg);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [ Copies the given cluster.]

  Description        [It is the callback function that the copy constructor
  virtually calls.]

  SideEffects        []

  SeeAlso            [cluster_copy_aux]   
  
******************************************************************************/
static Object_ptr cluster_copy(const Object_ptr object)
{
  Cluster_ptr self = CLUSTER(object);
  Cluster_ptr copy = CLUSTER(NULL);

  copy = ALLOC(Cluster, 1);
  CLUSTER_CHECK_INSTANCE(copy);

  cluster_copy_aux(self, copy);
  return OBJECT(copy);
}


/**Function********************************************************************

  Synopsis           [It helps to copy the given cluster.]

  Description        []

  SideEffects        []

  SeeAlso            [cluster_copy]   
  
******************************************************************************/
static void cluster_copy_aux(const Cluster_ptr self, Cluster_ptr copy)
{
  /* copies the base class: */
  object_copy_aux(OBJECT(self), OBJECT(copy));

  /* copies class members: */
  copy->curr_cluster = bdd_dup(self->curr_cluster);  
  copy->ex_state_input = bdd_dup(self->ex_state_input); 
  copy->ex_state = bdd_dup(self->ex_state);
}

/**Function********************************************************************

  Synopsis           [ Initializes Iwls95 cluster. ]

  Description        [The parameters passed to this private function correspond
  to cluster options and different factors (v_c, w_c, x_c, y_c, z_c, min_c and
  max_c) as explained in IWLS95 paper.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
static void cluster_iwls95_init(ClusterIwls95_ptr self, 
                                DdManager* dd, 
                                const ClusterOptions_ptr cl_options, 
                                const double v_c, 
                                const double w_c, 
                                const double x_c, 
                                const double y_c, 
                                const double z_c, 
                                const double min_c, 
                                const double max_c)

{
  double w1; 
  double w2; 

  cluster_init( CLUSTER(self), dd );
  
  w1 = (double) ClusterOptions_get_w1(cl_options); 
  w2 = (double) ClusterOptions_get_w2(cl_options); 
  
  if (w_c != 0)  self->benefit = (v_c / w_c) * w1; 
  else  self->benefit = 0.0; 

  if (x_c != 0)   self->benefit += (w_c / x_c) * w2; 
  if (z_c != 0)   self->benefit -= (y_c / z_c) * w2;
  if (max_c != 0) self->benefit += (min_c / max_c) * w2;

  OVERRIDE(Object, finalize) = cluster_iwls95_finalize;
  OVERRIDE(Object, copy) = cluster_iwls95_copy; 
}



/**Function********************************************************************

  Synopsis           [ Deinitialized Iwls95 cluster. ]

  Description        []

  SideEffects        []

  SeeAlso     []   
  
******************************************************************************/
static void cluster_iwls95_deinit(ClusterIwls95_ptr self, DdManager* dd)
{
  /* nothing to clean up in self */
  cluster_deinit( CLUSTER(self), dd );
}

/**Function********************************************************************

  Synopsis           [ Finalize iwls95 cluster. ]

  Description        [ The virtual destructor calls this method to destroy the
  instance self.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
static void cluster_iwls95_finalize(Object_ptr object, void* arg)
{
  ClusterIwls95_ptr self = CLUSTER_IWLS95(object); 
  cluster_iwls95_deinit(self, (DdManager*) arg);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [ Copies iwls95 cluster.]

  Description        [ Callback function that copy constructor virtually calls
  to copy an instance of iwls95 cluster.]

  SideEffects        []

  SeeAlso            [cluster_iwls95_copy_aux]   
  
******************************************************************************/
static Object_ptr cluster_iwls95_copy(const Object_ptr object)
{
  ClusterIwls95_ptr self = CLUSTER_IWLS95(object); 
  ClusterIwls95_ptr copy;

  copy = ALLOC(ClusterIwls95, 1);
  CLUSTER_IWLS95_CHECK_INSTANCE(copy); 

  cluster_iwls95_copy_aux(self, copy);
  return OBJECT(copy); 
}


/**Function********************************************************************

  Synopsis           [ It helps to copy iwls95 cluster.]

  Description        []

  SideEffects        []

  SeeAlso            [cluster_iwls95_copy]   
  
******************************************************************************/
static void cluster_iwls95_copy_aux(const ClusterIwls95_ptr self, 
                                    ClusterIwls95_ptr copy)
{
  cluster_copy_aux(CLUSTER(self), CLUSTER(copy));

  copy->benefit = self->benefit;
}

