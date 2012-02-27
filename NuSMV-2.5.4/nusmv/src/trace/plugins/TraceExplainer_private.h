/**CHeaderFile*****************************************************************

  FileName    [TraceExplainer_private.h]

  PackageName [trace.plugins]

  Synopsis    [The private header file for the TraceExplainer class.]

  Description []

  SeeAlso     []

  Author      [Ashutosh Trivedi]

  Copyright   [
  This file is part of the ``trace.plugins'' package of NuSMV version 2.
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
#ifndef __TRACE_EXPLAINER_PRIVATE__H
#define __TRACE_EXPLAINER_PRIVATE__H

#include "TracePlugin_private.h"
#include "TraceExplainer.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [TraceExplainer Class]

  Description [ This class contains information to explain a trace:<br>
        <dl>
            <dt><code>changes_only</code>
                <dd> If this field is true, then explainer will print only
                those variables whose value changes.
  </dl>
  <br>
  This Class inherits from TracePlugin class.
  ]

  SeeAlso     []

******************************************************************************/
typedef struct TraceExplainer_TAG
{
  INHERITS_FROM(TracePlugin);

  boolean changes_only;
} TraceExplainer;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
void trace_explainer_init ARGS((TraceExplainer_ptr self,
                                boolean changes_only));

void trace_explainer_deinit ARGS((TraceExplainer_ptr self));

int trace_explainer_action ARGS((const TracePlugin_ptr plugin));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_EXPLAIN_PRIVATE__H */

