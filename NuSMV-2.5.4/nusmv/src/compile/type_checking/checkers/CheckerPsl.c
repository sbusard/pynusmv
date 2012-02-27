/**CFile***********************************************************************

  FileName    [CheckerPsl.c]

  PackageName [compile.type_checking.checkers]

  Synopsis    [Implementaion of class 'CheckerPsl']

  Description []

  SeeAlso     [CheckerPsl.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compile.type_checking.checkers'' package of NuSMV version 2.
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

  Revision    [$Id: CheckerPsl.c,v 1.1.2.7.4.6 2009-10-15 19:02:01 nusmv Exp $]

******************************************************************************/

#include "CheckerPsl.h"
#include "CheckerPsl_private.h"

#include "compile/compile.h"
#include "compile/symb_table/symb_table.h"
#include "compile/symb_table/ResolveSymbol.h"

#include "parser/symbols.h"
#include "parser/psl/psl_symbols.h"
#include "parser/psl/pslNode.h"
#include "utils/utils.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: CheckerPsl.c,v 1.1.2.7.4.6 2009-10-15 19:02:01 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'CheckerPsl_private.h' for class 'CheckerPsl' definition. */

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

static void checker_psl_finalize ARGS((Object_ptr object, void* dummy));

static SymbType_ptr checker_psl_check_expr ARGS((CheckerBase_ptr self,
                                                 node_ptr e, node_ptr ctx));

static boolean
checker_psl_viol_handler ARGS((CheckerBase_ptr self,
                               TypeSystemViolation violation,
                               node_ptr expression));

static void print_operator ARGS((FILE* output_stream, PslNode_ptr expr));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The CheckerPsl class constructor]

  Description        [The CheckerPsl class constructor]

  SideEffects        []

  SeeAlso            [NodeWalker_destroy]

******************************************************************************/
CheckerPsl_ptr CheckerPsl_create()
{
  CheckerPsl_ptr self = ALLOC(CheckerPsl, 1);
  CHECKER_PSL_CHECK_INSTANCE(self);

  checker_psl_init(self);
  return self;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The CheckerPsl class private initializer]

  Description        [The CheckerPsl class private initializer]

  SideEffects        []

  SeeAlso            [CheckerPsl_create]

******************************************************************************/
void checker_psl_init(CheckerPsl_ptr self)
{
  /* base class initialization */
  checker_base_init(CHECKER_BASE(self), "PSL Type Checker",
                    NUSMV_PSL_SYMBOL_FIRST,
                    NUSMV_PSL_SYMBOL_LAST - NUSMV_PSL_SYMBOL_FIRST);

  /* members initialization */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = checker_psl_finalize;
  OVERRIDE(CheckerBase, check_expr) = checker_psl_check_expr;
  OVERRIDE(CheckerBase, viol_handler) = checker_psl_viol_handler;
}


/**Function********************************************************************

  Synopsis           [The CheckerPsl class private deinitializer]

  Description        [The CheckerPsl class private deinitializer]

  SideEffects        []

  SeeAlso            [CheckerPsl_destroy]

******************************************************************************/
void checker_psl_deinit(CheckerPsl_ptr self)
{
  /* members deinitialization */

  /* base class initialization */
  checker_base_deinit(CHECKER_BASE(self));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The CheckerPsl class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void checker_psl_finalize(Object_ptr object, void* dummy)
{
  CheckerPsl_ptr self = CHECKER_PSL(object);

  checker_psl_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static SymbType_ptr checker_psl_check_expr(CheckerBase_ptr self,
                                           node_ptr e, node_ptr ctx)
{
  /* converts and operates on PslNodes, not node_ptr: */
  PslNode_ptr expr = PslNode_convert_from_node_ptr(e);
  PslNode_ptr context = PslNode_convert_from_node_ptr(ctx);

  PslNode_ptr ctx_expr;

  /* wrap expr into the context. This is required by
     the facilities which remembers the type of expressions
     and by the violation handler. */
  if (context != PSL_NULL) {
    ctx_expr = PslNode_new_context(psl_node_context_to_main_context(context),
                                   expr);
  }
  else ctx_expr = expr;

  { /* checks memoizing */
    SymbType_ptr tmp = _GET_TYPE(ctx_expr);
    if (nullType != tmp) return tmp;
  }

  switch (psl_node_get_op(expr)) {
  case PSL_INF: return _SET_TYPE(ctx_expr, SymbTablePkg_integer_set_type());

  case PSL_SERE:
  case PSL_SERECOMPOUND:
    {
      SymbType_ptr type = _THROW(psl_node_get_left(expr), context);

      if (SymbType_is_error(type)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      if (SymbType_is_boolean(type)) return _SET_TYPE(ctx_expr, type);

      /* there is violation */
      if (_VIOLATION(SymbType_is_back_comp(type) ?
                     TC_VIOLATION_TYPE_BACK_COMP : TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }


      /* keeps the current type */
      return _SET_TYPE(ctx_expr, type);
    }

    /* concatenation and multiple concatenation */
  case PSL_CONCATENATION:
    {
      PslNode_ptr iter;
      SymbType_ptr left = SYMB_TYPE(NULL);
      SymbType_ptr right = SYMB_TYPE(NULL);
      boolean is_ok = true;

      if (psl_node_get_right(expr) != PSL_NULL) {
        /* multiple concatenation */
        right = _THROW(psl_node_get_right(expr), context);
        if (SymbType_is_error(right)) {
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }

        if (!SymbType_is_boolean(right)) {
          if (_VIOLATION(SymbType_is_back_comp(right) ?
                         TC_VIOLATION_TYPE_BACK_COMP :
                         TC_VIOLATION_TYPE_MANDATORY,
                         ctx_expr)) {
            return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
          }
        }
      }

      /* checks all the list's elements */
      iter = psl_node_get_left(expr);
      while (is_ok && iter != PSL_NULL) {
        left = _THROW(psl_node_get_left(iter), context);
        if (SymbType_is_error(left)) {
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }

        is_ok = SymbType_is_boolean(left);
        iter = psl_node_get_right(iter);
      }

      if (!is_ok && _VIOLATION(SymbType_is_back_comp(left) ?
                               TC_VIOLATION_TYPE_BACK_COMP :
                               TC_VIOLATION_TYPE_MANDATORY,
                               ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }

  case PSL_SERECONCAT:
  case PSL_SEREFUSION:
    {
      SymbType_ptr type1 = _THROW(psl_node_get_left(expr), context);
      SymbType_ptr type2 = _THROW(psl_node_get_right(expr), context);


      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      if (SymbType_is_boolean(type1) && SymbType_is_boolean(type2)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
      }

      /* there is violation */
      if (_VIOLATION(SymbType_is_back_comp(type1) &&
                     SymbType_is_back_comp(type2)?
                     TC_VIOLATION_TYPE_BACK_COMP : TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }


  case PSL_SEREREPEATED:
    {
      PslNode_ptr sere, count;
      boolean is_ok = true;
      SymbType_ptr tcount = SYMB_TYPE(NULL);
      SymbType_ptr tsere;

      nusmv_assert(psl_node_get_left(expr) != PSL_NULL);
      sere = psl_node_sere_repeated_get_expr(expr);
      count = psl_node_sere_repeated_get_count(expr);

      /* checks the count at first: */
      if (count != PSL_NULL) {
        tcount = _THROW(count, context);

        if (SymbType_is_error(tcount)) return _SET_TYPE(ctx_expr, tcount);
        if (tcount != SymbTablePkg_integer_type() &&
            tcount != SymbTablePkg_boolean_type()) {
          is_ok = false;
        }
      }

      /* checks the sere now: */
      if (sere != PSL_NULL && is_ok) {
        tsere = _THROW(sere, context);
        if (SymbType_is_error(tsere)) {
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }

        if (SymbType_is_boolean(tsere)) {
          /* ok */
          return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
        }
      }
      else tsere = SymbTablePkg_error_type();

      /* is this a type error ? */
      if (!is_ok && _VIOLATION(SymbType_is_back_comp(tcount) &&
                               SymbType_is_back_comp(tsere) ?
                               TC_VIOLATION_TYPE_BACK_COMP :
                               TC_VIOLATION_TYPE_MANDATORY,
                               ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }


  case PSL_REPLPROP:
    {
      SymbTable_ptr symb_table =
        TypeChecker_get_symb_table(TYPE_CHECKER(NODE_WALKER(self)->master));

      PslNode_ptr repl = psl_node_repl_prop_get_replicator(expr);
      PslNode_ptr prop = psl_node_repl_prop_get_property(expr);
      PslNode_ptr value_set = psl_node_get_replicator_value_set(repl);
      PslNode_ptr evalue_set =
        psl_node_get_replicator_normalized_value_set(repl);
      PslNode_ptr id = psl_node_get_replicator_id(repl);
      boolean first_loop = true;
      ResolveSymbol_ptr rs;
      node_ptr id_ctx;

      rs = SymbTable_resolve_symbol(symb_table,
                            PslNode_convert_to_node_ptr(id), ctx);
      id_ctx = ResolveSymbol_get_resolved_name(rs);

      /* Prepares the forall value set */
      if (!psl_node_is_boolean_type(value_set)) {
        /* not boolean checks the value_set content */
        SymbType_ptr tvs = SymbType_create(SYMB_TYPE_ENUM, evalue_set);
        if (!TypeChecker_is_type_wellformed(
                                    TYPE_CHECKER(NODE_WALKER(self)->master),
                                    tvs, id_ctx)) {
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }
      }

      /* loops over the set of possible values of the ID, and defines
         a new define for each value. Then checks the replicated
         property */
      while (evalue_set != PSL_NULL) {
        SymbType_ptr prop_type;
        boolean is_ok = true;

        /* creates a new temporary layer to contain the id as input var */
        SymbLayer_ptr layer =
          SymbTable_create_layer(symb_table, NULL, SYMB_LAYER_POS_DEFAULT);

        /* checks the forall identifier: */
        if (!SymbLayer_can_declare_define(layer, id_ctx)) {
          _VIOLATION(TC_VIOLATION_AMBIGUOUS_IDENTIFIER, id);
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }

        /* declares a new temporary define to represent the id */
        SymbLayer_declare_define(layer, id_ctx, ctx,
                         PslNode_convert_to_node_ptr(
                             psl_node_cons_get_element(evalue_set)));


        /* now checks the replicated property: */
        prop_type = _THROW(prop, context);
        if (SymbType_is_error(prop_type)) {
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }

        is_ok = SymbType_is_boolean(prop_type);

        /* handle violation */
        if (!is_ok && _VIOLATION(SymbType_is_back_comp(prop_type) ?
                                 TC_VIOLATION_TYPE_BACK_COMP :
                                 TC_VIOLATION_TYPE_MANDATORY,
                                 ctx_expr)) {
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }

        /* gets rid of the temporary layer: */
        SymbTable_remove_layer(symb_table, layer);

        /* Disables memoizing, to force checking of the property event
           if already checked, as the property that is going to be
           checked at next iteration is grammatically identically to
           the one that is being checked, but contain different value
           for the forall ID */
        if (first_loop) {
          type_checker_enable_memoizing(
                TYPE_CHECKER(NODE_WALKER(self)->master), false);
          first_loop = false;
        }

        evalue_set = psl_node_cons_get_next(evalue_set); /* iterates on */
      } /* loop over forall range */

      /* re-enables memoizing. The property has already been memoized
         during the first loop iteration */
      type_checker_enable_memoizing(TYPE_CHECKER(NODE_WALKER(self)->master),
                                    true);

      /* this is not an error after all -> return boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }

    case PSL_WSELECT:
      {
        SymbTable_ptr symb_table =
          TypeChecker_get_symb_table(TYPE_CHECKER(NODE_WALKER(self)->master));

        PslNode_ptr left = psl_node_get_left(expr);
        PslNode_ptr right = psl_node_get_right(expr);
        /* get the operand' type */
        SymbType_ptr type = _THROW(left, context);

        node_ptr hbit, lbit;
        int width;
        int highBound;
        int lowBound;

        if (SymbType_is_error(type)) { /* earlier error */
          return _SET_TYPE(ctx_expr, type);
        }

        hbit = PslNode_convert_to_node_ptr(psl_node_get_left(right));
        lbit = PslNode_convert_to_node_ptr(psl_node_get_right(right));

        hbit = CompileFlatten_resolve_number(symb_table, hbit, context);
        lbit = CompileFlatten_resolve_number(symb_table, lbit, context);

        nusmv_assert(COLON == psl_node_get_op(right));

        /* Non constant expressions for range */
        if ((Nil == hbit || Nil == lbit) ||
            (NUMBER != node_get_type(hbit) || NUMBER != node_get_type(lbit))) {
          if (_VIOLATION(TC_VIOLATION_UNCONSTANT_EXPRESSION, ctx_expr)) {
            return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
          }
          else { /* return arbitrary Word type */
            return _SET_TYPE(ctx_expr, SymbTablePkg_unsigned_word_type(1));
          }
        }

        /* check the first operand type */
        if (!SymbType_is_word(type)) { /* ERROR */
          if (_VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr)) {
            return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
          }
          else { /* return arbitrary Word type */
            return _SET_TYPE(ctx_expr, SymbTablePkg_unsigned_word_type(1));
          }
        }
        width = SymbType_get_word_width(type);
        highBound = NODE_TO_INT(car(hbit));
        lowBound  = NODE_TO_INT(car(lbit));

        /* checks the bit width */
        if (width <= highBound || highBound < lowBound || lowBound < 0) {
          /* ERROR */
          if (_VIOLATION(TC_VIOLATION_OUT_OF_WORD_WIDTH, ctx_expr)) {
            return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
          }
          else { /* give some realistic bit specifiers */
            highBound = 0;
            lowBound = 0;
          }
        }
        /* everything is OK */
        return _SET_TYPE(ctx_expr,
                         SymbTablePkg_unsigned_word_type(highBound - lowBound + 1));
      }

  case PSL_PIPEMINUSGT:
  case PSL_PIPEEQGT:
    {
      PslNode_ptr pre = psl_node_suffix_implication_get_premise(expr);
      PslNode_ptr con = psl_node_suffix_implication_get_consequence(expr);
      SymbType_ptr type1 = _THROW(pre, context);
      SymbType_ptr type2 = _THROW(con, context);

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      if (SymbType_is_boolean(type1) && SymbType_is_boolean(type2)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
      }

      /* there is violation */
      if (_VIOLATION(SymbType_is_back_comp(type1) &&
                     SymbType_is_back_comp(type2)?
                     TC_VIOLATION_TYPE_BACK_COMP : TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }


  case PSL_ALWAYS:
  case PSL_NEVER:
  case PSL_EVENTUALLYBANG:
    {
      SymbType_ptr type = _THROW(psl_node_get_left(expr), context);

      if (SymbType_is_error(type)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      if (SymbType_is_boolean(type)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
      }

      /* there is violation */
      if (_VIOLATION(SymbType_is_back_comp(type) ?
                     TC_VIOLATION_TYPE_BACK_COMP : TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }


  case PSL_WITHINBANG:
  case PSL_WITHIN:
  case PSL_WITHINBANG_:
  case PSL_WITHIN_:
    {
      PslNode_ptr n1 = psl_node_get_left(psl_node_get_left(expr));
      PslNode_ptr n2 = psl_node_get_right(psl_node_get_left(expr));
      PslNode_ptr n3 = psl_node_get_right(expr);

      SymbType_ptr t1 = _THROW(n1, context);
      SymbType_ptr t2 = _THROW(n2, context);
      SymbType_ptr t3 = _THROW(n3, context);

      if (SymbType_is_error(t1) || SymbType_is_error(t2) ||
          SymbType_is_error(t3) ) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      if (SymbType_is_boolean(t1) && SymbType_is_boolean(t2) &&
          SymbType_is_boolean(t3)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
      }

      /* there is violation */
      if (_VIOLATION(SymbType_is_back_comp(t1) &&
                     SymbType_is_back_comp(t2) && SymbType_is_back_comp(t3) ?
                     TC_VIOLATION_TYPE_BACK_COMP : TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }


  case PSL_NEXT_EVENT_ABANG:
  case PSL_NEXT_EVENT_A:
  case PSL_NEXT_EVENT_EBANG:
  case PSL_NEXT_EVENT_E:
  case PSL_NEXT_EVENTBANG:
  case PSL_NEXT_EVENT:
  case PSL_NEXT_ABANG:
  case PSL_NEXT_EBANG:
  case PSL_NEXT_A:
  case PSL_NEXT_E:
  case PSL_NEXTBANG:
  case PSL_NEXT:
  case PSL_X:
  case PSL_XBANG:
    {
      PslNode_ptr n1 = psl_node_extended_next_get_expr(expr);
      PslNode_ptr n2 = psl_node_extended_next_get_when(expr);
      PslNode_ptr n3 = psl_node_extended_next_get_condition(expr);

      SymbType_ptr t1 = _THROW(n1, context);

      SymbType_ptr t2 =
        (n2 != PSL_NULL) ? _THROW(n2, context) : SYMB_TYPE(NULL);

      SymbType_ptr t3 =
        (n3 != PSL_NULL) ? _THROW(n3, context) : SYMB_TYPE(NULL);

      if (SymbType_is_error(t1) ||
          (t2 != SYMB_TYPE(NULL) && SymbType_is_error(t2)) ||
          (t3 != SYMB_TYPE(NULL) && SymbType_is_error(t3))) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      if (SymbType_is_boolean(t1) &&
          (t3 == SYMB_TYPE(NULL) || SymbType_is_boolean(t3))) {
        switch (psl_node_get_op(expr)) {
        case PSL_NEXTBANG:
        case PSL_NEXT:
        case PSL_X:
        case PSL_XBANG:
          /* number */
          if ((t2 == SYMB_TYPE(NULL)) ||
              ((SymbType_get_tag(t2) == SYMB_TYPE_INTEGER ||
                SymbType_is_pure_int_enum(t2) ||
                SymbType_is_boolean(t2)) &&
               psl_node_number_get_value(n2) >= 0)) {
            return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
          }
          break;

        case PSL_NEXT_ABANG:
        case PSL_NEXT_EBANG:
        case PSL_NEXT_A:
        case PSL_NEXT_E:
          /* finite range */
          if (t2 == SYMB_TYPE(NULL)) {
            return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
          }

          if (SymbType_get_tag(t2) == SYMB_TYPE_SET_INT) {
            PslNode_ptr low = psl_node_range_get_low(n2);
            PslNode_ptr high = psl_node_range_get_high(n2);
            if (!psl_node_is_infinite(high) &&
                psl_node_number_get_value(low) >= 0 &&
                psl_node_number_get_value(low) <= psl_node_number_get_value(high)) {
              return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
            }
          }
          break;


        case PSL_NEXT_EVENT_ABANG:
        case PSL_NEXT_EVENT_A:
        case PSL_NEXT_EVENT_EBANG:
        case PSL_NEXT_EVENT_E:
          /* finite positive range */
          if (t2 == SYMB_TYPE(NULL)) {
            return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
          }

          if (SymbType_get_tag(t2) == SYMB_TYPE_SET_INT) {
            PslNode_ptr low = psl_node_range_get_low(n2);
            PslNode_ptr high = psl_node_range_get_high(n2);
            if (!psl_node_is_infinite(high) &&
                psl_node_number_get_value(low) > 0 &&
                psl_node_number_get_value(low) <= psl_node_number_get_value(high)) {
              return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
            }
          }
          break;

        case PSL_NEXT_EVENTBANG:
        case PSL_NEXT_EVENT:
          /* positive number */
          if ((t2 == SYMB_TYPE(NULL)) ||
              ((SymbType_get_tag(t2) == SYMB_TYPE_INTEGER ||
                SymbType_is_pure_int_enum(t2) ||
                SymbType_is_boolean(t2)) &&
               psl_node_number_get_value(n2) > 0)) {
            return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
          }
          break;

        default:
          error_unreachable_code(); /* no other cases */
        }
      }

      /* there is violation */
      if (_VIOLATION(SymbType_is_back_comp(t1) &&
                     (t2 == SYMB_TYPE(NULL) || SymbType_is_back_comp(t2)) &&
                     (t3 == SYMB_TYPE(NULL) || SymbType_is_back_comp(t3)) ?
                     TC_VIOLATION_TYPE_BACK_COMP : TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }


  case PSL_RANGE:
    /* colon is used only for index ranges */
    return _SET_TYPE(ctx_expr, SymbTablePkg_integer_set_type());


    /* binary operators */
  case PSL_BEFOREBANG:
  case PSL_BEFORE:
  case PSL_BEFOREBANG_:
  case PSL_BEFORE_:
  case PSL_UNTILBANG:
  case PSL_UNTIL:
  case PSL_UNTILBANG_:
  case PSL_UNTIL_:
  case PSL_ABORT:
  case PSL_W:
  case PSL_OR:
  case PSL_CARET:
  case PSL_TILDE:
  case PSL_EQEQ:
  case PSL_PIPEPIPE:
  case PSL_AMPERSANDAMPERSAND:
  case PSL_WHILENOTBANG:
  case PSL_WHILENOT:
  case PSL_WHILENOTBANG_:
  case PSL_WHILENOT_:
    {
      SymbType_ptr type1 = _THROW(psl_node_get_left(expr), context);
      SymbType_ptr type2 = _THROW(psl_node_get_right(expr), context);


      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      if (SymbType_is_boolean(type1) && SymbType_is_boolean(type2)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
      }

      /* there is violation */
      if (_VIOLATION(SymbType_is_back_comp(type1) &&
                     SymbType_is_back_comp(type2)?
                     TC_VIOLATION_TYPE_BACK_COMP : TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }


  case PSL_ITE:
  {
    /* get the operands' type */
    SymbType_ptr returnType;
    SymbType_ptr condType = _THROW(psl_node_get_ite_cond(expr), context);
    SymbType_ptr thenType = _THROW(psl_node_get_ite_then(expr), context);
    SymbType_ptr elseType = _THROW(psl_node_get_ite_else(expr), context);

    if (SymbType_is_error(condType) ||
        SymbType_is_error(thenType) || SymbType_is_error(elseType)) {
      /* earlier error */
      return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
    }

   /* condition should be boolean */
    if ( (!SymbType_is_boolean(condType)) &&
         _VIOLATION(SymbType_is_back_comp(condType) ?
                    TC_VIOLATION_TYPE_BACK_COMP :
                    TC_VIOLATION_TYPE_MANDATORY,
                    expr) ) {
      return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
    }

    /* both ITE expressions should be convertable to a common type.
       If one of the expressions is of a set-type then before implicit
       convertion the other expression is converted to a corresponding
       set-type.
    */
    {
      SymbType_ptr tmp1 = SymbType_is_set(elseType) ?
        SymbType_make_set_type(thenType) : thenType;

      SymbType_ptr tmp2 = SymbType_is_set(thenType) ?
        SymbType_make_set_type(elseType) : elseType;

      if (nullType != tmp1 && nullType != tmp2) {
        returnType = SymbType_get_minimal_common(tmp1, tmp2);
      }
      else returnType = nullType;
    }

    /* we do not care which type exactly is obtained, since only
       correct type could pass the above code
    */
    if (nullType != returnType) {
      return _SET_TYPE(ctx_expr, returnType);
    }

    /* is this a type error ? */
    if (_VIOLATION(SymbType_is_back_comp(thenType) &&
                   SymbType_is_back_comp(elseType) ?
                   TC_VIOLATION_TYPE_BACK_COMP :
                   TC_VIOLATION_TYPE_MANDATORY,
                   expr)) {
      return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
    }

    /* this is not an error after all -> return one of the types  */
    return _SET_TYPE(ctx_expr, thenType);
  }

  default:
    internal_error("checker_psl_check_expr: not supported type");
  }

  return nullType;
}


/**Function********************************************************************

  Synopsis           [The type core violation handler.]

  Description        [The violation handler is implemented as
  a virtual method, which is invoked by the checker when an expression
  being checked violates the type system.
  See the violation handler TypeCheckingViolationHandler_ptr
  for more explanations.

  The below function is the default violation handler, and a
  user can potentially define its own violation handler, by deriving
  a new class from this class and by overriding this virtual method.

  This violation handler outputs an error and warning message to
  nusmv_stderr. A warning is output if the detected violation is
  TC_VIOLATION_TYPE_BACK_COMP and the system variable
  "type_checking_backward_compatibility" is true. Also the
  TC_VIOLATION_TYPE_WARNING violation outputs a warning. Only in this
  case the false value is returned, indicating that this is NOT an
  error. Otherwise the true value is returned, indicating that this is
  an error.

  Also, if the system variable "type_check_warning_on" is false,
  warning messages are not output.

  NB: if the expression is checked in some context (context is not null) then
  before providing the expression to this function the expression should be
  wrapped into context, i.e. with find_node(CONEXT, context, expr)]

  SideEffects       []

  SeeAlso           [TypeSystemViolation]

******************************************************************************/
static boolean
checker_psl_viol_handler(CheckerBase_ptr self,
                         TypeSystemViolation violation, node_ptr expression)
{
  /* In the output message, the information about the expression
     location are output. So, make sure that the input file name and
     line number are correctly set!
  */

  boolean isError = true; /* is this error or warning */

  /* get rid of the context the expression may be wrapped in */
  PslNode_ptr context = PSL_NULL;
  PslNode_ptr expr = PslNode_convert_from_node_ptr(expression);

  if (node_get_type(expression) == CONTEXT) {
    context = PslNode_convert_from_node_ptr(car(expression));
    expr = PslNode_convert_from_node_ptr(cdr(expression));
  }

  /* checks the given violation */
  nusmv_assert(TypeSystemViolation_is_valid(violation));

  /* only violation TC_VIOLATION_TYPE_BACK_COMP and the variable
     type_checking_backward_compatibility being true, may make a
     warning from an error.
     TC_VIOLATION_TYPE_WARNING always forces a warning
  */
  if (  TC_VIOLATION_TYPE_WARNING == violation
     || ( TC_VIOLATION_TYPE_BACK_COMP == violation
         && opt_backward_comp(OptsHandler_get_instance()))) {
    isError = false;
  }

  if (!isError && !opt_type_checking_warning_on(OptsHandler_get_instance())) {
    /* this is a warning and warning messages are not allowed.
     So, do nothing, just return false (this is not an error)
    */
    return false;
  }

  _PRINT_ERROR_MSG(expr, isError);

  switch (violation) {
  case TC_VIOLATION_AMBIGUOUS_IDENTIFIER:
    fprintf(nusmv_stderr,  "identifier '");
    print_node(nusmv_stderr, PslNode_convert_to_node_ptr(expr));
    fprintf(nusmv_stderr, "' is ambiguous\n");
    break;

  case TC_VIOLATION_UNCONSTANT_EXPRESSION:
    fprintf(nusmv_stderr, "Expected constant expression in '");
    print_node(nusmv_stderr, PslNode_convert_to_node_ptr(expr));
    fprintf(nusmv_stderr, "'\n");
    break;

  case TC_VIOLATION_TYPE_MANDATORY:
  case TC_VIOLATION_TYPE_BACK_COMP:
  case TC_VIOLATION_TYPE_WARNING:
    if (isError) fprintf(nusmv_stderr, "illegal ");
    else         fprintf(nusmv_stderr, "potentially incorrect ");

    switch (psl_node_get_op(expr)) {

    case PSL_SERE:
    case PSL_SERECOMPOUND:
      fprintf(nusmv_stderr, "sere type of {");
      print_node(stderr, PslNode_convert_to_node_ptr(psl_node_get_left(expr)));
      fprintf(nusmv_stderr, "} : ");
      checker_base_print_type(self, nusmv_stderr,
                      PslNode_convert_to_node_ptr(psl_node_get_left(expr)),
                      PslNode_convert_to_node_ptr(context));
      break;

    case PSL_SERECONCAT:
      fprintf(nusmv_stderr, "operand types of sere concatenation: ");
      checker_base_print_type(self, nusmv_stderr,
                      PslNode_convert_to_node_ptr(psl_node_get_left(expr)),
                      PslNode_convert_to_node_ptr(context));
      fprintf(nusmv_stderr, " ");
      print_operator(nusmv_stderr, expr);
      fprintf(nusmv_stderr, " ");
      checker_base_print_type(self, nusmv_stderr,
                      PslNode_convert_to_node_ptr(psl_node_get_right(expr)),
                      PslNode_convert_to_node_ptr(context));
      break;

    case PSL_SEREFUSION:
      fprintf(nusmv_stderr, "operand types of sere fusion: ");
      checker_base_print_type(self, nusmv_stderr,
                      PslNode_convert_to_node_ptr(psl_node_get_left(expr)),
                      PslNode_convert_to_node_ptr(context));
      fprintf(nusmv_stderr, " ");
      print_operator(nusmv_stderr, expr);
      fprintf(nusmv_stderr, " ");
      checker_base_print_type(self, nusmv_stderr,
                      PslNode_convert_to_node_ptr(psl_node_get_right(expr)),
                      PslNode_convert_to_node_ptr(context));
      break;


    case PSL_SEREREPEATED:
      {
        PslNode_ptr sere = psl_node_sere_repeated_get_expr(expr);
        PslNode_ptr count = psl_node_sere_repeated_get_count(expr);

        fprintf(nusmv_stderr, "operand types of sere repeated: ");
        if (sere != PSL_NULL) {
          checker_base_print_type(self, nusmv_stderr,
                                  PslNode_convert_to_node_ptr(sere),
                                  PslNode_convert_to_node_ptr(context));
        }
        print_operator(nusmv_stderr, psl_node_get_left(expr));
        fprintf(nusmv_stderr, " ");

        if (count != PSL_NULL) {
          checker_base_print_type(self, nusmv_stderr,
                                  PslNode_convert_to_node_ptr(count),
                                  PslNode_convert_to_node_ptr(context));
        }
        fprintf(nusmv_stderr, " ]");
        break;
      }

    case PSL_REPLPROP:
      {
        PslNode_ptr prop = psl_node_repl_prop_get_property(expr);

        fprintf(nusmv_stderr, "operand type of ");
        print_operator(nusmv_stderr, expr);
        fprintf(nusmv_stderr, " property: ");
        checker_base_print_type(self, nusmv_stderr,
                                PslNode_convert_to_node_ptr(prop),
                                PslNode_convert_to_node_ptr(context));
        break;
      }

      /* suffix implication */
    case PSL_PIPEMINUSGT:
    case PSL_PIPEEQGT:
      {
        PslNode_ptr pre = psl_node_suffix_implication_get_premise(expr);
        PslNode_ptr con = psl_node_suffix_implication_get_consequence(expr);

        fprintf(nusmv_stderr, "operand types of suffix implication: ");
        checker_base_print_type(self, nusmv_stderr,
                                PslNode_convert_to_node_ptr(pre),
                                PslNode_convert_to_node_ptr(context));

        fprintf(nusmv_stderr, " ");
        print_operator(nusmv_stderr, expr);
        fprintf(nusmv_stderr, " ");

        checker_base_print_type(self, nusmv_stderr,
                                PslNode_convert_to_node_ptr(con),
                                PslNode_convert_to_node_ptr(context));
        break;
      }

      /* unary operators: */
    case PSL_ALWAYS:
    case PSL_NEVER:
    case PSL_EVENTUALLYBANG:
        fprintf(nusmv_stderr, "operand types of \"");
        print_operator(nusmv_stderr, expr);
        fprintf(nusmv_stderr, "\" : ");
        checker_base_print_type(self, nusmv_stderr,
                                PslNode_convert_to_node_ptr(psl_node_get_left(expr)),
                                PslNode_convert_to_node_ptr(context));
        break;

      /* within operators */
    case PSL_WITHINBANG:
    case PSL_WITHIN:
    case PSL_WITHINBANG_:
    case PSL_WITHIN_:
      {
        PslNode_ptr n1 = psl_node_get_left(psl_node_get_left(expr));
        PslNode_ptr n2 = psl_node_get_right(psl_node_get_left(expr));
        PslNode_ptr n3 = psl_node_get_right(expr);

        fprintf(nusmv_stderr, "operand types of \"");
        print_operator(nusmv_stderr, expr);
        fprintf(nusmv_stderr, "\" : (");
        checker_base_print_type(self, nusmv_stderr,
                                PslNode_convert_to_node_ptr(n1),
                                PslNode_convert_to_node_ptr(context));
        fprintf(nusmv_stderr, ", ");
        checker_base_print_type(self, nusmv_stderr,
                                PslNode_convert_to_node_ptr(n2),
                                PslNode_convert_to_node_ptr(context));
        fprintf(nusmv_stderr, ") ");
        checker_base_print_type(self, nusmv_stderr,
                                PslNode_convert_to_node_ptr(n3),
                                PslNode_convert_to_node_ptr(context));
        break;
      }

      /* next operators */
    case PSL_NEXT_EVENT_ABANG:
    case PSL_NEXT_EVENT_A:
    case PSL_NEXT_EVENT_EBANG:
    case PSL_NEXT_EVENT_E:
    case PSL_NEXT_EVENTBANG:
    case PSL_NEXT_EVENT:
    case PSL_NEXT_ABANG:
    case PSL_NEXT_EBANG:
    case PSL_NEXT_A:
    case PSL_NEXT_E:
    case PSL_NEXTBANG:
    case PSL_NEXT:
    case PSL_X:
    case PSL_XBANG:
      {
        PslNode_ptr n1 = psl_node_extended_next_get_expr(expr);
        PslNode_ptr n2 = psl_node_extended_next_get_when(expr);
        PslNode_ptr n3 = psl_node_extended_next_get_condition(expr);

        fprintf(nusmv_stderr, "operand types of \"");
        print_operator(nusmv_stderr, expr);
        fprintf(nusmv_stderr, "\" : ");

        if (n3 != PSL_NULL) {
          fprintf(nusmv_stderr, " (");
          checker_base_print_type(self, nusmv_stderr,
                                  PslNode_convert_to_node_ptr(n3),
                                  PslNode_convert_to_node_ptr(context));
          fprintf(nusmv_stderr, ")");
        }

        if (n2 != PSL_NULL) {
          fprintf(nusmv_stderr, " [");
          checker_base_print_type(self, nusmv_stderr,
                                  PslNode_convert_to_node_ptr(n2),
                                  PslNode_convert_to_node_ptr(context));
          fprintf(nusmv_stderr, "]");
        }

        nusmv_assert(n1 != PSL_NULL); /* n1 must occur here */
        fprintf(nusmv_stderr, " (");
        checker_base_print_type(self, nusmv_stderr,
                                PslNode_convert_to_node_ptr(n1),
                                PslNode_convert_to_node_ptr(context));
        fprintf(nusmv_stderr, ")");
        break;
      }

      /* Binary operators */
    case PSL_BEFOREBANG:
    case PSL_BEFORE:
    case PSL_BEFOREBANG_:
    case PSL_BEFORE_:
    case PSL_UNTILBANG:
    case PSL_UNTIL:
    case PSL_UNTILBANG_:
    case PSL_UNTIL_:
    case PSL_ABORT:
    case PSL_W:
    case PSL_OR:
    case PSL_CARET:
    case PSL_TILDE:
    case PSL_EQEQ:
    case PSL_PIPEPIPE:
    case PSL_AMPERSANDAMPERSAND:
    case PSL_WHILENOTBANG:
    case PSL_WHILENOT:
    case PSL_WHILENOTBANG_:
    case PSL_WHILENOT_:
      fprintf(nusmv_stderr, "operand types of \"");
      print_operator(nusmv_stderr, expr);
      fprintf(nusmv_stderr,"\" : ");
      checker_base_print_type(self, nusmv_stderr,
                      PslNode_convert_to_node_ptr(psl_node_get_left(expr)),
                      PslNode_convert_to_node_ptr(context));
      fprintf(nusmv_stderr," and ");
      checker_base_print_type(self, nusmv_stderr,
                      PslNode_convert_to_node_ptr(psl_node_get_right(expr)),
                      PslNode_convert_to_node_ptr(context));
      break;

    case PSL_ITE:
      fprintf(nusmv_stderr, "operand types of \"");
      print_operator(nusmv_stderr, expr);
      fprintf(nusmv_stderr,"\" : ");
      checker_base_print_type(self, nusmv_stderr,
                      PslNode_convert_to_node_ptr(psl_node_get_ite_cond(expr)),
                      PslNode_convert_to_node_ptr(context));
      fprintf(nusmv_stderr," ? ");
      checker_base_print_type(self, nusmv_stderr,
                      PslNode_convert_to_node_ptr(psl_node_get_ite_then(expr)),
                      PslNode_convert_to_node_ptr(context));
      fprintf(nusmv_stderr," : ");
      checker_base_print_type(self, nusmv_stderr,
                      PslNode_convert_to_node_ptr(psl_node_get_ite_else(expr)),
                      PslNode_convert_to_node_ptr(context));
      break;


    default: /* unknown kind of an expression */
      error_unreachable_code();
    } /* switch (node_get_type(expr)) */

    fprintf(nusmv_stderr,"\n");
    break;

  default:
    error_unreachable_code(); /* unknown kind of error */

  } /* switch (errorKind) */

  return isError;
}



/**Static function*************************************************************

  Synopsis           [Just prints an expression's operator to output_stream]

  Description        [This function is the almost the same as
  print_sexp, except this function does not print the children of the node.
  The expr must be a correct expression.
  The function is used in printing of an error messages only.]

  SideEffects        []

  SeeAlso            [print_sexp]
******************************************************************************/
static void print_operator(FILE* output_stream, PslNode_ptr expr)
{
  nusmv_assert(expr != PSL_NULL);
  switch (psl_node_get_op(expr)) {

  case PSL_INF: fprintf(output_stream, "inf"); return;

  case PSL_SERECONCAT: fprintf(output_stream, ";"); return;
  case PSL_SEREFUSION: fprintf(output_stream, ":"); return;

  case PSL_LBSPLAT: fprintf(output_stream, "[*"); return;
  case PSL_LBPLUSRB: fprintf(output_stream, "[+"); return;
  case PSL_LBEQ: fprintf(output_stream, "[="); return;
  case PSL_LBMINUSGT: fprintf(output_stream, "[->"); return;

  case PSL_REPLPROP: fprintf(output_stream, "forall"); return;

  case PSL_PIPEMINUSGT: fprintf(output_stream, "|->"); return;
  case PSL_PIPEEQGT: fprintf(output_stream, "|=>"); return;

  case PSL_ALWAYS: fprintf(output_stream, "always"); return;
  case PSL_NEVER: fprintf(output_stream, "never"); return;
  case PSL_EVENTUALLYBANG: fprintf(output_stream, "eventually!"); return;

  case PSL_WITHINBANG: fprintf(output_stream, "within!"); return;
  case PSL_WITHIN: fprintf(output_stream, "within"); return;
  case PSL_WITHINBANG_: fprintf(output_stream, "within!_"); return;
  case PSL_WITHIN_: fprintf(output_stream, "within_"); return;

  case PSL_NEXT_EVENT_ABANG: fprintf(output_stream, "next_event_a!"); return;
  case PSL_NEXT_EVENT_A: fprintf(output_stream, "next_event_a"); return;
  case PSL_NEXT_EVENT_EBANG: fprintf(output_stream, "next_event_e!"); return;
  case PSL_NEXT_EVENT_E: fprintf(output_stream, "next_event_e"); return;
  case PSL_NEXT_EVENTBANG: fprintf(output_stream, "next_event!"); return;
  case PSL_NEXT_EVENT: fprintf(output_stream, "next_event"); return;
  case PSL_NEXT_ABANG: fprintf(output_stream, "next_a!"); return;
  case PSL_NEXT_EBANG: fprintf(output_stream, "next_e!"); return;
  case PSL_NEXT_A: fprintf(output_stream, "next_a"); return;
  case PSL_NEXT_E: fprintf(output_stream, "next_e"); return;
  case PSL_NEXTBANG: fprintf(output_stream, "next!"); return;
  case PSL_NEXT: fprintf(output_stream, "next"); return;
  case PSL_X: fprintf(output_stream, "X"); return;
  case PSL_XBANG: fprintf(output_stream, "X!"); return;

  case PSL_BEFOREBANG: fprintf(output_stream, "before!"); return;
  case PSL_BEFORE: fprintf(output_stream, "before"); return;
  case PSL_BEFOREBANG_: fprintf(output_stream, "before!_"); return;
  case PSL_BEFORE_: fprintf(output_stream, "before_"); return;
  case PSL_UNTILBANG: fprintf(output_stream, "until!"); return;
  case PSL_UNTIL: fprintf(output_stream, "until"); return;
  case PSL_UNTILBANG_: fprintf(output_stream,"until!_"); return;
  case PSL_UNTIL_: fprintf(output_stream,"until_"); return;
  case PSL_ABORT: fprintf(output_stream,"abort"); return;
  case PSL_W: fprintf(output_stream,"W"); return;
  case PSL_OR: fprintf(output_stream,"or"); return;
  case PSL_CARET: fprintf(output_stream,"^"); return;
  case PSL_TILDE: fprintf(output_stream,"~"); return;
  case PSL_EQEQ: fprintf(output_stream,"=="); return;
  case PSL_PIPEPIPE: fprintf(output_stream,"||"); return;
  case PSL_AMPERSANDAMPERSAND: fprintf(output_stream,"&&"); return;
  case PSL_WHILENOTBANG: fprintf(output_stream,"whilenot!"); return;
  case PSL_WHILENOT: fprintf(output_stream,"whilenot"); return;
  case PSL_WHILENOTBANG_: fprintf(output_stream,"whilenot!_"); return;
  case PSL_WHILENOT_: fprintf(output_stream,"whilenot_"); return;

  case PSL_ITE: fprintf(output_stream,"ITE"); return;

    /* these are for sere compound */
  case AND: fprintf(output_stream,"&"); return;
  case OR: fprintf(output_stream,"|"); return;

  default:
    fprintf(nusmv_stderr, "\n%d\n", psl_node_get_op(expr));
    error_unreachable_code();
  }

}




/**AutomaticEnd***************************************************************/

