/**CHeaderFile*****************************************************************

   FileName    [OptsHandler.h]

   PackageName [opt]

   Synopsis    [Generic handler of options]

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

   Revision    []

******************************************************************************/

#ifndef _opts_h
#define _opts_h

#include "utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef enum {
  GENERIC_OPTION,
  USER_OPTION,
  INTEGER_OPTION,
  ENUM_OPTION,
  BOOL_OPTION
} Option_Type;

typedef enum {
  ACTION_SET,
  ACTION_RESET,
  ACTION_GET
} Trigger_Action;

typedef struct _OptsHandler_Rec   OptsHandler_Rec;
typedef struct _OptsHandler_Rec * OptsHandler_ptr;
typedef struct _Opts_EnumRec Opts_EnumRec;
typedef boolean (*Opts_CheckFnType)(OptsHandler_ptr, const char *);
typedef void * (*Opts_ReturnFnType)(OptsHandler_ptr, const char *);
typedef boolean (*Opts_TriggerFnType)(OptsHandler_ptr, const char *, 
                                      const char*, Trigger_Action);

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/
struct _Opts_EnumRec {
  char * v;
  int  e;
};


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define OPTS_TRUE_VALUE "1"
#define OPTS_FALSE_VALUE "0"
#define OPTS_VALUE_ERROR (void *)-9999


/**Macros**********************************************************************

   Synopsis    [To cast and check instances of class ModelSimplifier]

   Description [These macros must be used respectively to cast and to check
   instances of class ModelSimplifier]

******************************************************************************/
#define OPTS_HANDLER(self)                      \
  ((OptsHandler_ptr) self)

#define OPTS_HANDLER_CHECK_INSTANCE(self)                       \
  (nusmv_assert(OPTS_HANDLER(self) != OPTS_HANDLER(NULL)))


/**Macro***********************************************************************

   Synopsis     [Operates on each entry of the option handler]

   Description  [Operates on each entry of the option handler. name and
   value must be declared to be char **.]

   SeeAlso      [Opts_GenInit Opts_Gen Opts_GenFree]

******************************************************************************/
#define OPTS_FOREACH_OPTION(h, name, value)                             \
  for (Opts_Gen_init(h); Opts_Gen_next(h, name, value) || (Opts_Gen_deinit(h), 0); )


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN OptsHandler_ptr OptsHandler_get_instance ARGS((void));
EXTERN void OptsHandler_instance_destroy ARGS((void));

EXTERN OptsHandler_ptr OptsHandler_create ARGS((void));
EXTERN void OptsHandler_destroy ARGS((OptsHandler_ptr h));

EXTERN boolean OptsHandler_is_option_registered ARGS((OptsHandler_ptr self,
                                                      const char * name));

EXTERN boolean OptsHandler_is_option_not_registered ARGS((OptsHandler_ptr self,
                                                          const char * name));

EXTERN boolean
OptsHandler_register_option ARGS((OptsHandler_ptr self,
                                  const char * name,
                                  const char * def,
                                  Opts_CheckFnType check, Opts_ReturnFnType get,
                                  boolean is_public,
                                  Option_Type type));
EXTERN boolean
OptsHandler_register_generic_option ARGS((OptsHandler_ptr self,
                                          const char * name,
                                          const char * def,
                                          boolean is_public));

EXTERN boolean
OptsHandler_register_user_option ARGS((OptsHandler_ptr self,
                                       const char * name,
                                       const char * def));

EXTERN boolean
OptsHandler_register_bool_option ARGS(( OptsHandler_ptr self,
                                        const char * name,
                                        boolean value,
                                        boolean is_public));

EXTERN boolean
OptsHandler_register_enum_option ARGS((OptsHandler_ptr self,
                                       const char * name,
                                       const char * def,
                                       Opts_EnumRec pv[], int npv,
                                       boolean is_public));

EXTERN boolean
OptsHandler_register_int_option ARGS((OptsHandler_ptr self,
                                      const char * name,
                                      int value,
                                      boolean is_public));

EXTERN boolean
OptsHandler_is_option_public ARGS((OptsHandler_ptr self,
                                   const char* name));
EXTERN boolean
OptsHandler_is_enum_option ARGS((OptsHandler_ptr self,
                                 const char* name));
EXTERN boolean
OptsHandler_is_generic_option ARGS((OptsHandler_ptr self,
                                    const char* name));
EXTERN boolean
OptsHandler_is_user_option ARGS((OptsHandler_ptr self,
                                 const char* name));
EXTERN boolean
OptsHandler_is_bool_option ARGS((OptsHandler_ptr self,
                                 const char* name));
EXTERN boolean
OptsHandler_is_int_option ARGS((OptsHandler_ptr self,
                                const char* name));

EXTERN boolean
OptsHandler_unregister_option ARGS((OptsHandler_ptr self,
                                    const char * name));

EXTERN boolean
OptsHandler_set_option_value ARGS((OptsHandler_ptr self,
                                   const char * name,
                                   const char * value));

EXTERN void
OptsHandler_get_enum_option_values ARGS((OptsHandler_ptr self,
                                         const char * name,
                                         char *** values,
                                         int * num_values));

EXTERN boolean
OptsHandler_set_enum_option_value ARGS((OptsHandler_ptr self,
                                        const char * name,
                                        const char * value));

EXTERN int
OptsHandler_get_enum_option_value ARGS((OptsHandler_ptr self,
                                        const char * name));

EXTERN int
OptsHandler_get_enum_option_default_value ARGS((OptsHandler_ptr self,
                                                const char * name));

EXTERN boolean
OptsHandler_set_bool_option_value ARGS((OptsHandler_ptr self,
                                        const char * name,
                                        boolean value));

EXTERN boolean
OptsHandler_set_int_option_value ARGS((OptsHandler_ptr self,
                                       const char * name,
                                       int value));

EXTERN boolean
OptsHandler_reset_option_value ARGS((OptsHandler_ptr self,
                                     const char * name));

EXTERN void *
OptsHandler_get_option_value ARGS((OptsHandler_ptr self,
                                   const char * name));

EXTERN void *
OptsHandler_get_option_default_value ARGS((OptsHandler_ptr self,
                                           const char * name));

EXTERN char *
OptsHandler_get_string_option_value ARGS((OptsHandler_ptr self,
                                          const char * name));

EXTERN char*
OptsHandler_get_string_representation_option_default_value ARGS((OptsHandler_ptr self,
                                                                 const char * name));

EXTERN char*
OptsHandler_get_string_representation_option_value ARGS((OptsHandler_ptr self,
                                                         const char * name));

EXTERN boolean
OptsHandler_get_bool_option_value ARGS((OptsHandler_ptr self,
                                        const char * name));

EXTERN boolean
OptsHandler_get_bool_option_default_value ARGS((OptsHandler_ptr self,
                                                const char * name));

EXTERN int
OptsHandler_get_int_option_default_value ARGS((OptsHandler_ptr self,
                                               const char * name));

EXTERN int
OptsHandler_get_int_option_value ARGS((OptsHandler_ptr self,
                                       const char * name));

EXTERN boolean
OptsHandler_add_option_trigger ARGS((OptsHandler_ptr self, const char* name,
                                     Opts_TriggerFnType trigger));

EXTERN boolean
OptsHandler_remove_option_trigger ARGS((OptsHandler_ptr self, const char* name,
                                        Opts_TriggerFnType trigger));

EXTERN void Opts_Gen_init ARGS((OptsHandler_ptr self));

EXTERN int Opts_Gen_next ARGS((OptsHandler_ptr self, char ** name, char ** value));

EXTERN void Opts_Gen_deinit ARGS((OptsHandler_ptr self));

EXTERN void OptsHandler_print_all_options ARGS((OptsHandler_ptr self, FILE * fd,
                                                boolean print_private));

EXTERN void OptsHandler_generate_test ARGS((OptsHandler_ptr self, FILE* of,
                                            boolean gen_unset));

/**AutomaticEnd***************************************************************/

#endif /* _ */
