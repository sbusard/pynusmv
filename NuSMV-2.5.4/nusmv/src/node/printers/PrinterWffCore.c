
/**CFile***********************************************************************

  FileName    [PrinterWffCore.c]

  PackageName [node.printers]

  Synopsis    [Implementaion of class 'PrinterWffCore']

  Description []

  SeeAlso     [PrinterWffCore.h]

  Author      [Marco Roveri, Roberto Cavada]

  Copyright   [
  This file is part of the ``node.printers'' package of NuSMV version 2. 
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

  Revision    [$Id: PrinterWffCore.c,v 1.1.2.14.4.15 2009/11/02 23:03:05 nusmv Exp $]

******************************************************************************/

#include "PrinterWffCore.h" 
#include "PrinterWffCore_private.h" 

#include "parser/symbols.h"

#include "utils/WordNumber.h"
#include "utils/utils.h" 
#include "utils/ustring.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: PrinterWffCore.c,v 1.1.2.14.4.15 2009/11/02 23:03:05 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'PrinterWffCore_private.h' for class 'PrinterWffCore' definition. */ 

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis           [Short way of calling printer_base_throw_print_node]

  Description        [Use this macro to recursively recall print_node]

  SeeAlso            []
  
******************************************************************************/
#define _THROW(n, p)  printer_base_throw_print_node(PRINTER_BASE(self), n, p) 


/**Macro***********************************************************************

  Synopsis           [Short way of calling printer_base_print_string]

  Description [Use to print a string (that will be redirected to the
  currently used stream)]

  SeeAlso            []
  
******************************************************************************/
#define _PRINT(str)  printer_base_print_string(PRINTER_BASE(self), str)



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void printer_wff_core_finalize ARGS((Object_ptr object, void* dummy));

static int 
printer_wff_core_print_case ARGS((PrinterWffCore_ptr self, node_ptr n));

static int 
printer_wff_core_print_case_body ARGS((PrinterWffCore_ptr self, node_ptr n));



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The PrinterWffCore class constructor]

  Description        [The PrinterWffCore class constructor]

  SideEffects        []

  SeeAlso            [PrinterWffCore_destroy]   
  
******************************************************************************/
PrinterWffCore_ptr PrinterWffCore_create(const char* name)
{
  PrinterWffCore_ptr self = ALLOC(PrinterWffCore, 1);
  PRINTER_WFF_CORE_CHECK_INSTANCE(self);

  printer_wff_core_init(self, name, 
                        NUSMV_CORE_SYMBOL_FIRST, 
                        NUSMV_CORE_SYMBOL_LAST - NUSMV_CORE_SYMBOL_FIRST);
  return self;
}




/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The PrinterWffCore class private initializer]

  Description        [The PrinterWffCore class private initializer]

  SideEffects        []

  SeeAlso            [PrinterWffCore_create]   
  
******************************************************************************/
void printer_wff_core_init(PrinterWffCore_ptr self, 
                           const char* name, int low, size_t num)
{
  /* base class initialization */
  printer_base_init(PRINTER_BASE(self), name, low, num, true /*handles NULL*/);
  
  /* members initialization */

  /* virtual methods settings */  
  OVERRIDE(Object, finalize) = printer_wff_core_finalize;
  OVERRIDE(PrinterBase, print_node) = printer_wff_core_print_node;

}


/**Function********************************************************************

  Synopsis           [The PrinterWffCore class private deinitializer]

  Description        [The PrinterWffCore class private deinitializer]

  SideEffects        []

  SeeAlso            [PrinterWffCore_destroy]   
  
******************************************************************************/
void printer_wff_core_deinit(PrinterWffCore_ptr self)
{
  /* members deinitialization */


  /* base class initialization */
  printer_base_deinit(PRINTER_BASE(self));
}


/**Function********************************************************************

  Synopsis    [Virtual menthod that prints the given node 
  (core nodes are handled here)]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int printer_wff_core_print_node(PrinterBase_ptr self, node_ptr n, int priority)
{
  char* op;
  int pr_tmp;
  int arity;     /* 0: unary, 1: binary, 2: terciary, 3:quad */  
  int brckts;

  op = (char*) NULL;
  pr_tmp = 0;
  arity = 0;     /* 0: unary, 1: binary, 2: terciary, 3:quad */  
  brckts = 0;

  if (n == Nil) return 1;
  if (n == (node_ptr) -1) return _PRINT("*no value*");

  switch (node_get_type(n)) {
  case FAILURE: {
    char buf[20];
    int chars = snprintf(buf, 20, "\", line %d)", failure_get_lineno(n));
    SNPRINTF_CHECK(chars, 20);

    return _PRINT("FAILURE(\"") &&
      _PRINT(failure_get_msg(n)) &&
      _PRINT(buf);
  }

  case ATTIME:
    return _PRINT("@") &&
      _THROW(cdr(n), 0) &&
      _PRINT("{") &&
      _THROW(car(n), 0) && _PRINT("}");
 
  case TRUEEXP: return _PRINT("TRUE");
  case FALSEEXP: return _PRINT("FALSE");
  case SELF: return _PRINT("self");
  case BOOLEAN: return _PRINT("boolean");
  case SCALAR: return _PRINT("{") && _THROW(car(n), 0) && _PRINT("}");

  case ATOM:
    if (!_PRINT(get_text((string_ptr) car(n)))) return 0;
    if (cdr(n)) {
      char buf[20];
      int chars = snprintf(buf, 20, "_%d", NODE_TO_INT(cdr(n)));
      SNPRINTF_CHECK(chars, 20);

      return _PRINT(buf);
    }
    return 1;

  case NUMBER:
    {
      char buf[20];
      int c = snprintf(buf, 20, "%d", NODE_TO_INT(car(n)));
      SNPRINTF_CHECK(c, 20);

      return _PRINT(buf);
    }

  case NUMBER_UNSIGNED_WORD:
    return _PRINT(WordNumber_to_string(WORD_NUMBER(car(n)), false));
  case NUMBER_SIGNED_WORD:
    return _PRINT(WordNumber_to_string(WORD_NUMBER(car(n)), true));

  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
    return _PRINT(get_text((string_ptr) car(n)));

  case UWCONST:
    return _PRINT("uwconst(") && _THROW(car(n), 0) && _PRINT(", ") &&
      _THROW(cdr(n), 0) && _PRINT(")");

  case SWCONST:
    return _PRINT("swconst(") && _THROW(car(n), 0) && _PRINT(", ") &&
      _THROW(cdr(n), 0) && _PRINT(")");

  case WRESIZE:
    return _PRINT("resize(") && _THROW(car(n), 0) && _PRINT(", ") &&
      _THROW(cdr(n), 0) && _PRINT(")");
  case WSIZEOF:
    return _PRINT("sizeof(") && _THROW(car(n), 0) && _PRINT(")");
  case CAST_TOINT:
    return _PRINT("toint(") && _THROW(car(n), 0) && _PRINT(")");

  case COUNT:
    return _PRINT("count(") && _THROW(car(n), 0) && _PRINT(")");

  case DOT:
    if (car(n) == Nil) return _THROW(cdr(n), 0);
    return _THROW(car(n), 0) &&
      _PRINT(".") &&
      _THROW(cdr(n), 0);

  case BIT:
    {
      char buf[30];
      int c = snprintf(buf, 30, ".%d", NODE_TO_INT(cdr(n)));
      SNPRINTF_CHECK(c, 30);
      
      return _THROW(car(n), 0) && _PRINT(buf);
    }
  
  case CONTEXT:
    return _THROW(cdr(n), 0) && 
      ((car(n) == Nil) || 
       (_PRINT(" IN ") && _THROW(car(n), 0)));
      
  case CONS:
    return _THROW(car(n), 0) &&
      ((cdr(n) == Nil) || 
       (_PRINT(", ") && _THROW(cdr(n), 0)));

  case CASE: 
    return printer_wff_core_print_case(PRINTER_WFF_CORE(self), n);

  case IFTHENELSE:
    return _PRINT(" (") && _THROW(car(car(n)), 0) &&
      (_PRINT(" ? ") && _THROW(cdr(car(n)), 0) && 
       _PRINT(" : ") && _THROW(cdr(n), 0)) && _PRINT(") ");

  case ARRAY: /* array access */
    return _THROW(car(n), 0) && _PRINT("[") &&
      _THROW(cdr(n), 0) && _PRINT("]");

  case ARRAY_DEF: /* array definition */
    return _PRINT("[") && _THROW(car(n), 0) && _PRINT("]");

  case BIT_SELECTION:
    /* 18 is bigger than max possible priority */
    return _THROW(car(n), 18) && _PRINT("[") &&
      _THROW(car(cdr(n)), 0) && _PRINT(" : ") &&
      _THROW(cdr(cdr(n)), 0) && _PRINT("]");

  case CONSTANTS:
    return _PRINT("CONSTANTS ") && 
      _THROW(car(n), 0) && _PRINT(";");

    /* this is a expression of word type, i.e. a list of concatenated bits.
       Since the assumed operation is CONCATENATION, its priority is used.
       The expression may be as signed as well as unsigned.
    */
  case UNSIGNED_WORD: {

    node_ptr iter = car(n);
    pr_tmp = 16;
    nusmv_assert(CONS == node_get_type(iter)); /* a list of bits */

    if (pr_tmp <= priority && !_PRINT("(")) return 0;
    if (!_PRINT("word1(")) return 0;
    if (!_THROW(car(iter), pr_tmp)) return 0;
    if (!_PRINT(")")) return 0;

    for (iter = cdr(iter); iter != Nil; iter = cdr(iter)) {
      if (!_PRINT(" :: word1(")) return 0;
      if (!_THROW(car(iter), pr_tmp)) return 0;
      if (!_PRINT(")")) return 0;
    }

    if (pr_tmp <= priority && !_PRINT(")")) return 0;
    return 1;
  }

  case WAWRITE: 
    return _PRINT("WRITE(") && 
      _THROW(car(n), 0) && _PRINT(", ") && 
      _THROW(car(cdr(n)), 0) && _PRINT(", ") && 
      _THROW(cdr(cdr(n)), 0) && _PRINT(")");

  case WAREAD: 
    return _PRINT("READ(") && 
      _THROW(car(n), 0) && _PRINT(", ") &&
      _THROW(cdr(n), 0) && _PRINT(")");

  case NFUNCTION: 
    return _THROW(car(n), 0) && _PRINT("(") &&
      _THROW(cdr(n), 0) && _PRINT(")");

  case NEXT:
    if (!_PRINT("next")) return 0;
    op = ""; pr_tmp = 0; priority = 1; arity = 0; break;

  case SMALLINIT:
    if (!_PRINT("init")) return 0;
    op = ""; pr_tmp = 0; priority = 1; arity = 0; break;

  case CAST_WORD1:
    if (!_PRINT("word1")) return 0;
    op = ""; pr_tmp = 0; priority = 1; arity = 0; break;

  case CAST_BOOL:
    if (!_PRINT("bool")) return 0;
    op = ""; pr_tmp = 0; priority = 1; arity = 0; break;

  case CAST_SIGNED:
    if (!_PRINT("signed")) return 0;
    op = ""; pr_tmp = 0; priority = 1; arity = 0; break;
  case CAST_UNSIGNED:
    if (!_PRINT("unsigned")) return 0;
    op = ""; pr_tmp = 0; priority = 1; arity = 0; break;
  case EXTEND:
    if (!_PRINT("extend")) return 0;
    op = ","; pr_tmp = 0; priority = 1; arity = 1; break;

  case TWODOTS: op = ".."; pr_tmp = 3; arity = 1; break;
  case IMPLIES: op = "->"; pr_tmp = 4; arity = 1; priority = 5; break;
  case IFF: op = "<->"; pr_tmp = 4; arity = 1; priority = 5; break;
  case XOR: op = "xor"; pr_tmp = 4; arity = 1; priority = 5; break;
  case XNOR: op = "xnor"; pr_tmp = 4; arity = 1; priority = 5; break;
  case OR: op = "|"; pr_tmp = 5; arity = 1;  priority = 6; break;
  case AND: op = "&"; pr_tmp = 6; arity = 1; priority = 7; break;
  case EX: op = "EX "; pr_tmp = 8; arity = 0; break;
  case AX: op = "AX "; pr_tmp = 8; arity = 0; break;
  case EF: op = "EF "; pr_tmp = 8; arity = 0; break;
  case AF: op = "AF "; pr_tmp = 8; arity = 0; break;
  case EG: op = "EG "; pr_tmp = 8; arity = 0; break;
  case AG: op = "AG "; pr_tmp = 8; arity = 0; break;
  case OP_NEXT: op = " X "; pr_tmp = 8; arity = 0; break;
  case OP_PREC: op = " Y "; pr_tmp = 8; arity = 0; break;
  case OP_NOTPRECNOT: op = " Z "; pr_tmp = 8; arity = 0; break;
  case OP_GLOBAL: op = " G "; pr_tmp = 8; arity = 0; break;
  case OP_HISTORICAL: op = " H "; pr_tmp = 8; arity = 0; break;
  case OP_FUTURE: op = " F "; pr_tmp = 8; arity = 0; break;
  case OP_ONCE: op = " O "; pr_tmp = 8; arity = 0; break;
  case UNTIL: op = "U"; pr_tmp = 8; priority = 9; arity = 1; break;
  case SINCE: op = "S"; pr_tmp = 8; priority = 9; arity = 1; break;
  case RELEASES: op = "V"; pr_tmp = 8; priority = 9; arity = 1; break;
  case TRIGGERED: op = "T"; pr_tmp = 8; priority = 9; arity = 1; break;

  case EU:
    if (!_PRINT("E")) return 0;
    op = "U"; pr_tmp = 8; priority = 9; arity = 1; brckts = 1; break;

  case AU:
    if (!_PRINT("A")) return 0;
    op = "U"; pr_tmp = 8; priority = 9; arity = 1; brckts = 1; break;

  case EBU:
    if (!_PRINT("E")) return 0;
    op = "BU"; pr_tmp = 8; priority = 9; arity = 3; brckts = 1; break;

  case ABU:
    if (!_PRINT("A")) return 0;
    op = "BU"; pr_tmp = 8; priority = 9; arity = 3; brckts = 1; break;

  case EBF: op = "EBF "; pr_tmp = 8; arity = 2; break;
  case ABF: op = "ABF "; pr_tmp = 8; arity = 2; break;
  case EBG: op = "EBG "; pr_tmp = 8; arity = 2; break;
  case ABG: op = "ABG "; pr_tmp = 8; arity = 2; break;

  case MINU:
    if (!_PRINT("MIN")) return 0;
    op = ","; pr_tmp = 8; priority = 9; arity = 1; brckts = 1; break;

  case MAXU:
    if (!_PRINT("MAX")) return 0;
    op = ","; pr_tmp = 8; priority = 9; arity = 1; brckts = 1; break;

  case EQUAL: op = "="; pr_tmp = 9; arity = 1; break;
  case NOTEQUAL: op = "!="; pr_tmp = 9; arity = 1; break;
  case LT:    op = "<"; pr_tmp = 9; arity = 1; break;
  case GT:    op = ">"; pr_tmp = 9; arity = 1; break;
  case LE:    op = "<="; pr_tmp = 9; arity = 1; break;
  case GE:    op = ">="; pr_tmp = 9; arity = 1; break;

  case SETIN: op = "in"; pr_tmp = 10; arity = 1; break;
  case UNION: op = "union"; pr_tmp = 11; arity = 1; break;
  case LSHIFT: op = "<<"; pr_tmp = 12; arity = 1; break;
  case RSHIFT: op = ">>"; pr_tmp = 12; arity = 1; break;
  case LROTATE: op = "<<<"; pr_tmp = 12; arity = 1; break;
  case RROTATE: op = ">>>"; pr_tmp = 12; arity = 1; break;

  case PLUS:  
    op = "+"; 
    if (cdr(n) == (node_ptr) NULL) { /* checks if unary */
      pr_tmp = 17; 
      arity = 0; 
    }
    else {
      pr_tmp = 13; 
      arity = 1; 
    }
    break;

  case UMINUS:
    nusmv_assert((node_ptr) NULL != car(n));
    if (NUMBER_SIGNED_WORD == node_get_type(car(n)) &&
        WordNumber_get_sign(WORD_NUMBER(caar(n))) == true) {
      op = "- "; /* see issue #0001366 */
    }
    else op = "-";
    pr_tmp = 17; 
    arity = 0;     
    break;
    
  case MINUS: 
    op = "-"; 
    if (cdr(n) == (node_ptr) NULL) { /* checks if unary */
      pr_tmp = 17; 
      arity = 0; 
    }
    else {
      pr_tmp = 13; 
      arity = 1; 
    }
    break;

  case MOD:   op = "mod"; pr_tmp = 14; arity = 1; break;
  case TIMES: op = "*"; pr_tmp = 15; arity = 1; break;
  case DIVIDE: op = "/"; pr_tmp = 15; arity = 1; break;
  case CONCATENATION: op = "::"; pr_tmp = 16; arity = 1; break;
  case NOT: op = "!"; pr_tmp = 17; arity = 0; break;

  case VAR: op = "VAR "; pr_tmp = 8 ; arity = 0; break;
  case IVAR: op = "IVAR "; pr_tmp = 8 ; arity = 0; break;
  case EQDEF: op = ":="; pr_tmp = 1; arity = 1; break;

  default:
    internal_error("printer_wff_core_print_node: not supported type = %d", 
                   node_get_type(n));
    
  }
  
  if (brckts == 1 && pr_tmp < priority && !_PRINT(" [ ")) return 0;
  if (brckts == 0 && pr_tmp <= priority && !_PRINT("(")) return 0;

  switch (arity) {
  case 0:
    if (!_PRINT(op)) return 0;
    if (!_THROW(car(n), pr_tmp)) return 0;
    break;

  case 1:
    if (car(n) != (node_ptr) NULL) {
      if (!_THROW(car(n), pr_tmp)) return 0;
      if (!_PRINT(" ")) return 0;
      if (!_PRINT(op)) return 0;
      if (!_PRINT(" ")) return 0;
    }
    if (!_THROW(cdr(n), pr_tmp)) return 0;
    break;

  case 2:
    /* EF a..b f */
    if (!_PRINT(op)) return 0;                /* EF */
    if (!_THROW(car(cdr(n)), pr_tmp)) return 0; /* a */
    if (!_PRINT("..")) return 0;
    if (!_THROW(cdr(cdr(n)), pr_tmp)) return 0; /* b */
    if (!_PRINT(" ")) return 0;
    if (!_THROW(car(n), pr_tmp)) return 0; /* f */
    break;

  case 3:
    /* E[f BU a..b g] */
    if (!_THROW(car(car(n)), pr_tmp)) return 0; /* f */
    if (!_PRINT(" ")) return 0;
    if (!_PRINT(op)) return 0;                /* BU */
    if (!_PRINT(" ")) return 0;
    if (!_THROW(car(cdr(n)), pr_tmp)) return 0; /* a */
    if (!_PRINT("..")) return 0;
    if (!_THROW(cdr(cdr(n)), pr_tmp)) return 0; /* b */
    if (!_PRINT(" ")) return 0;
    if (!_THROW(cdr(car(n)), pr_tmp)) return 0; /* g */
    break;
  } /* switch on arity */

  if (brckts == 0 && pr_tmp <= priority && !_PRINT(")")) return 0;
  if (brckts == 1 && pr_tmp < priority && !_PRINT(" ] ")) return 0;
  return 1;
  
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The PrinterWffCore class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void printer_wff_core_finalize(Object_ptr object, void* dummy) 
{
  PrinterWffCore_ptr self = PRINTER_WFF_CORE(object);

  printer_wff_core_deinit(self);
  FREE(self);
}



/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int printer_wff_core_print_case(PrinterWffCore_ptr self, node_ptr n)
{
  return _PRINT("case\n") &&
    printer_wff_core_print_case_body(self, n) && _PRINT("esac");
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int printer_wff_core_print_case_body(PrinterWffCore_ptr self, node_ptr n)
{
  int res;

  nusmv_assert(n != Nil);
  res = _THROW(car(car(n)), 0) && _PRINT(" : ") &&
    _THROW(cdr(car(n)), 0) && _PRINT(";\n");

  if (res == 0) return 0; /* previous error */

  nusmv_assert(cdr(n) != Nil); /* Now there is always a last(default) case */ 

  /* continue to print the tail of the case list */
  if (node_get_type(cdr(n)) == CASE || node_get_type(cdr(n)) == IFTHENELSE) {
    return printer_wff_core_print_case_body(self, cdr(n));
  }
  /* print the last(default) element. Do not print artificial FAILURE node */
  else if (node_get_type(cdr(n)) != FAILURE) {
    return _PRINT("TRUE : ") && /* the last (default) element */
      _THROW(cdr(n), 0) && _PRINT(";\n");
  }

  return res; /* the last element is FAILURE node */
}



/**AutomaticEnd***************************************************************/

