/**CHeaderFile*****************************************************************

  FileName    [symbols.h]

  PackageName [parser]

  Synopsis    [Parse-tree symbols set]

  Description [This file defines an enum containing all the core parse
  tree symbols set]

  Author      [Nusmv Team]

  Copyright   [
  This file is part of the ``parser'' package of NuSMV version 2.
  Copyright (C) 2005 by FBK-irst.

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

  Revision    [$Id: symbols.h,v 1.1.2.1.4.20.4.15 2010/02/02 10:09:34 nusmv Exp $]

******************************************************************************/


#ifndef __PARSER_SYMBOLS_H__
#define __PARSER_SYMBOLS_H__

enum NUSMV_CORE_SYMBOLS {
  NUSMV_CORE_SYMBOL_FIRST = 100,

  /* ---------------------------------------------------------------------- */
  NUSMV_STATEMENTS_SYMBOL_FIRST = NUSMV_CORE_SYMBOL_FIRST,

  TRANS, /* 101 */
  INIT,
  INVAR,
  ASSIGN,
  FAIRNESS,
  JUSTICE,
  COMPASSION,
  SPEC,
  LTLSPEC,
  PSLSPEC, /* 110 */
  INVARSPEC,
  COMPUTE,
  DEFINE,
  ISA,
  GOTO,
  CONSTRAINT,
  MODULE,
  PROCESS,
  MODTYPE,
  LAMBDA, /* 120 */
  CONSTANTS,

  PRED,
  ATTIME,
  PREDS_LIST,
  MIRROR,

  SYNTAX_ERROR,

  NUSMV_STATEMENTS_SYMBOL_LAST,
  /* ---------------------------------------------------------------------- */


  /* ---------------------------------------------------------------------- */
  NUSMV_EXPR_SYMBOL_FIRST,

  FAILURE,
  CONTEXT,
  EU,
  AU,
  EBU,
  ABU,
  MINU,
  MAXU,
  VAR,
  FROZENVAR,
  IVAR,
  BOOLEAN, /* 140 */
  ARRAY,
  SCALAR,
  CONS,
  BDD,
  SEMI,
  EQDEF,
  TWODOTS,
  FALSEEXP,
  TRUEEXP,
  SELF,
  CASE,
  COLON,
  IFTHENELSE,
  SIMPWFF,
  NEXTWFF,
  LTLWFF,
  CTLWFF,
  COMPWFF,
  ATOM,
  NUMBER, /* 160 */
  COMMA,
  IMPLIES,
  IFF,
  OR,
  XOR,
  XNOR,
  AND,
  NOT,
  EX,
  AX,
  EF,
  AF,
  EG,
  AG,
  SINCE,
  UNTIL,
  TRIGGERED,
  RELEASES,
  EBF,
  EBG, /* 180 */
  ABF,
  ABG,
  OP_NEXT,
  OP_GLOBAL,
  OP_FUTURE,
  OP_PREC,
  OP_NOTPRECNOT,
  OP_HISTORICAL,
  OP_ONCE,
  EQUAL,
  NOTEQUAL,
  LT,
  GT,
  LE,
  GE,
  UNION,
  SETIN,
  MOD,
  PLUS,
  MINUS, /* 200 */
  TIMES,
  DIVIDE,
  UMINUS,
  NEXT,
  SMALLINIT,
  DOT,
  BIT,
  RANGE,
  UNSIGNED_WORD, /* identifies expressions and types */
  SIGNED_WORD,   /* identifies types */
  INTEGER,
  REAL,
  NUMBER_UNSIGNED_WORD, /* identifies constants */
  NUMBER_SIGNED_WORD,   /* identifies constants */
  NUMBER_FRAC,
  NUMBER_REAL,
  NUMBER_EXP,
  LSHIFT,
  RSHIFT,
  LROTATE, /* 220 */
  RROTATE,
  BIT_SELECTION,
  CONCATENATION,
  CAST_BOOL,
  CAST_WORD1,
  CAST_SIGNED,
  CAST_UNSIGNED,
  EXTEND, /* extend the width of a word (signed or unsigned) */
  WORDARRAY,
  WAREAD, /* 230 */
  WAWRITE,
  UWCONST, /* these are removed by the flattener, so */
  SWCONST, /* after flattening they do not exist anywhere */
  WRESIZE,
  WSIZEOF,
  CAST_TOINT,

  COMPID, /* Used for comparing properties ids */

  ARRAY_TYPE, /* Used to represent generic array type */
  ARRAY_DEF,  /* Define obtained via Matrix-Define */

  NFUNCTION, /* 240 */

  COUNT,

  NUSMV_EXPR_SYMBOL_LAST,
  /* ---------------------------------------------------------------------- */

  NUSMV_CORE_SYMBOL_LAST
};



#endif /* __PARSER_SYMBOLS_H__ */
