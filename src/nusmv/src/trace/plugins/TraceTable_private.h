/**CHeaderFile*****************************************************************

  FileName    [TraceTable_private.h]

  PackageName [trace.plugins]

  Synopsis    [The private header file for the TraceTable class.]

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
#ifndef __TRACE_TABLE_PRIVATE__H
#define __TRACE_TABLE_PRIVATE__H

#include "TracePlugin_private.h"
#include "TraceTable.h"

#include "compile/symb_table/SymbTable.h"
#include "enc/bdd/BddEnc.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [TraceTable Class]

  Description [ This class contains information to explain a trace:<br>
        <dl>
            <dt><code>state_vars_list </code>
                <dd> list of state variables.
            <dt><code>input_vars_list</code>
                <dd>  list of input variables.
            <dt><code> style </code>
                <dd> Printing style of the table. Available choices are:<br>
                      a) TRACE_TABLE_TYPE_ROW  <br>
                      b)TRACE_TABLE_TYPE_COLUMN <br>
  </dl>
  <br>
  This Class inherits from TracePlugin class.
  ]

  SeeAlso     []

******************************************************************************/
typedef struct TraceTable_TAG
{
  INHERITS_FROM(TracePlugin);

  TraceTableStyle style;
} TraceTable;

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

void trace_table_init ARGS((TraceTable_ptr self, TraceTableStyle style));

void trace_table_deinit ARGS((TraceTable_ptr self));

int trace_table_action ARGS((const TracePlugin_ptr plugin));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_TABLE_PRIVATE__H */

