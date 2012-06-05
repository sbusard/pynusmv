/**CFile***********************************************************************

  FileName    [TracePlugin_private.c]

  PackageName [trace.plugins]

  Synopsis    [The private interface of class TracePlugin]

  Description [Private definition to be used by derived classes]

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
#ifndef __TRACE_PLUGIN_PRIVATE__H
#define __TRACE_PLUGIN_PRIVATE__H

#include "TracePlugin.h"
#include "../TraceOpt.h"

#include "utils/utils.h"
#include "utils/object.h"
#include "utils/object_private.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [TracePlugin Class]

  Description [This class defines a prototype for a generic
               TracePlugin. This class is virtual and must be
               specialized.]

  SeeAlso     []

******************************************************************************/
typedef struct TracePlugin_TAG
{
  INHERITS_FROM(Object);

  char* desc; /* Short description of the plugin */

  /* current trace */
  Trace_ptr trace;

  /* options from the caller */
  TraceOpt_ptr opt;

  /* used for filtering */
  hash_ptr visibility_map;

  /* used for obfuscation */
  hash_ptr obfuscation_map;

  /* ---------------------------------------------------------------------- */
  /*     Virtual Methods                                                    */
  /* ---------------------------------------------------------------------- */

  /* action */
  VIRTUAL int (*action)(const TracePlugin_ptr self);

  /* protected virtual methods for printing */
  VIRTUAL void (*print_symbol)(const TracePlugin_ptr self, node_ptr symbol);

  VIRTUAL void (*print_list)(const TracePlugin_ptr self, node_ptr list);

  VIRTUAL void (*print_assignment)(const TracePlugin_ptr self,
                                   node_ptr symbol, node_ptr val);
} TracePlugin;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/


/* protected methods */
EXTERN VIRTUAL void
TracePlugin_print_symbol ARGS((const TracePlugin_ptr self, node_ptr symb));

EXTERN VIRTUAL void
TracePlugin_print_list ARGS((const TracePlugin_ptr self, node_ptr list));

EXTERN VIRTUAL void
TracePlugin_print_assignment ARGS((const TracePlugin_ptr self,
                                   node_ptr symb, node_ptr val));


EXTERN boolean trace_plugin_is_visible_symbol ARGS((TracePlugin_ptr self,
                                                    node_ptr symb));

EXTERN void trace_plugin_print_symbol ARGS((const TracePlugin_ptr self,
                                            node_ptr symbol));

EXTERN void trace_plugin_print_list ARGS((const TracePlugin_ptr self,
                                          node_ptr list));

EXTERN void trace_plugin_print_assignment ARGS((const TracePlugin_ptr self,
                                                node_ptr symb, node_ptr val));

EXTERN void trace_plugin_init ARGS((TracePlugin_ptr self, char* desc));

EXTERN void trace_plugin_deinit ARGS((TracePlugin_ptr self));

EXTERN int trace_plugin_action ARGS((const TracePlugin_ptr self));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_PLUGIN_PRIVATE__H */
