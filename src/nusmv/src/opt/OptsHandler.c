/**CFile***********************************************************************

  FileName    [OptsHandler.c]

  PackageName [opt]

  Synopsis    [Generic handler of options. ]

  Description [Generic handler of options. An option is uniquely
  identified in an option handler by:
  <ul>
    <li> a name (it is the string of its name).
    <li> a default value (it is the string of its name).
    <li> a value (it is the string of its name).
  </ul>
  When registering an option the user must specify two functions. The
  first is responsible of checking that the value passed while setting
  a value is a valid value for the given option. The second function
  is a function that transforms the stroed value in a value suitable
  to be used in the calling program. <br>

  For boolean options are provided special methods to register the
  option and for setting and getting a value associated to it.<br>

  For enumerative options are provided special methods to register the
  option and for setting and getting a value associated to it. An
  enumertive option is registered by providing an array of structures
  of type Opts_EnumRec. Similarly to the below declaration:<br>
  <pre>
    typedef enum {foo1, ...., fooN} fooenumtype;
    Opts_EnumRec foo[] = {"foo1", foo1,
                          "foo2", foo2,
                          ....
                          "fooN", fooN};
   ....
   handler = OptsHandler_create();
   OptsHandler_register_enum_option(handler, "foooption", "foo1", foo, N);

  if (OptsHandler_get_enum_option_value(handler, "foooption") == foo2) {
     ...
  }

  ...

  switch(OptsHandler_get_enum_option_value(handler, "foooption")) {
    case foo1:
    ...
    case fooN:
    ...
    default:
    ...
  }
  </pre>
  ]

  SeeAlso     []

  Author      [Marco Roveri, Alessandro Mariotti]

  Copyright   [
  This file is part of the ``opt'' package of NuSMV version 2.
  Copyright (C) 2009 by FBK.

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

#include <string.h>
#include "OptsHandler.h"
#include "st.h"
#include "utils/Slist.h"
#include "utils/avl.h"
#include "utils/error.h"
#include "utils/ustring.h"

static char rcsid[] UTIL_UNUSED = "$Id: opts.c,v 1.7 2002/05/21 13:43:24 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef enum st_retval opts_st_retval;
typedef struct _option_structure opt_rec;
typedef struct _option_structure* opt_ptr;
typedef struct _option_value_list ovl_rec;
typedef struct _option_value_list* ovl_ptr;
typedef boolean (* Opts_CheckOplFnType)(OptsHandler_ptr,
                                        const char *, ovl_ptr);
typedef void* (* Opts_GetOplFnType)(OptsHandler_ptr, const char *, ovl_ptr);

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

struct _OptsHandler_Rec {
  hash_ptr table;
  st_generator* gen;
  unsigned int opt_max_length;
};

struct _option_structure {
  char* name;
  char* default_value;
  char* value;
  ovl_ptr possible_values;
  boolean public;
  Opts_CheckFnType check;
  Opts_ReturnFnType getvalue;
  Option_Type type;
  boolean user_defined;
  Slist_ptr triggers;
};

struct _option_value_list {
  ovl_ptr next;
  char* values;
  int valuee;
};

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

extern FILE* nusmv_stderr;

static OptsHandler_ptr instance = OPTS_HANDLER(NULL);

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define OPTS_STR_CMP_FUNC strcmp
#define OPTS_BOOLEAN_REC {{OPTS_FALSE_VALUE, false},{OPTS_TRUE_VALUE, true}}
#define OPTS_DELETE_ENTRY ST_DELETE

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static boolean opts_isinteger ARGS((OptsHandler_ptr opts, const char *name));
static void* opts_getinteger ARGS((OptsHandler_ptr opts, const char *value));

static boolean opts_check_string ARGS((OptsHandler_ptr opts, const char* name));
static void* opts_get_string ARGS((OptsHandler_ptr opts, const char* value));

static opts_st_retval opts_hash_free ARGS((char *key, char *data, char* arg));
static opt_ptr option_alloc ARGS((void));
static void option_free ARGS((opt_ptr* o));
static char* opts_strsav ARGS((const char *s));
static opt_ptr option_create ARGS((const char* name,
                                   const char* default_value,
                                   const char* value,
                                   ovl_ptr opl,
                                   Opts_CheckFnType check,
                                   Opts_ReturnFnType getvalue,
                                   boolean is_public,
                                   Option_Type opt_type,
                                   boolean user_defined));

static boolean opt_enum_check ARGS((OptsHandler_ptr opts,
                                    const char* value, ovl_ptr l));
static void* opt_enum_get ARGS((OptsHandler_ptr opts,
                                 const char* value, ovl_ptr l));
static ovl_ptr ovl_rec_alloc ARGS((void));
static ovl_ptr ovl_create_empty ARGS((void));
static int ovl_isempty ARGS((ovl_ptr l));
static int ovl_isnotempty ARGS((ovl_ptr l));
static ovl_ptr ovl_create ARGS((const char* value, int valuee));
static ovl_ptr ovl_get_next ARGS((ovl_ptr l));
static ovl_ptr ovl_set_next ARGS((ovl_ptr l, ovl_ptr n));
static void ovl_free ARGS((ovl_ptr *l));
static int ovl_ispresent ARGS((ovl_ptr l, const char *value));
static boolean check_boolean ARGS((ovl_ptr l));

static boolean opts_handler_register_generic_option ARGS((OptsHandler_ptr self,
                                                          const char* name,
                                                          const char* def,
                                                          ovl_ptr ovl,
                                                          Opts_CheckFnType check,
                                                          Opts_ReturnFnType get,
                                                          boolean is_public,
                                                          Option_Type type,
                                                          boolean user_defined));

static boolean
opts_handler_run_triggers ARGS((OptsHandler_ptr self, opt_ptr opt,
                                const char* name, const char* val,
                                Trigger_Action action));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Get the global options handler instance]

  Description        [Get the global options handler instance]

  SideEffects        [If called for the first time,
                      instanciates the options handler]

  SeeAlso            [OptsHandler_instance_destroy]

******************************************************************************/
OptsHandler_ptr OptsHandler_get_instance()
{
  if (OPTS_HANDLER(NULL) == instance) {
    instance = OptsHandler_create();
  }

  return instance;
}

/**Function********************************************************************

  Synopsis           [Free the global options handler instance]

  Description        [Free the global options handler instance]

  SideEffects        []

  SeeAlso            [OptsHandler_get_instance]

******************************************************************************/
void OptsHandler_instance_destroy()
{
  if (OPTS_HANDLER(NULL) != instance) {
    OptsHandler_destroy(instance);
    instance = OPTS_HANDLER(NULL);
  }
}


/**Function********************************************************************

  Synopsis           [Creates an empty option handler]

  Description        [Creates an empty option handler. ]

  SideEffects        [None]

  SeeAlso            [OptsHandler_destroy]

******************************************************************************/
OptsHandler_ptr OptsHandler_create()
{
  OptsHandler_ptr result;

  result = ALLOC(OptsHandler_Rec, 1);
  OPTS_HANDLER_CHECK_INSTANCE(result);

  {
    hash_ptr h = new_assoc();

    if (NIL(st_table) == h) {
      error_unreachable_code();
    }
    else {
      result->table = h;
      result->gen = (st_generator *)NULL;
      result->opt_max_length = 0;
    }
  }

  return result;
}

/**Function********************************************************************

  Synopsis           [Frees an option handler.]

  Description        [Frees an option handler.]

  SideEffects        [None]

  SeeAlso            [OptsHandler_create]

******************************************************************************/
void OptsHandler_destroy(OptsHandler_ptr self)
{
  OPTS_HANDLER_CHECK_INSTANCE(self);

  assoc_foreach(self->table, opts_hash_free, (char*)NULL);
  free_assoc(self->table);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Registers an option in an option handler.]

  Description        [Registers an option in an option handler. Returns
  true if the registration of the option succeeds, false otherwise.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_register_option(OptsHandler_ptr self,
                                    const char* name,
                                    const char* def,
                                    Opts_CheckFnType check,
                                    Opts_ReturnFnType get,
                                    boolean is_public,
                                    Option_Type type)
{
  ovl_ptr ovl = ovl_create_empty();

  OPTS_HANDLER_CHECK_INSTANCE(self);

  return opts_handler_register_generic_option(self, name, def, ovl, check, get,
                                              is_public, type, false);
}

/**Function********************************************************************

  Synopsis           [Registers a generic option in the option handler.]

  Description        [Registers an option in an option handler. Returns
  true if the registration of the option succeeds, false otherwise.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_register_generic_option(OptsHandler_ptr self,
                                            const char* name,
                                            const char* def,
                                            boolean is_public)
{
  ovl_ptr ovl = ovl_create_empty();

  OPTS_HANDLER_CHECK_INSTANCE(self);

  return opts_handler_register_generic_option(self, name, def, ovl,
                                              opts_check_string,
                                              opts_get_string,
                                              is_public, GENERIC_OPTION,
                                              false);
}

/**Function********************************************************************

  Synopsis           [Registers a user-defined option in the option handler.]

  Description        [Registers a user-defined option in the option handler.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_register_user_option(OptsHandler_ptr self,
                                         const char* name,
                                         const char* value)
{
  boolean result = false;
  ovl_ptr ovl =  ovl_create_empty();

  OPTS_HANDLER_CHECK_INSTANCE(self);

  result = opts_handler_register_generic_option(self, name, value, ovl,
                                                opts_check_string,
                                                opts_get_string,
                                                true, USER_OPTION, true);

  return(result);
}

/**Function********************************************************************

  Synopsis           [Registers an enumerative option in an option handler.]

  Description        [Registers an enumerative option in an option
  handler. The possible values are stored in an array of strings given
  in input. The user is not required to provide any function to check
  and return a value. Returns true if the registration of the option succeeds,
  false otherwise.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_register_enum_option(OptsHandler_ptr self,
                                         const char* name,
                                         const char* def,
                                         Opts_EnumRec pv[],
                                         int npv,
                                         boolean is_public)
{
  int n;
  opt_ptr opt;
  ovl_ptr ovl;
  boolean result = false;
  string_ptr uname = find_string((char*)name);

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(uname));

  if ((opt_ptr)NULL == opt) {
    ovl = ovl_create_empty();

    for (n = 0; n < npv; n++) {
      ovl_ptr l = ovl_create(pv[n].v, pv[n].e);

      ovl_set_next(l, ovl);
      ovl = l;
    }

    result =
      opts_handler_register_generic_option(self, name, def, ovl,
                                           (Opts_CheckFnType)opt_enum_check,
                                           (Opts_ReturnFnType)opt_enum_get,
                                           is_public, ENUM_OPTION, false);
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Registers a boolean option in an option handler.]

  Description        [Registers a boolean option in an option
  handler. The user is not required to provide any function to check
  and return a value. Returns true if the registration of the option succeeds,
  false otherwise.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_register_bool_option(OptsHandler_ptr self,
                                         const char* name,
                                         boolean value,
                                         boolean is_public)
{
  boolean result = false;
  char* def;
  ovl_ptr ovl;
  Opts_EnumRec pv[2] = OPTS_BOOLEAN_REC;
  int i;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  ovl = ovl_create_empty();
  for (i = 0; i < 2; ++i) {
    ovl_ptr l = ovl_create(pv[i].v, pv[i].e);
    ovl_set_next(l, ovl);
    ovl = l;
  }

  def = (value == true) ? OPTS_TRUE_VALUE : OPTS_FALSE_VALUE;

  result =
    opts_handler_register_generic_option(self, name, def, ovl,
                                         (Opts_CheckFnType)opt_enum_check,
                                         (Opts_ReturnFnType)opt_enum_get,
                                         is_public, BOOL_OPTION, false);

  return(result);
}

/**Function********************************************************************

  Synopsis           [Registers an integer option in an option handler.]

  Description        [Registers an integer option in an option
  handler. The user is not required to provide any function to check
  and return a value. Returns true if the registration of the option succeeds,
  false otherwise.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_register_int_option(OptsHandler_ptr self,
                                        const char* name,
                                        int value,
                                        boolean is_public)
{
  boolean result = false;
  char def[100];
  ovl_ptr ovl = ovl_create_empty();
  int chars;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  chars = snprintf(def, 100, "%d", value);
  SNPRINTF_CHECK(chars, 100);

  result = opts_handler_register_generic_option(self, name, def, ovl,
                                                opts_isinteger, opts_getinteger,
                                                is_public, INTEGER_OPTION, false);
  return(result);
}

/**Function********************************************************************

  Synopsis           [Checks if the given is public or not.]

  Description        [Checks if the given is public or not.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_is_option_public(OptsHandler_ptr self,
                                     const char* name)
{
  opt_ptr opt = (opt_ptr)NULL;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));
  nusmv_assert((opt_ptr)NULL != opt);

  return opt->public;
}

/**Function********************************************************************

  Synopsis           [Checks if the given option is user-defined]

  Description        [Checks if the given option is user-defined]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_is_user_option(OptsHandler_ptr self,
                                   const char* name)
{
  opt_ptr opt;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));
  nusmv_assert((opt_ptr)NULL != opt);

  return (USER_OPTION == opt->type);
}


/**Function********************************************************************

  Synopsis           [Checks if the given option is boolean]

  Description        [Checks if the given option is boolean]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_is_bool_option(OptsHandler_ptr self,
                                   const char* name)
{
  opt_ptr opt;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));
  nusmv_assert((opt_ptr)NULL != opt);

  return (BOOL_OPTION == opt->type);
}

/**Function********************************************************************

  Synopsis           [Checks if the given option is integer]

  Description        [Checks if the given option is integer]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_is_int_option(OptsHandler_ptr self,
                                  const char* name)
{
  opt_ptr opt;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));
  nusmv_assert((opt_ptr)NULL != opt);

  return (INTEGER_OPTION == opt->type);
}


/**Function********************************************************************

  Synopsis           [Checks if the given option is enumerative]

  Description        [Checks if the given option is enumerative]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_is_enum_option(OptsHandler_ptr self,
                                   const char* name)
{
  opt_ptr opt;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));
  nusmv_assert((opt_ptr)NULL != opt);

  return (ENUM_OPTION == opt->type);
}


/**Function********************************************************************

  Synopsis           [Checks if the given option is generic]

  Description        [Checks if the given option is generic]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_is_generic_option(OptsHandler_ptr self,
                                      const char* name)
{
  opt_ptr opt;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));
  nusmv_assert((opt_ptr)NULL != opt);

  return (GENERIC_OPTION == opt->type);
}

/**Function********************************************************************

  Synopsis           [Get the string representation of option's possible values]

  Description        [Get the string representation of option's possible values]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void OptsHandler_get_enum_option_values(OptsHandler_ptr self,
                                        const char* name,
                                        char*** values,
                                        int* num_values)
{
  opt_ptr opt;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));
  nusmv_assert((opt_ptr)NULL != opt);

  nusmv_assert(ENUM_OPTION == opt->type || BOOL_OPTION == opt->type);

  {
    ovl_ptr l = opt->possible_values;
    int num = 0;
    for ( ; ovl_isnotempty(l); l = ovl_get_next(l)) {
      ++num;
    }

    *values = ALLOC(char*, num);
    *num_values = num;
    l = opt->possible_values;
    num = 0;
    for ( ; ovl_isnotempty(l); l = ovl_get_next(l), ++num) {
      (*values)[num] = strdup(l->values);
    }
  }
}

/**Function********************************************************************

  Synopsis           [Checks if an option has already been registered.]

  Description        [Checks if an option has already been
  registered. Returns true if it has been already registered false otherwise.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_is_option_registered(OptsHandler_ptr self,
                                         const char* name)
{
  opt_ptr opt;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));
  return ((opt_ptr)NULL != opt);
}

/**Function********************************************************************

  Synopsis           [Checks if an option has already been registered.]

  Description        [Checks if an option has already been
  registered. Returns false if it has been already registered true otherwise.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_is_option_not_registered(OptsHandler_ptr self,
                                             const char* name)
{
  opt_ptr opt;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));
  return ((opt_ptr)NULL == opt);
}

/**Function********************************************************************

  Synopsis           [Unregisters an option in an option handler.]

  Description        [Unregisters an option in an option handler. Returns
  true if the unregistration of the option succeeds, false otherwise.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_unregister_option(OptsHandler_ptr self, const char* name)
{
  opt_ptr opt;
  boolean result = false;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)remove_assoc(self->table, NODE_PTR(find_string((char*)name)));
  if ((opt_ptr)NULL != opt) {
    option_free(&opt);
    result = true;
  }

  return(result);
}

/**Function********************************************************************

  Synopsis           [Assigns the given value to a registered option.]

  Description        [Assigns the given value to an option registered
  in an option handler. Returns true if the setting of the value
  succeeds, false if the option name is not registered in the option
  handler or if the value to assigns does not is of the type allowed
  for the option.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_set_option_value(OptsHandler_ptr self,
                                     const char* name,
                                     const char* value)
{
  opt_ptr opt;
  boolean result = false;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));

  if ((opt_ptr)NULL != opt) {

    if (ovl_isempty(opt->possible_values) == 1) {
      /* user defined option */
      if ( (* opt->check)(self, (char *)value) == true ) {
        /* Run triggers, if any.. */
        if (!opts_handler_run_triggers(self, opt, name, value, ACTION_SET)) {
          return false;
        }

        if ((char *)NULL != opt->value) {
          FREE(opt->value);
        }
        opt->value = opts_strsav(value);
        result = true;
      }
    }
    else {
      Opts_CheckOplFnType f;

      f = (Opts_CheckOplFnType)opt->check;
      /* internally handled options */
      if ((*f)(self, value, opt->possible_values) == true) {
        /* Run triggers, if any.. */
        if (!opts_handler_run_triggers(self, opt, name, value, ACTION_SET)) {
          return false;
        }

        if ((char *)NULL != opt->value) {
          FREE(opt->value);
        }
        opt->value = opts_strsav(value);
        result = true;
      }
    }
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Assigns the given value to a registered option.]

  Description        [Assigns the given value to an option registered
  in an option handler. Returns true if the setting of the value
  succeeds, false if the option name is not registered in the option
  handler or if the value to assigns does not is of the type allowed
  for the option.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_set_enum_option_value(OptsHandler_ptr self,
                                          const char* name,
                                          const char* value)
{
  OPTS_HANDLER_CHECK_INSTANCE(self);
  return(OptsHandler_set_option_value(self, name, value));
}

/**Function********************************************************************

  Synopsis           [Assigns the given value to a registered option.]

  Description        [Assigns the given value to an option registered
  in an option handler. Returns true if the setting of the value
  succeeds, false if the option name is not registered in the option
  handler or if the value to assigns does not is of the type allowed
  for the option.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_set_bool_option_value(OptsHandler_ptr self,
                                          const char* name,
                                          boolean value)
{
  char* v;
  opt_ptr opt;
  boolean result = false;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));

  if ((opt_ptr)NULL != opt) {
    nusmv_assert(check_boolean(opt->possible_values));

    v = (value == true) ? OPTS_TRUE_VALUE : OPTS_FALSE_VALUE;

    /* Run triggers, if any.. */
    if (!opts_handler_run_triggers(self, opt, name, v, ACTION_SET)) {
      return false;
    }

    result = true;
    if ((char *)NULL != opt->value) {
      FREE(opt->value);
    }
    opt->value = opts_strsav(v);
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Assigns the given value to a registered option.]

  Description        [Assigns the given value to an option registered
  in an option handler. Returns true if the setting of the value
  succeeds, false if the option name is not registered in the option
  handler or if the value to assigns does not is of the type allowed
  for the option.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_set_int_option_value(OptsHandler_ptr self,
                                         const char* name,
                                         int value)
{
  char val[100];
  int chars;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  chars = snprintf(val, 100, "%d", value);
  SNPRINTF_CHECK(chars, 100);

  return(OptsHandler_set_option_value(self, name, val));
}

/**Function********************************************************************

  Synopsis           [Assigns the default value to a registered option.]

  Description        [Assigns the default value to an option registered
  in an option handler. Returns true if the setting of the value
  succeeds, false if the option name is not registered in the option
  handler.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_reset_option_value(OptsHandler_ptr self, const char* name)
{
  opt_ptr opt;
  boolean result = false;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));

  if ((opt_ptr)NULL != opt) {

    /* Run triggers, if any.. */
    if (!opts_handler_run_triggers(self, opt, name, opt->value, ACTION_RESET)) {
      return false;
    }

    if ((char *)NULL != opt->value) {
      FREE(opt->value);
    }
    if ((char *)NULL != opt->default_value) {
      opt->value = opts_strsav(opt->default_value);
    }
    else {
      opt->value = (char*)NULL;
    }
    result = true;
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Returns the value of a registered option.]

  Description        [Returns the value of an option registered
  in an option. OPTS_VALUE_ERROR is returned if the option is not a
  registered option.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void* OptsHandler_get_option_value(OptsHandler_ptr self, const char* name)
{
  opt_ptr opt;
  void* result = NULL;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));

  if ((opt_ptr)NULL != opt) {

    /* Run triggers, if any.. */
    opts_handler_run_triggers(self, opt, name, opt->value, ACTION_GET);

    if ((char *)NULL != opt->value) {
      if (ovl_isempty(opt->possible_values) == 1) {
        /* user defined option */
        result = (* opt->getvalue)(self, opt->value);
      }
      else {
        /* internally handled option */
        Opts_GetOplFnType f;

        f = (Opts_GetOplFnType)opt->getvalue;
        result = (*f)(self, opt->value, opt->possible_values);
      }
    }
  }
  else {
    result = OPTS_VALUE_ERROR;
  }

  return(result);
}

/**Function********************************************************************

  Synopsis           [Returns the default value of a registered option.]

  Description        [Returns the default value of an option registered
                      in an option. OPTS_VALUE_ERROR is returned if
                      the option is not a registered option.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void* OptsHandler_get_option_default_value(OptsHandler_ptr self,
                                            const char* name)
{
  opt_ptr opt;
  void* result = NULL;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));

  if ((opt_ptr)NULL != opt) {
    /* Run triggers, if any.. */
    opts_handler_run_triggers(self, opt, name, opt->value, ACTION_GET);

    if ((char *)NULL != opt->default_value) {
      if (ovl_isempty(opt->possible_values) == 1) {
        /* user defined option */
        result = (* opt->getvalue)(self, opt->default_value);
      }
      else {
        /* internally handled option */
        Opts_GetOplFnType f;

        f = (Opts_GetOplFnType)opt->getvalue;
        result = (*f)(self, opt->default_value, opt->possible_values);
      }
    }
  }
  else {
    result = OPTS_VALUE_ERROR;
  }

  return(result);
}

/**Function********************************************************************

  Synopsis           [Returns the string representation of the value]

  Description        [Returns the string representation of the value.
                      The returned string must be freed, if not NULL.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
char* OptsHandler_get_string_representation_option_value(OptsHandler_ptr self,
                                                         const char* name)
{
  opt_ptr opt = (opt_ptr)NULL;
  char* result = NULL;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));

  if ((opt_ptr)NULL != opt) {

    /* Run triggers, if any.. */
    opts_handler_run_triggers(self, opt, name, opt->value, ACTION_GET);

    if ((char*)NULL != opt->value) {
      result = opts_strsav(opt->value);
    }
    else {
      result = opts_strsav("NULL");
    }
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [Returns the string representation of the default value]

  Description        [Returns the string representation of the default value.
                      The returned string must be freed, if not NULL.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
char*
OptsHandler_get_string_representation_option_default_value(OptsHandler_ptr self,
                                                           const char* name)
{
  opt_ptr opt = (opt_ptr)NULL;
  char* result = NULL;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));

  if ((opt_ptr)NULL != opt) {

    /* Run triggers, if any.. */
    opts_handler_run_triggers(self, opt, name, opt->value, ACTION_GET);

    if ((char*)NULL != opt->default_value) {
      result = opts_strsav(opt->default_value);
    }
    else {
      result = opts_strsav("NULL");
    }
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [Returns the value of a string option]

  Description        [Returns the value of a string option.
                      Depending on the return function, the string may be freed.
                      The internal getter function duplicates the string.
                      Caller should free the string]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
char* OptsHandler_get_string_option_value(OptsHandler_ptr self, 
                                          const char* name)
{
  OPTS_HANDLER_CHECK_INSTANCE(self);

  return (char*)OptsHandler_get_option_value(self, name);
}

/**Function********************************************************************

  Synopsis           [Returns the default value of a string option]

  Description        [Returns the default value of a string option.
                      Depending on the return function, the string may be freed.
                      The internal getter function duplicates the string.
                      Caller should free the string]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
char* OptsHandler_get_string_option_default_value(OptsHandler_ptr self,
                                                  const char* name)
{
  OPTS_HANDLER_CHECK_INSTANCE(self);

  return (char*)OptsHandler_get_option_default_value(self, name);
}

/**Function********************************************************************

  Synopsis           [Returns the value of an enum option.]

  Description        [Returns the value of an enum option
  value. OPTS_VALUE_ERROR is returned if the option is not a
  registered option.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int OptsHandler_get_enum_option_value(OptsHandler_ptr self, const char* name)
{
  OPTS_HANDLER_CHECK_INSTANCE(self);

  return PTR_TO_INT(OptsHandler_get_option_value(self, name));
}

/**Function********************************************************************

  Synopsis           [Returns the default value of an enum option.]

  Description        [Returns the default value of an enum option
  value. OPTS_VALUE_ERROR is returned if the option is not a
  registered option.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int OptsHandler_get_enum_option_default_value(OptsHandler_ptr self,
                                              const char* name)
{
  OPTS_HANDLER_CHECK_INSTANCE(self);

  return PTR_TO_INT(OptsHandler_get_option_default_value(self, name));
}

/**Function********************************************************************

  Synopsis           [Returns the value of a boolean option.]

  Description        [Returns the value of a boolean option
  value. OPTS_VALUE_ERROR is returned if the option is not a
  registered option.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_get_bool_option_value(OptsHandler_ptr self, 
                                          const char* name)
{
  opt_ptr opt;
  boolean result = (boolean)OPTS_VALUE_ERROR;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));

  if ((opt_ptr)NULL != opt) {
    nusmv_assert(check_boolean(opt->possible_values));

    /* Run triggers, if any.. */
    opts_handler_run_triggers(self, opt, name, opt->value, ACTION_GET);

    if ((char *)NULL != opt->value) {
        Opts_GetOplFnType f;

        f = (Opts_GetOplFnType)opt->getvalue;
        result = (boolean)(*f)(self, opt->value, opt->possible_values);
    }
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Returns the default value of a boolean option.]

  Description        [Returns the default value of a boolean option
                      value. OPTS_VALUE_ERROR is returned if the
                      option is not a registered option.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean OptsHandler_get_bool_option_default_value(OptsHandler_ptr self,
                                                  const char* name)
{
  opt_ptr opt;
  boolean result = (boolean)OPTS_VALUE_ERROR;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));

  if ((opt_ptr)NULL != opt) {
    nusmv_assert(check_boolean(opt->possible_values));

    /* Run triggers, if any.. */
    opts_handler_run_triggers(self, opt, name, opt->value, ACTION_GET);

    if ((char *)NULL != opt->default_value) {
      Opts_GetOplFnType f;

      f = (Opts_GetOplFnType)opt->getvalue;
      result = (boolean)(*f)(self, opt->default_value, opt->possible_values);
    }
  }

 return result;
}

/**Function********************************************************************

  Synopsis           [Returns the value of an int option.]

  Description        [Returns the value of an enum option
  value. OPTS_VALUE_ERROR is returned if the option is not a
  registered option.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int OptsHandler_get_int_option_value(OptsHandler_ptr self, const char* name)
{
  OPTS_HANDLER_CHECK_INSTANCE(self);
  return PTR_TO_INT(OptsHandler_get_option_value(self, name));
}

/**Function********************************************************************

  Synopsis           [Returns the value of an int option.]

  Description        [Returns the value of an enum option
  value. OPTS_VALUE_ERROR is returned if the option is not a
  registered option.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int OptsHandler_get_int_option_default_value(OptsHandler_ptr self,
                                             const char* name)
{
  OPTS_HANDLER_CHECK_INSTANCE(self);
  return PTR_TO_INT(OptsHandler_get_option_default_value(self, name));
}

/**Function********************************************************************

  Synopsis           [Initializes a generator for an option handler.]

  Description        [Initializes a generator handler which when used
  with Opts_Gen_next() will progressively return each (name, value)
  record in the option handler.]

  SideEffects        [None]

  SeeAlso            [Opts_Gen_next Opts_Gen_deinit]

******************************************************************************/
void Opts_Gen_init(OptsHandler_ptr self)
{
  st_generator* gen;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  nusmv_assert((st_generator *)NULL == self->gen);

  gen = st_init_gen((st_table*)self->table);
  if ((st_generator *)NULL != gen) {
    self->gen = gen;
  }
  else {
    fprintf(nusmv_stderr, "Opts_GenInit: Unable to allocate generator\n");
  }
}

/**Function********************************************************************

  Synopsis           [Gets the next pair (name, value) for an option handler.]

  Description        [Given a generator created by Opts_GenInit(),
     this routine returns the next (name, value) pair in the
     generation sequence. When there are no more items in the
     generation sequence,  the routine returns 0.]

  SideEffects        [None]

  SeeAlso            [Opts_Gen_init Opts_Gen_deinit]

******************************************************************************/
int Opts_Gen_next(OptsHandler_ptr self, char **name, char **value)
{
  string_ptr n;
  opt_ptr opt = (opt_ptr)1;
  int result;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  nusmv_assert((st_generator *)NULL != self->gen);

  *value = (char *)NULL;
  result = st_gen(self->gen, (char**)&n, (char **)&opt);
  if (result != 0) {
    nusmv_assert((opt_ptr)NULL != opt);
    *name = opt->name;
    *value = opt->value;
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Frees an option generator for an option handler.]

  Description        [After generating all items in a generation
  sequence, this routine must be called to reclaim the resources
  associated with the created generator.]

  SideEffects        [None]

  SeeAlso            [Opts_Gen_next Opts_Gen_init]

******************************************************************************/
void Opts_Gen_deinit(OptsHandler_ptr self)
{
  OPTS_HANDLER_CHECK_INSTANCE(self);

  nusmv_assert((st_generator *)NULL != self->gen);

  st_free_gen(self->gen);
  self->gen = (st_generator *)NULL;
}


/**Function********************************************************************

  Synopsis           [Prints all the options on a file]

  Description        [Prints all the options stored in the option
  handler on a given file.]

  SeeAlso            [Opts_GenFree Opts_Gen Opts_GenInit Opts_PrintAllOptions]

******************************************************************************/
boolean OptsHandler_add_option_trigger(OptsHandler_ptr self, const char* name,
                                       Opts_TriggerFnType trigger)
{
  opt_ptr opt;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));

  if ((opt_ptr)NULL != opt) {
    Slist_push(opt->triggers, (void*) trigger);
    return true;
  }

  return false;
}

/**Function********************************************************************

  Synopsis           [Prints all the options on a file]

  Description        [Prints all the options stored in the option
  handler on a given file.]

  SeeAlso            [Opts_GenFree Opts_Gen Opts_GenInit Opts_PrintAllOptions]

******************************************************************************/
boolean OptsHandler_remove_option_trigger(OptsHandler_ptr self, const char* name,
                                          Opts_TriggerFnType trigger)
{
  opt_ptr opt;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));

  if ((opt_ptr)NULL != opt) {
    return Slist_remove(opt->triggers, (void*) trigger);
  }

  return false;
}

/**Function********************************************************************

  Synopsis           [Prints all the options on a file]

  Description        [Prints all the options stored in the option
  handler on a given file.]

  SeeAlso            [Opts_GenFree Opts_Gen Opts_GenInit Opts_PrintAllOptions]

******************************************************************************/
void OptsHandler_print_all_options(OptsHandler_ptr self, FILE* fd,
                                   boolean print_private)
{
  char* name;
  char* value;
  int j = 0;
  avl_tree* avl = avl_init_table(Utils_strcasecmp);
  avl_generator* gen;

  OPTS_HANDLER_CHECK_INSTANCE(self);

  nusmv_assert((FILE *)NULL != fd);

  OPTS_FOREACH_OPTION(self, &name, &value) {
    if (print_private || OptsHandler_is_option_public(self, name)) {
      avl_insert(avl, name, value);
    }
  }

  gen = avl_init_gen(avl, AVL_FORWARD);

  while (avl_gen(gen, &name, &value) == 1) {
    fprintf(fd, "%s ", name);

    /* Indent.. */
    for (j = 0; j <= (self->opt_max_length - strlen(name)); ++j) {
      fprintf(fd, " ");
    }

    if ((char*)NULL != value) {
      fprintf(fd, " \"%s\"\n", value);
    }
    else {
      fprintf(fd, " NULL\n");
    }
  }

  avl_free_gen(gen);
  avl_free_table(avl, 0, 0);
}

/* Internal function for generating test for SET / UNSET commands */
void OptsHandler_generate_test(OptsHandler_ptr self, FILE* of,
                               boolean gen_unset)
{
  if (gen_unset) { fprintf(of, "COMMAND unset\n"); }
  else { fprintf(of, "COMMAND set\n"); }

  fprintf(of, "MODELS nomodelneeded\n");
  if (gen_unset) { fprintf(of, "OPTS {\"-h\", FAIL[usage: unset]}\n"); }
  else { fprintf(of, "OPTS {\"-h\", FAIL[usage: set]}\n"); }
  fprintf(of, "OPTS ");

  char* name;
  char* value;

  OPTS_FOREACH_OPTION(self, &name, &value) {
    if (OptsHandler_is_option_public(self, name)) {
      if (gen_unset) {
        fprintf(of, " {%s, PASS[]}\n|", name);
      }
      else {
        /* SPECIAL CASES */
        if (strcmp(name, "output_word_format") == 0) {
          fprintf(of, " {%s 2, PASS[]}\n|", name);
          fprintf(of, " {%s 8, PASS[]}\n|", name);
          fprintf(of, " {%s 10, PASS[]}\n|", name);
          fprintf(of, " {%s 16, PASS[]}\n|", name);
          fprintf(of, " {%s 1, FAIL[]}\n|", name);
          fprintf(of, " {%s 7, FAIL[]}\n|", name);
        }
        else if (strcmp(name, "sat_solver") == 0) {
          fprintf(of, " {%s zchaff, PASS[]}\n|", name);
          fprintf(of, " {%s minisat, PASS[]}\n|", name);
          fprintf(of, " {%s iamnotasatsolver, FAIL[]}\n|", name);
        }
        else if (strcmp(name, "pp_list") == 0) {
          fprintf(of, " {%s m4, PASS[]}\n|", name);
          fprintf(of, " {%s \"m4 cpp\", PASS[]}\n|", name);
          fprintf(of, " {%s \"cpp\", PASS[]}\n|", name);
        }
        else if (strcmp(name, "bmc_loopback") == 0) {
          fprintf(of, " {%s \"*\", PASS[]}\n|", name);
        }
        else if (strcmp(name, "bmc_inc_invar_alg") == 0) {
          fprintf(of, " {%s zigzag, PASS[]}\n|", name);
          fprintf(of, " {%s dual, PASS[]}\n|", name);
          fprintf(of, " {%s falsification, PASS[]}\n|", name);
        }
        else if (strcmp(name, "bmc_invar_alg") == 0) {
          fprintf(of, " {%s classic, PASS[]}\n|", name);
          fprintf(of, " {%s een-sorensson, PASS[]}\n|", name);
        }
        /* STANDARD VALUES VARIABLES */
        else if (OptsHandler_is_bool_option(self, name)) {
          fprintf(of, " {%s, PASS[]}\n|", name);
          fprintf(of, " {%s 1, PASS[]}\n|", name);
          fprintf(of, " {%s 0, PASS[]}\n|", name);
          fprintf(of, " {%s 2, FAIL[]}\n|", name);
        }
        else if (OptsHandler_is_enum_option(self, name)) {
          char** values;
          int num_values, i;
          OptsHandler_get_enum_option_values(self, name, &values, &num_values);
          for (i = 0; i < num_values; ++i) {
            fprintf(of, " {%s %s, PASS[]}\n|", name, values[i]);
          }
          fprintf(of, " {%s __i_am_not_valid__, FAIL[]}\n|", name);
        }
        else if (OptsHandler_is_int_option(self, name)) {
          fprintf(of, " {%s 1, PASS[]}\n|", name);
          fprintf(of, " {%s 2, PASS[]}\n|", name);
          fprintf(of, " {%s 3, PASS[]}\n|", name);
          fprintf(of, " {%s NaN, FAIL[]}\n|", name);
        }
        else if (OptsHandler_is_generic_option(self, name)) {
          fprintf(of, " {%s \"\", PASS[]}\n|", name);
          fprintf(of, " {%s \"custom_string\", PASS[]}\n|", name);
        }
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Creates a copy of the given string]

  Description        [Creates a copy of the given string. Must be freed]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static char* opts_strsav(const char *s)
{
  if ((char*)NULL != s) {
    return(strcpy(ALLOC(char, strlen(s)+1), s));
  }

  return (char*) NULL;
}

/**Function********************************************************************

  Synopsis           [Dummy function for string options handling]

  Description        [Dummy function for string options handling]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean opts_check_string(OptsHandler_ptr opts, const char* name)
{
  return true;
}

/**Function********************************************************************

  Synopsis           [Dummy function for string options handling]

  Description        [Dummy function for string options handling]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void* opts_get_string(OptsHandler_ptr opts, const char* value)
{
  return (void *)value;
}

/**Function********************************************************************

  Synopsis           [Check if a string represents an integer]

  Description        [Check if a string represents an integer]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean opts_isinteger(OptsHandler_ptr opts, const char *name)
{
  int l;
  char* e[1];
  boolean result = false;

  e[0] = "";
  l = (int)strtol(name, e, 10);
  if (strcmp(e[0], "") == 0) {
    result = true;
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Get the integer representation of the given string]

  Description        [Get the integer representation of the given string]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void* opts_getinteger(OptsHandler_ptr opts, const char *value)
{
  int result;
  char* e[1];

  e[0] = "";
  result = (int)strtol(value, e, 10);
  if (strcmp(e[0], "") != 0) {
    return OPTS_VALUE_ERROR;
  }
  return PTR_FROM_INT(void*, result);
}

/**Function********************************************************************

  Synopsis           [Allocates an instance of option]

  Description        [Allocates an instance of option]

  SideEffects        []

  SeeAlso            [option_free]

******************************************************************************/
static opt_ptr option_alloc(void)
{
  opt_ptr result;

  result = ALLOC(opt_rec, 1);
  if (NIL(opt_rec) == result) {
    fprintf(nusmv_stderr, "option_alloc: unable to allocate option entry.\n");
    return((opt_ptr)NULL);
  }
  result->name            = (char *)NULL;
  result->default_value   = (char *)NULL;
  result->value           = (char *)NULL;
  result->possible_values = ovl_create_empty();
  result->check           = (Opts_CheckFnType)NULL;
  result->getvalue        = (Opts_ReturnFnType)NULL;
  result->public          = false;
  result->type            = GENERIC_OPTION;
  result->user_defined    = false;
  result->triggers        = SLIST(NULL);
  return(result);
}

/**Function********************************************************************

  Synopsis           [Frees the given option instance]

  Description        [Frees the given option instance]

  SideEffects        []

  SeeAlso            [option_alloc]

******************************************************************************/
static void option_free(opt_ptr* p)
{
  opt_ptr o = *p;
  nusmv_assert( o != (opt_ptr)NULL);

  if ((char *)NULL != o->name) FREE(o->name);
  if ((char *)NULL != o->default_value) FREE(o->default_value);
  if ((char *)NULL != o->value) FREE(o->value);
  if (ovl_isnotempty(o->possible_values) == 1) {
    ovl_ptr q = o->possible_values;
    ovl_free(&q);
  }
  if (SLIST(NULL) != o->triggers) {
    Slist_destroy(o->triggers);
  }
  FREE(*p);
}

/**Function********************************************************************

  Synopsis           [Option Hash table freeing function]

  Description        [Option Hash table freeing function]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static opts_st_retval opts_hash_free(char *key, char *data, char* arg)
{
  opt_ptr entry = (opt_ptr)data;

  if ((opt_ptr)NULL != entry) {
    option_free(&entry);
  }
  return(OPTS_DELETE_ENTRY);
}

/**Function********************************************************************

  Synopsis           [Creates and initializes a new instance of option]

  Description        [Creates and initializes a new instance of option]

  SideEffects        []

  SeeAlso            [option_alloc]

******************************************************************************/
static opt_ptr option_create(const char* name,
                             const char* default_value,
                             const char* value,
                             ovl_ptr       pvalues,
                             Opts_CheckFnType check,
                             Opts_ReturnFnType getvalue,
                             boolean is_public,
                             Option_Type type,
                             boolean user_defined)
{
  opt_ptr result = option_alloc();

  if ((opt_ptr)NULL != result) {
    result->name            = opts_strsav(name);
    result->default_value   = opts_strsav(default_value);
    result->value           = opts_strsav(value);
    result->possible_values = pvalues;
    result->check           = check;
    result->getvalue        = getvalue;
    result->public          = is_public;
    result->type            = type;
    result->user_defined    = user_defined;
    result->triggers        = Slist_create();
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Checks if the given enumerative is in the given ovl]

  Description        [Checks if the given enumerative is in the given ovl]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean opt_enum_check(OptsHandler_ptr opts, const char* value, ovl_ptr l)
{
  boolean result = false;
  if (ovl_ispresent(l, value) == 1) {
    result = true;
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Gen the given enumerative value]

  Description        [Gen the given enumerative value]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void* opt_enum_get(OptsHandler_ptr opts, const char* value, ovl_ptr l)
{
  void* result = OPTS_VALUE_ERROR;
  int found = 0;

  for ( ; (ovl_isnotempty(l) && (found == 0)); l = ovl_get_next(l)) {
    if (strcmp(l->values, value) == 0) {
      found = 1;
      result = PTR_FROM_INT(void*, l->valuee);
    }
  }
  return(result);
}


/**Function********************************************************************

  Synopsis           [Allocates and initializes an ovl_ptr]

  Description        [Allocates and initializes an ovl_ptr]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static ovl_ptr ovl_rec_alloc(void)
{
  ovl_ptr result;

  result = ALLOC(ovl_rec, 1);
  if ((ovl_ptr)NULL == result) {
    fprintf(nusmv_stderr, 
            "ovl_rec_alloc: unable to allocate a value record.\n");
    error_unreachable_code();
  }

  result->next = ovl_create_empty();
  result->values = (char *)NULL;
  result->valuee = PTR_TO_INT(OPTS_VALUE_ERROR);
  return(result);
}

/**Function********************************************************************

  Synopsis           [Creates an empty ovl_ptr (represented by NULL)]

  Description        [Creates an empty ovl_ptr (represented by NULL)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static ovl_ptr ovl_create_empty(void)
{
  return ((ovl_ptr)NULL);
}

/**Function********************************************************************

  Synopsis           [Checks if the given ovl is empty]

  Description        [Checks if the given ovl is empty]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int ovl_isempty(ovl_ptr l)
{
  return((l == ovl_create_empty()));
}

/**Function********************************************************************

  Synopsis           [Checks if the given ovl is not empty]

  Description        [Checks if the given ovl is not empty]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int ovl_isnotempty(ovl_ptr l)
{
  return((l != ovl_create_empty()));
}

/**Function********************************************************************

  Synopsis           [Creates a new instance of ovl and sets
                      it with the given values]

  Description        [Creates a new instance of ovl and sets
                      it with the given values]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static ovl_ptr ovl_create(const char* values, int valuee)
{
  ovl_ptr result = (ovl_ptr)NULL;

  result = ovl_rec_alloc();
  if ((ovl_ptr)NULL != result) {
    result->values = opts_strsav(values);
    result->valuee = valuee;
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Get the next ovl in the list]

  Description        [Get the next ovl in the list]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static ovl_ptr ovl_get_next(ovl_ptr l)
{
  nusmv_assert(ovl_isnotempty(l) == 1);
  return(l->next);
}

/**Function********************************************************************

  Synopsis           [Sets the next ovl in the list]

  Description        [Sets the next ovl in the list]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static ovl_ptr ovl_set_next(ovl_ptr l, ovl_ptr n)
{
  nusmv_assert(ovl_isnotempty(l) == 1);
  l->next = n;
  return(l);
}

/**Function********************************************************************

  Synopsis           [Frees the given ovl instance]

  Description        [Frees the given ovl instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void ovl_free(ovl_ptr *l)
{
  ovl_ptr p;

  p = *l;
  while (ovl_isnotempty(p)) {
    ovl_ptr q = p;

    p = ovl_get_next(p);
    FREE(q->values);
    FREE(q);
  }
}

/**Function********************************************************************

  Synopsis           [Checks whatever the given value is in the given list]

  Description        [Checks whatever the given value is in the given list]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int ovl_ispresent(ovl_ptr l, const char *value)
{
  int result = 0;

  for ( ; (ovl_isnotempty(l) && (result == 0)); l = ovl_get_next(l)) {
    result = (strcmp(l->values, value) == 0);
  }
  return(result);
}


/**Function********************************************************************

  Synopsis           [Check if the given list contains boolean values]

  Description        [Check if the given list contains boolean values]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean check_boolean(ovl_ptr l)
{
  int result = true;

  for ( ; (ovl_isnotempty(l) && (result == 0)); l = ovl_get_next(l)) {
    result &= ((strcmp(l->values, OPTS_TRUE_VALUE) == 0) ||
               (strcmp(l->values, OPTS_FALSE_VALUE) == 0));
  }
  return result;
}

/**Function********************************************************************

  Synopsis           [Internal function for option registration]

  Description        [Internal function for option registration]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean opts_handler_register_generic_option(OptsHandler_ptr self,
                                                    const char* name,
                                                    const char* def,
                                                    ovl_ptr ovl,
                                                    Opts_CheckFnType check,
                                                    Opts_ReturnFnType get,
                                                    boolean is_public,
                                                    Option_Type type,
                                                    boolean user_defined)
{
  opt_ptr opt;
  boolean result = false;

  opt = (opt_ptr)find_assoc(self->table, NODE_PTR(find_string((char*)name)));

  if ((opt_ptr)NULL == opt) {
    opt = option_create(name, def, def, ovl, check, get,
                        is_public, type, user_defined);

    if ((opt_ptr)NULL != opt) {
      insert_assoc(self->table, (node_ptr)NODE_PTR(find_string((char*)name)), (node_ptr)opt);

      int l;
      result = true;

      /* Remember the longest command name, for good indentation. */
      l = strlen(name);
      if (l > self->opt_max_length) {
        self->opt_max_length = l;
      }
    }
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Internal function for trigger run]

  Description        [Internal function for trigger run]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean opts_handler_run_triggers(OptsHandler_ptr self, opt_ptr opt,
                                         const char* name, const char* val,
                                         Trigger_Action action)
{
  boolean result = true;
  Siter iter;

  SLIST_FOREACH(opt->triggers, iter) {
    Opts_TriggerFnType f = (Opts_TriggerFnType) Siter_element(iter);

    result &= (*f)(self, name, val, action);
  }

  return result;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/



