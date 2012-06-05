/**CFile***********************************************************************

  FileName    [TraceManager.c]

  PackageName [trace]

  Synopsis    [Routines related to TraceManager's functionality.]

  Description [Primitives to create, query and manipulate TraceManager are
  provided.]

  SeeAlso     []

  Author      [Ashutosh Trivedi, Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2.
  Copyright (C) 2003 by FBK-irst.

  NuSMV version 2 is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be
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
#if NUSMV_HAVE_CONFIG
# include "nusmv-config.h"
#endif
#include "pkg_trace.h"
#include "pkg_traceInt.h"

#include "utils/ustring.h"
#include "Trace.h"
#include "TraceLabel.h"
#include "TraceManager.h"
#include "Trace_private.h" /* To access special methods of Trace class */

/* plugins */
#include "trace/plugins/TraceExplainer.h"
#include "trace/plugins/TraceTable.h"
#include "trace/plugins/TraceCompact.h"
#include "trace/plugins/TraceXmlDumper.h"

/* executors */
#include "trace/exec/BaseTraceExecutor.h"
#include "trace/exec/CompleteTraceExecutor.h"
#include "trace/exec/PartialTraceExecutor.h"

/* evaluators */
#include "trace/eval/BaseEvaluator.h"

#include "compile/compile.h"
#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: TraceManager.c,v 1.1.2.29.4.4.4.16 2010-02-17 14:50:14 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static enum st_retval
trace_manager_destroy_executor_entry ARGS((char *key, char *data, char * arg));

static int cmp_string_ptr ARGS((string_ptr* a, string_ptr* b));

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [TraceManager Class]

  Description [ This class contains informations about TraceManager:<br>
        <dl>
        <dt><code>trace_list</code>
            <dd>  List of Traces registered with TraceManager.
        <dt><code>plugin_list</code>
            <dd>  List of plugins registered with TraceManager.
        <dt><code>layer_names</code>
            <dd>  List of symb layers registered with TraceManager.
        <dt><code>complete_trace_executors</code>
            <dd>  Dictionary str->object of complete trace executors
            registered with TraceManager.
        <dt><code>partial_trace_executors</code>
            <dd>  Dictionary str->object of partial trace executors
            registered with TraceManager.
        <dt><code>evaluator</code>
            <dd>  Currently registered evaluator.
        <dt><code>default_opt</code>
            <dd>  Internal TraceOpt object.
        <dt><code>current_trace_number</code>
            <dd>  Index of the current trace.
        <dt><code>default_plugin</code>
            <dd>  default plugin to print traces.
        <dt><code>internal_plugins_num</code>
            <dd> The number of plugins registered within NuSMV. All
                 the possibly existing other external plugins will be
                 assigned to indices greater or equal to this value.
        </dl>
        <br>
        ]

  SeeAlso     []

******************************************************************************/
typedef struct TraceManager_TAG
{
  array_t* trace_list;
  array_t* plugin_list;
  array_t* layer_names;

  hash_ptr complete_trace_executors;
  hash_ptr partial_trace_executors;
  BaseEvaluator_ptr evaluator;
  TraceOpt_ptr default_opt;

  int current_trace_number;
  int default_plugin;

  int internal_plugins_num;
} TraceManager;


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Initializes the TraceManager.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceManager_ptr TraceManager_create(void)
{
  TraceManager_ptr self = ALLOC(TraceManager, 1);

  TRACE_MANAGER_CHECK_INSTANCE(self);

  self->trace_list = array_alloc(Trace_ptr, 1);
  nusmv_assert(self->trace_list != (array_t *) ARRAY_OUT_OF_MEM);

  self->plugin_list = array_alloc(TracePlugin_ptr, 1);
  nusmv_assert(self->plugin_list != (array_t *) ARRAY_OUT_OF_MEM);

  self->layer_names = array_alloc(const char*, 1);
  nusmv_assert(self->layer_names != (array_t *) ARRAY_OUT_OF_MEM);

  self->complete_trace_executors = new_assoc();
  nusmv_assert((hash_ptr)(NULL) != self->complete_trace_executors);

  self->partial_trace_executors = new_assoc();
  nusmv_assert((hash_ptr)(NULL) != self->partial_trace_executors);

  self->current_trace_number = -1 ;  /* Not yet assigned */
  self->evaluator = BASE_EVALUATOR(NULL);

  /* Default plugin */
  self->default_plugin = get_default_trace_plugin(OptsHandler_get_instance());

  self->internal_plugins_num = 0; /* number of plugins within NuSMV */
  self->default_opt = TraceOpt_create_from_env(OptsHandler_get_instance());

  return self;
}

/**Function********************************************************************

  Synopsis    [Destroys the TraceManager with all the registered traces and
  plugins]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void TraceManager_destroy(TraceManager_ptr self)
{
  int i, num;

  TRACE_MANAGER_CHECK_INSTANCE(self);

  num = TraceManager_get_size(self);
  for (i = 0; i < num; ++i) {
    Trace_ptr trace = TraceManager_get_trace_at_index(self, i);
    Trace_unregister(trace);
    Trace_destroy(trace);
  }
  array_free(self->trace_list);

  num = array_n(self->plugin_list);
  for (i = 0; i < num; ++i) {
    Object_destroy(OBJECT(TraceManager_get_plugin_at_index(self, i)), NULL);
  }
  array_free(self->plugin_list);

  num = array_n(self->layer_names);
  for (i = 0; i < num; ++i) {
    const char* name = array_fetch(const char*, self->layer_names, i);
    if (name != (const char*) NULL) { FREE(name); }
  }
  array_free(self->layer_names);

  /* destroying registered executors */
  clear_assoc_and_free_entries(self->complete_trace_executors,
                               trace_manager_destroy_executor_entry);
  clear_assoc_and_free_entries(self->partial_trace_executors,
                               trace_manager_destroy_executor_entry);
  free_assoc(self->complete_trace_executors);
  free_assoc(self->partial_trace_executors);

  /* destroying registered evaluator (if any) */
  TraceManager_unregister_evaluator(self);

  /* free default trace opt instance */
  TraceOpt_destroy(self->default_opt);

  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Returns the size of the TraceManager.]

  Description [This function returns the number of traces registered with
  traceManager]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int TraceManager_get_size(const TraceManager_ptr self)
{
  TRACE_MANAGER_CHECK_INSTANCE(self);

  return array_n(self->trace_list);
}

/**Function********************************************************************

  Synopsis [Returns the total number of plugins registered with
  TraceManager.]

  Description [This function returns the total number of plugins
  registered with traceManager]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int TraceManager_get_plugin_size(const TraceManager_ptr self)
{
  TRACE_MANAGER_CHECK_INSTANCE(self);

  return array_n(self->plugin_list);
}

/**Function********************************************************************

  Synopsis [Returns the number of internal plugins registered with
  TraceManager.]

  Description [This function returns the number of internal plugins
  registered with traceManager]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int TraceManager_get_internal_plugin_size(const TraceManager_ptr self)
{
  TRACE_MANAGER_CHECK_INSTANCE(self);

  return self->internal_plugins_num;
}

/**Function********************************************************************

  Synopsis    [Returns the trace stored at given index]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
Trace_ptr TraceManager_get_trace_at_index(const TraceManager_ptr self,
                                          int index)
{
  Trace_ptr trace;

  TRACE_MANAGER_CHECK_INSTANCE(self);

  nusmv_assert(index < array_n(self->trace_list));
  nusmv_assert(index >= 0);

  trace = array_fetch(Trace_ptr, self->trace_list, index);

  return trace;
}

/**Function********************************************************************

  Synopsis    [Returns the plugin stored at given index]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
TracePlugin_ptr
TraceManager_get_plugin_at_index(const TraceManager_ptr self, int index)
{
  TracePlugin_ptr plugin;

  TRACE_MANAGER_CHECK_INSTANCE(self);
  nusmv_assert(index >= 0);
  nusmv_assert(index < array_n(self->plugin_list) );

  plugin = array_fetch(TracePlugin_ptr, self->plugin_list, index);

  return plugin;
}

/**Function********************************************************************

  Synopsis    [Registers a trace with TraceManager.]

  Description [It registers a trace with the TraceManager and returns
  the corresponding index. The given trace can not be previously
  registered with any Trace Manager.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int TraceManager_register_trace(TraceManager_ptr self, Trace_ptr trace)
{
  int index;
  boolean status;

  TRACE_MANAGER_CHECK_INSTANCE(self);

  nusmv_assert(!Trace_is_registered(trace));

  status = array_insert_last(Trace_ptr, self->trace_list, trace);
  nusmv_assert(status != ARRAY_OUT_OF_MEM);

  index = array_n(self->trace_list) - 1;
  Trace_register(trace, index+1);  /* Friend function of Trace Class */

  return index;
}

/**Function********************************************************************

  Synopsis    [Registers a plugin with TraceManager.]

  Description [It registers a plugin with the TraceManager and returns the
  corresponding index.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int TraceManager_register_plugin(TraceManager_ptr self, TracePlugin_ptr plugin)
{
  int res;
  boolean status;

  TRACE_MANAGER_CHECK_INSTANCE(self);

  status = array_insert_last(TracePlugin_ptr, self->plugin_list, plugin);
  nusmv_assert(status != ARRAY_OUT_OF_MEM);

  res = array_n(self->plugin_list);

  /* returns the index of the plugin for further use as handle */
  return (res - 1);
}


/**Function********************************************************************

  Synopsis    [Retrieves currently registered evaluator]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
BaseEvaluator_ptr TraceManager_get_evaluator(TraceManager_ptr self)

{
  TRACE_MANAGER_CHECK_INSTANCE(self);
  return self->evaluator;
}


/**Function********************************************************************

  Synopsis    [Registers an evaluator]

  Description [Registers an evaluator. If some evaluator was already
  registered it is destroyed]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void TraceManager_register_evaluator(TraceManager_ptr self,
                                     BaseEvaluator_ptr eval)
{
  TRACE_MANAGER_CHECK_INSTANCE(self);
  if (BASE_EVALUATOR(NULL) != self->evaluator) {
    BaseEvaluator_destroy(self->evaluator);
    self->evaluator = BASE_EVALUATOR(NULL);
  }

  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
    fprintf(nusmv_stderr, "Registering evaluator\n");
  }

  self->evaluator = eval;
}


/**Function********************************************************************

  Synopsis    [Unregisters current evaluator]

  Description [Unregisters currently registered evaluator. If some
  evaluator was already registered it is destroyed. If no evaluator
  was registered, no action is performed]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void TraceManager_unregister_evaluator(TraceManager_ptr self)

{
  TRACE_MANAGER_CHECK_INSTANCE(self);
  if (BASE_EVALUATOR(NULL) != self->evaluator) {
    BaseEvaluator_destroy(self->evaluator);
    self->evaluator = BASE_EVALUATOR(NULL);
  }
}


/**Function********************************************************************

  Synopsis    [Registers a complete trace executor plugin with TraceManager.]

  Description [It registers a complete trace executor with the
  TraceManager and returns the corresponding index]

  SideEffects [A previously registered executor (if any existing) is destroyed]

  SeeAlso     [TraceManager_register_partial_trace_executor]

******************************************************************************/
void TraceManager_register_complete_trace_executor(TraceManager_ptr self,
                  const char* executor_name, const char* executor_desc,
                  const CompleteTraceExecutor_ptr executor)
{
  node_ptr entry;
  CompleteTraceExecutor_ptr previous = COMPLETE_TRACE_EXECUTOR(NULL);
  string_ptr executor_id;

  /* get rid of const warnings */
  char* _executor_name = (char*)(executor_name);
  char* _executor_desc = (char*)(executor_desc);

  TRACE_MANAGER_CHECK_INSTANCE(self);
  COMPLETE_TRACE_EXECUTOR_CHECK_INSTANCE(executor);
  nusmv_assert(NIL(char) != _executor_name);

  /* retrieve previously registered executor instance */
  executor_id = find_string(_executor_name);
  entry = find_assoc(self->complete_trace_executors, NODE_PTR(executor_id));

  /* if a previous executor was registered, it has to be replaced */
  if (Nil != entry) {
    if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
      fprintf(nusmv_stderr, "Replacing complete trace executor '%s'\n",
              get_text(executor_id));
    }

    /* destroying previously registered executor instance */
    previous = COMPLETE_TRACE_EXECUTOR(cdr(entry));
    BaseTraceExecutor_destroy(BASE_TRACE_EXECUTOR(previous));

    /* replacing description */
    setcar(entry, NODE_PTR(find_string(_executor_desc)));

    /* replacing instance */
    setcdr(entry, NODE_PTR(executor));
  }

  else {
    if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
      fprintf(nusmv_stderr, "Registering complete trace executor '%s'\n",
              get_text(executor_id));
    }

    insert_assoc(self->complete_trace_executors,
                 NODE_PTR(executor_id),
                 cons(NODE_PTR(find_string(_executor_desc)),
                      NODE_PTR(executor)));
  }
}


/**Function********************************************************************

  Synopsis [Returns an array of registered complete trace executor
  IDs. IDs are alphabetically sorted using lexicographical ordering]

  Description [Returned array must be destroyed by the caller]

  SideEffects []

  SeeAlso     [TraceManager_register_complete_trace_executor]

******************************************************************************/
array_t*
TraceManager_get_complete_trace_executor_ids (const TraceManager_ptr self)
{
  array_t* res = array_alloc(string_ptr, 1);
  node_ptr e;
  assoc_iter iter;

  TRACE_MANAGER_CHECK_INSTANCE(self);

  ASSOC_FOREACH(self->complete_trace_executors, iter, &e, NULL) {
    array_insert_last(string_ptr, res, (string_ptr)(e));
  }

  /* sort entries according to lexicographic order */
  array_sort(res, cmp_string_ptr);

  return res;
}


/**Function********************************************************************

  Synopsis    [Returns default registered complete trace executor]

  Description [Returns default registered complete trace executor, if
  any. If no executor has yet been registered NULL is returned.]

  SideEffects [none]

  SeeAlso     [TraceManager_register_complete_trace_executor]

******************************************************************************/
CompleteTraceExecutor_ptr
TraceManager_get_default_complete_trace_executor (const TraceManager_ptr self)
{
  string_ptr executor_id;
  node_ptr entry;
  array_t* tmp = \
    TraceManager_get_complete_trace_executor_ids(global_trace_manager);

  if (0 == array_n(tmp)) { return COMPLETE_TRACE_EXECUTOR(NULL); }

  executor_id = array_fetch(string_ptr, tmp, 0);
  entry = find_assoc(self->complete_trace_executors, NODE_PTR(executor_id));
  array_free(tmp);

  return COMPLETE_TRACE_EXECUTOR(cdr(entry));
}


/**Function********************************************************************

  Synopsis    [Returns default registered partial trace executor]

  Description [Returns default registered partial trace executor, if
  any. If no executor has yet been registered NULL is returned.]

  SideEffects [none]

  SeeAlso     [TraceManager_register_partial_trace_executor]

******************************************************************************/
PartialTraceExecutor_ptr
TraceManager_get_default_partial_trace_executor (const TraceManager_ptr self)
{
  string_ptr executor_id;
  node_ptr entry;
  array_t* tmp = \
    TraceManager_get_partial_trace_executor_ids(global_trace_manager);

  if (0 == array_n(tmp)) { return PARTIAL_TRACE_EXECUTOR(NULL); }

  executor_id = array_fetch(string_ptr, tmp, 0);
  entry = find_assoc(self->partial_trace_executors, NODE_PTR(executor_id));
  array_free(tmp);

  return PARTIAL_TRACE_EXECUTOR(cdr(entry));
}


/**Function********************************************************************

  Synopsis    [Registers a partial trace executor plugin with TraceManager.]

  Description [It registers a partial trace executor with the
  TraceManager and returns the corresponding index.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void TraceManager_register_partial_trace_executor(TraceManager_ptr self,
                  const char* executor_name, const char* executor_desc,
                  const PartialTraceExecutor_ptr executor)
{
  node_ptr entry;
  PartialTraceExecutor_ptr previous = PARTIAL_TRACE_EXECUTOR(NULL);
  string_ptr executor_id;

  /* get rid of const warnings */
  char* _executor_name = (char*)(executor_name);
  char* _executor_desc = (char*)(executor_desc);

  TRACE_MANAGER_CHECK_INSTANCE(self);
  PARTIAL_TRACE_EXECUTOR_CHECK_INSTANCE(executor);
  nusmv_assert(NIL(char) != _executor_name);

  executor_id = find_string(_executor_name);
  entry = find_assoc(self->partial_trace_executors, NODE_PTR(executor_id));

  /* if a previous executor was registered, it has to be replaced */
  if (Nil != entry) {
    if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
      fprintf(nusmv_stderr, "Replacing partial trace executor '%s'\n",
              get_text(executor_id));
    }

    previous = PARTIAL_TRACE_EXECUTOR(cdr(entry));
    BaseTraceExecutor_destroy(BASE_TRACE_EXECUTOR(previous));

    /* replacing description */
    setcar(entry, NODE_PTR(find_string(_executor_desc)));

    /* replacing instance */
    setcdr(entry, NODE_PTR(executor));
  }

  else {
    if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
      fprintf(nusmv_stderr, "Registering partial trace executor '%s'\n",
              get_text(executor_id));
    }

    insert_assoc(self->partial_trace_executors,
                 NODE_PTR(executor_id),
                 cons(NODE_PTR(find_string(_executor_desc)),
                      NODE_PTR(executor)));
  }
}


/**Function********************************************************************

  Synopsis [Returns an array of registered partial trace executor
  IDs. IDs are alphabetically sorted using lexicographical ordering]

  Description [Returned array must be destroyed by the caller]

  SideEffects []

  SeeAlso     [TraceManager_register_complete_trace_executor]

******************************************************************************/
array_t*
TraceManager_get_partial_trace_executor_ids (const TraceManager_ptr self)
{
  array_t* res = array_alloc(string_ptr, 1);
  node_ptr e;
  assoc_iter iter;
  TRACE_MANAGER_CHECK_INSTANCE(self);

  ASSOC_FOREACH(self->partial_trace_executors, iter, &e, NULL) {
    array_insert_last(string_ptr, res, (string_ptr)(e));
  }

  /* sort entries according to lexicographic order */
  array_sort(res, cmp_string_ptr);

  return res;
}


/**Function********************************************************************

  Synopsis [Registers a new layer name to be used later by the
  explainers when printing symbols. Only the symbols into registered
  layers will be shown.]

  Description [Use this method to control which symbols will be shown
  when a trace is shown. Only symbols occurring inside registered
  layers will be presented by plugins. Warning: before renaming or
  deleting a previoulsy registered layer, the layer should be
  unregistered. If not unregistered, the behaviour is unpredictable.]

  SideEffects []

  SeeAlso     [unregister_layer]

******************************************************************************/
void TraceManager_register_layer(TraceManager_ptr self, const char* layer_name)
{
  const char* name;
  int idx;
  boolean found = false;

  TRACE_MANAGER_CHECK_INSTANCE(self);

  /* does not registers if already registered */
  if (TraceManager_is_layer_registered(self, layer_name)) return;

  /* first search for a hole */
  arrayForEachItem(const char*, self->layer_names, idx, name) {
    if (name == (const char*) NULL) {
      array_insert(const char*, self->layer_names, idx,
                   util_strsav((char*) layer_name));
      found = true;
      break;
    }
  }

  if (!found) {
    /* if not inserted in a hole, push at the end */
    array_insert_last(const char*, self->layer_names,
                      util_strsav((char*) layer_name));
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "TraceManager: registered layer '%s'\n", layer_name);
  }

}


/**Function********************************************************************

  Synopsis    [Unregisters a previoulsy registered layer]

  Description [The given layer must be registered before calling this method,
  otherwise an internal error occurs]

  SideEffects []

  SeeAlso     [register_layer]

******************************************************************************/
void TraceManager_unregister_layer(TraceManager_ptr self, const char* layer_name)
{
  int idx;
  const char* name;

  TRACE_MANAGER_CHECK_INSTANCE(self);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "TraceManager: unregistering layer '%s'...\n",
            layer_name);
  }

  arrayForEachItem(const char*, self->layer_names, idx, name) {
    if ((name != (const char*) NULL) && strcmp(name, layer_name) == 0) {
      FREE(name);
      array_insert(const char*, self->layer_names, idx, NULL);
      return;
    }
  }

  internal_error("TraceManager_unregister_layer: "\
                 "given layer had not been registered\n");
}


/**Function********************************************************************

  Synopsis    [Returns true if the given layer names was previously
  registered]

  Description []

  SideEffects []

  SeeAlso     [unregister_layer]

******************************************************************************/
boolean TraceManager_is_layer_registered(const TraceManager_ptr self,
                                         const char* layer_name)
{
  int idx;
  const char* name;

  TRACE_MANAGER_CHECK_INSTANCE(self);

  arrayForEachItem(const char*, self->layer_names, idx, name) {
    if ((name != (const char*) NULL) &&
        strcmp(name, layer_name) == 0) return true;
  }

  return false;
}


/**Function********************************************************************

  Synopsis    [Returns an array of names (strings) of the registered layers]

  Description [Returned array belongs to self, do not change or delete it]

  SideEffects []

  SeeAlso     []

******************************************************************************/
const array_t* TraceManager_get_registered_layers(const TraceManager_ptr self)
{
  TRACE_MANAGER_CHECK_INSTANCE(self);
  return self->layer_names;
}


/**Function********************************************************************

  Synopsis    [Executes the given trace plugin on given trace]

  Description [\"opt\" is either a valid TraceOpt instance or NULL. Defaults
               are provided by the trace manager in the latter case.

               plugin_index is either a non-negative integer, to which
               must correspond a registered plugin, or a negative
               integer. Default plugin is used in the latter case.

               trace_index is either a non_negative integerm to which
               must correspond a valid registered trace, or a negative
               integer. Last registered trace is used in the latter case.]

  SideEffects [none]

  SeeAlso     [TRACE_MANAGER_DEFAULT_PLUGIN, TRACE_MANAGER_LAST_TRACE]

******************************************************************************/
int TraceManager_execute_plugin(const TraceManager_ptr self,
                                const TraceOpt_ptr opt,
                                int plugin_index,
                                int trace_index)
{
  Trace_ptr trace;
  TracePlugin_ptr plugin;
  int res;

  TRACE_MANAGER_CHECK_INSTANCE(self);

  trace = \
    TraceManager_get_trace_at_index(self, (0 <= trace_index)
                                    ? trace_index
                                    : (array_n(self->trace_list) - 1));

  plugin = \
    TraceManager_get_plugin_at_index(self, (0 <= plugin_index)
                                     ? plugin_index
                                     : TraceManager_get_default_plugin(self));

  if (TraceManager_get_plugin_size(self) < plugin_index) {
    fprintf(nusmv_stderr,
            "Warning: Requested plugin %d is not available.\n", plugin_index);
    return 1;
  }

  /* if no options were provided, update internal defaults from env */
  if (TRACE_OPT(NULL) == opt) {
    TraceOpt_update_from_env(self->default_opt, OptsHandler_get_instance());
  }

  res = TracePlugin_action(plugin, trace, TRACE_OPT(NULL) != opt
                           ? opt : self->default_opt);

  return res;
}

/**Function********************************************************************

  Synopsis    [Sets trace_id as ths current trace of the TraceManager. ]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void TraceManager_set_current_trace_number(TraceManager_ptr self, int trace_id)
{
  self->current_trace_number = trace_id;
}

/**Function********************************************************************

  Synopsis    [Returns the trace_id of the current trace of the TraceManager.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int TraceManager_get_current_trace_number(TraceManager_ptr self)
{
  return self->current_trace_number;
}

/**Function********************************************************************

  Synopsis    [Sets plugin_id as ths default_plugin of the TraceManager. ]

  Description [ Default plugin is the plugin to be used to print a trace by
  default.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void TraceManager_set_default_plugin(TraceManager_ptr self, int plugin_id)
{
  self->default_plugin = plugin_id;
}

/**Function********************************************************************

  Synopsis    [Returns the default plugin of the TraceManager.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int TraceManager_get_default_plugin(TraceManager_ptr self)
{
  return self->default_plugin;
}


/**Function********************************************************************

  Synopsis    [Returns true if the plugin whose index is provided is
  internal to NuSMV. It returns false if the given plugin has been
  externally registered.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean TraceManager_is_plugin_internal(const TraceManager_ptr self, int index)
{
  return (index >= 0 && index < self->internal_plugins_num);
}


/**Function********************************************************************

  Synopsis    [Returns a trace iterator pointing to the particular trace step
               indicated by the given label.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceIter TraceManager_get_iterator_from_label(TraceManager_ptr self,
                                               TraceLabel label)
{
  Trace_ptr trace = TRACE(NULL);
  TraceIter res = TRACE_END_ITER;
  int state_no, i;

  TRACE_MANAGER_CHECK_INSTANCE(self);

  state_no = TraceManager_get_abs_index_from_label(self, label);
  trace = TraceManager_get_trace_at_index(self,
                                          TraceLabel_get_trace(label));
  res = Trace_first_iter(trace);
  for (i = 0; i < state_no; ++ i) {
    res = TraceIter_get_next(res);
    if (TRACE_END_ITER == res) {
      internal_error("%s:%d:%s: invalid trace number (%d)",
                     __FILE__, __LINE__, __func__, state_no);
    }
  }

  return res;
}


/**Function********************************************************************

  Synopsis    [Returns the absolute state index pointed by the label.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int TraceManager_get_abs_index_from_label(TraceManager_ptr self,
                                          TraceLabel label)
{
  Trace_ptr trace;
  int trace_no = TraceLabel_get_trace(label);
  int state_no = TraceLabel_get_state(label);

  nusmv_assert(trace_no >= 0 && trace_no < TraceManager_get_size(self));

  trace = TraceManager_get_trace_at_index(self, trace_no);

  if (state_no < -1) state_no = Trace_get_length(trace) + (state_no+2);

  nusmv_assert(state_no >= 0 && state_no <= Trace_get_length(trace));
  return state_no;
}


/**Function********************************************************************

  Synopsis    [Checks if the label is valid label in a registered trace.]

  Description [This function can be safely used to determine whether a
  label denotes a valid <trace, state> pair. This is guaranteed to
  raise no errors (exceptions, assertions) and should be used before
  any other label-related function to avoid any subsequent failure.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean TraceManager_is_label_valid(TraceManager_ptr self, TraceLabel label)
{
  int trace_no, trace_len, state_no;

  trace_no = TraceLabel_get_trace(label) ;
  if (trace_no < 0 || trace_no >= TraceManager_get_size(self))  { return false; }

  trace_len = Trace_get_length(TraceManager_get_trace_at_index(self, trace_no));

  /* negative state numbers are legal, and they are used to denote
     states rightwards from the last state of the trace (i.e. -1 is
     last state, -2 is previous and so forth) */
  state_no = abs(TraceLabel_get_state(label) +1);

  if (state_no < 1 || (1 + trace_len) < state_no) { return false; }

  /* here label is valid */
  return true;
}

/**Function********************************************************************

  Synopsis    [Registers default plugins.]

  Description [Statically registers available plugins]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void TraceManager_init_plugins(TraceManager_ptr self)
{
  TracePlugin_ptr plugin;

  /* Not previously registered: */
  nusmv_assert(self->internal_plugins_num == 0);

  /* 0. TRACE EXPLAINER - changes only */
  plugin = TRACE_PLUGIN(TraceExplainer_create(true));
  TraceManager_register_plugin(self, plugin);

  /* 1. BASIC TRACE EXPLAINER  */
  plugin = TRACE_PLUGIN(TraceExplainer_create(false));
  TraceManager_register_plugin(self, plugin);

  /* 2. TRACE TABLE PLUGIN -- column format */
  plugin = TRACE_PLUGIN(TraceTable_create(TRACE_TABLE_TYPE_COLUMN));
  TraceManager_register_plugin(self, plugin);

  /* 3. TRACE TABLE PLUGIN -- row format */
  plugin = TRACE_PLUGIN(TraceTable_create(TRACE_TABLE_TYPE_ROW));
  TraceManager_register_plugin(self, plugin);

  /* 4. TRACE XML DUMP PLUGIN */
  plugin = TRACE_PLUGIN(TraceXmlDumper_create());
  TraceManager_register_plugin(self, plugin);

  /* 5. TRACE COMPACT PLUGIN  */
  plugin = TRACE_PLUGIN(TraceCompact_create());
  TraceManager_register_plugin(self, plugin);

  self->internal_plugins_num = TraceManager_get_plugin_size(self);
}


/**Function********************************************************************

  Synopsis    [Retrieves description for a registered complete trace executor]

  Description [Retrieves description for a registered complete trace
  executor. The executor must have been previously registered within
  the trace manager using TraceManager_register_complete_trace_executor.

  Trying to retrieve description for a non-registered executor results
  in an assertion failure.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
const char*
TraceManager_get_complete_trace_executor_desc(const TraceManager_ptr self,
                                              const char* name)
{
  TRACE_MANAGER_CHECK_INSTANCE(self);
  nusmv_assert(NIL(char) != name);
  node_ptr entry = find_assoc(self->complete_trace_executors,
                              NODE_PTR(find_string((char*)(name))));
  nusmv_assert(Nil != entry);
  return get_text((string_ptr)(car(entry)));
}


/**Function********************************************************************

  Synopsis [Retrieves a registered complete trace executor instance
  with given name]

  Description [Returns a valid complete trace executor instance if any
  suitable such object has been previously registered with given name,
  using TraceManager_register_complete_trace_executor. If no such
  object is found, NULL is returned]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
CompleteTraceExecutor_ptr
TraceManager_get_complete_trace_executor(const TraceManager_ptr self,
                                         const char* name)
{
  TRACE_MANAGER_CHECK_INSTANCE(self);
  nusmv_assert(NIL(char) != name);
  node_ptr entry = find_assoc(self->complete_trace_executors,
                              NODE_PTR(find_string((char*)(name))));
  if (Nil == entry) { return COMPLETE_TRACE_EXECUTOR(NULL); }
  return COMPLETE_TRACE_EXECUTOR(cdr(entry));
}


/**Function********************************************************************

  Synopsis    [Retrieves description for partial trace executor]

  Description [Retrieves description for partial trace executor
  registered with given name, or NULL if no such executor exists]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
const char*
TraceManager_get_partial_trace_executor_desc(const TraceManager_ptr self,
                                             const char* name)
{
  TRACE_MANAGER_CHECK_INSTANCE(self);
  nusmv_assert(NIL(char) != name);
  node_ptr entry = find_assoc(self->partial_trace_executors,
                              NODE_PTR(find_string((char*)(name))));
  nusmv_assert(Nil != entry);
  return get_text((string_ptr)(car(entry)));
}


/**Function********************************************************************

  Synopsis    [Retrieves partial trace registered with given name]

  Description [Retrieves description for a registered partial trace
  executor. The executor must have been previously registered within
  the trace manager using TraceManager_register_complete_trace_executor.

  Trying to retrieve description for a non-registered executor results
  in an assertion failure.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
PartialTraceExecutor_ptr
TraceManager_get_partial_trace_executor(const TraceManager_ptr self,
                                        const char* name)
{
  TRACE_MANAGER_CHECK_INSTANCE(self);
  nusmv_assert(NIL(char) != name);
  node_ptr entry = find_assoc(self->partial_trace_executors,
                              NODE_PTR(find_string((char*)(name))));
  if (Nil == entry) { return PARTIAL_TRACE_EXECUTOR(NULL); }
  return PARTIAL_TRACE_EXECUTOR(cdr(entry));
}


/**Function********************************************************************

  Synopsis [Checks whether a symbol is visible]

  Description [Returns true iff the symbol is visible according to the
  following criteria:

  1. symbol name does not contain the prefix defined in system
  variable traces_hiding_prefix.

  2. if system variable traces_regexp is not empty, (1)
  holds and symbol name matches the regexp described in
  traces_regexp]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
boolean TraceManager_is_visible_symbol(TraceManager_ptr self, node_ptr symbol)
{
  boolean res = true;
  char* name = NIL(char);

  /* update internal defaults from env */
  TraceOpt_update_from_env(self->default_opt, OptsHandler_get_instance());
  const char* pref = TraceOpt_hiding_prefix(self->default_opt);

  nusmv_assert(Nil != symbol);
  node_ptr iter = symbol;

  while ((Nil != iter) && (NIL(char) == name)) {

    switch (node_get_type(iter))
      {
      case ATOM: case BIT: name = sprint_node(iter); break;
      case ARRAY: iter = car(iter); break;
      case CONTEXT: iter = cdr(iter); break;
      case DOT: { /* check if it name.number */
        node_ptr r = cdr(iter);

        if ((Nil != r) && (NUMBER == node_get_type(r))) { 
          /* found a name */
          name = sprint_node(iter);
        }
        else { iter = cdr(iter); }

        break;
      }

      default: error_unreachable_code(); /* not handled */
      } /* switch */
  } /* while */

  nusmv_assert(NIL(char) != name);

  if ((NIL(const char) != pref) && (name == strstr(name, pref))) {
    /* matches hidden prefix, this node is not printable */
    res = false;
  }

#if NUSMV_HAVE_REGEX_H
  if (res) {
    /* tries with matching regular expression */
    int cr = 1;

    regex_t* re = TraceOpt_regexp(self->default_opt);
    if ((regex_t*) NULL != re) {
      cr = regexec(re, name, 0, NULL, 0);
      res = (0 == cr);
    }
  }
#endif

  FREE(name);
  return res;
}

/*---------------------------------------------------------------------------*/
/* Static functions definitions                                              */
/*---------------------------------------------------------------------------*/
static enum st_retval
trace_manager_destroy_executor_entry (char *key, char *data, char * arg)
{
  node_ptr entry = NODE_PTR(data);
  BaseTraceExecutor_ptr executor = BASE_TRACE_EXECUTOR(cdr(entry));
  BaseTraceExecutor_destroy(executor); free_node(entry);
  return ST_DELETE;
}

/* string comparison function used by
   TraceManager_get_partial_trace_executor_ids and
   TraceManager_get_complete_trace_executor_ids */
static int cmp_string_ptr(string_ptr* a, string_ptr* b)
{
  return strcmp(get_text(*a), get_text(*b));
}
