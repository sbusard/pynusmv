/**CFile***********************************************************************

  FileName    [ClusterOptions.c]

  PackageName [trans.bdd]

  Synopsis    [ Routines related to "ClusterOptions" class.]

  Description [ This file contains class "ClusterOptions" which  stores various
  options available for Clusters like threshold, affinity, preorder and options
  related to Iwls95 algo. ]

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

#include "ClusterOptions.h"
#include "transInt.h"

#include "opt/opt.h" /* for options_ptr */

static char rcsid[] UTIL_UNUSED = "$Id: ClusterOptions.c,v 1.1.2.2.4.1.6.2 2007-04-30 16:41:18 nusmv Exp $";

/**Struct**********************************************************************

  Synopsis    [ ClusterOptions Class.]

  Description [This class contains the options to perform ordering
  of clusters in the IWLS95 partitioning method. <br>
  <code>cluster_size</code> is the threshold value used to create
  clusters. <code>w1</code>, <code>w2</code> etc are the weights used
  in the heuristic algorithms to order the clusters for early
  quantification.]

******************************************************************************/
typedef struct ClusterOptions_TAG {

  int _threshold;
  boolean _is_affinity; 
  boolean _is_preorder;
  
  boolean _cluster_appended; /* clusters must be prepended or appended */

  int _cluster_size;
  
  int _w1;

  int _w2; /* Weights attached to various parameters. */

  int _w3; /* For details please refer to the paper */

  int _w4; /* (insert reference of the paper IWLS95) */

} ClusterOptions;



/* ---------------------------------------------------------------------- */
/* Methods                                                                */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis           [ "ClusterOptions" class constructor.]

  Description        [  Creates a ClusterOptions instance. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ClusterOptions_ptr ClusterOptions_create(OptsHandler_ptr opt)
{
  ClusterOptions_ptr self;
  char* flag_value;
  
  self = ALLOC(ClusterOptions, 1);
  CLUSTER_OPTIONS_CHECK_INSTANCE(self);
  
  /* fills out the structure */
  
  self->_threshold = get_conj_part_threshold(opt);
  self->_is_affinity = opt_affinity(opt);
  self->_is_preorder = opt_iwls95_preorder(opt);
  self->_cluster_appended = opt_append_clusters(opt);

  self->_cluster_size = get_image_cluster_size(opt);

  if (!OptsHandler_is_option_registered(opt, "image_W1")){
    self->_w1 = 6; /* the default value */
  }
  else {
    flag_value =
      OptsHandler_get_string_representation_option_value(opt, "image_W1");
    self->_w1 = atoi(flag_value);
  }

  if (!OptsHandler_is_option_registered(opt, "image_W2")){
    self->_w2 = 1; /* the default value */
  }
  else {
    flag_value =
      OptsHandler_get_string_representation_option_value(opt, "image_W2");
    self->_w2 = atoi(flag_value);
  }

  if (!OptsHandler_is_option_registered(opt, "image_W3")){
    self->_w3 = 1; /* the default value */
  }
  else {
    flag_value =
      OptsHandler_get_string_representation_option_value(opt, "image_W3");
    self->_w3 = atoi(flag_value);
  }

  if (!OptsHandler_is_option_registered(opt, "image_W4")){
    self->_w4 = 2; /* the default value */
  }
  else {
    flag_value =
      OptsHandler_get_string_representation_option_value(opt, "image_W4");
    self->_w4 = atoi(flag_value);
  }

  return self;
}

/**Function********************************************************************

  Synopsis           [ ClusterOption class destructor.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ClusterOptions_destroy(ClusterOptions_ptr self)
{
  CLUSTER_OPTIONS_CHECK_INSTANCE(self);
  FREE(self);
}

/**Function********************************************************************

  Synopsis           [ Returns the threshold field. ]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int ClusterOptions_get_threshold(const ClusterOptions_ptr self)
{
  CLUSTER_OPTIONS_CHECK_INSTANCE(self);

  return self->_threshold;
}


/**Function********************************************************************

  Synopsis           [ Checks whether Affinity is enabled. ]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ClusterOptions_is_affinity(const ClusterOptions_ptr self)
{
  CLUSTER_OPTIONS_CHECK_INSTANCE(self);

  return self->_is_affinity;
}


/**Function********************************************************************

  Synopsis           [ Returns true if clusters must be appended, false if 
  clusters must be prepended ]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ClusterOptions_clusters_appended(const ClusterOptions_ptr self)
{
  CLUSTER_OPTIONS_CHECK_INSTANCE(self);
  return self->_cluster_appended;
}


/**Function********************************************************************

  Synopsis           [ Checks whether preordering is enabled. ]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ClusterOptions_is_iwls95_preorder(const ClusterOptions_ptr self)
{
  CLUSTER_OPTIONS_CHECK_INSTANCE(self);

  return self->_is_preorder;
}


/**Function********************************************************************

  Synopsis           [ Returns the cluster_size field. ]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int ClusterOptions_get_cluster_size(const ClusterOptions_ptr self)
{
  CLUSTER_OPTIONS_CHECK_INSTANCE(self);

  return self->_cluster_size;
}

/**Function********************************************************************

  Synopsis           [ Retrieves the parameter w1. ]

  Description        [ According to the IWLS95 paper parameter w1 represents the
  weight attached to the R^1_c( =v_c/w_c) factor.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int ClusterOptions_get_w1(const ClusterOptions_ptr self)
{
  CLUSTER_OPTIONS_CHECK_INSTANCE(self);

  return self->_w1;
}

/**Function********************************************************************

  Synopsis           [Retrieves the parameter w2. ]

  Description        [According to the IWLS95 paper parameter w2 represents the
    weight attached to the R^2_c( =w_c/x_c) factor.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int ClusterOptions_get_w2(const ClusterOptions_ptr self)
{
  CLUSTER_OPTIONS_CHECK_INSTANCE(self);

  return self->_w2;
}

/**Function********************************************************************

  Synopsis           [Retrieves the parameter w3. ]

  Description        [According to the IWLS95 paper parameter w3 represents the
    weight attached to the R^3_c( =y_c/z_c) factor.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int ClusterOptions_get_w3(const ClusterOptions_ptr self)
{
  CLUSTER_OPTIONS_CHECK_INSTANCE(self);

  return self->_w3;
}

/**Function********************************************************************

  Synopsis           [Retrieves the parameter w4. ]

  Description        [According to the IWLS95 paper parameter w4 represents the
    weight attached to the R^4_c( =min_c/max_c) factor.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int ClusterOptions_get_w4(const ClusterOptions_ptr self)
{
  CLUSTER_OPTIONS_CHECK_INSTANCE(self);

  return self->_w4;
}

/**Function********************************************************************

  Synopsis           [ Prints all the cluster options inside the specified file.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ClusterOptions_print(const ClusterOptions_ptr self, FILE* file)
{
  CLUSTER_OPTIONS_CHECK_INSTANCE(self);

  fprintf(file, "Printing cluster options:\n"); 
  fprintf(file, "   Threshold Value of Bdd Size For Creating Clusters = %d\n",
          self-> _cluster_size);
  fprintf(file, "   Affinity = %s\n", (self->_is_affinity) ? "enabled": "disabled");
  fprintf(file, "   Preorder = %s\n", (self->_is_preorder) ? "enabled": "disabled");
  fprintf(file, "   W1 =%3d\n", self->_w1);
  fprintf(file, "   W2 =%2d\n", self->_w2);
  fprintf(file, "   W3 =%2d\n", self->_w3);
  fprintf(file, "   W4 =%2d\n", self->_w4);
  fprintf(file, "Use \"set image_cluster_size value\" to set this to desired value.\n"); 
  fprintf(file, "Use \"set image_verbosity value\" to set this to desired value.\n");
  fprintf(file, "Use \"set image_W? value\" to set these to the desired values.\n");
}


