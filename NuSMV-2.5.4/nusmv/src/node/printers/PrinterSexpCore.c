/**CFile***********************************************************************

   FileName    [PrinterSexpCore.c]

   PackageName [node.printers]

   Synopsis    [Implementaion of class 'PrinterSexpCore']

   Description []

   SeeAlso     [PrinterSexpCore.h]

   Author      [Alessandro Mariotti]

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

******************************************************************************/

#include "PrinterSexpCore.h" 
#include "PrinterSexpCore_private.h" 

#include "parser/symbols.h"

#include "utils/WordNumber.h"
#include "utils/utils.h" 
#include "utils/ustring.h"
#include "utils/error.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'PrinterSexpCore_private.h' for class 'PrinterSexpCore' definition. */ 

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

static void printer_sexp_core_finalize ARGS((Object_ptr object, void* dummy));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [The PrinterSexpCore class constructor]

   Description        [The PrinterSexpCore class constructor]

   SideEffects        []

   SeeAlso            [PrinterSexpCore_destroy]   
  
******************************************************************************/
PrinterSexpCore_ptr PrinterSexpCore_create(const char* name)
{
  PrinterSexpCore_ptr self = ALLOC(PrinterSexpCore, 1);
  PRINTER_SEXP_CORE_CHECK_INSTANCE(self);

  printer_sexp_core_init(self, name, 
                         NUSMV_CORE_SYMBOL_FIRST, 
                         NUSMV_CORE_SYMBOL_LAST - NUSMV_CORE_SYMBOL_FIRST);
  return self;
}




/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [The PrinterSexpCore class private initializer]

   Description        [The PrinterSexpCore class private initializer]

   SideEffects        []

   SeeAlso            [PrinterSexpCore_create]   
  
******************************************************************************/
void printer_sexp_core_init(PrinterSexpCore_ptr self, 
                            const char* name, int low, size_t num)
{
  /* base class initialization */
  printer_base_init(PRINTER_BASE(self), name, low, num, true /*handles NULL*/);
  
  /* members initialization */

  /* virtual methods settings */  
  OVERRIDE(Object, finalize) = printer_sexp_core_finalize;
  OVERRIDE(PrinterBase, print_node) = printer_sexp_core_print_node;

}


/**Function********************************************************************

   Synopsis           [The PrinterSexpCore class private deinitializer]

   Description        [The PrinterSexpCore class private deinitializer]

   SideEffects        []

   SeeAlso            [PrinterSexpCore_destroy]   
  
******************************************************************************/
void printer_sexp_core_deinit(PrinterSexpCore_ptr self)
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
int printer_sexp_core_print_node(PrinterBase_ptr self, node_ptr node,
                                 int priority)
{
  int result = 1;

  if (node == Nil) {
    _PRINT(" Nil ");
    return 1;
  }
  if (node == (node_ptr) -1) {
    return _PRINT("No value");
  }

  switch (node_get_type(node)) {
  case MODULE:
    result = _PRINT("\n(MODULE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(") ");
    break;

  case MODTYPE:
    result = _PRINT("\n(MODTYPE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case CASE:
    result = _PRINT("\n(CASE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case CONS:
    result = _PRINT("\n(CONS ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case NFUNCTION:
    result = _PRINT("\n(NFUNCTION ") &&
      _THROW(car(node), 0) && _PRINT("(") &&
      _THROW(cdr(node), 0) && _PRINT("))");
    break;

  case VAR:
    result = _PRINT("\n(VAR ");

    if ((car(node) != Nil) && (cdr(node) != Nil)) {
      result &= _PRINT("(CAR ") &&
        _PRINT("(BDD TO BE PRINTED)") &&
        _PRINT(")(CDR ") &&
        _THROW(cdr(node), 0) &&
        _PRINT(")");
    }
    else if (cdr(node) != Nil) {
      result &= _PRINT("(CDR ") &&
        _THROW(cdr(node), 0) &&
        _PRINT(")");
    }
    else {
      result &= _THROW(car(node), 0);
    }
    result &= _PRINT(")\n");
    break;

  case FROZENVAR:
    result = _PRINT("\n (FROZENVAR ");

    if ((car(node) != Nil) && (cdr(node) != Nil)) {
      result &= _PRINT("(CAR ") &&
        _PRINT("(BDD TO BE PRINTED)") &&
        _PRINT(")(CDR ") &&
        _THROW(cdr(node), 0) &&
        _PRINT(")");
    }
    else if (cdr(node) != Nil) {
      result &= _PRINT("(CDR ") &&
        _THROW(cdr(node), 0) &&
        _PRINT(")");
    }
    else {
      result &= _THROW(car(node), 0);
    }
    result &= _PRINT(")\n");
    break;

  case IVAR:
    result = _PRINT("\n (IVAR ");

    if ((car(node) != Nil) && (cdr(node) != Nil)) {
      result &= _PRINT("(CAR ") &&
        _PRINT("(BDD TO BE PRINTED)") &&
        _PRINT(")(CDR ") &&
        _THROW(cdr(node), 0) &&
        _PRINT(")");
    }
    else if (cdr(node) != Nil) {
      result &= _PRINT("(CDR ") &&
        _THROW(cdr(node), 0) &&
        _PRINT(")");
    }
    else {
      result &= _THROW(car(node), 0);
    }
    result &= _PRINT(")\n");
    break;

  case ASSIGN:
    result = _PRINT("\n(ASSIGN ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case INVAR:
    result = _PRINT("\n(INVAR ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case TRANS:
    result = _PRINT("\n(TRANS ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case INIT:
    result = _PRINT("\n(INIT ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case SPEC:
    result = _PRINT("\n(SPEC ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case LTLSPEC:
    result = _PRINT("\n(LTLSPEC ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case INVARSPEC:
    result = _PRINT("\n(INVARSPEC ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case COMPUTE:
    result = _PRINT("\n(COMPUTE ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case PSLSPEC:
    result = _PRINT("\n(PSL specification)");
    break;

  case CONSTANTS:
    result = _PRINT("\n(CONSTANTS ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case FAILURE:
    {
      char lin[20], kind[20];
      int c = snprintf(lin, 20, "%d", failure_get_lineno(node));
      SNPRINTF_CHECK(c, 20);

      c = snprintf(kind, 20, "%d", failure_get_kind(node));
      SNPRINTF_CHECK(c, 20);

      result = _PRINT("(FAILURE line = ") &&
        _PRINT(lin) &&
        _PRINT(", message = \"") &&
        _PRINT(failure_get_msg(node)) &&
        _PRINT("\", kind = ") &&
        _PRINT(kind) &&
        _PRINT(")");
      break;
    }

  case ATOM:
    {
      string_ptr tmp_strp = (string_ptr) car(node);
      char * str = (char *) tmp_strp->text;
      result = _PRINT("(ATOM ") &&
        _PRINT(str) && _PRINT(")");
      break;
    }

  case NUMBER:
    {
      char buf[20];
      int c = snprintf(buf, 20, "%d", NODE_TO_INT(car(node)));
      SNPRINTF_CHECK(c, 20);
      result = _PRINT("\n(NUMBER ") &&
        _PRINT(buf) && _PRINT(")");
      break;
    }

  case NUMBER_UNSIGNED_WORD:
    result = _PRINT("\n(NUMBER_UNSIGNED_WORD ") &&
      _PRINT(WordNumber_to_string(WORD_NUMBER(car(node)), false)) &&
      _PRINT(")");
    break;

  case NUMBER_SIGNED_WORD:
    result = _PRINT("\n(NUMBER_SIGNED_WORD ") &&
      _PRINT(WordNumber_to_string(WORD_NUMBER(car(node)), true)) &&
      _PRINT(")");
    break;

  case NUMBER_REAL:
    {
      const char* num = get_text((string_ptr)car(node));
      result = _PRINT("\n(NUMBER_REAL ") &&
        _PRINT(num) && _PRINT(")");
      break;
    }

  case NUMBER_FRAC:
    {
      const char* num = get_text((string_ptr)car(node));
      result = _PRINT("\n(NUMBER_FRAC ") &&
        _PRINT(num) && _PRINT(")");
      break;
    }

  case NUMBER_EXP:
    {
      const char* num = get_text((string_ptr)car(node));
      result = _PRINT("\n(NUMBER_EXP ") &&
        _PRINT(num) && _PRINT(")");
      break;
    }

  case UWCONST:
    result = _PRINT("\n(UWCONST ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case SWCONST:
    result = _PRINT("\n(SWCONST ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");

    break;

  case WRESIZE:
    result = _PRINT("\n(WRESIZE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case WSIZEOF:
    result = _PRINT("\n(WSIZEOF ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case CAST_TOINT:
    result = _PRINT("\n(CAST_TOINT ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case COUNT:
    result = _PRINT("\n(COUNT ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case TRUEEXP:
    result =  _PRINT("(TRUE)");
    break;

  case FALSEEXP:
    result = _PRINT("(FALSE)");
    break;

  case BIT:
    {
      char buf[20];
      int c = snprintf(buf, 20, " %d)", NODE_TO_INT(cdr(node)));
      SNPRINTF_CHECK(c, 20);

      result = _PRINT("\n(BIT ") &&
        _THROW(car(node), 0) &&
        _PRINT(buf);
      break;
    }

  case BOOLEAN:
    result = _PRINT("(BOOLEAN)");
    break;

  case INTEGER:
    result = _PRINT("(INT)");
    break;

  case REAL:
    result = _PRINT("(REAL)");
    break;

  case UNSIGNED_WORD:
    result = _PRINT("(UNSIGNED_WORD ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case SIGNED_WORD:
    result = _PRINT("(SIGNED_WORD ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case SELF:
    result = _PRINT("(SELF)");
    break;

  case SCALAR:
    result = _PRINT("(SCALAR ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case ARRAY:
    result = _PRINT("\n(ARRAY ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(") ");
    break;

  case DOT:
    result = _PRINT("(DOT ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case CONCATENATION:
    result = _PRINT("(CONCATENATION ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case BIT_SELECTION:
    result = _PRINT("(BIT_SELECTION ") &&
      _THROW(car(node), 0) &&
      _THROW(car(cdr(node)), 0) &&
      _THROW(cdr(cdr(node)), 0) &&
      _PRINT(")");
    break;

  case CAST_BOOL:
    result = _PRINT("(CAST_BOOL ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case CAST_WORD1:
    result = _PRINT("(CAST_WORD1 ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case CAST_SIGNED:
    result = _PRINT("(CAST_SIGNED ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case CAST_UNSIGNED:
    result = _PRINT("(CAST_UNSIGNED ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case EXTEND:
    result = _PRINT("(EXTEND ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case TWODOTS:
    result = _PRINT("\n(TWODOTS ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case COLON:
    result = _PRINT("\n(COLON ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case EQDEF:
    result = _PRINT("\n\t(EQDEF ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case NEXT:
    result = _PRINT("\n(NEXT (") &&
      _THROW(car(node), 0) &&
      _PRINT(")) ");
    break;

  case SMALLINIT:
    result = _PRINT("\n(SMALLINIT ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case AND:
    result = _PRINT("\n(AND ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case OR:
    result = _PRINT("\n(OR ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case XOR:
    result = _PRINT("\n(XOR ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case XNOR:
    result = _PRINT("\n(XNOR ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case IMPLIES:
    result = _PRINT("\n(IMPLIES ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case IFF:
    result = _PRINT("\n(IFF ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case NOT:
    result = _PRINT("\n(NOT ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case EX:
    result = _PRINT("\n(EX ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case AX:
    result = _PRINT("\n(AX ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case EF:
    result = _PRINT("\n(EF ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case AF:
    result = _PRINT("\n(AF ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case EG:
    result = _PRINT("\n(EG ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case AG:
    result = _PRINT("\n(AG ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case OP_GLOBAL:
    result = _PRINT("\n(G ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case OP_HISTORICAL:
    result = _PRINT("\n(H ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case RELEASES:
    result = _PRINT("\n(V ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case TRIGGERED:
    result = _PRINT("\n(T ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case UNTIL:
    result = _PRINT("\n(U ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case SINCE:
    result = _PRINT("\n(S ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case OP_NEXT:
    result = _PRINT("\n(X ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case OP_PREC:
    result = _PRINT("\n(Y ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case OP_NOTPRECNOT:
    result = _PRINT("\n(Z ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case OP_FUTURE:
    result = _PRINT("\n(F ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case OP_ONCE:
    result = _PRINT("\n(O ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case AU:
    result = _PRINT("\n(AU ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case EU:
    result = _PRINT("\n(EU ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case EBF:
    result = _PRINT("\n(EBF ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case ABF:
    result = _PRINT("\n(ABF ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case EBG:
    result = _PRINT("\n(EBG ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case ABG:
    result = _PRINT("\n(ABG ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case ABU:
    result = _PRINT("\n(ABU ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case EBU:
    result = _PRINT("\n(EBU ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case LSHIFT:
    result = _PRINT("\n(LSHIFT ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case RSHIFT:
    result = _PRINT("\n(RSHIFT ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case LROTATE:
    result = _PRINT("\n(LROTATE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case RROTATE:
    result = _PRINT("\n(RROTATE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case WORDARRAY:
    result = _PRINT("\n(WORDARRAY ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case WAREAD:
    result = _PRINT("\n(WAREAD ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case WAWRITE:
    result = _PRINT("\n(WAWRITE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case PLUS:
    result = _PRINT("\n(PLUS ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case MINUS:
    result = _PRINT("\n(MINUS ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case UMINUS:
    result = _PRINT("\n(UMINUS ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case TIMES:
    result = _PRINT("\n(TIMES ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case DIVIDE:
    result = _PRINT("\n(DIVIDE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case MOD:
    result = _PRINT("\n(MOD ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case EQUAL:
    result = _PRINT("\n(EQUAL ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case NOTEQUAL:
    result = _PRINT("\n(NOTEQUAL ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case LT:
    result = _PRINT("\n(LT ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case GT:
    result = _PRINT("\n(GT ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case LE:
    result = _PRINT("\n(LE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case GE:
    result = _PRINT("\n(GE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case UNION:
    result = _PRINT("\n(UNION ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case SETIN:
    result = _PRINT("\n(SETIN ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case MINU:
    result = _PRINT("\n(MINU ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case MAXU:
    result = _PRINT("\n(MAXU ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case PROCESS:
    result = _PRINT("\n(PROCESS ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case FAIRNESS:
    result = _PRINT("\n(FAIRNESS ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case JUSTICE:
    result = _PRINT("\n(JUSTICE ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case COMPASSION:
    result = _PRINT("\n(COMPASSION ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case ISA:
    result = _PRINT("\n(ISA ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case DEFINE:
    result = _PRINT("\n(DEFINE ") &&
      _THROW(car(node), 0) &&
      _PRINT(")");
    break;

  case CONTEXT:
    result = _PRINT("\n(CONTEXT ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")");
    break;

  case BDD:
    result = /* print_bdd(file,(bdd_ptr)car(node)); */
      _PRINT("(BDD TO BE PRINTED)");
    break;

  case SEMI:
    result = _PRINT("\n(SEMI ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")\n");
    break;

  case IFTHENELSE:
    result = _PRINT("\n (IFTHENELSE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")\n");
    break;

  case ATTIME:
    result = _PRINT("\n (ATTIME ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")\n");
    break;

  case ARRAY_TYPE:
    result = _PRINT("\n (ARRAY_TYPE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")\n");
    break;

  case ARRAY_DEF:
    result = _PRINT("\n (ARRAY_DEF ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")\n");
    break;

  case COMMA:
    result = _PRINT("\n (COMMA ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")\n");
    break;

  case COMPID:
    result = _PRINT("\n (COMPID ") &&
      _THROW(car(node), 0) &&
      _PRINT(")\n");
    break;

  case COMPWFF:
    result = _PRINT("\n (COMPWFF ") &&
      _THROW(car(node), 0) &&
      _PRINT(")\n");
    break;

  case SIMPWFF:
    result = _PRINT("\n (SIMPWFF ") &&
      _THROW(car(node), 0) &&
      _PRINT(")\n");
    break;

  case CTLWFF:
    result = _PRINT("\n (CTLWFF ") &&
      _THROW(car(node), 0) &&
      _PRINT(")\n");
    break;

  case LTLWFF:
    result = _PRINT("\n (LTLWFF ") &&
      _THROW(car(node), 0) &&
      _PRINT(")\n");
    break;

  case NEXTWFF:
    result = _PRINT("\n (NEXTWFF ") &&
      _THROW(car(node), 0) &&
      _PRINT(")\n");
    break;

  case CONSTRAINT:
    result = _PRINT("\n (CONSTRAINT ") &&
      _THROW(car(node), 0) &&
      _PRINT(")\n");
    break;

  case GOTO:
    {
      char buf[20];
      sprintf(buf, " %d)\n", NODE_TO_INT(cdr(node)));

      result = _PRINT("\n (GOTO ") &&
        _THROW(car(node), 0) &&
        _PRINT(buf);
      break;
    }

  case LAMBDA:
    result = _PRINT("\n (LAMBDA ");
    if (Nil == car(node)) {
      result &= _PRINT("Nil, ");
    }
    else {
      result &= _PRINT("<can be node or bdd>, ");
    }

    if (Nil == cdr(node)) {
      result &= _PRINT("Nil");
    }
    else {
      result &= _PRINT("<can be node or bdd>");
    }
    result &= _PRINT(")\n");
    break;

  case MIRROR:
    result = _PRINT("\n (MIRROR ") &&
      _THROW(car(node), 0) &&
      _PRINT(")\n");
    break;

  case PRED:
    result = _PRINT("\n (PRED ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")\n");
    break;

  case PREDS_LIST:
    result = _PRINT("\n (PREDS_LIST ") &&
      _THROW(car(node), 0) &&
      _PRINT(")\n");
    break;

  case RANGE:
    result = _PRINT("\n (RANGE ") &&
      _THROW(car(node), 0) &&
      _THROW(cdr(node), 0) &&
      _PRINT(")\n");
    break;

  case SYNTAX_ERROR:
    {
      const char* fname = get_text((string_ptr) caar(node));
      const char* token = (const char*) cadr(node);
      const char* msg = (const char*) cddr(node);
      int lineno = PTR_TO_INT(cdar(node));

      char buf[20];
      sprintf(buf, " %d", lineno);

      result = _PRINT("\n (SYNTAX_ERROR ") &&
        _PRINT("(COLON ") &&
        _PRINT(fname) &&
        _PRINT(buf) &&
        _PRINT(") (COLON ") &&
        _PRINT(token) &&
        _PRINT(msg) &&
        _PRINT("))\n");
      break;
    }

  default:
    if (node_get_type(node) <= NUSMV_CORE_SYMBOL_LAST) {
      char buf[20];
      int c = snprintf(buf, 20, "%d \n", node_get_type(node));

      SNPRINTF_CHECK(c, 20);

      result = _PRINT("\n\n ********* No Match \n") &&
        _PRINT("********** Descriptor: ") &&
        _PRINT(buf);
    }
    break;
  }

  result &= _PRINT("\n");
  return result;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions*/
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis[The PrinterSexpCore class virtual finalizer]

   Description [Called by the class destructor]

   SideEffects []

   SeeAlso []

******************************************************************************/
static void printer_sexp_core_finalize(Object_ptr object, void* dummy) 
{
  PrinterSexpCore_ptr self = PRINTER_SEXP_CORE(object);

  printer_sexp_core_deinit(self);
  FREE(self);
}

/**AutomaticEnd***************************************************************/

