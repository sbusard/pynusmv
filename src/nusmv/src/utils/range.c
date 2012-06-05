/**CFile***********************************************************************

  FileName    [range.c]

  PackageName [utils]

  Synopsis    [Contains function for checking of ranges and subranges]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 2004 by FBK-irst. 

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

#include "range.h"

#include "parser/symbols.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: range.c,v 1.1.2.2.4.7.6.2 2009-05-15 14:39:31 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

  Synopsis    [Used by Utils_range_check]

  Description [Used by Utils_range_check callback]

  SeeAlso     [Utils_range_check, Utils_set_data_for_range_check]

******************************************************************************/
static node_ptr the_range = (node_ptr) NULL;


/**Variable********************************************************************

  Synopsis    [Used by Utils_range_check]

  Description [Used by Utils_range_check callback]

  SeeAlso     [Utils_range_check, Utils_set_data_for_range_check]

******************************************************************************/
static boolean  range_err = true;

/**Variable********************************************************************

  Synopsis    [Used by Utils_range_check]

  Description [Used by Utils_range_check callback]

  SeeAlso     [Utils_range_check, Utils_set_data_for_range_check]

******************************************************************************/
static node_ptr the_var = (node_ptr) NULL;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Called before using Utils_range_check callback function]

  Description        []

  SideEffects        [Utils_range_check]

******************************************************************************/
void Utils_set_data_for_range_check(node_ptr var, node_ptr range)
{
  the_var = var;
  the_range = range;
}


/**Function********************************************************************

  Synopsis           [Called before using Utils_range_check callback function]

  Description        []

  SideEffects        [Utils_range_check]

******************************************************************************/
void Utils_set_mode_for_range_check(boolean is_fatal)
{
  range_err = is_fatal;
}

/**Function********************************************************************

  Synopsis           [Checks if the values of <code>n</code> is in the
  range allowed for the variable.]

  Description        [Checks if the values of <code>n</code> is in the
  range allowed for the variable. The allowed values are stored in the
  global variable <code>the_range</code>, which should be set before
  invocation of this function. 
  An error occure if: 
   1. the value is not in the range (all FAILURE node are, of course, irgnored)
  ]
   
  SideEffects        [Utils_set_data_for_range_check]

******************************************************************************/
void Utils_range_check(node_ptr n)
{
  if (n == Nil) { internal_error("Utils_range_check: n == Nil"); }

  if (node_get_type(n) == CONS) {
    while (n != (node_ptr) NULL) {
      /* ignore FAILURE nodes */
      if (node_get_type(car(n)) != FAILURE && !in_list(car(n), the_range)) {
        if (range_err) { range_error(car(n), the_var); }
        else { range_warning(car(n), the_var); }
      }
      n = cdr(n);
    }
  }
  else {
    /* ignore FAILURE nodes */
    if (node_get_type(n) != FAILURE && !in_list(n, the_range)) {
      if (range_err) { range_error(n, the_var); }
      else { range_warning(n, the_var); }
    }
  }
}


/**Function********************************************************************

  Synopsis           [Checks if the values of <code>n</code> does not 
  contains FAILURE node. If they do then report and terminate.]

  Description        []

  SideEffects        [Utils_set_data_for_range_check]

******************************************************************************/
void Utils_failure_node_check(node_ptr n)
{
  if (node_get_type(n) == FAILURE) report_failure_node(n);
}


/**Function********************************************************************

  Synopsis           [Checks if the first argument is contained in the second.]

  Description        [Returns true if the first argument is contained in the
  set represented by the second, false otherwise. If the first
  argument is not a CONS, then it is considered to be a singleton.]

  SideEffects        [None]

  SeeAlso            [in_list]

******************************************************************************/
boolean Utils_is_in_range(node_ptr s, node_ptr d) 
{
  if (d == Nil) return false;

  if (node_get_type(s) == CONS) {
    while (s != Nil) {
      nusmv_assert(CONS == node_get_type(s));
      if (in_list(car(s), d) == 0) return false;
      s = cdr(s);
    }
    return true;
  }

  /* s is a singleton */
  return (in_list(s, d) == 1);
}

 
/**Function********************************************************************

  Synopsis           [Checks that in given subrange n..m, n<=m]

  Description        [Returns True if in given subrange n..m n <= m. 
  Given node_ptr must be of TWODOTS type]

  SideEffects        []

  SeeAlso            [Utils_check_subrange_not_negative]

******************************************************************************/
boolean Utils_check_subrange(node_ptr subrange)
{
  int inf, sup;
  nusmv_assert(node_get_type(subrange) == TWODOTS);

  inf = node_get_int(car(subrange));
  sup = node_get_int(cdr(subrange));

  return inf <= sup;		     
}


/**Function********************************************************************

  Synopsis           [Checks that in given subrange n..m, n<=m, and that n,m 
  are not negative]

  Description        [Check for correct positive (or zero) range]

  SideEffects        []

  SeeAlso            [Utils_check_subrange]

******************************************************************************/
boolean Utils_check_subrange_not_negative(node_ptr subrange)
{
  int inf;
  nusmv_assert(node_get_type(subrange) == TWODOTS);

  inf = node_get_int(car(subrange));
  return (inf >= 0) && Utils_check_subrange(subrange);
}

