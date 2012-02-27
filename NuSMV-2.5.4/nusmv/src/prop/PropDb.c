/**CFile***********************************************************************

  FileName    [PropDb.c]

  PackageName [prop]

  Synopsis    [Implementation of class 'PropDb']

  Description []

  SeeAlso     [PropDb.h]

  Author      [marco Roveri, Roberto Cavada]

  Copyright   [
  This file is part of the ``prop'' package of NuSMV version 2. 
  Copyright (C) 2010 by FBK-irst. 

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

  Revision    [$Id: $]

******************************************************************************/

#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "PropDb.h"
#include "PropDb_private.h"
#include "Prop.h"
#include "Prop_private.h"
#include "propInt.h"


#include "compile/compile.h"
#include "compile/symb_table/SymbTable.h"
#include "parser/symbols.h"
#include "parser/parser.h"
#include "parser/psl/pslNode.h"
#include "utils/utils.h"
#include "utils/ucmd.h"


static char rcsid[] UTIL_UNUSED = "$Id: $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'PropDb_private.h' for class 'PropDb' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void prop_db_finalize ARGS((Object_ptr object, void* dummy));

static int
prop_db_prop_parse_from_arg_and_add ARGS((PropDb_ptr self,
                                          SymbTable_ptr symb_table,
                                          int argc, const char** argv,
                                          const Prop_Type type));
static const char*
prop_db_get_prop_type_as_parsing_string ARGS((PropDb_ptr self,
                                              const Prop_Type type));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The PropDb class constructor]

  Description        [The PropDb class constructor]

  SideEffects        []

  SeeAlso            [PropDb_destroy]

******************************************************************************/
PropDb_ptr PropDb_create()
{
  PropDb_ptr self = ALLOC(PropDb, 1);
  PROP_DB_CHECK_INSTANCE(self);

  prop_db_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [The PropDb class destructor]

  Description        [The PropDb class destructor]

  SideEffects        []

  SeeAlso            [PropDb_create]

******************************************************************************/
void PropDb_destroy(PropDb_ptr self)
{
  PROP_DB_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/**Function********************************************************************

  Synopsis           [Disposes the DB of properties]

  Description        [Disposes the DB of properties]

  SideEffects        []

******************************************************************************/
void PropDb_clean(PropDb_ptr self)
{
  PROP_DB_CHECK_INSTANCE(self);
  prop_db_deinit(self);
  prop_db_init(self);
}


/**Function********************************************************************

  Synopsis           [Fills the DB of properties]

  Description        [Given for each kind of property a list of
  respective formulae, this function is responsible to fill the DB with
  them. Returns 1 if an error occurred, 0 otherwise]

  SideEffects        []

******************************************************************************/
int PropDb_fill(PropDb_ptr self, SymbTable_ptr symb_table,
                node_ptr ctlspec, node_ptr computespec,
                node_ptr ltlspec, node_ptr pslspec,
                node_ptr invarspec)
{
  node_ptr l;
  int res;
  Prop_ptr prop;

  PROP_DB_CHECK_INSTANCE(self);
  SYMB_TABLE_CHECK_INSTANCE(symb_table);

  /* [AM] Named specs
   * Named:
   *    [LTL;PSL;COMPUTE;INVAR;]SPEC
   *       /                 \
   *      CONTEXT            [p_name (DOT/ATOM)]
   *     /    \
   *   ...    ...
   */

  for (l = ctlspec; l != Nil; l = cdr(l)) {
    res = PropDb_prop_create_and_add(self, symb_table, car(car(l)),
                                     Prop_Ctl);
    if (res == -1) return 1;
    if (Nil != cdr(car(l))){
      prop = PropDb_get_prop_at_index(self, res);
      Prop_set_name(prop, cdr(car(l)));
    }
  }
  for (l = computespec; l != Nil; l = cdr(l)) {
    res = PropDb_prop_create_and_add(self, symb_table, car(car(l)),
                                     Prop_Compute);
    if (res == -1) return 1;
    if (Nil != cdr(car(l))){
      prop = PropDb_get_prop_at_index(self, res);
      Prop_set_name(prop, cdr(car(l)));
    }
  }
  for (l = ltlspec; l != Nil; l = cdr(l)) {
    res = PropDb_prop_create_and_add(self, symb_table, car(car(l)),
                                     Prop_Ltl);
    if (res == -1) return 1;
    if (Nil != cdr(car(l))){
      prop = PropDb_get_prop_at_index(self, res);
      Prop_set_name(prop, cdr(car(l)));
    }
  }
  for (l = pslspec; l != Nil; l = cdr(l)) {
    res = PropDb_prop_create_and_add(self, symb_table,
                                     car(car(l)), Prop_Psl);
    if (res == -1) return 1;
    if (Nil != cdr(car(l))){
      prop = PropDb_get_prop_at_index(self, res);
      Prop_set_name(prop, cdr(car(l)));
    }
  }
  for (l = invarspec; l != Nil; l = cdr(l)) {
    res = PropDb_prop_create_and_add(self, symb_table, car(car(l)),
                                     Prop_Invar);
    if (res == -1) return 1;
    if (Nil != cdr(car(l))){
      prop = PropDb_get_prop_at_index(self, res);
      Prop_set_name(prop, cdr(car(l)));
    }
  }

  return 0;
}


/**Function********************************************************************

  Synopsis           [Inserts a property in the DB of properties]

  Description        [Insert a property in the DB of properties.
  If not previously set, sets the property index.
  Returns true if out of memory]

  SideEffects        []

******************************************************************************/
boolean PropDb_add(PropDb_ptr self, Prop_ptr p)
{
  PROP_DB_CHECK_INSTANCE(self);
  PROP_CHECK_INSTANCE(p);

  if (Prop_get_index(p) == -1) {
    Prop_set_index(p, PropDb_get_size(self));
  }
  return (array_insert_last(Prop_ptr, self->prop_database, p) ==
          ARRAY_OUT_OF_MEM);
}


/**Function********************************************************************

  Synopsis           [Inserts a property in the DB of properties]

  Description        [Given a formula and its type, a property is
  created and stored in the DB of properties. It returns either -1 in
  case of failure, or the index of the inserted property.
  ]

  SideEffects        []

******************************************************************************/
VIRTUAL int PropDb_prop_create_and_add(PropDb_ptr self,
                                       SymbTable_ptr symb_table,
                                       node_ptr spec,
                                       Prop_Type type)
{
  PROP_DB_CHECK_INSTANCE(self);
  SYMB_TABLE_CHECK_INSTANCE(symb_table);
  return self->prop_create_and_add(self, symb_table, spec, type);
}


/**Function********************************************************************

  Synopsis           [Returns the last entered property in the DB]

  Description        [Returns the last entered property in the DB of
                      properties.]

  SideEffects        []

******************************************************************************/
Prop_ptr PropDb_get_last(const PropDb_ptr self)
{
  PROP_DB_CHECK_INSTANCE(self);
  return array_fetch_last(Prop_ptr, self->prop_database);
}


/**Function********************************************************************

  Synopsis           [Returns the property indexed by index]

  Description        [Returns the property whose unique identifier is
  provided in input. Returns NULL if not found.]

  SideEffects        []

******************************************************************************/
Prop_ptr PropDb_get_prop_at_index(const PropDb_ptr self, int index)
{
  Prop_ptr res;
  PROP_DB_CHECK_INSTANCE(self);

  if (index >= array_n(self->prop_database)) res = PROP(NULL);
  else res = array_fetch(Prop_ptr, self->prop_database, index);

  return res;
}


/**Function********************************************************************

  Synopsis           [Returns the property with the given name]

  Description        [Returns the property with the given name, rapresented
                      as flattened nodes hierarchy, -1 if not found.]

  SideEffects        []

******************************************************************************/
int PropDb_get_prop_name_index(const PropDb_ptr self, const node_ptr name)
{
  int i;

  PROP_DB_CHECK_INSTANCE(self);

  for (i = 0; i < PropDb_get_size(self); ++i) {
    Prop_ptr prop = PropDb_get_prop_at_index(self, i);
    if (Prop_get_name(prop) == name) return i;
  }

  return -1; /* not found */
}


/**Function********************************************************************

  Synopsis           [Returns the size of the DB]

  Description        [Returns the size (i.e. the number of entries)
  stored in the DB of properties.]

  SideEffects        []

******************************************************************************/
int PropDb_get_size(const PropDb_ptr self)
{
  PROP_DB_CHECK_INSTANCE(self);
  return array_n(self->prop_database);
}


/**Function********************************************************************

  Synopsis           [Returns the master property]

  Description        [Returned property does NOT belong to the caller]

  SideEffects        []

******************************************************************************/
Prop_ptr PropDb_get_master(const PropDb_ptr self)
{
  PROP_DB_CHECK_INSTANCE(self);
  return self->master;
}


/**Function********************************************************************

  Synopsis           [Sets the master property]

  Description        [Passed property no-longer belongs to the caller.
  Returns the previoulsy set property]

  SideEffects        []

******************************************************************************/
Prop_ptr PropDb_set_master(PropDb_ptr self, Prop_ptr prop)
{
  Prop_ptr old;

  PROP_DB_CHECK_INSTANCE(self);

  old = self->master;
  self->master = prop;
  return old;
}


/**Function********************************************************************

  Synopsis           [Returns the scalar FSM]

  Description        [Returns the scalar FSM stored in the master
                      property. Returned instance DOES not belong
                      to the caller.]

  SideEffects        []

******************************************************************************/
SexpFsm_ptr PropDb_master_get_scalar_sexp_fsm(const PropDb_ptr self)
{
  PROP_DB_CHECK_INSTANCE(self);
  return Prop_get_scalar_sexp_fsm(self->master);
}


/**Function********************************************************************

  Synopsis           [Set the scalar FSM]

  Description        [Set the scalar FSM of the master prop. This method
                      destroys the previously set FSM if any. self
                      becomes the owner of the fsm, and the passed
                      fsm no longer belongs to the caller.]

  SideEffects        []

******************************************************************************/
void PropDb_master_set_scalar_sexp_fsm(PropDb_ptr self, SexpFsm_ptr fsm)
{
  PROP_DB_CHECK_INSTANCE(self);
  prop_set_scalar_sexp_fsm(self->master, fsm, false /*do not duplicate*/);
}


/**Function********************************************************************

  Synopsis           [Returns the boolean FSM in sexp]

  Description [Returns the boolean FSM in sexp stored in the master
  prop. self becomes the owner of the given fsm. The
  returned value may be NULL when coi is enabled]

  SideEffects        []

******************************************************************************/
BoolSexpFsm_ptr PropDb_master_get_bool_sexp_fsm(const PropDb_ptr self)
{
  PROP_DB_CHECK_INSTANCE(self);
  return Prop_get_bool_sexp_fsm(self->master);
}


/**Function********************************************************************

  Synopsis           [Set the boolean FSM in sexp]

  Description        [Set the boolean FSM in sexp of the master prop. The
  prop package becomes the owner of the given fsm. This method
  destroys the previously set FSM if any.]

  SideEffects        []

******************************************************************************/
void PropDb_master_set_bool_sexp_fsm(PropDb_ptr self, BoolSexpFsm_ptr fsm)
{
  PROP_DB_CHECK_INSTANCE(self);
  prop_set_bool_sexp_fsm(self->master, fsm, false /*do not duplicate*/);
}


/**Function********************************************************************

  Synopsis           [Returns the boolean FSM in BDD]

  Description        [Returns the boolean FSM in BDD stored in the master
                      prop. The returned value may be NULL when coi
                      is enabled. Returned fsm belongs to self, and
                      NOT to the caller.]

  SideEffects        []

******************************************************************************/
BddFsm_ptr PropDb_master_get_bdd_fsm(const PropDb_ptr self)
{
  PROP_DB_CHECK_INSTANCE(self);
  return Prop_get_bdd_fsm(self->master);
}



/**Function********************************************************************

  Synopsis           [Set the boolean FSM in BDD]

  Description        [Set the boolean FSM in BDD of the master prop. self
                      becomes the owner of the given fsm. This
                      method destroys the previously set FSM if any.]

  SideEffects        []

******************************************************************************/
void PropDb_master_set_bdd_fsm(PropDb_ptr self, BddFsm_ptr fsm)
{
  PROP_DB_CHECK_INSTANCE(self);
  prop_set_bdd_fsm(self->master, fsm, false /*do not duplicate*/);
}


/**Function********************************************************************

  Synopsis           [Returns the boolean FSM in BE]

  Description        [Returns the boolean FSM in BE stored in the master
                      prop. The returned value may be NULL when coi
                      is enabled. Returned fsm belongs to self]

  SideEffects        []

******************************************************************************/
BeFsm_ptr PropDb_master_get_be_fsm(const PropDb_ptr self)
{
  PROP_DB_CHECK_INSTANCE(self);
  return Prop_get_be_fsm(self->master);
}


/**Function********************************************************************

  Synopsis           [Set the boolean FSM in BE]

  Description        [Set the boolean FSM in BE of the master prop. self
                      becomes the owner of the given fsm. This
                      method destroys the previously set FSM if any.]

  SideEffects        []

******************************************************************************/
void PropDb_master_set_be_fsm(PropDb_ptr self, BeFsm_ptr fsm)
{
  PROP_DB_CHECK_INSTANCE(self);
  prop_set_be_fsm(self->master, fsm, false /*do not duplicate*/);
}


/**Function********************************************************************

  Synopsis           [Copies master prop FSM data into prop]

  Description        [Copies the FSM informations stored in the master
  prop into the corresponding fields of the given prop structure.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
VIRTUAL void PropDb_set_fsm_to_master(PropDb_ptr self, Prop_ptr prop)
{
  PROP_DB_CHECK_INSTANCE(self);
  self->set_fsm_to_master(self, prop);
}


/**Function********************************************************************

  Synopsis           [Returns the currently set print format]

  Description        [When printing, the currenlty set format is used.]

  SideEffects        []

  SeeAlso            [PropDb_set_print_fmt]

******************************************************************************/
PropDb_PrintFmt PropDb_get_print_fmt(const PropDb_ptr self)
{
  PROP_DB_CHECK_INSTANCE(self);
  return self->print_fmt;
}


/**Function********************************************************************

  Synopsis           [Sets the current print format]

  Description        [When printing, the given format will be used.
  Returns the previously set format.]

  SideEffects        []

  SeeAlso            [PropDb_get_print_fmt]

******************************************************************************/
PropDb_PrintFmt PropDb_set_print_fmt(const PropDb_ptr self,
                                     PropDb_PrintFmt new_fmt)
{
  PropDb_PrintFmt old;

  PROP_DB_CHECK_INSTANCE(self);

  old = self->print_fmt;
  self->print_fmt = new_fmt;
  return old;
}


/**Function********************************************************************

  Synopsis           [Prints the header of the property list]

  Description        [This method has to be called before
  PropDb_print_list_footer.]

  SideEffects        [PropDb_print_list_footer]

******************************************************************************/
void PropDb_print_list_header(const PropDb_ptr self, FILE* file)
{
  PROP_DB_CHECK_INSTANCE(self);

  switch (PropDb_get_print_fmt(self)) {
  case PROPDB_PRINT_FMT_TABULAR:
    fprintf(file,
            "**** PROPERTY LIST [ Type, Status, Counter-example Number, Name ] ****\n");
    fprintf(file,
            "--------------------------  PROPERTY LIST  -------------------------\n");
    break;
    
  case PROPDB_PRINT_FMT_XML:
    fprintf(file, "<?xml version=\"1.0\"?>\n");
    fprintf(file, "<properties xmlns=\"http://es.fbk.eu\"\n");
    fprintf(file, "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
    fprintf(file, "xsi:schemaLocation=\"http://es.fbk.eu/xsd properties.xsd\">\n\n");
    break;

  default:
    internal_error("Unsupported prop print format");
  }
}


/**Function********************************************************************

  Synopsis           [Prints the footer of the property list]

  Description        [This method has to be called after
  PropDb_print_list_header.]

  SideEffects        [PropDb_print_list_header]

******************************************************************************/
void PropDb_print_list_footer(const PropDb_ptr self, FILE* file)
{
  PROP_DB_CHECK_INSTANCE(self);

  switch (PropDb_get_print_fmt(self)) {
  case PROPDB_PRINT_FMT_TABULAR:
    break;
    
  case PROPDB_PRINT_FMT_XML:
    fprintf(file, "</properties>\n");
    break;

  default:
    internal_error("Unsupported prop print format");
  }
}


/**Function********************************************************************

  Synopsis           [Prints the specified property from the DB]

  Description        [Prints on the given file stream the property
  whose unique identifier is specified]

  SideEffects        []

******************************************************************************/
int PropDb_print_prop_at_index(const PropDb_ptr self,
                               FILE* file, const int index)
{
  int retval;
  Prop_ptr prop;

  PROP_DB_CHECK_INSTANCE(self);

  prop = PropDb_get_prop_at_index(self, index);
  if (prop != PROP(NULL)) {
    Prop_print_db(prop, file, PropDb_get_print_fmt(self));
    retval = 0;
  }
  else {
    retval = 1;
  }

  return retval;
}


/**Function********************************************************************

  Synopsis           [Prints all the properties stored in the DB]

  Description        [Prints on the given file stream all the property
  stored in the DB of properties.]

  SideEffects        []

******************************************************************************/
void PropDb_print_all(const PropDb_ptr self, FILE* file)
{
  PROP_DB_CHECK_INSTANCE(self);

  PropDb_print_all_status_type(self, file, Prop_NoStatus, Prop_NoType);

  if (PropDb_get_size(self) == 0) {
    switch (PropDb_get_print_fmt(self)) {
    case PROPDB_PRINT_FMT_TABULAR:
      fprintf(file, "The properties DB is empty.\n");
      break;
    case PROPDB_PRINT_FMT_XML:
      fprintf(file, "  <!-- The properties DB is empty. -->\n");
      break;
    default:
      internal_error("Invalid print format");
    }
  }
}


/**Function********************************************************************

  Synopsis           [Prints all the properties stored in the DB]

  Description        [Prints on the given file stream all the property
  stored in the DB of properties whose type and status match the
  requested ones. Prop_NoStatus and Prop_NoType serve as wildcards.]

  SideEffects        []

******************************************************************************/
void PropDb_print_all_status_type(const PropDb_ptr self, FILE* file,
                                  Prop_Status status, Prop_Type type)
{
  int i;

  PROP_DB_CHECK_INSTANCE(self);

  for (i = 0; i < PropDb_get_size(self); ++i) {
    Prop_ptr p = PropDb_get_prop_at_index(self, i);

    if (((type == Prop_NoType) || (Prop_get_type(p) == type)) &&
        ((status == Prop_NoStatus) || (Prop_get_status(p) == status))) {
      Prop_print_db(p, file, PropDb_get_print_fmt(self));
    }
  }
}


/**Function********************************************************************

  Synopsis           [Prints all the properties stored in the DB]

  Description        [Prints on the given file stream all the property
  stored in the DB of properties whose type match the requested one.]

  SideEffects        []

******************************************************************************/
void PropDb_print_all_type(const PropDb_ptr self, FILE* file, Prop_Type type)
{
  PROP_DB_CHECK_INSTANCE(self);

  PropDb_print_all_status_type(self, file, Prop_NoStatus, type);
}


/**Function********************************************************************

  Synopsis           [Prints all the properties stored in the DB]

  Description        [Prints on the given file stream all the property
  stored in the DB of properties whose status match the requested one.]

  SideEffects        []

******************************************************************************/
void PropDb_print_all_status(const PropDb_ptr self,
                             FILE* file, Prop_Status status)
{
  PROP_DB_CHECK_INSTANCE(self);

  PropDb_print_all_status_type(self, file, status, Prop_NoType);
}


/**Function********************************************************************

  Synopsis           [Return the list of properties of a given type,
                      ordered by COI size]

  Synopsis           [Given a property type returns the list of properties
  of that type currently located into the property database]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
lsList PropDb_get_ordered_props_of_type(const PropDb_ptr self,
                                        const FlatHierarchy_ptr hierarchy,
                                        const Prop_Type type)
{
  NodeList_ptr list;
  lsList result;
  ListIter_ptr iter;

  PROP_DB_CHECK_INSTANCE(self);

  result = lsCreate();
  nusmv_assert((lsList) NULL != result);

  list = PropDb_get_ordered_properties(self, hierarchy);
  NODE_LIST_FOREACH(list, iter) {
    node_ptr couple = NodeList_get_elem_at(list, iter);
    Prop_ptr prop = PROP(car(couple));

    if (Prop_get_type(prop) == type) {
      lsNewEnd(result, (lsGeneric)prop, LS_NH);
    }

    Set_ReleaseSet((Set_t)cdr(couple));
    free_node(couple);
  }

  NodeList_destroy(list);
  return result;
}


/**Function********************************************************************

  Synopsis           [Return the list of properties of a given type]

  Synopsis           [Given a property type returns the list of properties
  of that type currently located into the property database]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
lsList PropDb_get_props_of_type(const PropDb_ptr self, const Prop_Type type)
{
  lsList result;
  int i;

  PROP_DB_CHECK_INSTANCE(self);

  result = lsCreate();
  nusmv_assert((lsList) NULL != result);

  for (i=0; i < PropDb_get_size(self); ++i) {
    Prop_ptr p = PropDb_get_prop_at_index(self, i);

    if (Prop_get_type(p) == type) {
      lsNewEnd(result, (lsGeneric)p, LS_NH);
    }
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [Add a property to the database from a string and a type]

  Description        [Parses and creates a property of a given type from
  a string. If the formula is correct, it is added to the
  property database and its index is returned.
  Otherwise, -1 is returned.
  Valid types are Prop_Ctl, Prop_Ltl, Prop_Psl, Prop_Invar and Prop_Compute.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int PropDb_prop_parse_and_add(const PropDb_ptr self,
                              SymbTable_ptr symb_table,
                              const char* str, const Prop_Type type)
{
  const char* argv[2];
  int argc = 2;

  PROP_DB_CHECK_INSTANCE(self);
  nusmv_assert(str != (char*) NULL);

  argv[0] = (char*) NULL;
  argv[1] = (char*) str;

  return prop_db_prop_parse_from_arg_and_add(self, symb_table,
                                             argc, argv, type);
}

/**Function********************************************************************

  Synopsis           [Given a string representing a property name,
                      returns the property index, if it exists]

  Description        [Parses the given name, builds it's node_ptr
                      interpretation and looks into the PropDb if the
                      property exists.]

  SideEffects        []

  SeeAlso            [PropDb_get_prop_name_index]

******************************************************************************/
int PropDb_prop_parse_name(const PropDb_ptr self, const char* str)
{
  node_ptr property;
  node_ptr parsed_command = Nil;
  int parse_result;

  PROP_DB_CHECK_INSTANCE(self);
  nusmv_assert(str != (char*) NULL);

  parse_result = Parser_ReadIdentifierExprFromString(str, &parsed_command);

  if (parse_result != 0 || parsed_command == Nil) {
    fprintf(nusmv_stderr,
            "Parsing error: expected a property name.\n");
    return -1;
  }

  property = car(parsed_command);
  property = CompileFlatten_concat_contexts(Nil, property);

  return PropDb_get_prop_name_index(self, property);
}


/**Function********************************************************************

  Synopsis           [Get a valid property index from a string]

  Description        [Gets the index of a property form a string.
  If the string does not contain a valid index, an error message is emitted
  and -1 is returned.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int PropDb_get_prop_index_from_string(const PropDb_ptr self, const char* idx)
{
  int idxTemp, db_size;

  PROP_DB_CHECK_INSTANCE(self);

  db_size = PropDb_get_size(self);
  if ( db_size <= 0 ) {
    if (cmp_struct_get_flatten_hrc(cmps) == 0) {
      fprintf(nusmv_stderr,
              "The hierarchy must be flattened before. "\
              "Use the \"flatten_hierarchy\" command.\n");
    }
    else {
      fprintf(nusmv_stderr,"Error: there isn\'t any property available.\n");
    }
    return -1;
  }

  if (util_str2int(idx, &idxTemp) != 0) {
    fprintf(nusmv_stderr,
            "Error: property index \"%s\" is not a valid value "\
            "(must be integer).\n", idx);
    return -1;
  }

  if ( (idxTemp < 0) || (idxTemp >= db_size) ) {
    fprintf(nusmv_stderr,
            "Error: property index \"%d\" is not valid (must be in "\
            "the range [0,%d]).\n",
            idxTemp, db_size-1);
    return -1;
  }

  return idxTemp;
}


/**Function********************************************************************

  Synopsis           [Returns the index of the property associated to a trace.]

  Description        [Returns the index of the property associated to a trace.
  -1 if no property is associated to the given trace.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int PropDb_get_prop_index_from_trace_index(const PropDb_ptr self,
                                           const int trace_idx)
{
  int i, result;

  PROP_DB_CHECK_INSTANCE(self);

  result = -1;
  for (i=0; i < PropDb_get_size(self); ++i) {
    Prop_ptr prop = PropDb_get_prop_at_index(self, i);

    if (Prop_get_trace(prop) == trace_idx) {
      result = trace_idx;
    }
  }

  return result;
}


/**Function********************************************************************
  Synopsis           [Verifies a given property]

  Description        [The DB of properties is searched for a property
  whose unique identifier match the identifier provided and then if
  such a property exists it will be verified calling the appropriate
  model checking algorithm. If the property was checked before, then
  the property is not checked again.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void PropDb_verify_prop_at_index(const PropDb_ptr self, const int index)
{
  int size;
  PROP_DB_CHECK_INSTANCE(self);

  size = PropDb_get_size(self);
  if (size < index) {
    fprintf(nusmv_stderr,
            "Property indexed by %d not present in the database.\n", index);
    fprintf(nusmv_stderr,
            "Valid index are in the range [0..%d]\n", size-1);
    nusmv_exit(1);
  }
  else {
    Prop_ptr p = PropDb_get_prop_at_index(self, index);
    Prop_verify(p);
  }
}


/**Function********************************************************************

  Synopsis           [Verifies all properties of a given type]

  Description        [The DB of properties is searched for a property
  of the given type. All the found properties are then verified
  calling the appropriate model checking algorithm. Properties already
  checked will be ignored.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void PropDb_verify_all_type(const PropDb_ptr self, Prop_Type type)
{
  int i;
  PROP_DB_CHECK_INSTANCE(self);

  for (i=0; i < PropDb_get_size(self); ++i) {
    Prop_ptr p = PropDb_get_prop_at_index(self, i);
    if (Prop_get_type(p) == type) Prop_verify(p);
  }
}


/**Function********************************************************************

  Synopsis           [Verifies all the properties in the DB]

  Description        [All the properties stored in the database not
  yet verified will be verified. The properties are verified following
  the order CTL/COMPUTE/LTL/INVAR.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
VIRTUAL void PropDb_verify_all(const PropDb_ptr self)
{
  PROP_DB_CHECK_INSTANCE(self);
  self->verify_all(self);
}


/**Function********************************************************************

  Synopsis           [Verifies all the properties in the DB]

  Description        [All the properties stored in the database not
  yet verified will be verified. The properties are verified following
  the COI size order (from smaller to bigger)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void PropDb_ordered_verify_all(const PropDb_ptr self,
                               const FlatHierarchy_ptr hierarchy)
{
  PROP_DB_CHECK_INSTANCE(self);
  PropDb_ordered_verify_all_type(self, hierarchy, Prop_NoType);
}


/**Function********************************************************************

  Synopsis           [Verifies all properties of a given type, ordered by COI
                      size]

  Description        [The DB of properties is searched for a property
  of the given type. All the found properties are then verified
  calling the appropriate model checking algorithm. Properties already
  checked will be ignored. Properties found with the given type are checked
  in order, based on the COI size. If type is Prop_NoType, all properties
  are checked]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void PropDb_ordered_verify_all_type(const PropDb_ptr self,
                                    const FlatHierarchy_ptr hierarchy,
                                    const Prop_Type type)
{
  NodeList_ptr list;
  ListIter_ptr iter;

  PROP_DB_CHECK_INSTANCE(self);

  list = PropDb_get_ordered_properties(self, hierarchy);
  NODE_LIST_FOREACH(list, iter) {
    node_ptr couple = NodeList_get_elem_at(list, iter);
    Prop_ptr prop = PROP(car(couple));

    if ((Prop_NoType == type) || (Prop_get_type(prop) == type)) {
      Prop_verify(prop);
    }

    Set_ReleaseSet((Set_t)cdr(couple));
    free_node(couple);
  }

  NodeList_destroy(list);
}


/**Function********************************************************************

  Synopsis           [Get the list of properties ordered by COI size]

  Description        [Get the list of properties ordered by COI size.

                      List elements are couples made using cons: the
                      car part points to the property, while the cdr
                      part points to the COI. The list and it's
                      elements (cons nodes and COI sets) should be
                      freed by the caller]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr PropDb_get_ordered_properties(const PropDb_ptr self,
                                           const FlatHierarchy_ptr hierarchy)
{
  SymbTable_ptr symb_table;
  NodeList_ptr res;
  int i;

  PROP_DB_CHECK_INSTANCE(self);

  symb_table = FlatHierarchy_get_symb_table(hierarchy);
  res = NodeList_create();

  if (opt_verbose_level_ge(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Ordering properties by COI size\n");
  }

  for (i = 0; i < PropDb_get_size(self); ++i) {
    Prop_ptr prop = PropDb_get_prop_at_index(self, i);
    Set_t cone = Prop_compute_cone(prop, hierarchy, symb_table);
    int card = Set_GiveCardinality(cone);
    boolean inserted = false;
    /* Order insert into the list */
    node_ptr new_entry = cons(NODE_PTR(prop), NODE_PTR(cone));

    ListIter_ptr iter;
    NODE_LIST_FOREACH(res, iter) {
      node_ptr entry = NodeList_get_elem_at(res, iter);
      Set_t entry_cone = (Set_t)cdr(entry);

      if (Set_GiveCardinality(entry_cone) > card) {
        NodeList_insert_before(res, iter, new_entry);
        inserted = true;
        break;
      }
    }

    if (!inserted) NodeList_append(res, new_entry);
  }

  if (opt_verbose_level_ge(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Properties ordering done\n");
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Get the list of properties, grouped by COI]

  Description        [Get the list of properties, grouped by COI.
                      A list of couples is returned. The left part of
                      the couple is the COI (represented as a
                      Set_t). The right part of the couple is a Set
                      containing all properties with that COI.  The
                      returned list is ordered by COI size.  The list,
                      all couples and all sets should be freed by the
                      caller ]

  SideEffects        []

  SeeAlso            [PropDb_get_ordered_properties]

******************************************************************************/
NodeList_ptr
PropDb_get_coi_grouped_properties(const PropDb_ptr self,
                                  const FlatHierarchy_ptr hierarchy)
{
  NodeList_ptr result, order;
  ListIter_ptr iter;

  PROP_DB_CHECK_INSTANCE(self);

  result = NodeList_create();
  order = PropDb_get_ordered_properties(self, hierarchy);
  NODE_LIST_FOREACH(order, iter) {
    boolean found = false;
    ListIter_ptr res_iter;
    node_ptr entry = NodeList_get_elem_at(order, iter);
    Set_t cone = (Set_t)cdr(entry);
    Prop_ptr prop = PROP(car(entry));

    NODE_LIST_FOREACH(result, res_iter) {
      node_ptr res_entry = NodeList_get_elem_at(result, res_iter);
      Set_t props = (Set_t)cdr(res_entry);
      Set_t res_cone = (Set_t)car(res_entry);
      if (Set_Equals(res_cone, cone)) {
        props = Set_AddMember(props, (Set_Element_t)prop);
        setcdr(res_entry, NODE_PTR(props));
        found = true;
        break;
      }
    }

    if (!found) {
      Set_t props = Set_MakeSingleton((Set_Element_t)prop);
      node_ptr new_entry = cons(NODE_PTR(cone), NODE_PTR(props));
      NodeList_append(result, new_entry);
    }
    else {
      Set_ReleaseSet(cone);
    }

    free_node(entry);
  }

  return result;
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The PropDb class private initializer]

  Description        [The PropDb class private initializer]

  SideEffects        []

  SeeAlso            [PropDb_create]

******************************************************************************/
void prop_db_init(PropDb_ptr self)
{
  /* base class initialization */
  object_init(OBJECT(self));

  /* members initialization */
  self->prop_database = array_alloc(Prop_ptr, 1);
  assert((array_t*) NULL != self->prop_database);
  self->master = Prop_create();

  self->print_fmt = PROPDB_PRINT_FMT_DEFAULT;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = prop_db_finalize;
  OVERRIDE(PropDb, prop_create_and_add) = prop_db_prop_create_and_add;
  OVERRIDE(PropDb, set_fsm_to_master) = prop_db_set_fsm_to_master;
  OVERRIDE(PropDb, verify_all) = prop_db_verify_all;
}


/**Function********************************************************************

  Synopsis           [The PropDb class private deinitializer]

  Description        [The PropDb class private deinitializer]

  SideEffects        []

  SeeAlso            [PropDb_destroy]

******************************************************************************/
void prop_db_deinit(PropDb_ptr self)
{
  /* members deinitialization */
  int i;

  for (i = 0; i < PropDb_get_size(self); ++i) {
    Prop_ptr prop = PropDb_get_prop_at_index(self, i);
    Prop_destroy(prop);
    /* if prop master is a property in the database as well */
    if (PropDb_get_master(self) == prop) {
      PropDb_set_master(self, (Prop_ptr) NULL);
    }
  }
  array_free(self->prop_database);
  {
    Prop_ptr m = PropDb_get_master(self);
    if (m != PROP(NULL)) {
      Prop_destroy(m);
      PropDb_set_master(self, (Prop_ptr) NULL);
    }
  }

  /* base class deinitialization */
  object_deinit(OBJECT(self));
}


/**Function********************************************************************

  Synopsis           [Inserts a property in the DB of properties]

  Description        [Given a formula and its type, a property is
  created and stored in the DB of properties. It returns either -1 in
  case of failure, or the index of the inserted property.
  ]

  SideEffects        []

******************************************************************************/
int prop_db_prop_create_and_add(PropDb_ptr self, SymbTable_ptr symb_table,
                                node_ptr spec, Prop_Type type)
{
  int retval, index;
  boolean allow_adding, allow_checking, is_ctl;
  Prop_ptr prop;

  retval = 0;
  index = PropDb_get_size(self);
  allow_adding = true;
  allow_checking = true;
  is_ctl = (type == Prop_Ctl);
  prop = NULL;

  /* PSL properties need to be converted to CTL or LTL specifications */
  if (type == Prop_Psl) {
    PslNode_ptr psl_prop = PslNode_convert_from_node_ptr(spec);
    /* removal of forall */
    psl_prop = PslNode_remove_forall_replicators(psl_prop);
    if (!PslNode_is_handled_psl(psl_prop)) {
      /* here the property may be either OBE or unmanageable */
      if (PslNode_is_obe(psl_prop)) is_ctl = true;
      else {
        /* it is not supported */
        warning_psl_not_supported_feature(spec, index);
        allow_checking = false;
      }
    }
  }

  prop = Prop_create_partial(spec, type);

  Prop_set_index(prop, index);

  if (allow_checking) {
    if (!TypeChecker_check_property(SymbTable_get_type_checker(symb_table),
                                    prop)) {
      fprintf(nusmv_stderr, "ERROR: Property \"");
      Prop_print(prop, nusmv_stderr,
                 get_prop_print_method(OptsHandler_get_instance()));
      fprintf(nusmv_stderr, "\b\" is not correct or not well typed.\n");
      return -1; /* type violation */
    }

    /* Checks for input vars */
    if (is_ctl || (type == Prop_Compute)) {
      Set_t expr_vars;

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 5)) {
        fprintf(nusmv_stdout,
                "Checking %s property (index %d) for input variables. \n",
                Prop_get_type_as_string(prop), index);
      }

      /* Get list of variables in the expression, and check for inputs */
      expr_vars = Formula_GetDependencies(symb_table,
                                          Prop_get_expr_core(prop),
                                          Nil);

      allow_adding = !SymbTable_list_contains_input_var(symb_table, Set_Set2List(expr_vars));
      Set_ReleaseSet(expr_vars);
    }

    /* Check for next operators. Only invarspecs can contain next
       operators */
    Compile_check_next(symb_table, Prop_get_expr_core(prop), Nil,
                       (Prop_Invar == type));
  }

  /* If no input vars present then add property to database */
  if (allow_adding) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
      fprintf( nusmv_stdout,
               "Attempting to add %s property (index %d) to property list.\n",
               Prop_get_type_as_string(prop), index);
    }
    retval = PropDb_add(self, prop);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
      if (retval == 1) {
        fprintf(nusmv_stdout, \
                "Failing to add %s property (index %d) to property list.\n", \
                Prop_get_type_as_string(prop), index);
      }
      else {
        fprintf(nusmv_stdout, \
                "%s property (index %d) successfully added to property list.\n",\
                Prop_get_type_as_string(prop), index);
      }
    }
  }
  else {
    /* Property contains input variables */
    error_property_contains_input_vars(prop);
  }

  retval = (retval == 1) ? -1 : index;
  return retval;
}


/**Function********************************************************************

  Synopsis           [Copies master prop FSM data into prop]

  Description        [Copies the FSM informations stored in the master
  prop into the corresponding fields of the given prop structure.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void prop_db_set_fsm_to_master(PropDb_ptr self, Prop_ptr prop)
{
  Prop_set_scalar_sexp_fsm(prop, PropDb_master_get_scalar_sexp_fsm(self));
  Prop_set_bool_sexp_fsm(prop, PropDb_master_get_bool_sexp_fsm(self));
  Prop_set_bdd_fsm(prop, PropDb_master_get_bdd_fsm(self));
  Prop_set_be_fsm(prop, PropDb_master_get_be_fsm(self));
}


/**Function********************************************************************

  Synopsis           [Verifies all the properties in the DB]

  Description        [All the properties stored in the database not
  yet verified will be verified. The properties are verified following
  the order CTL/COMPUTE/LTL/INVAR.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void prop_db_verify_all(const PropDb_ptr self)
{
  PropDb_verify_all_type(self, Prop_Ctl);
  PropDb_verify_all_type(self, Prop_Compute);
  PropDb_verify_all_type(self, Prop_Ltl);
  PropDb_verify_all_type(self, Prop_Psl);
  PropDb_verify_all_type(self, Prop_Invar);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The PropDb class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void prop_db_finalize(Object_ptr object, void* dummy)
{
  PropDb_ptr self = PROP_DB(object);

  prop_db_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Add a property to the database from an arg structure
  and a type]

  Description        [Parses and creates a property of a given type from
  an arg structure. If the formula is correct, it is added to the
  property database and its index is returned.
  Otherwise, -1 is returned.
  Valid types are Prop_Ctl, Prop_Ltl, Prop_Psl, Prop_Invar and Prop_Compute.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
prop_db_prop_parse_from_arg_and_add(PropDb_ptr self,
                                    SymbTable_ptr symb_table,
                                    int argc, const char** argv,
                                    const Prop_Type type)
{
  switch (type) {
  case Prop_Ctl:
  case Prop_Ltl:
  case Prop_Psl:
  case Prop_Invar:
  case Prop_Compute:
    /* All ok */
    break;

    /* Property name given as command argument, should use PropDb_prop_parse_name */
  case Prop_CompId:
    {
      fprintf(nusmv_stderr, "Required to parse a property of Prop_CompId. "
              "Use PropDb_prop_parse_name instead\n");
      return -1;
    }
    break;

  case Prop_NoType:
    fprintf(nusmv_stderr, "Required to parse a property of unknonw type.\n");
    return -1;
    break;

  default:
    fprintf(nusmv_stderr, "Required to parse a property of unsupported type.\n");
    return -1;
    break;
  } /* switch */

  {
    node_ptr property;
    node_ptr parsed_command = Nil;

    if (type != Prop_Psl) {
      const char* parsing_type =
        prop_db_get_prop_type_as_parsing_string(self, type);
      int parse_result = Parser_ReadCmdFromString(argc, argv,
                                                  (char*) parsing_type,
                                                  ";\n", &parsed_command);

      if (parse_result != 0 || parsed_command == Nil) {
        fprintf(nusmv_stderr,
                "Parsing error: expected an \"%s\" expression.\n",
                PropType_to_string(type));
        return -1;
      }
      property = car(parsed_command);
    }
    else {
      int parse_result = Parser_read_psl_from_string(argc, argv,
                                                     &parsed_command);
      if (parse_result != 0 || parsed_command == Nil) {
        fprintf(nusmv_stderr,
                "Parsing error: expected an \"%s\" expression.\n",
                PropType_to_string(type));
        return -1;
      }
      /* makes possible context absolute */
      if (node_get_type(parsed_command) == CONTEXT) {
        node_ptr new_ctx = CompileFlatten_concat_contexts(Nil, car(parsed_command));
        property = PslNode_new_context(new_ctx, cdr(parsed_command));
      }
      else {
        property = PslNode_new_context(NULL, parsed_command);
      }
    }

    return PropDb_prop_create_and_add(self, symb_table, property, type);
  }
}


/**Function********************************************************************

  Synopsis           [Returns the parsing type given the property type]

  Description        [Returns the parsing type given the property type.
  The returned string must NOT be freed.]

  SideEffects        []

******************************************************************************/
static const char*
prop_db_get_prop_type_as_parsing_string(PropDb_ptr self, const Prop_Type type)
{
  switch (type) {
  case Prop_NoType: break; /* to suppress compiler's warnings */
  case Prop_Ctl: return "CTLWFF ";
  case Prop_Ltl: return "LTLWFF ";
  case Prop_Psl: return "PSLWFF ";
  case Prop_Invar: return "NEXTWFF ";
  case Prop_Compute: return "COMPWFF ";
  case Prop_CompId:  return "COMPID ";
  default: break; /* to suppress compiler's warnings */
  }

  return "SIMPWFF ";
}


/**AutomaticEnd***************************************************************/
