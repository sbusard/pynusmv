/**CFile***********************************************************************

  FileName    [TraceLabel.c]

  PackageName [trace]

  Synopsis    [Routines related to functionality related to a node of a trace.]

  Description [This file contains the definition of the \"TraceLabel\" class.]

  SeeAlso     []

  Author      [Ashutosh Trivedi]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2.
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

#include "TraceLabel.h"
#include "node/node.h"
#include "parser/symbols.h"
#include "parser/parser.h"

#include "utils/portability.h" /* for errno */
#include <limits.h> /* for INT_MAX */

static char rcsid[] UTIL_UNUSED = "$Id: TraceLabel.c,v 1.1.2.4.4.2.4.2 2010-02-02 10:09:35 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [TraceLabel Constructor]

  Description [returns a label for the specified trace and state index.]

  SideEffects []

  SeeAlso     [TraceLabel_create_from_string]

******************************************************************************/
TraceLabel TraceLabel_create(int trace_id, int state_id)
{
  TraceLabel self;

  self = find_node(DOT, find_node(NUMBER, NODE_FROM_INT(trace_id), Nil),
                   find_node(NUMBER, NODE_FROM_INT(state_id), Nil));
  return self;
}

/**Function********************************************************************

  Synopsis    [TraceLabel Constructor]

  Description [creates the label from the specified string. In case of any
  error, it returns TRACE_LABEL_INVALID as result.

  The string 'str' should follow this format: ^\s*(\d+)\s*\.\s*(-?\d+)$ in which
  the first group matches the trace number and the second matches the state
  number. ]

  SideEffects []

  SeeAlso     [TraceLabel_create]

******************************************************************************/
TraceLabel TraceLabel_create_from_string(const char* str)
{
  char* startptr;
  char* endptr;
  long traceno, stateno;

  /* Start reading the string */
  startptr = (char*)str;

  /* First number must be a positive integer */
  errno = 0;
  traceno = strtol(str, &endptr, 10);

  if((startptr == endptr) || (errno == ERANGE) ||
     (errno == EINVAL) || (traceno < 0) || (traceno > INT_MAX)) {
    /*
      No chars read or error in reading or negative number or
      number too big
    */
    return TRACE_LABEL_INVALID;
  }
  startptr = endptr;

  /* We can have some spaces */
  while(' ' == *startptr) {
    startptr++;
  }

  /* Then we have a '.' char */
  if ('.' == *startptr) {
    startptr++;
  }
  else {
    /* We have something which is not a '.' */
    return TRACE_LABEL_INVALID;
  }

  /* And then the state number */
  errno = 0;
  stateno = strtol(startptr, &endptr, 10);

  if((startptr == endptr) || (errno == ERANGE) ||
     (errno == EINVAL) || (stateno > INT_MAX) || (stateno < INT_MIN)) {
    /* No chars read or error in reading or invalid int */
    return TRACE_LABEL_INVALID;
  }
  startptr = endptr;

  /* Skip final spaces */
  while(' ' == *startptr) {
    startptr++;
  }

  if ('\0' == *startptr) {
    return TraceLabel_create((int)(traceno - 1), (int)(stateno - 1));
  }
  else {
    /* Something unexpected at the end of the file */
    return TRACE_LABEL_INVALID;
  }
}

/**Function********************************************************************

  Synopsis    [Returns the trace index associated with the TraceLabel.]

  Description []

  SideEffects []

  SeeAlso     [TraceLabel_get_state]

******************************************************************************/
int TraceLabel_get_trace(TraceLabel self)
{
  int result = NODE_TO_INT(car(car((node_ptr)self)));

  return result;
}

/**Function********************************************************************

  Synopsis    [Returns the state index associated with the TraceLabel.]

  Description []

  SideEffects []

  SeeAlso     [TraceLabel_get_trace]

******************************************************************************/
int TraceLabel_get_state(TraceLabel self)
{
  int result = NODE_TO_INT(car(cdr((node_ptr)self)));

  return result;
}

