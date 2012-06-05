/**CFile***********************************************************************

   FileName    [CheckerCore.c]

   PackageName [compile.type_checking.checkers]

   Synopsis    [Implementaion of class 'CheckerCore']

   Description []

   SeeAlso     [CheckerCore.h]

   Author      [Andrei Tchaltsev, Roberto Cavada]

   Copyright   [
   This file is part of the ``compile.type_checking.checkers''
   package of NuSMV version 2.
   Copyright (C) 2006 by FBK-irst.

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

   Revision    [$Id: CheckerCore.c,v 1.1.2.16.4.30 2009/12/02 20:54:08 nusmv Exp $]

******************************************************************************/

#include "CheckerCore.h"
#include "CheckerCore_private.h"
#include "checkersInt.h"

#include "compile/compile.h"
#include "compile/type_checking/TypeChecker_private.h"
#include "compile/symb_table/symb_table.h"
#include "utils/WordNumber.h"
#include "parser/symbols.h"
#include "utils/utils.h"
#include "utils/error.h"
#include "utils/ustring.h"
#include "utils/range.h"
#include "compile/symb_table/NFunction.h"
#include "compile/symb_table/ResolveSymbol.h"

static char rcsid[] UTIL_UNUSED = "$Id: CheckerCore.c,v 1.1.2.16.4.30 2009/12/02 20:54:08 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'CheckerCore_private.h' for class 'CheckerCore' definition. */

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

static void checker_core_finalize ARGS((Object_ptr object, void* dummy));

static SymbType_ptr
checker_core_check_expr ARGS((CheckerBase_ptr self,
                              node_ptr expression, node_ptr context));

static boolean
checker_core_viol_handler ARGS((CheckerBase_ptr checker,
                                TypeSystemViolation violation,
                                node_ptr expression));

static void
print_operator ARGS((FILE* output_stream, node_ptr expr));



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [The CheckerCore class constructor]

   Description        [The CheckerCore class constructor]

   SideEffects        []

   SeeAlso            [NodeWalker_destroy]

******************************************************************************/
CheckerCore_ptr CheckerCore_create()
{
  CheckerCore_ptr self = ALLOC(CheckerCore, 1);
  CHECKER_CORE_CHECK_INSTANCE(self);

  checker_core_init(self, "Core SMV Type Checker",
                    NUSMV_EXPR_SYMBOL_FIRST,
                    NUSMV_EXPR_SYMBOL_LAST - NUSMV_EXPR_SYMBOL_FIRST);
  return self;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [The CheckerCore class private initializer]

   Description        [The CheckerCore class private initializer]

   SideEffects        []

   SeeAlso            [CheckerCore_create]

******************************************************************************/
void checker_core_init(CheckerCore_ptr self,
                       const char* name, int low, size_t num)
{
  /* base class initialization */
  checker_base_init(CHECKER_BASE(self), name, low, num);

  /* members initialization */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = checker_core_finalize;
  OVERRIDE(CheckerBase, check_expr) = checker_core_check_expr;
  OVERRIDE(CheckerBase, viol_handler) = checker_core_viol_handler;
}


/**Function********************************************************************

   Synopsis           [The CheckerCore class private deinitializer]

   Description        [The CheckerCore class private deinitializer]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void checker_core_deinit(CheckerCore_ptr self)
{
  /* members deinitialization */

  /* base class initialization */
  checker_base_deinit(CHECKER_BASE(self));
}




/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis    [The CheckerCore class virtual finalizer]

   Description [Called by the class destructor]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static void checker_core_finalize(Object_ptr object, void* dummy)
{
  CheckerCore_ptr self = CHECKER_CORE(object);

  checker_core_deinit(self);
  FREE(self);
}


/**Function********************************************************************

   Synopsis    []

   Description []

   SideEffects []

   SeeAlso     []

******************************************************************************/
static SymbType_ptr checker_core_check_expr(CheckerBase_ptr self,
                                            node_ptr expr, node_ptr context)
{
  /* wrap expr into the context. This is required by
     the facilities which remembers the type of expressions
     and by the violation handler.
  */
  node_ptr ctx_expr;
  if (context != Nil) ctx_expr = find_node(CONTEXT, context, expr);
  else ctx_expr = expr;

  { /* checks memoizing */
    SymbType_ptr tmp = _GET_TYPE(ctx_expr);
    if (nullType != tmp) return tmp;
  }

  switch (node_get_type(expr)) {
  case SELF:
    return  _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
  case CONTEXT:
    {
      SymbType_ptr type;
      type = _THROW(cdr(expr), car(expr));

      /* since before remembering the type an expression is
         wrapped into CONTEXT, that CONTEXT can be equal to this
         CONTEXT expression, and therefore its type has been
         remembered in the above invocation of
         type_checker_check_expression
      */
      if (nullType == _GET_TYPE(ctx_expr)) {
        _SET_TYPE(ctx_expr, type);
      }
      return type;
    }

  case TRUEEXP:
  case FALSEEXP:
    return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());

  case NUMBER:
    return _SET_TYPE(ctx_expr, SymbTablePkg_integer_type());

    /* during parsing Word constants are normilised to WordNumber_ptr
       (the left chilf).
    */
  case NUMBER_UNSIGNED_WORD: {
    int wordWidth = WordNumber_get_width(WORD_NUMBER(car(expr)));
    return _SET_TYPE(ctx_expr, SymbTablePkg_unsigned_word_type(wordWidth));
  }
  case NUMBER_SIGNED_WORD: {
    int wordWidth = WordNumber_get_width(WORD_NUMBER(car(expr)));
    return _SET_TYPE(ctx_expr, SymbTablePkg_signed_word_type(wordWidth));
  }

  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
    return _SET_TYPE(ctx_expr, SymbTablePkg_real_type());

  case UWCONST:
  case SWCONST: {
    /* constant number and constant number or word number, result
       is a (constant) signed word */

    /* here we rely on the flattener, which is in charge of getting
       rid of SWCONST and UWCONST */
    SymbTable_ptr st = TypeChecker_get_symb_table(
                               TYPE_CHECKER(NODE_WALKER(self)->master));

    node_ptr wn = Compile_FlattenSexp(st, expr, context);
    return _THROW(wn, Nil);
  }

  case TWODOTS:
    /* two-dots have two NUMBER and nothing else */
    nusmv_assert(node_get_type(car(expr)) == NUMBER &&
                 node_get_type(cdr(expr)) == NUMBER);
    return _SET_TYPE(ctx_expr, SymbTablePkg_integer_set_type());

  case BIT:
    /* this kind of expressions is not checked, since it is
       artificially created */
    return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());

  case ARRAY:
    {
      ResolveSymbol_ptr rs;
      node_ptr resolvedName;
      SymbTable_ptr symb_table = TypeChecker_get_symb_table(
                                     TYPE_CHECKER(NODE_WALKER(self)->master));

      rs = SymbTable_resolve_symbol(symb_table, expr, context);

      resolvedName = ResolveSymbol_get_resolved_name(rs);

      /* ARRAY may be an identifier-with-brackets and may be an
         expression.  See description of function
         compileFlattenSexpRecur for details.

         Identifiers are passed to ATOM and DOT cases, and expressions are
         processed here.
      */
      if (ResolveSymbol_is_undefined(rs)) {

        SymbType_ptr l_type = checker_core_check_expr(self, car(expr), context);
        SymbType_ptr r_type = checker_core_check_expr(self, cdr(expr), context);

        if (SymbType_is_error(l_type) || SymbType_is_error(r_type)) {
          /* earlier error */
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }

        /* the left operand must be array and the right one is bool, int
           or word */
        if (!SymbType_is_array(l_type) ||
            !(SymbType_is_boolean(r_type) ||
              SymbType_is_integer(r_type) ||
              SymbType_is_word(r_type))) {
          _VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr);
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }
        /* here type already have to be memory shared */
        SymbType_ptr type = SymbType_get_array_subtype(l_type);

        /* NOTE that out-of-range is checked in compileFlattenSexpRecur
           where array expressions are normalized, e.g. dynamic indexes
           are resolved, etc.
        */

        return _SET_TYPE(ctx_expr, type);
      } /* ARRAY */

      /* process the identifier below together with DOT and ATOM */
      /* !! NO BREAK HERE !!! */
    }

    /* these exprs mean a variable, a constant, a define or a parameter */
  case DOT:
  case ATOM:
    {
      ResolveSymbol_ptr rs;
      node_ptr resolvedName;
      SymbTable_ptr symb_table = TypeChecker_get_symb_table(
                                     TYPE_CHECKER(NODE_WALKER(self)->master));

      rs = SymbTable_resolve_symbol(symb_table, expr, context);

      resolvedName = ResolveSymbol_get_resolved_name(rs);

      /* An identifier has not been declared => error */
      if (ResolveSymbol_is_undefined(rs)) {
        SymbType_ptr type = _VIOLATION(TC_VIOLATION_UNDEF_IDENTIFIER,
                                       resolvedName) ?
          SymbTablePkg_error_type() /* ERROR */
          : SymbTablePkg_pure_symbolic_enum_type();
        return _SET_TYPE(ctx_expr, type);
      }

      /* An identifier has more than one meaning => error */
      if (ResolveSymbol_is_ambiguous(rs)) {
        SymbType_ptr type = _VIOLATION(TC_VIOLATION_AMBIGUOUS_IDENTIFIER,
                                       ctx_expr) ?
          SymbTablePkg_error_type() /* ERROR */
          : SymbTablePkg_pure_symbolic_enum_type();
        return _SET_TYPE(ctx_expr, type);
      }

      /* this is a constant */
      if (ResolveSymbol_is_constant(rs)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_pure_symbolic_enum_type());
      }

      /* this is a parameter */
      if (ResolveSymbol_is_parameter(rs)) {
        node_ptr parameter =
          SymbTable_get_flatten_actual_parameter(symb_table,
                                                 resolvedName);
        SymbType_ptr type = _THROW(parameter, context);
        return _SET_TYPE(ctx_expr, type);
      }

      /* this is a define */
      if (ResolveSymbol_is_define(rs)) {
        /* the bodies of defines are cached(in BddEnc).
           So, the type of expressions will also be correctly cached.
        */
        node_ptr bodyDef = SymbTable_get_define_body(symb_table,
                                                     resolvedName);
        node_ptr ctxDef = SymbTable_get_define_context(symb_table,
                                                       resolvedName);
        /* the context is Nil because expr is already flattened */
        SymbType_ptr type = _THROW(bodyDef, ctxDef);
        return _SET_TYPE(ctx_expr, type);
      }

      /* this is a array define */
      if (ResolveSymbol_is_array_def(rs)) {
        node_ptr arraydef = SymbTable_get_array_define_body(symb_table,
                                                            resolvedName);
        node_ptr array_ctx = SymbTable_get_array_define_context(symb_table,
                                                                resolvedName);
        SymbType_ptr type = _THROW(arraydef, array_ctx);
        return _SET_TYPE(ctx_expr, type);
      }

      /* An array type */
      if (ResolveSymbol_is_array(rs)) {
        SymbType_ptr type = SymbTable_get_variable_array_type(symb_table,
                                                              resolvedName);
        /* Here we share the variable's type to allow
           comparison of pointers instead of content of the types
        */
        type = SymbType_make_memory_shared(type);
        return _SET_TYPE(ctx_expr, type);
      }

      /* This is a variable */
      {
        SymbType_ptr type;
        nusmv_assert(ResolveSymbol_is_var(rs));
        type = SymbTable_get_var_type(symb_table, resolvedName);
        /* Convert the syntactic type to a symbol type.
           Here we share the variable's type to allow
           comparison of pointers instead of content of the types
        */
        type = SymbType_make_memory_shared(type);
        return _SET_TYPE(ctx_expr, type);
      } /* end of isVar */
    } /* case ATOM, DOT */

    /*  boolean unary operand */
  case EX:
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:

  case NOT: /* "NOT" may also have Word (unsigned and signed) */

  case OP_GLOBAL:
  case OP_PREC:
  case OP_NOTPRECNOT:
  case OP_FUTURE:
  case OP_NEXT:
  case OP_HISTORICAL:
  case OP_ONCE:

    /* For EBF ABF EBG ABG - check the range to be ok!  */
  case EBF:
  case ABF:
  case EBG:
  case ABG:
    {
      /* get the operand's type */
      SymbType_ptr type = _THROW(car(expr), context);

      if (SymbType_is_error(type)) { /* earlier error */
        return _SET_TYPE(ctx_expr, type);
      }

      /* Check real-time specifications range */
      if (Nil != cdr(expr) && !Utils_check_subrange_not_negative(cdr(expr))) {
        if (_VIOLATION(TC_VIOLATION_INVALID_RANGE, ctx_expr)) {
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }
      }

      /* the operand must be boolean (or Word for NOT) */
      if (SymbType_is_boolean(type) ||
          (node_get_type(expr) == NOT &&
           SymbType_is_word(type))) {
        return _SET_TYPE(ctx_expr, type);
      }

      /* is this a type error ? */
      if (_VIOLATION(SymbType_is_back_comp(type) ?
                     TC_VIOLATION_TYPE_BACK_COMP : TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> keep the type what ever it is */
      return _SET_TYPE(ctx_expr, type);
    }


    /* casts: boolean -> Word[1] and Word[1] -> boolean. */
  case CAST_BOOL:
  case CAST_WORD1: {
    /* get the operands' type */
    SymbType_ptr type = _THROW(car(expr), context);

    if (SymbType_is_error(type)) { /* earlier error */
      return _SET_TYPE(ctx_expr, type);
    }

    /* if the expression is cast_bool, operand is word[1] */
    if (node_get_type(expr) == CAST_BOOL &&
        (SymbType_is_word_1(type) ||
         SymbType_get_tag(type) == SYMB_TYPE_INTEGER)) {
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }

    /* OR the expression must be cast_word1, operand is boolean */
    if (node_get_type(expr) == CAST_WORD1 && SymbType_is_boolean(type)) {
      return _SET_TYPE(ctx_expr, SymbTablePkg_unsigned_word_type(1));
    }

    /* is this a type error ? */
    if (_VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr)) {
      return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
    }

    /* this is not an error after all -> create the proper return type */
    return _SET_TYPE(ctx_expr,
                     node_get_type(expr) == CAST_BOOL
                     ? SymbTablePkg_boolean_type()
                     : SymbTablePkg_unsigned_word_type(1));
  }


    /* cast: unsigned word -> signed word, signed word -> unsigned word */
  case CAST_SIGNED:
  case CAST_UNSIGNED: {
    /* get the operands' type */
    SymbType_ptr type = _THROW(car(expr), context);

    if (SymbType_is_error(type)) { /* earlier error */
      return _SET_TYPE(ctx_expr, type);
    }

    if (node_get_type(expr) == CAST_SIGNED && SymbType_is_unsigned_word(type)) {
      int width = SymbType_get_word_width(type);
      return _SET_TYPE(ctx_expr, SymbTablePkg_signed_word_type(width));
    }
    if (node_get_type(expr) == CAST_UNSIGNED && SymbType_is_signed_word(type)) {
      int width = SymbType_get_word_width(type);
      return _SET_TYPE(ctx_expr, SymbTablePkg_unsigned_word_type(width));
    }

    /* is this a type error ? */
    if (_VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr)) {
      return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
    }

    /* this is not an error after all -> create the proper return type */
    return _SET_TYPE(ctx_expr,
                     node_get_type(expr) == CAST_SIGNED
                     ? SymbTablePkg_unsigned_word_type(1)
                     : SymbTablePkg_signed_word_type(1));
  }

    /* size of a word expression word[N] -> N */
  case WSIZEOF: {
    /* get the operand' type */
    SymbType_ptr type = _THROW(car(expr), context);

    if (SymbType_is_error(type)) { /* earlier error */
      return _SET_TYPE(ctx_expr, type);
    }

    /* check the operand type */
    if (!SymbType_is_word(type)) { /* ERROR */
      if (_VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }
      else { /* Return arbitrary type */
        return _SET_TYPE(ctx_expr, SymbTablePkg_signed_word_type(1));
      }
    }

    return _SET_TYPE(ctx_expr, SymbTablePkg_integer_type());
  }

    /* integer representation of of a constant word expression
       word[N] -> Integer or Boolean -> Integer */
  case CAST_TOINT: {
    /* get the operand' type */
    SymbType_ptr type = _THROW(car(expr), context);

    if (SymbType_is_error(type)) { /* earlier error */
      return _SET_TYPE(ctx_expr, type);
    }

    if (!(SymbType_is_word(type) ||
          SymbType_is_boolean(type) ||
          (SymbType_get_tag(type) == SYMB_TYPE_INTEGER))) { /* ERROR */
      if (_VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }
      else { /* return arbitrary type */
        return _SET_TYPE(ctx_expr, SymbTablePkg_signed_word_type(1));
      }
    }

    return _SET_TYPE(ctx_expr, SymbTablePkg_integer_type());
  }

  case COUNT: {
    node_ptr list = car(expr);

    while (Nil != list) {
      SymbType_ptr t = _THROW(car(list), context);

      if (! SymbType_is_boolean(t)) {
        _VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr);
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      list = cdr(list);
    }

    return _SET_TYPE(ctx_expr, SymbTablePkg_integer_type());
  }

    /* resize (for signed and unsigned words): word [M] * N -> word [N] */
  case WRESIZE: {
    SymbTable_ptr symb_table = TypeChecker_get_symb_table(
                                        TYPE_CHECKER(NODE_WALKER(self)->master));

    SymbType_ptr type1 = _THROW(car(expr), context);

    if (SymbType_is_error(type1)) { /* earlier error */
      return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
    }

    if (!SymbType_is_word(type1)) { /* ERROR */
      if (_VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }
      else { /* return arbitrary Word type */
        return _SET_TYPE(ctx_expr, SymbTablePkg_unsigned_word_type(1));
      }
    }

    /* Check the correctness of new size */
    {
      node_ptr _size = CompileFlatten_resolve_number(symb_table,
                                                     cdr(expr),
                                                     context);

      if ((Nil == _size) || (NUMBER != node_get_type(_size))) {
        /*
          We are not able to resolve the new size as a constant.
          This can be caused by:
          - Variable, but well typed number (eg a range variable)
          - Malformed expression (e.g. an unresolvable identifier)
        */
        SymbType_ptr test_type = _THROW(cdr(expr), context);
        if (SymbType_is_error(test_type)) {
          /* Malformed expression */
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }
        else {
          /* Non-constant expression */
          if (_VIOLATION(TC_VIOLATION_UNCONSTANT_EXPRESSION, ctx_expr)) {
            return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
          }
          else {
            return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
          }
        }
      }

      int newWidth = node_get_int(_size);

      if ((0 < newWidth) && newWidth <= WordNumber_max_width()) {
        /* Return the type of second operand */
        return _SET_TYPE(ctx_expr,
                         SymbType_is_signed_word(type1)
                         ? SymbTablePkg_signed_word_type(newWidth)
                         : SymbTablePkg_unsigned_word_type(newWidth));
      }
    }

    if (_VIOLATION(TC_VIOLATION_OUT_OF_WORD_WIDTH, ctx_expr)) {
      return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
    }

    /* return arbitrary Word type */
    return _SET_TYPE(ctx_expr, SymbTablePkg_unsigned_word_type(1));
  }

    /* extend (for signed and unsigned words): word [N] * M -> word [N+M] */
  case EXTEND: {
    SymbTable_ptr st = TypeChecker_get_symb_table(
                                       TYPE_CHECKER(NODE_WALKER(self)->master));

    /* get the operand' type */
    SymbType_ptr type = _THROW(car(expr), context);

    if (SymbType_is_error(type)) { /* earlier error */
      return _SET_TYPE(ctx_expr, type);
    }

    /* At the moment, second operand can be NUMBER
       or a const_expr that can be evaluated to NUMBER */

    /*just consistency check*/
    node_ptr delta = CompileFlatten_resolve_number(st, cdr(expr), context);
    nusmv_assert(Nil != delta && NUMBER == node_get_type(delta));

    int deltaWidth = node_get_int(delta);

    /* check the first operand type */
    if (!SymbType_is_word(type)) { /* ERROR */
      if (_VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }
      else { /* return arbitrary Word type */
        return _SET_TYPE(ctx_expr, SymbTablePkg_unsigned_word_type(1));
      }
    }

    int oldWidth = SymbType_get_word_width(type);
    if (0 <= deltaWidth && deltaWidth+oldWidth<=WordNumber_max_width()) {
      return _SET_TYPE(ctx_expr,
                       SymbType_is_unsigned_word(type)
                       ? SymbTablePkg_unsigned_word_type(oldWidth + deltaWidth)
                       : SymbTablePkg_signed_word_type(oldWidth + deltaWidth));
    }

    if (_VIOLATION(TC_VIOLATION_OUT_OF_WORD_WIDTH, ctx_expr)) {
      return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
    }

    /* return arbitrary Word type */
    return _SET_TYPE(ctx_expr, SymbTablePkg_unsigned_word_type(1));
  }


    /* 'init' and 'next' do not change the type of the expression */
  case SMALLINIT:
  case NEXT:
    {
      /* apply find_atom on the 'init' or 'next' expression and check
         the obtained expression for being already checked.

         The obtained expression will be the same as original one but
         some parts of NuSMV generate 'init' and 'next' expression
         with find_node, so both original and find_atom'ed expression
         should be checked
      */
      node_ptr normalisedExpr = find_atom(expr);
      SymbType_ptr type;
      /*wrap expression into context.This is required by tc_get_expression_type*/
      if (Nil != context) {
        normalisedExpr = find_node(CONTEXT, context, normalisedExpr);
      }

      type = _GET_TYPE(normalisedExpr);
      /* find_atom'ed expression has been already checked. Remeber and return */
      if (nullType != type) {
        return _SET_TYPE(ctx_expr, type);
      }

      /* check the expression now */
      type = _THROW(car(expr), context);
      /* associate both the original expression and the find_atom'ed one.*/
      _SET_TYPE(ctx_expr, type);
      if (normalisedExpr != ctx_expr) {
        _SET_TYPE(normalisedExpr, type);
      }
      return type;
    }


    /* binary: w[N]*w[N] or w[N]*boolean or boolean*W[N] or boolean*boolean,
       where w is an unsigned word or signed word type.
       the result is of unsigned word type always.*/
  case CONCATENATION:
    {
      /* get the operands' type */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2 = _THROW(cdr(expr), context);

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* both operands must be Word (signed or unsigned) */
      if (SymbType_is_word(type1) && SymbType_is_word(type2)) {
        int newWidth = SymbType_get_word_width(type1) +
          SymbType_get_word_width(type2);
        return _SET_TYPE(ctx_expr, SymbTablePkg_unsigned_word_type(newWidth));
      }

      /* is this a type error ? */
      if (_VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> create Word type of some width*/
      return _SET_TYPE(ctx_expr, SymbTablePkg_unsigned_word_type(1));
    }

    /* first operand is Word[N] (signed or unsigned),
       then two numbers K and L, K >=L and N>=K.
       The result is unsigned Word[K-L+1] */
  case BIT_SELECTION:
    {
      SymbTable_ptr st = TypeChecker_get_symb_table(
                                  TYPE_CHECKER(NODE_WALKER(self)->master));
      /* get the operand' type */
      SymbType_ptr type = _THROW(car(expr), context);
      int width;
      int highBound;
      int lowBound;
      node_ptr l;
      node_ptr h;

      if (SymbType_is_error(type)) { /* earlier error */
        return _SET_TYPE(ctx_expr, type);
      }

      /* At the moment, second and third operands must be NUMBERS */

      l = CompileFlatten_resolve_number(st, cdr(cdr(expr)), context);
      h = CompileFlatten_resolve_number(st, car(cdr(expr)), context);

      /*just consistency check*/
      nusmv_assert(COLON == node_get_type(cdr(expr)));

      /* Check that the range is formed by constant expression that
         can resolve into a NUMBER */
      if ((Nil == l || Nil == h) ||
          (NUMBER != node_get_type(l) || NUMBER != node_get_type(h))) {
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
      highBound = NODE_TO_INT(car(h));
      lowBound  = NODE_TO_INT(car(l));

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

  case WAWRITE:
    {
      /* WRITE : WORDARRAY[a,v] x WORD[a] x WORD[v] -> WORDARRAY[a,v] */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2 = _THROW(car(cdr(expr)), context);
      SymbType_ptr type3 = _THROW(cdr(cdr(expr)), context);

      nusmv_assert(node_get_type(cdr(expr)) == WAWRITE);

      if (SymbType_is_error(type1) || SymbType_is_error(type2) ||
          SymbType_is_error(type3)) {
        /* handle error in arguments */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* ERROR with kinds of subexpressions */
      if (SymbType_get_tag(type1) != SYMB_TYPE_WORDARRAY ||
          !SymbType_is_unsigned_word(type2) ||
          !SymbType_is_unsigned_word(type3)) {
        if (_VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr)) {
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }
        else { /* return arbitrary WordArray type */
          return _SET_TYPE(ctx_expr, SymbTablePkg_wordarray_type(1,1));
        }
      }

      /* ERROR with the widths */
      if ((SymbType_get_wordarray_awidth(type1) !=
           SymbType_get_word_width(type2)) ||
          (SymbType_get_wordarray_vwidth(type1) !=
           SymbType_get_word_width(type3))) {
        if (_VIOLATION(TC_VIOLATION_OUT_OF_WORDARRAY_WIDTH, ctx_expr)) {
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }
      }

      /* No errors, then return the type of the first argument
         (whatever it is) */
      return _SET_TYPE(ctx_expr, type1);
    }

  case WAREAD:
    {
      /* WORDARRAY[a,v] x WORD[a] -> WORD[v] */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2 = _THROW(cdr(expr), context);

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* handle error in arguments */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* ERROR with kinds of subexpressions */
      if (SymbType_get_tag(type1) != SYMB_TYPE_WORDARRAY ||
          !SymbType_is_unsigned_word(type2)) {
        if (_VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr)) {
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }
        else { /* return arbitrary WordArray type */
          return _SET_TYPE(ctx_expr, SymbTablePkg_wordarray_type(1, 1));
        }
      }

      /* ERROR with the widths */
      if (SymbType_get_wordarray_awidth(type1) !=
          SymbType_get_word_width(type2)) {
        if (_VIOLATION(TC_VIOLATION_OUT_OF_WORDARRAY_WIDTH, ctx_expr)) {
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }
      }

      /* No errors, then return WORD[v] */
      return _SET_TYPE(ctx_expr,
                       SymbTablePkg_unsigned_word_type(SymbType_get_wordarray_vwidth(type1)));
    }

    /* unary minus */
  case UMINUS:
    {
      /* get the operands' type */
      SymbType_ptr type = _THROW(car(expr), context);

      if (SymbType_is_error(type)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* correct types are boolean, int ,b*b->i, i*i->i, r*r->r,
         w[n]*w[n]->w[n] */
      if (nullType != type &&
          (SymbType_get_tag(type) == SYMB_TYPE_INTEGER ||
           SymbType_get_tag(type) == SYMB_TYPE_REAL ||
           SymbType_is_word(type))) {
        return _SET_TYPE(ctx_expr, type);
      }

      /* is this a type error ? */
      if (_VIOLATION(SymbType_is_back_comp(type) ?
                     TC_VIOLATION_TYPE_BACK_COMP :
                     TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> keep one of the types
         whatever it is*/
      return _SET_TYPE(ctx_expr, type);
    }

    /* arithmetic operators: b*b->i, i*i->i, r*r->r, w[n]*w[n]->w[n]
       + implicit conversion
    */
  case PLUS:
  case MINUS:
  case TIMES:
  case DIVIDE:
    {
      /* get the operands' type */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2;
      SymbType_ptr returnType;

      /* PLUS and MINUS may be unary */
      if (cdr(expr) != (node_ptr) NULL) type2 = _THROW(cdr(expr), context);
      else {
        /* only PLUS and MINUS are supposed to be possibly unary */
        nusmv_assert(node_get_type(expr) == PLUS ||
                     node_get_type(expr) == MINUS);
        type2 = type1;
      }

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* implicit conversion of one of the operands */
      returnType = SymbType_get_greater(type1, type2);

      /* correct types are int ,b*b->i, i*i->i, r*r->r,
         w[n]*w[n]->w[n] */
      if (nullType != returnType &&
          (SymbType_get_tag(returnType) == SYMB_TYPE_INTEGER ||
           SymbType_get_tag(returnType) == SYMB_TYPE_REAL ||
           SymbType_is_word(returnType))) {
        return _SET_TYPE(ctx_expr, returnType);
      }

      /* is this a type error ? */
      if (_VIOLATION((SymbType_is_back_comp(type1) &&
                      SymbType_is_back_comp(type2)) ?
                     TC_VIOLATION_TYPE_BACK_COMP :
                     TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> keep one of the types
         whatever it is*/
      return _SET_TYPE(ctx_expr, type1);
    }

    /* mod operator: i*i->i, w[n]*w[n]->w[n] (no implicit conversion)
       where w can be signed or unsigned word */
  case MOD:
    {
      /* get the operands' type */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2 = _THROW(cdr(expr), context);

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* w[N] * w[N] -> w[N] */
      if (type1 == type2 && /*types are shared => same types == same pointers*/
          SymbType_is_word(type1)) {
        return _SET_TYPE(ctx_expr, type1);
      }

      /* int * int -> int, int * 2 -> bool */
      if (type1 == type2 && /*types are shared => same types == same pointers*/
          SymbType_get_tag(type1) == SYMB_TYPE_INTEGER) {
        return _SET_TYPE(ctx_expr, type1);
      }

      /* is this a type error ? */
      if (_VIOLATION((SymbType_is_back_comp(type1) &&
                      SymbType_is_back_comp(type2)) ?
                     TC_VIOLATION_TYPE_BACK_COMP :
                     TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> keep one of the types
         whatever it is*/
      return _SET_TYPE(ctx_expr, type1);
    }

    /* left operand is word (signed or unsigned) and
       the right is Int, Bool or unsigned word */
  case LSHIFT: case RSHIFT: case LROTATE: case RROTATE:
    {
      /* get the operands' type */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2 = _THROW(cdr(expr), context);

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* correct types are  w[n]*int */
      if (SymbType_is_word(type1) &&
          (SymbType_get_tag(type2) == SYMB_TYPE_INTEGER ||
           SymbType_is_unsigned_word(type2))) {
        /* checks bound */
        int width = SymbType_get_word_width(type1);
        int sval;

        switch (node_get_type(cdr(expr))) {
        case NUMBER: sval = node_get_int(cdr(expr)); break;
        case NUMBER_UNSIGNED_WORD:
          sval = WordNumber_get_unsigned_value(WORD_NUMBER(car(cdr(expr))));
          break;
        case NUMBER_SIGNED_WORD:
          sval = WordNumber_get_signed_value(WORD_NUMBER(car(cdr(expr))));
          break;
        default: sval = -1;
        }

        /* shifting value too high */
        if (sval > width) {
          if (_VIOLATION(TC_VIOLATION_OUT_OF_WORD_WIDTH, ctx_expr)) {
            return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
          }
        }

        return _SET_TYPE(ctx_expr, type1);
      }

      /* is this a type error ? */
      if (_VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> keep the left operand type */
      return _SET_TYPE(ctx_expr, type1);
    }


    /* the operands should be implicitly converted to one type,
       otherwise they must be either boolean or Word[1]
    */
  case EQUAL:
  case NOTEQUAL:
    {
      /* get the operands' type */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2 = _THROW(cdr(expr), context);
      SymbType_ptr returnType;

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* implicit conversion of one of the operands */
      returnType = SymbType_get_greater(type1, type2);

      /* returned type is enum, int, real, word or word-array
         or operands are boolean with Unsigned Word[1].
         Virtually, only set types cannot be compared.
      */
      if ( (nullType != returnType &&
            (SymbType_get_tag(returnType) == SYMB_TYPE_ENUM ||
             SymbType_get_tag(returnType) == SYMB_TYPE_INTEGER ||
             SymbType_get_tag(returnType) == SYMB_TYPE_SIGNED_WORD ||
             SymbType_get_tag(returnType) == SYMB_TYPE_UNSIGNED_WORD ||
             SymbType_get_tag(returnType) == SYMB_TYPE_WORDARRAY ||
             SymbType_get_tag(returnType) == SYMB_TYPE_REAL ||
             SymbType_get_tag(returnType) == SYMB_TYPE_BOOLEAN)) ) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
      }
      /* is this a type error ? */
      if (_VIOLATION(SymbType_is_back_comp(type1) &&
                     SymbType_is_back_comp(type2) ?
                     TC_VIOLATION_TYPE_BACK_COMP :
                     TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return the boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }


    /* Assignment. The right operand should be implicitly converted to the
       left operand's type (or its set-type counterpart),
       otherwise they must be boolean with Word[1].
       The type of the whole expression is statement-type.
       It is required to process EQDEF with other expressions since
       in symbolic-FSM after predicate-normalisation EQDEF can become
       a part of case-expression in init, trans, etc, lists.

       NB: the left operand is not checked for being a variable since
       parser guarantees this.
    */
  case EQDEF:
    {
      /* get the operands' type */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2 = _THROW(cdr(expr), context);
      SymbType_ptr returnType;

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* If the  right operand is of a set-type, convert the set-type to a
         usual one.
         Convert the right operand's type then to the type of the left
         operands.
      */
      returnType = SymbType_convert_right_to_left(type1,
                                                  SymbType_make_from_set_type(type2));

      /* returned type is any enum-type, int, real, word or
         word-array
         Virtually, only set types cannot be compared.
      */
      if ( (nullType != returnType &&
            (SymbType_get_tag(returnType) == SYMB_TYPE_ENUM ||
             SymbType_get_tag(returnType) == SYMB_TYPE_INTEGER ||
             SymbType_get_tag(returnType) == SYMB_TYPE_UNSIGNED_WORD ||
             SymbType_get_tag(returnType) == SYMB_TYPE_SIGNED_WORD ||
             SymbType_get_tag(returnType) == SYMB_TYPE_WORDARRAY ||
             SymbType_get_tag(returnType) == SYMB_TYPE_REAL ||
             SymbType_get_tag(returnType) == SYMB_TYPE_BOOLEAN)) ) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
      }

      /* is this a type error ? */
      if (_VIOLATION(SymbType_is_back_comp(type1) &&
                     SymbType_is_back_comp(type2) ?
                     TC_VIOLATION_TYPE_BACK_COMP :
                     TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return the statement-type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }


    /*  allowed types(with implicit conversion):
         i*i, r*r, w[n]*w[n]
        w can be signed or unsigned (in both operands).
    */
  case LT:
  case GT:
  case LE:
  case GE:
    {
      /* get the operands' type */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2 = _THROW(cdr(expr), context);
      SymbType_ptr returnType;

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* implicit conversion on both operands */
      returnType = SymbType_get_greater(type1, type2);

      /* return type is not-null and arithmetic
         or operands or Word[1] */
      if ( (nullType != returnType &&
            (SymbType_get_tag(returnType) == SYMB_TYPE_INTEGER ||
             SymbType_get_tag(returnType) == SYMB_TYPE_REAL ||
             SymbType_get_tag(returnType) == SYMB_TYPE_UNSIGNED_WORD ||
             SymbType_get_tag(returnType) == SYMB_TYPE_SIGNED_WORD)) ) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
      }
      /* is this a type error ? */
      if (_VIOLATION(SymbType_is_back_comp(type1) &&
                     SymbType_is_back_comp(type2) ?
                     TC_VIOLATION_TYPE_BACK_COMP :
                     TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return the boolean type*/
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }

    /* the operator are boolean or Words of the same size */
  case OR:
  case XOR:
  case XNOR:
  case IMPLIES:
  case IFF:
    {
      /* get the operands' type */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2 = _THROW(cdr(expr), context);

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* both operands must be boolean or Word of the same size */
      if ((SymbType_is_boolean(type1) && SymbType_is_boolean(type2)) ||
          (SymbType_is_word(type1) && type1 == type2)) {
        return _SET_TYPE(ctx_expr, type1);
      }

      /* is this a type error ? */
      if (_VIOLATION(SymbType_is_back_comp(type1) &&
                     SymbType_is_back_comp(type2) ?
                     TC_VIOLATION_TYPE_BACK_COMP :
                     TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> keep one of the types
         whatever it is*/
      return _SET_TYPE(ctx_expr, type1);
    }


    /* AND is an unusual binary operator because sometimes it is used
       as connector in expression lists.
       The distinction from usual binary logical operator:
       1. if one of the operands is Nill => it is skipped
       2. operands can be boolean
       3. operands can be statement-type. This can happen only for
       artificially created expressions (not parsed ones)
    */
  case AND:
    {
      SymbType_ptr type1;
      SymbType_ptr type2;
      if (Nil == car(expr)) {
        type2 = _THROW(cdr(expr), context);
        return _SET_TYPE(ctx_expr, type2);
      }

      if (Nil == cdr(expr)) {
        type1 = _THROW(car(expr), context);
        return _SET_TYPE(ctx_expr, type1);
      }

      /* get the operands' type */
      type1 = _THROW(car(expr), context);
      type2 = _THROW(cdr(expr), context);

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* both operands must be boolean or Word of the same size */
      if ((SymbType_is_boolean(type1) && SymbType_is_boolean(type2)) ||
          (SymbType_is_word(type1) && type1 == type2)) {
        return _SET_TYPE(ctx_expr, type1);
      }

      /* AND is a list connector; operands must be boolean or statement-type */
      if (SymbType_get_tag(type1) == SYMB_TYPE_STATEMENT &&
          SymbType_get_tag(type2) == SYMB_TYPE_STATEMENT) {
        return _SET_TYPE(ctx_expr, type1);
      }
      if ( (SymbType_is_boolean(type1) ||
            SymbType_get_tag(type1) == SYMB_TYPE_STATEMENT) &&
           (SymbType_is_boolean(type2) ||
            SymbType_get_tag(type2) == SYMB_TYPE_STATEMENT) ) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
      }

      /* is this a type error ? */
      if (_VIOLATION(SymbType_is_back_comp(type1) &&
                     SymbType_is_back_comp(type2) ?
                     TC_VIOLATION_TYPE_BACK_COMP :
                     TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> keep one of the types whatever it is*/
      return _SET_TYPE(ctx_expr, type1);
    }


    /* both operands are converted to (corresponding) set-types and
       then to the minimal common type.
       As result only the following types can be here:
       any of enum types (bool, symb, int-symb), int  or any of set type
       (i.e. integer-set, symbolic-set, integer-symbolic-set)
    */
  case UNION:
    {
      /* get the operands' type */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2 = _THROW(cdr(expr), context);
      SymbType_ptr returnType;

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* convert operands to set types (if this is possible) and then
         convert them to one type.
      */
      {
        SymbType_ptr tmp1 = SymbType_make_set_type(type1);
        SymbType_ptr tmp2 = SymbType_make_set_type(type2);

        if (nullType != tmp1 && nullType != tmp2) {
          returnType = SymbType_get_minimal_common(tmp1, tmp2);
        }
        else returnType = nullType;
      }

      /* both types are convertable to a set type */
      if (nullType != returnType) {
        return _SET_TYPE(ctx_expr, returnType);
      }

      /* is this a type error ? */
      if (_VIOLATION(SymbType_is_back_comp(type1) &&
                     SymbType_is_back_comp(type2) ?
                     TC_VIOLATION_TYPE_BACK_COMP :
                     TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return the boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }

    /* both operands are converted to (corresponding) set-types and
       then ONE of the operand is converted to the type of the othe one.
       As result only the following types can be here:
       any of enum types (bool, symb, int-symb), int  or any of set type
       (i.e. integer-set, symbolic-set, integer-symbolic-set)
    */
  case SETIN:  {
    /* get the operands' type */
    SymbType_ptr type1 = _THROW(car(expr), context);
    SymbType_ptr type2 = _THROW(cdr(expr), context);
    SymbType_ptr returnType;

    if (SymbType_is_error(type1) || SymbType_is_error(type2)){/*earlier error*/
      return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
    }

    /* convert operands to set types (if this is possible) and then
       convert one of the them to the other one.
    */
    {
      SymbType_ptr tmp1 = SymbType_make_set_type(type1);
      SymbType_ptr tmp2 = SymbType_make_set_type(type2);

      if (nullType != tmp1 && nullType != tmp2) {
        returnType = SymbType_get_greater(tmp1, tmp2);
      }
      else returnType = nullType;
    }

    /* Note that the types should be convertable to one of the set types (
       and be therefore, any of enum (i.e. Bool, Symb and IntSymb),
       Int, any of set types (i.e. int-set, symb-set, int-symb-set)).
       Real, Word and WordArray should not be here because
       even though assignment internally behaves the same way as
       "in"-operatoris, assignment is never syntactically converted to
       SETIN expression.
    */
    if (nullType != returnType) {
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }

    /* is this a type error ? */
    if (_VIOLATION(SymbType_is_back_comp(type1) &&
                   SymbType_is_back_comp(type2) ?
                   TC_VIOLATION_TYPE_BACK_COMP :
                   TC_VIOLATION_TYPE_MANDATORY,
                   ctx_expr)) {
      return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
    }

    /* this is not an error after all -> return the boolean type */
    return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
  }

    /* left expression is bool, and the union of right expression
       and the next case (if it is not the end of list) is returned.
    */
  case IFTHENELSE:
  case CASE:
    {
      /* get the operands' type */
      SymbType_ptr condType = _THROW(car(car(expr)), context);
      SymbType_ptr expType = _THROW(cdr(car(expr)), context);
      SymbType_ptr nextCaseType;
      SymbType_ptr returnType;

      if (SymbType_is_error(condType) || SymbType_is_error(expType)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* left operand of ':' should be boolean */
      if ( (!SymbType_is_boolean(condType)) &&
           _VIOLATION(SymbType_is_back_comp(condType) ?
                      TC_VIOLATION_TYPE_BACK_COMP :
                      TC_VIOLATION_TYPE_MANDATORY,
                      Nil == context ?
                      car(expr) :
                      find_node(CONTEXT, context, car(expr)))) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is the last element of case-list */
      if (FAILURE == node_get_type(cdr(expr))) {
        return _SET_TYPE(ctx_expr, expType);
      }

      /* there are more case-elements in the list */
      nextCaseType = _THROW(cdr(expr), context);
      if (SymbType_is_error(nextCaseType)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* both CASE expressions should be convertable to a common type.
         If one of the expressions is of a set-type then before implicit
         convertion the other expression is converted to
         a corresponding set-type.
      */
      {
        /* if one of the type is a set-type, convert also the other one to
           a corresponding set type
        */
        SymbType_ptr tmp1 = SymbType_is_set(nextCaseType) ?
          SymbType_make_set_type(expType) : expType;

        SymbType_ptr tmp2 = SymbType_is_set(expType) ?
          SymbType_make_set_type(nextCaseType) : nextCaseType;

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
      if (_VIOLATION(SymbType_is_back_comp(expType) &&
                     SymbType_is_back_comp(nextCaseType) ?
                     TC_VIOLATION_TYPE_BACK_COMP :
                     TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return one of the types  */
      return _SET_TYPE(ctx_expr, expType);
    }


    /* both operands are boolean */
  case AU:
  case EU:
  case UNTIL:
  case SINCE:
    {
      /* get the operands' type */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2 = _THROW(cdr(expr), context);

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        /*earlier error*/
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* both operands must be boolean */
      if (SymbType_is_boolean(type1) && SymbType_is_boolean(type2)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
      }

      /* is this a type error ? */
      if (_VIOLATION(SymbType_is_back_comp(type1) &&
                     SymbType_is_back_comp(type2) ?
                     TC_VIOLATION_TYPE_BACK_COMP :
                     TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }

    /* check the number..number part and the A-U or E-U expressions */
  case ABU:
  case EBU:
    {
      SymbType_ptr type = _THROW(car(expr), context);

      /* Check real-time specifications range */
      if (Nil != cdr(expr) && !Utils_check_subrange_not_negative(cdr(expr))) {
        if (_VIOLATION(TC_VIOLATION_INVALID_RANGE, ctx_expr)) {
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }
      }

      return _SET_TYPE(ctx_expr, type);
    }

  case FAILURE: /* FAILURE is a part of CASE-expression.
                   Should be analysed there.
                   NB: If a more complex use of FAILURE will be adopted,
                   then a new failure-type should be created, and
                   all expression-checking procedures should take into
                   account this type.*/
    /* This is needed
       to deal with the expansion of a case expression into the
       corresponding boolean formula (C & T) | (!C & E) performed in ltl2smv
       originally return _SET_TYPE(ctx_expr, SymbTablePkg_error_type()); */
    return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());

  case COLON: /* COLON is a part of BIT_SELECTION or CASE.
                 Should be analysed there
              */
    error_unreachable_code();

    /* these are used into COMPUTE statement */
  case MINU:
  case MAXU:
    {
      /* get the operands' type */
      SymbType_ptr type1 = _THROW(car(expr), context);
      SymbType_ptr type2 = _THROW(cdr(expr), context);

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type()); /*earlier error*/
      }

      /* both operands must be boolean */
      if (SymbType_is_boolean(type1) && SymbType_is_boolean(type2)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
      }

      /* is this a type error ? */
      if (_VIOLATION(SymbType_is_back_comp(type1) &&
                     SymbType_is_back_comp(type2) ?
                     TC_VIOLATION_TYPE_BACK_COMP :
                     TC_VIOLATION_TYPE_MANDATORY,
                     ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return boolean type */
      return _SET_TYPE(ctx_expr, SymbTablePkg_boolean_type());
    }

    /* The type of a list is the least common type of its expressions.
       CONS is NOT a possible input (parsed) expression, but can only be
       generated inside NuSMV.  At the moment at least GENREACTIVITY
       and BUCHIGAME
       specification generates such expressions.  The invoker should
       know what he is doing when CONS expression is type checked.
    */
  case CONS:
    {
      SymbType_ptr type1;
      SymbType_ptr type2;
      SymbType_ptr returnType;

      if (Nil == cdr(expr)) { /* one element list */
        type1 = _THROW(car(expr), context);
        return _SET_TYPE(ctx_expr, type1);
      }

      /* get the operands' type */
      type1 = _THROW(car(expr), context);
      type2 = _THROW(cdr(expr), context);

      if (SymbType_is_error(type1) || SymbType_is_error(type2)) {/* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* implicit conversion to the least common type of the operands */
      returnType = SymbType_get_minimal_common(type1, type2);

      /* types can be implicitily converted to one type */
      if (nullType != returnType) {
        return _SET_TYPE(ctx_expr, returnType);
      }
      /* is this a type error ? */
      if (_VIOLATION(TC_VIOLATION_TYPE_MANDATORY, ctx_expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* this is not an error after all -> return left operand's type */
      return _SET_TYPE(ctx_expr, type1);
    }

  case ARRAY_DEF:
    {
      int size;
      SymbType_ptr type = SYMB_TYPE(NULL);
      node_ptr iter;

      nusmv_assert(Nil == cdr(expr));

      /* iterate over array element and obtain their common type */
      for (size = 0, iter = car(expr);
           iter != Nil;
           size += 1, iter = cdr(iter)) {

        /* array is a list of expressions */
        nusmv_assert(CONS == node_get_type(iter));

        SymbType_ptr elemType = _THROW(car(iter), context);
        if (SymbType_is_error(elemType)) { /* error */
          return _SET_TYPE(ctx_expr, elemType);
        }
        else if (NULL == type) { /* remember the type of first element */
          type = elemType;
        }
        else {  /* infer the common type from current and previous one */
          SymbType_ptr common = SymbType_get_minimal_common(elemType, type);
          /* check if compatible type was created */
          if (NULL == common) {
            return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
          }
          type = common;
        }
      } /* for */

      type = SymbTablePkg_array_type(type, 0, size-1);
      return _SET_TYPE(ctx_expr, type);
    }

  case NFUNCTION:
    {
      SymbTable_ptr st = TypeChecker_get_symb_table(
                             TYPE_CHECKER(NODE_WALKER(self)->master));

      node_ptr name = car(expr);
      node_ptr res_name;
      NFunction_ptr fun;

      node_ptr actual_args = cdr(expr);
      NodeList_ptr formal_args;
      ListIter_ptr formal_iter;

      int actual_num = 0;
      int formal_num;

      SymbType_ptr res;

      boolean has_bitvector = false;
      boolean has_real_int = false;

      /* First of all, get the complete contextualized name */
      {
        ResolveSymbol_ptr rs;
        rs = SymbTable_resolve_symbol(st, name, context);
        res_name = ResolveSymbol_get_resolved_name(rs);
      }

      /* Check if the function is defined.. */
      if (!SymbTable_is_symbol_function(st, res_name)) {
        _VIOLATION(TC_VIOLATION_UNDEF_IDENTIFIER, res_name);
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      fun = SymbTable_get_function(st, res_name);
      formal_args = NFunction_get_args(fun);
      formal_num = NFunction_get_args_number(fun);
      formal_iter = NodeList_get_first_iter(formal_args);

      /* Loop over actual parameters and compare the type with the
         formal ones */
      while (Nil != actual_args) {
        SymbType_ptr actual_type;
        SymbType_ptr formal_type;
        SymbType_ptr shared_formal_type;
        SymbType_ptr cmp_type;

        /* There are more actual parameters than formal ones! */
        if (actual_num >= formal_num) {
          _VIOLATION(TC_VIOLATION_PARAMS_NUM_ERROR, expr);
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }

        nusmv_assert(!ListIter_is_end(formal_iter));

        formal_type = (SymbType_ptr)NodeList_get_elem_at(formal_args, formal_iter);
        shared_formal_type = SymbType_make_memory_shared(formal_type);

        /* Recursively compute the type of the actual parameter expression */
        actual_type = _THROW(car(actual_args), context);

        if (SymbType_is_error(actual_type)) {
          /* earlier error */
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }

        /* Check parameters to be all of the same family */
        if (SymbType_is_word(formal_type)) {
          has_bitvector = true;
        }
        else if (SymbType_is_real(formal_type) ||
                 SymbType_is_pure_int_enum(formal_type) ||
                 SymbType_is_integer(formal_type)) {
          has_real_int = true;
        }

        /* implicit conversion of one of the operands */
        cmp_type = SymbType_get_greater(actual_type, shared_formal_type);

        if (nullType == cmp_type) {
          _VIOLATION(TC_VIOLATION_PARAMS_TYPE_ERROR, expr);
          return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
        }

        actual_args = cdr(actual_args);
        formal_iter = ListIter_get_next(formal_iter);
        ++actual_num;
      }

      /* Some actual parameters are missing! */
      if (formal_num != actual_num) {
        _VIOLATION(TC_VIOLATION_PARAMS_NUM_ERROR, expr);
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      /* Return the shared version of the function return type */
      res = NFunction_get_return_type(fun);

      /* Check parameters to be all of the same family */
      if (SymbType_is_word(res)) {
        has_bitvector = true;
      }
      else if (SymbType_is_real(res) ||
               SymbType_is_pure_int_enum(res) ||
               SymbType_is_integer(res)) {
        has_real_int = true;
      }

      /* Mixed parameters are not allowed by MathSAT. Currently, due
         to theory combination issues, only */
      if (has_bitvector && has_real_int) {
        _VIOLATION(TC_VIOLATION_DIFFERENT_TYPE_PARAMS_ERROR, expr);
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type());
      }

      res = SymbType_make_memory_shared(res);
      return _SET_TYPE(ctx_expr, res);
    }
    break;

  default: /* there are no other kinds of expressions */
    fprintf(nusmv_stderr, "*** type_checker_check_expression:\n");
    fprintf(nusmv_stderr, "*** UNABLE TO HANDLE EXPRESSION: ");
    print_node(nusmv_stderr, expr);
    fprintf(nusmv_stderr, "\n");
    error_unreachable_code();
  } /* switch */

  return nullType;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


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
   TC_VIOLATION_TYPE_WARNING violation outputs a warning. Only in
   this case the false value is returned, indicating that this is NOT
   an error. Otherwise the true value is returned, indicating that this
   is an error.

   Also, if the system variable "type_check_warning_on" is false,
   warning messages are not output.

   NB: if the expression is checked in some context (context is not null) then
   before providing the expression to this function the expression should be
   wrapped into context, i.e. with find_node(CONEXT, context, expr)]

   SideEffects       []

   SeeAlso           [TypeSystemViolation]

******************************************************************************/
static boolean
checker_core_viol_handler(CheckerBase_ptr self,
                          TypeSystemViolation violation, node_ptr expression)
{
  /* In the output message, the information about the expression
     location are output. So, make sure that the input file name and
     line number are correctly set!
  */

  boolean isError = true; /* is this error or warning */

  /* get rid of the context the expression may be wrapped in */
  node_ptr context = Nil;
  node_ptr expr = expression;

  if (node_get_type(expression) == CONTEXT) {
    context = car(expression);
    expr = cdr(expression);
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
  case TC_VIOLATION_UNDEF_IDENTIFIER:
    fprintf(nusmv_stderr,  "undefined identifier '");
    print_node(nusmv_stderr, expr);
    fprintf(nusmv_stderr, "'\n");
    break;

  case TC_VIOLATION_AMBIGUOUS_IDENTIFIER:
    fprintf(nusmv_stderr,  "identifier '");
    print_node(nusmv_stderr, expr);
    fprintf(nusmv_stderr, "' is ambiguous\n");
    break;

  case TC_VIOLATION_OUT_OF_WORD_WIDTH:
    if (BIT_SELECTION == node_get_type(expr)) {
      fprintf(nusmv_stderr,  "out of Word's width index or "
              "first index smaller than the second one ('");
    }
    else if (EXTEND == node_get_type(expr)){
      fprintf(nusmv_stderr,  "word width is out of range [1..%d] ('",
              WordNumber_max_width());
    }
    else if (LSHIFT == node_get_type(expr) || RSHIFT == node_get_type(expr)) {
      fprintf(nusmv_stderr,  "word shift is out of range [0..width] ('");
    }
    else if (LROTATE == node_get_type(expr) || RROTATE == node_get_type(expr)) {
      fprintf(nusmv_stderr,  "word rotate is out of range [0..width] ('");
    }
    else {
      /* only shift, rotate, bit-selection and word-extend may
         have this kind of error */
      error_unreachable_code();
    }
    print_operator(nusmv_stderr, expr);
    fprintf(nusmv_stderr,  "' operator)\n");
    break;

  case TC_VIOLATION_OUT_OF_WORDARRAY_WIDTH:
    nusmv_assert(WAWRITE == node_get_type(expr) ||
                 WAREAD == node_get_type(expr));
    if (WAWRITE == node_get_type(expr)) {
      fprintf(nusmv_stderr,
              "in WRITE expression the width of address or value operands"
              " is not consistent with word-array operand\n");
    }
    else if (WAREAD == node_get_type(expr)) {
      fprintf(nusmv_stderr,
              "in READ operator the width of address operand"
              " is not consistent with word-array operand\n");
    }
    else {
      /* only WRITE or READ (of word-array) may have this kind of error */
      error_unreachable_code();
    }
    break;

  case TC_VIOLATION_PARAMS_NUM_ERROR:
    fprintf(nusmv_stderr, "Wrong number of parameters for function call '");
    print_node(nusmv_stderr, expr);
    fprintf(nusmv_stderr, "'\n");
    break;

  case TC_VIOLATION_PARAMS_TYPE_ERROR:
    fprintf(nusmv_stderr, "Wrong type of parameters for function call '");
    print_node(nusmv_stderr, expr);
    fprintf(nusmv_stderr, "'\n");
    break;

  case TC_VIOLATION_UNCONSTANT_EXPRESSION:
    fprintf(nusmv_stderr, "Expected constant expression in '");
    print_node(nusmv_stderr, expr);
    fprintf(nusmv_stderr, "'\n");
    break;

  case TC_VIOLATION_INVALID_RANGE:
    fprintf(nusmv_stderr, "Invalid range in expression '");
    print_node(nusmv_stderr, expr);
    fprintf(nusmv_stderr, "'\n");
    break;

  case TC_VIOLATION_DIFFERENT_TYPE_PARAMS_ERROR:
    fprintf(nusmv_stderr,
            "Multiple function parameter types not supported in function call '");
    print_node(nusmv_stderr, expr);
    fprintf(nusmv_stderr, "'\n");
    fprintf(nusmv_stderr, "Supported sets are: [(BV|Bool) | (R|I|Bool)]\n");
    break;

  case TC_VIOLATION_TYPE_MANDATORY:
  case TC_VIOLATION_TYPE_BACK_COMP:
  case TC_VIOLATION_TYPE_WARNING:
    if (isError) fprintf(nusmv_stderr, "illegal ");
    else         fprintf(nusmv_stderr, "potentially incorrect ");

    switch (node_get_type(expr)) {
    case FAILURE: /* FAILURE does not participate in the type-checking */
    case ATOM: /* undeclared identifies checked earlier */
    case NUMBER:
    case NUMBER_UNSIGNED_WORD:
    case NUMBER_SIGNED_WORD:
    case NUMBER_REAL:
    case NUMBER_FRAC:
    case NUMBER_EXP:
    case TRUEEXP:
    case FALSEEXP:
    case BIT:   /* undeclared identifies checked earlier */
    case BOOLEAN:
    case SELF:
    case INTEGER:
    case REAL:
    case SCALAR:
    case ARRAY:
    case DOT:
    case TWODOTS:
    case NEXT:
    case SMALLINIT:

    case CONTEXT:
    case BDD:
    case SEMI:
      print_sexp(nusmv_stderr, expr);
      error_unreachable_code(); /* this is impossible */

    case WSIZEOF:
    case CAST_TOINT:
      fprintf(nusmv_stderr, "operand type of \"");
      print_operator(nusmv_stderr, expr);
      fprintf(nusmv_stderr,"\" : ");
      checker_base_print_type(self, nusmv_stderr, car(expr), context);
      break;

      /* possibly unary or binary operators */
    case PLUS:
    case MINUS:
    case UMINUS:
      fprintf(nusmv_stderr, "operand types of \"");
      print_operator(nusmv_stderr, expr);
      fprintf(nusmv_stderr,"\" : ");
      checker_base_print_type(self, nusmv_stderr, car(expr), context);
      if (cdr(expr) != (node_ptr) NULL) {
        fprintf(nusmv_stderr," and ");
        checker_base_print_type(self, nusmv_stderr, cdr(expr), context);
      }
      break;

      /* Binary operators */
    case CONCATENATION:
    case CONS:
    case OR:
    case XOR:
    case XNOR:
    case IMPLIES:
    case IFF:
    case EQDEF:
    case AND:
    case RELEASES:
    case TRIGGERED:
    case UNTIL:
    case SINCE:
    case AU:
    case EU:
    case LSHIFT:
    case RSHIFT:
    case LROTATE:
    case RROTATE:
    case TIMES:
    case DIVIDE:
    case MOD:
    case EQUAL:
    case NOTEQUAL:
    case LT:
    case GT:
    case LE:
    case GE:
    case UNION:
    case SETIN:
    case MINU:
    case MAXU:
    case WAREAD:
    case WRESIZE:
      fprintf(nusmv_stderr, "operand types of \"");
      print_operator(nusmv_stderr, expr);
      fprintf(nusmv_stderr,"\" : ");
      checker_base_print_type(self, nusmv_stderr, car(expr), context);
      fprintf(nusmv_stderr," and ");
      checker_base_print_type(self, nusmv_stderr, cdr(expr), context);
      break;

      /* Unary operators */
    case CAST_BOOL:
    case CAST_WORD1:
    case CAST_SIGNED:
    case CAST_UNSIGNED:
    case EXTEND:
    case NOT:
    case EX:
    case AX:
    case EF:
    case AF:
    case EG:
    case AG:
    case OP_GLOBAL:
    case OP_HISTORICAL:
    case OP_NEXT:
    case OP_PREC:
    case OP_NOTPRECNOT:
    case OP_FUTURE:
    case OP_ONCE:
      fprintf(nusmv_stderr,"operand type of \"");
      print_operator(nusmv_stderr, expr);
      fprintf(nusmv_stderr,"\" : ");
      checker_base_print_type(self, nusmv_stderr, car(expr), context);
      break;

    case COLON: /* for CASE */
      fprintf(nusmv_stderr,"left operand type of \":\". "
              "It should be boolean, but is ");
      checker_base_print_type(self, nusmv_stderr, car(expr), context);
      break;

    case BIT_SELECTION:
      fprintf(nusmv_stderr,"operand types of \"[ : ]\" : ");
      checker_base_print_type(self, nusmv_stderr, car(expr), context);
      fprintf(nusmv_stderr,", ");
      checker_base_print_type(self, nusmv_stderr, car(cdr(expr)), context);
      fprintf(nusmv_stderr," and ");
      checker_base_print_type(self, nusmv_stderr, cdr(cdr(expr)), context);
      break;

    case WAWRITE:
      fprintf(nusmv_stderr,"operand types of \"WRITE\" : ");
      checker_base_print_type(self, nusmv_stderr, car(expr), context);
      fprintf(nusmv_stderr,", ");
      checker_base_print_type(self, nusmv_stderr, car(cdr(expr)), context);
      fprintf(nusmv_stderr," and ");
      checker_base_print_type(self, nusmv_stderr, cdr(cdr(expr)), context);
      break;

    case CASE:
      fprintf(nusmv_stderr,"types of \"case\" list elements : ");
      checker_base_print_type(self, nusmv_stderr, cdr(car(expr)), context);
      fprintf(nusmv_stderr," and ");
      checker_base_print_type(self, nusmv_stderr, cdr(expr), context);
      break;

    case COUNT:
      fprintf(nusmv_stderr, "non-boolean expressions for \"count\" operator");
      break;

    case IFTHENELSE:
      fprintf(nusmv_stderr,"types of \"then\" and \"else\" expressions : ");
      checker_base_print_type(self, nusmv_stderr, cdr(car(expr)), context);
      fprintf(nusmv_stderr," and ");
      checker_base_print_type(self, nusmv_stderr, cdr(expr), context);
      break;

    case EBF:
    case ABF:
    case EBG:
    case ABG:
      fprintf(nusmv_stderr,"operand type of \"");
      print_operator(nusmv_stderr, expr);
      fprintf(nusmv_stderr,"\" : ");
      checker_base_print_type(self, nusmv_stderr, car(expr), context);
      break;

    case ABU:
    case EBU:
      fprintf(nusmv_stderr,"operand types of \"");
      print_operator(nusmv_stderr, expr);
      fprintf(nusmv_stderr,"\" : ");
      checker_base_print_type(self, nusmv_stderr, car(car(expr)), context);
      fprintf(nusmv_stderr," and ");
      checker_base_print_type(self, nusmv_stderr, cdr(car(expr)), context);
      break;


    default: /* unknown kind of an expression */
      error_unreachable_code();
    } /* switch (node_get_type(expr)) */
    fprintf(nusmv_stderr,"\n");
    break;

  default:
    error_unreachable_code(); /* unknown kind of an error */
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
static void print_operator(FILE* output_stream, node_ptr expr)
{
  nusmv_assert((node_ptr) Nil != expr);
  switch (node_get_type(expr)){

  case CONCATENATION: fprintf(output_stream,"::"); return;
  case BIT_SELECTION: fprintf(output_stream,"[ : ]"); return;
  case CAST_BOOL:     fprintf(output_stream,"bool"); return;
  case CAST_WORD1:    fprintf(output_stream,"word1"); return;
  case CAST_SIGNED:   fprintf(output_stream,"signed"); return;
  case CAST_UNSIGNED: fprintf(output_stream,"unsigned"); return;
  case EXTEND:   fprintf(output_stream,"extend"); return;
  case AND:           fprintf(output_stream,"&"); return;
  case OR:            fprintf(output_stream,"|"); return;
  case XOR:           fprintf(output_stream,"xor"); return;
  case XNOR:          fprintf(output_stream,"xnor"); return;
  case IMPLIES:       fprintf(output_stream,"->"); return;
  case IFF:   fprintf(output_stream,"<->"); return;
  case NOT:   fprintf(output_stream,"!"); return;
  case EX:    fprintf(output_stream,"EX"); return;
  case AX:    fprintf(output_stream,"AX"); return;
  case EF:    fprintf(output_stream,"EF"); return;
  case AF:    fprintf(output_stream,"AF"); return;
  case EG:    fprintf(output_stream,"EG"); return;
  case AG:    fprintf(output_stream,"AG"); return;
  case OP_GLOBAL:     fprintf(output_stream,"G"); return;
  case OP_HISTORICAL: fprintf(output_stream,"H"); return;
  case RELEASES:      fprintf(output_stream,"V"); return;
  case TRIGGERED:     fprintf(output_stream,"T"); return;
  case UNTIL:    fprintf(output_stream,"U"); return;
  case SINCE:    fprintf(output_stream,"S"); return;
  case OP_NEXT:  fprintf(output_stream,"X"); return;
  case OP_PREC:  fprintf(output_stream,"Y"); return;
  case OP_NOTPRECNOT:    fprintf(output_stream,"Z"); return;
  case OP_FUTURE:        fprintf(output_stream,"F"); return;
  case OP_ONCE:          fprintf(output_stream,"O"); return;
  case AU:   fprintf(output_stream,"A-U"); return;
  case EU:   fprintf(output_stream,"E-U"); return;
  case EBF:  fprintf(output_stream,"EBF"); return;
  case ABF:  fprintf(output_stream,"ABF"); return;
  case EBG:  fprintf(output_stream,"EBG"); return;
  case ABG:  fprintf(output_stream,"ABG"); return;
  case ABU:  fprintf(output_stream,"ABU"); return;
  case EBU:  fprintf(output_stream,"EBU"); return;

  case WRESIZE: fprintf(output_stream,"resize"); return;
  case WSIZEOF: fprintf(output_stream,"sizeof"); return;
  case CAST_TOINT:  fprintf(output_stream,"toint"); return;

  case LSHIFT:    fprintf(output_stream,"<<"); return;
  case RSHIFT:    fprintf(output_stream,">>"); return;
  case LROTATE:   fprintf(output_stream,"<<<"); return;
  case RROTATE:   fprintf(output_stream,">>>"); return;
  case PLUS:      fprintf(output_stream,"+"); return;
  case MINUS:     fprintf(output_stream,"-"); return;
  case UMINUS:    fprintf(output_stream,"unary -"); return;
  case TIMES:     fprintf(output_stream,"*"); return;
  case DIVIDE:    fprintf(output_stream,"/"); return;
  case MOD:       fprintf(output_stream,"mod"); return;
  case EQUAL:     fprintf(output_stream,"="); return;
  case NOTEQUAL:  fprintf(output_stream,"!="); return;
  case LT:    fprintf(output_stream,"<"); return;
  case GT:    fprintf(output_stream,">"); return;
  case LE:    fprintf(output_stream,"<="); return;
  case GE:    fprintf(output_stream,">="); return;
  case UNION: fprintf(output_stream,"union"); return;
  case SETIN: fprintf(output_stream,"in"); return;
  case COLON: fprintf(output_stream,":"); return;
  case WAREAD: fprintf(output_stream,"READ"); return;
  case WAWRITE: fprintf(output_stream,"WRITE"); return;

  case EQDEF:     fprintf(output_stream,":="); return;
  case MINU:    fprintf(output_stream,"MIN"); return;
  case MAXU:    fprintf(output_stream,"MAX"); return;
  case CASE:      fprintf(output_stream,"\n(CASE "); return;
  case CONS:      fprintf(output_stream,","); return;
  case VAR:       fprintf(output_stream,"\n(VAR "); return;
  case FAILURE:      fprintf(output_stream,"\n(FAILURE)"); return;
  case ATOM:      fprintf(output_stream,"\n(ATOM "); return;
  case NUMBER:
    fprintf(output_stream,"\n(NUMBER %d)",NODE_TO_INT(car(expr)));
    return;
  case NUMBER_UNSIGNED_WORD: fprintf(output_stream,"\n(NUMBER_UNSIGNED_WORD ");
    WordNumber_print(output_stream, WORD_NUMBER(car(expr)), false);
    fprintf(output_stream,")"); return;
  case NUMBER_SIGNED_WORD: fprintf(output_stream,"\n(NUMBER_SIGNED_WORD ");
    WordNumber_print(output_stream, WORD_NUMBER(car(expr)), true);
    fprintf(output_stream,")"); return;
  case NUMBER_REAL: fprintf(output_stream,"\n(NUMBER_REAL %s)",
                            get_text((string_ptr)car(expr))); return;
  case NUMBER_FRAC: fprintf(output_stream,"\n(NUMBER_FRAC %s)",
                            get_text((string_ptr)car(expr))); return;
  case NUMBER_EXP: fprintf(output_stream,"\n(NUMBER_EXP %s)",
                           get_text((string_ptr)car(expr))); return;
  case TRUEEXP:     fprintf(output_stream,"(TRUE)"); return;
  case FALSEEXP:    fprintf(output_stream,"(FALSE)"); return;
  case BIT:         fprintf(output_stream,"\n(BIT "); return;
  case BOOLEAN:     fprintf(output_stream,"(BOOLEAN)"); return;
  case INTEGER:     fprintf(output_stream,"(INT)"); return;
  case REAL:    fprintf(output_stream,"(REAL)"); return;
  case SELF:    fprintf(output_stream,"(SELF)"); return;
  case SCALAR:  fprintf(output_stream,"(SCALAR "); return;
  case ARRAY:   fprintf(output_stream,"\n(ARRAY "); return;
  case DOT:     fprintf(output_stream,"(DOT "); return;
  case TWODOTS:    fprintf(output_stream, ".."); return;
  case NEXT:    fprintf(output_stream,"NEXT"); return;
  case SMALLINIT:    fprintf(output_stream,"\n(SMALLINIT "); return;

  case CONTEXT:    fprintf(output_stream,"\n(CONTEXT "); return;
  case BDD:   fprintf(output_stream,"(BDD TO BE PRINTED)"); return;
  case SEMI:    fprintf(output_stream, "\n(SEMI "); return;
  case IFTHENELSE:    fprintf(output_stream, "\n (IFTHENELSE "); return;

  default:
    fprintf(nusmv_stderr, "\n%d\n", node_get_type(expr));
    error_unreachable_code();
  }

}



/**AutomaticEnd***************************************************************/

