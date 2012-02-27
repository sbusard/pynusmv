/**CHeaderFile*****************************************************************

  FileName    [TraceManager.h]

  PackageName [trace]

  Synopsis    [The header file for the <tt>TraceManager</tt> class.]

  Description []

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
#ifndef __TRACE_MANAGER__H
#define __TRACE_MANAGER__H

#include "dd/dd.h"
#include "utils/utils.h"
#include "utils/array.h"
#include "utils/error.h"
#include "trace/plugins/TracePlugin.h"
#include "trace/exec/PartialTraceExecutor.h"
#include "trace/exec/CompleteTraceExecutor.h"
#include "trace/eval/BaseEvaluator.h"
#include "TraceLabel.h"
#include "TraceOpt.h"
#include "node/node.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define TRACE_MANAGER_DEFAULT_PLUGIN -1
#define TRACE_MANAGER_LAST_TRACE -1

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/
typedef struct TraceManager_TAG* TraceManager_ptr;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define TRACE_MANAGER(x) \
        ((TraceManager_ptr) x)

#define TRACE_MANAGER_CHECK_INSTANCE(x) \
        (nusmv_assert(TRACE_MANAGER(x) != TRACE_MANAGER(NULL)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototype                                                        */
/*---------------------------------------------------------------------------*/

/* TraceManager Constructor/Destructor */
EXTERN TraceManager_ptr TraceManager_create ARGS((void)) ;
EXTERN void TraceManager_destroy ARGS((TraceManager_ptr self));

/* TraceManager Getters */
EXTERN int TraceManager_get_size ARGS((const TraceManager_ptr self));
EXTERN int TraceManager_get_plugin_size ARGS((const TraceManager_ptr self));
EXTERN int
TraceManager_get_internal_plugin_size ARGS((const TraceManager_ptr self));

EXTERN Trace_ptr
TraceManager_get_trace_at_index ARGS((const TraceManager_ptr self,
                                      int index));
EXTERN TracePlugin_ptr
TraceManager_get_plugin_at_index ARGS((const TraceManager_ptr self,
                                       int index));

/* TraceManager register functions */
EXTERN int TraceManager_register_trace ARGS((TraceManager_ptr self,
                                             Trace_ptr trace));

EXTERN void TraceManager_init_plugins ARGS((TraceManager_ptr self));

EXTERN int TraceManager_register_plugin ARGS((TraceManager_ptr self,
                                              TracePlugin_ptr plugin));

EXTERN BaseEvaluator_ptr
TraceManager_create_evaluator ARGS((TraceManager_ptr self,
                                    const SymbTable_ptr st,
                                    const hash_ptr env));

/* Evaluators management */
EXTERN BaseEvaluator_ptr
TraceManager_get_evaluator ARGS((TraceManager_ptr self));

EXTERN void
TraceManager_register_evaluator ARGS((TraceManager_ptr self,
                                      BaseEvaluator_ptr eval));

EXTERN void TraceManager_unregister_evaluator ARGS((TraceManager_ptr self));

/* Complete trace executors management functions */
EXTERN void
TraceManager_register_complete_trace_executor ARGS((TraceManager_ptr self,
             const char* executor_name, const char* executor_desc,
             const CompleteTraceExecutor_ptr executor));

EXTERN CompleteTraceExecutor_ptr
TraceManager_get_complete_trace_executor ARGS((const TraceManager_ptr self,
                                               const char* name));

EXTERN array_t*
TraceManager_get_complete_trace_executor_ids ARGS((const TraceManager_ptr self));

EXTERN const char*
TraceManager_get_complete_trace_executor_desc ARGS((const TraceManager_ptr self,
                                                    const char* name));

EXTERN CompleteTraceExecutor_ptr
TraceManager_get_default_complete_trace_executor
ARGS((const TraceManager_ptr global_trace_manager));

/* Partial trace executors management functions */
EXTERN void
TraceManager_register_partial_trace_executor ARGS((TraceManager_ptr self,
             const char* executor_name, const char* executor_desc,
             const PartialTraceExecutor_ptr executor));

EXTERN array_t*
TraceManager_get_partial_trace_executor_ids ARGS((const TraceManager_ptr self));

EXTERN PartialTraceExecutor_ptr
TraceManager_get_partial_trace_executor ARGS((const TraceManager_ptr self,
                                              const char* name));

EXTERN const char*
TraceManager_get_partial_trace_executor_desc ARGS((const TraceManager_ptr self,
                                                   const char* name));

EXTERN PartialTraceExecutor_ptr
TraceManager_get_default_partial_trace_executor
ARGS((const TraceManager_ptr global_trace_manager));

/* Registration of layers */
EXTERN void
TraceManager_register_layer ARGS((TraceManager_ptr self,
                                  const char* layer_name));
EXTERN void
TraceManager_unregister_layer ARGS((TraceManager_ptr self,
                                    const char* layer_name));
EXTERN boolean
TraceManager_is_layer_registered ARGS((const TraceManager_ptr self,
                                       const char* layer_name));
EXTERN const array_t*
TraceManager_get_registered_layers ARGS((const TraceManager_ptr self));


/* Other Functions */
EXTERN int TraceManager_execute_plugin ARGS((const TraceManager_ptr self,
                                             const TraceOpt_ptr opt,
                                             int plugin_index,
                                             int trace_index));

EXTERN void
TraceManager_set_current_trace_number ARGS((TraceManager_ptr self,
                                            int trace_id));
EXTERN int
TraceManager_get_current_trace_number ARGS((TraceManager_ptr self));

EXTERN void
TraceManager_set_default_plugin ARGS((TraceManager_ptr self,
                                      int plugin_id));
EXTERN int
TraceManager_get_default_plugin ARGS((TraceManager_ptr self));

EXTERN boolean
TraceManager_is_visible_symbol ARGS((TraceManager_ptr self, node_ptr symbol));

/* Functions related to Labels */
EXTERN boolean
TraceManager_is_label_valid ARGS((TraceManager_ptr self, TraceLabel label));

EXTERN boolean
TraceManager_is_plugin_internal ARGS((const TraceManager_ptr self, int index));

EXTERN TraceIter
TraceManager_get_iterator_from_label ARGS((TraceManager_ptr self,
                                           TraceLabel label));

EXTERN int
TraceManager_get_abs_index_from_label ARGS((TraceManager_ptr self,
                                            TraceLabel label));

/**AutomaticEnd***************************************************************/


#endif /* __TRACE_MANAGER__H */
