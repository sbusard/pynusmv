%{
/**CFile***********************************************************************

  FileName    [ap_grammar.y]

  PackageName [parser.ap]

  Synopsis    [Yacc for apability input file]

  SeeAlso     [ap_input.l]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``compass.parser.ap'' package of NuSMV version 2.
  Copyright (C) 2008 by FBK-irst.

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

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include <setjmp.h>

#if NUSMV_HAVE_MALLOC_H
# if NUSMV_HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# include <malloc.h>
#elif NUSMV_HAVE_SYS_MALLOC_H
# if NUSMV_HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# include <sys/malloc.h>
#elif NUSMV_HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include "ParserAp.h"
#include "ParserAp_private.h"
#include "apInt.h"

#include "node/node.h"
#include "utils/error.h"
#include "utils/utils.h"

#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: ";

extern FILE* nusmv_stderr;


void parser_ap_error(char *s);

enum EXP_KIND {EXP_SIMPLE, EXP_NEXT, EXP_LTL, EXP_CTL};
static boolean isCorrectExp ARGS((node_ptr exp, enum EXP_KIND expectedKind));


%}

%union {
  node_ptr node;
}

/*
  All of the terminal grammar symbols (tokens recognized by the
  lexical analyzer) Note: all binary operators associate from left
  to right, operators are listed from lowest to highest priority

  Note: The following token are not used inside the grammar, but
  are used by other modules inside the system (i.e. the compiler,
  mc).  CONTEXT EU AU EBU ABU MINU MAXU CONS OVER BIT
*/

%left <node> TOK_ATOM TOK_FALSEEXP TOK_TRUEEXP
%left <node> TOK_NUMBER TOK_NUMBER_FRAC TOK_NUMBER_REAL TOK_NUMBER_EXP
%left <node> TOK_NUMBER_WORD

%left TOK_CONS TOK_SEMI
%left TOK_LP TOK_RP TOK_RB TOK_LCB TOK_RCB
%left TOK_TWODOTS TOK_SELF TOK_CASE TOK_ESAC TOK_COLON
%left TOK_COMMA TOK_IMPLIES TOK_IFF  TOK_OR TOK_XOR TOK_XNOR TOK_AND TOK_NOT
%left TOK_EX TOK_AX TOK_EF TOK_AF TOK_EG TOK_AG TOK_EE TOK_AA
%left TOK_SINCE TOK_UNTIL TOK_TRIGGERED TOK_RELEASES
%left TOK_EBF TOK_EBG TOK_ABF TOK_ABG TOK_BUNTIL TOK_MMIN TOK_MMAX
%left TOK_OP_NEXT TOK_OP_GLOBAL TOK_OP_FUTURE
%left TOK_OP_PREC TOK_OP_NOTPRECNOT TOK_OP_HISTORICAL TOK_OP_ONCE
%left TOK_EQUAL TOK_NOTEQUAL TOK_LT TOK_GT TOK_LE TOK_GE
%left TOK_UNION TOK_SETIN TOK_LSHIFT TOK_RSHIFT TOK_LROTATE TOK_RROTATE
%left TOK_MOD TOK_PLUS TOK_MINUS TOK_TIMES TOK_DIVIDE
%left TOK_NEXT TOK_SMALLINIT TOK_CONCATENATION
%left TOK_LB TOK_DOT TOK_BIT
%left TOK_SIGNED TOK_UNSIGNED TOK_EXTEND
%left TOK_BOOL TOK_WORD1


/* all nonterminals return a parse tree node */
%type <node> ap_list_item  ap_list

%type <node> number integer number_word number_frac number_real number_exp subrange

%type <node> constant primary_expr case_element_expr case_element_list_expr
%type <node> concatination_expr multiplicative_expr
%type <node> additive_expr shift_expr
%type <node> set_expr set_list_expr union_expr in_expr relational_expr
%type <node> ctl_expr pure_ctl_expr ctl_and_expr
%type <node> ctl_or_expr ctl_iff_expr ctl_implies_expr ctl_basic_expr
%type <node> ltl_binary_expr ltl_unary_expr pure_ltl_unary_expr
%type <node> and_expr or_expr iff_expr implies_expr basic_expr
%type <node> simple_expression
/*
%type <node> next_expression ctl_expression ltl_expression
*/

%start begin
%%
begin         : ap_list { }
              ;

/* Repetition of variables */
ap_list     : {}
             | ap_list_item ap_list
             {
               parser_ap_add(parser_ap_get_global_parser(), $1);
             }
;

ap_list_item : TOK_ATOM TOK_COLON simple_expression optsemi
{
  $$ = parser_ap_mk_ap(parser_ap_get_global_parser(), $1, $3);
}
;

/* parse an optional semicolon */
optsemi : | TOK_SEMI {};

number        : TOK_NUMBER
              | TOK_PLUS TOK_NUMBER { $$ = $2; }
              ;

integer       : TOK_NUMBER
              | TOK_PLUS TOK_NUMBER { $$ = $2; }
              | TOK_MINUS TOK_NUMBER
                  {node_int_setcar($2, -(node_get_int($2))); $$ = $2;}
              ;

number_word   : TOK_NUMBER_WORD
              ;
number_frac   : TOK_NUMBER_FRAC
              ;
number_real   : TOK_NUMBER_REAL
              ;
number_exp    : TOK_NUMBER_EXP
              ;

subrange      : integer TOK_TWODOTS integer
                  {$$ = new_node(TWODOTS, $1, $3);}
              ;

constant     : TOK_FALSEEXP
             | TOK_TRUEEXP
             | number
             | number_word
             | number_frac
               {
                 $$ = $1;
               }
             | number_exp
               {
                 $$ = $1;
               }
             | number_real
               {
                 $$ = $1;
               }
             ;

/* expression has to have "var_identifier", but it is ambiguous with
   bit-selection (the problem is with "left-bracket" (TOK_LB)).
   So they are put in one place and "var_idenitifier" alternatives have
   additional assertions to check that array's and
   dot's rules are applied to var_idintifier only.
*/
primary_expr :
               constant
             | TOK_MINUS primary_expr { $$ = new_node(UMINUS, $2, Nil); }
             | TOK_ATOM
             | TOK_SELF         {$$ = new_node(SELF,Nil,Nil);}
             | primary_expr TOK_DOT TOK_ATOM
                    {
                      if (ATOM != node_get_type($1) &&
                          DOT != node_get_type($1) &&
                          ARRAY != node_get_type($1) &&
                          SELF != node_get_type($1)) {
                        yyerror("incorrect DOT expression");
                        YYABORT;
                      }
                      $$ = new_node(DOT, $1, $3) ;
                    }
             | primary_expr TOK_DOT TOK_NUMBER
                    {
                      if (ATOM != node_get_type($1) &&
                          DOT != node_get_type($1) &&
                          ARRAY != node_get_type($1) &&
                          SELF != node_get_type($1)) {
                        yyerror("incorrect DOT expression");
                        YYABORT;
                      }
                      $$ = new_node(DOT, $1, $3) ;
                    }
             | primary_expr TOK_LB simple_expression TOK_RB
                     {
                       if (ATOM != node_get_type($1) &&
                           DOT != node_get_type($1) &&
                           ARRAY != node_get_type($1) &&
                           SELF != node_get_type($1)) {
                        yyerror("incorrect ARRAY expression");
                        YYABORT;
                       }
                       $$ = new_node(ARRAY, $1, $3);
                     }
             | primary_expr TOK_LB simple_expression TOK_COLON simple_expression TOK_RB
                       {
                        $$ = new_node(BIT_SELECTION, $1, new_node(COLON, $3, $5));
                       }
             | TOK_LP basic_expr TOK_RP             { $$ = $2; }
             | TOK_NOT primary_expr                 { $$ = new_node(NOT, $2, Nil); }
             | TOK_BOOL  TOK_LP basic_expr TOK_RP   { $$ = new_node(CAST_BOOL, $3, Nil); }
             | TOK_WORD1 TOK_LP basic_expr TOK_RP   { $$ = new_node(CAST_WORD1, $3, Nil); }
             | TOK_NEXT  TOK_LP basic_expr TOK_RP   { $$ = new_node(NEXT, $3, Nil); }
             | TOK_SIGNED   TOK_LP basic_expr TOK_RP   { $$ = new_node(CAST_SIGNED, $3, Nil); }
             | TOK_UNSIGNED TOK_LP basic_expr TOK_RP   { $$ = new_node(CAST_UNSIGNED, $3, Nil); }
             | TOK_EXTEND   TOK_LP basic_expr TOK_COMMA basic_expr TOK_RP   { $$ = new_node(EXTEND, $3, $5); }
             | TOK_CASE case_element_list_expr TOK_ESAC { $$ = $2; }
             ;

case_element_list_expr
             : case_element_expr /* last element in the list. Add FAILURE node */
                   { $$ = new_node(CASE, $1, failure_make("case conditions are not exhaustive", FAILURE_CASE_NOT_EXHAUSTIVE, parser_ap_lineno));}
             | case_element_expr case_element_list_expr { $$ = new_node(CASE, $1, $2); }
             ;

case_element_expr
             : basic_expr TOK_COLON basic_expr TOK_SEMI
                                    {$$ = new_node(COLON, $1, $3);}
             ;

concatination_expr :
               primary_expr
             | concatination_expr TOK_CONCATENATION primary_expr { $$ = new_node(CONCATENATION, $1, $3); }
             ;

multiplicative_expr :
               concatination_expr
             | multiplicative_expr TOK_TIMES concatination_expr  { $$ = new_node(TIMES, $1, $3); }
             | multiplicative_expr TOK_DIVIDE concatination_expr { $$ = new_node(DIVIDE, $1, $3); }
             | multiplicative_expr TOK_MOD concatination_expr    { $$ = new_node(MOD, $1, $3); }
             ;

additive_expr :
               multiplicative_expr
             | additive_expr TOK_PLUS multiplicative_expr  { $$ = new_node(PLUS, $1, $3); }
             | additive_expr TOK_MINUS multiplicative_expr { $$ = new_node(MINUS, $1, $3); }
             ;

shift_expr :   additive_expr
             | shift_expr TOK_LSHIFT additive_expr   { $$ = new_node(LSHIFT, $1, $3); }
             | shift_expr TOK_RSHIFT additive_expr   { $$ = new_node(RSHIFT, $1, $3); }
/*
             | shift_expr TOK_LROTATE additive_expr  { $$ = new_node(LROTATE, $1, $3, $2); }
             | shift_expr TOK_RROTATE additive_expr  { $$ = new_node(RROTATE, $1, $3, $2); } */
             ;

set_expr     : shift_expr
             | subrange
             | TOK_LCB set_list_expr TOK_RCB   { $$ = $2; }
             ;

set_list_expr: basic_expr
             | set_list_expr TOK_COMMA basic_expr {$$ = new_node(UNION, $1, $3);}
             ;


union_expr   : set_expr
             | union_expr TOK_UNION set_expr { $$ = new_node(UNION, $1, $3); }
             ;

in_expr :      union_expr
             | in_expr TOK_SETIN union_expr { $$ = new_node(SETIN, $1, $3); }
             ;

relational_expr :
               in_expr
             | relational_expr TOK_EQUAL in_expr { $$ = new_node(EQUAL, $1, $3); }
             | relational_expr TOK_NOTEQUAL in_expr { $$ = new_node(NOTEQUAL, $1, $3); }
             | relational_expr TOK_LT in_expr { $$ = new_node(LT, $1, $3); }
             | relational_expr TOK_GT in_expr { $$ = new_node(GT, $1, $3); }
             | relational_expr TOK_LE in_expr { $$ = new_node(LE, $1, $3); }
             | relational_expr TOK_GE in_expr { $$ = new_node(GE, $1, $3); }
             ;

ctl_expr     : relational_expr
             | pure_ctl_expr /* all CTL operators */
             ;
/* pure ctl_expr is introduced to allow NOT before the ctl expressions */
pure_ctl_expr
             : TOK_EX ctl_expr       { $$ = new_node(EX, $2, Nil); }
             | TOK_AX ctl_expr       { $$ = new_node(AX, $2, Nil); }
             | TOK_EF ctl_expr       { $$ = new_node(EF, $2, Nil); }
             | TOK_AF ctl_expr       { $$ = new_node(AF, $2, Nil); }
             | TOK_EG ctl_expr       { $$ = new_node(EG, $2, Nil); }
             | TOK_AG ctl_expr       { $$ = new_node(AG, $2, Nil); }
             | TOK_AA TOK_LB ctl_basic_expr TOK_UNTIL ctl_basic_expr TOK_RB
                                     { $$ = new_node(AU, $3, $5); }
             | TOK_EE TOK_LB ctl_basic_expr TOK_UNTIL ctl_basic_expr TOK_RB
                                     { $$ = new_node(EU, $3, $5); }
             | TOK_AA TOK_LB ctl_basic_expr TOK_BUNTIL subrange ctl_basic_expr TOK_RB
                                     { $$ = new_node(ABU, new_node(AU, $3, $6), $5); }
             | TOK_EE TOK_LB ctl_basic_expr TOK_BUNTIL subrange ctl_basic_expr TOK_RB
                                     { $$ = new_node(EBU, new_node(EU, $3, $6), $5); }
             | TOK_EBF subrange ctl_expr { $$ = new_node(EBF, $3, $2); }
             | TOK_ABF subrange ctl_expr { $$ = new_node(ABF, $3, $2); }
             | TOK_EBG subrange ctl_expr { $$ = new_node(EBG, $3, $2); }
             | TOK_ABG subrange ctl_expr { $$ = new_node(ABG, $3, $2); }

             /* NOT is required here to allow such expr as "! EX a" */
             | TOK_NOT pure_ctl_expr { $$ = new_node(NOT, $2, Nil); }
             ;
/* there are separate CTL rules for propositional expressions
   to avoid ambiguity related to TOK_UNTIL token in LTL and CTL.
*/
ctl_and_expr :
               ctl_expr
             | ctl_and_expr TOK_AND ctl_expr  { $$ = new_node(AND, $1, $3); }
             ;
ctl_or_expr :
               ctl_and_expr
             | ctl_or_expr TOK_OR ctl_and_expr    { $$ = new_node(OR,$1, $3); }
             | ctl_or_expr TOK_XOR ctl_and_expr   { $$ = new_node(XOR,$1, $3); }
             | ctl_or_expr TOK_XNOR ctl_and_expr  { $$ = new_node(XNOR,$1, $3); }
             ;
ctl_iff_expr :
               ctl_or_expr
             | ctl_iff_expr TOK_IFF ctl_or_expr   { $$ = new_node(IFF, $1, $3); }
             ;

ctl_implies_expr : /* right association */
               ctl_iff_expr
             | ctl_iff_expr TOK_IMPLIES ctl_implies_expr { $$ = new_node(IMPLIES, $1, $3); }
             ;

ctl_basic_expr : ctl_implies_expr;

/* LTL has to include CTL to allow paranthesis around CTL (and everything) */
ltl_unary_expr
             : ctl_expr
             | pure_ltl_unary_expr /* all unary LTL operators */
             ;

/* pure ltl_unary_expr is introduced to allow NOT before the ltl expressions */
pure_ltl_unary_expr
             : TOK_OP_NEXT ltl_unary_expr  {$$ = new_node(OP_NEXT, $2, Nil);}
             | TOK_OP_PREC ltl_unary_expr  {$$ = new_node(OP_PREC, $2, Nil);}
             | TOK_OP_NOTPRECNOT ltl_unary_expr {$$ = new_node(OP_NOTPRECNOT, $2, Nil);}
             | TOK_OP_GLOBAL ltl_unary_expr {$$ = new_node(OP_GLOBAL, $2, Nil);}
             | TOK_OP_HISTORICAL ltl_unary_expr {$$ = new_node(OP_HISTORICAL, $2, Nil);}
             | TOK_OP_FUTURE ltl_unary_expr {$$ = new_node(OP_FUTURE, $2, Nil);}
             | TOK_OP_ONCE ltl_unary_expr {$$ = new_node(OP_ONCE, $2, Nil);}
             /* NOT is required here to allow such expr as "! X a" */
             | TOK_NOT pure_ltl_unary_expr { $$ = new_node(NOT, $2, Nil); }
             ;

/* all LTL binary operators */
ltl_binary_expr :
                ltl_unary_expr
              | ltl_binary_expr TOK_UNTIL ltl_unary_expr
                                {$$ = new_node(UNTIL, $1, $3);}
              | ltl_binary_expr TOK_SINCE ltl_unary_expr
                                {$$ = new_node(SINCE, $1, $3);}
              | ltl_binary_expr TOK_RELEASES ltl_unary_expr
                  {$$ = new_node(NOT,
                           new_node(UNTIL,
                             new_node(NOT, $1, Nil),
                             new_node(NOT, $3, Nil)), Nil);
                  }
              | ltl_binary_expr TOK_TRIGGERED ltl_unary_expr
                  {$$ = new_node(NOT,
                          new_node(SINCE,
                              new_node(NOT, $1, Nil),
                              new_node(NOT, $3, Nil)), Nil);
                  }
              ;

and_expr :
               ltl_binary_expr
             | and_expr TOK_AND ltl_binary_expr  { $$ = new_node(AND, $1, $3); }
             ;

or_expr :
               and_expr
             | or_expr TOK_OR and_expr    { $$ = new_node(OR,$1, $3); }
             | or_expr TOK_XOR and_expr   { $$ = new_node(XOR,$1, $3); }
             | or_expr TOK_XNOR and_expr  { $$ = new_node(XNOR,$1, $3); }
             ;

iff_expr :
               or_expr
             | iff_expr TOK_IFF or_expr   { $$ = new_node(IFF, $1, $3); }
             ;

implies_expr : /* right association */
               iff_expr
             | iff_expr TOK_IMPLIES implies_expr { $$ = new_node(IMPLIES, $1, $3); }
             ;

basic_expr : implies_expr;

simple_expression : basic_expr   {if (!isCorrectExp($$, EXP_SIMPLE)) YYABORT;}
                  ;

/*
next_expression   : basic_expr   {if (!isCorrectExp($$, EXP_NEXT)) YYABORT;}
                  ;

ctl_expression    : basic_expr   {if (!isCorrectExp($$, EXP_CTL)) YYABORT;}
                  ;

ltl_expression    : basic_expr   {if (!isCorrectExp($$, EXP_LTL)) YYABORT;}
                  ;
*/

%%


/* Additional source code */
void parser_ap_error(char *s)
{
    extern char parser_ap_text[];

    fprintf(nusmv_stderr,"\n");
    if (get_output_order_file(OptsHandler_get_instance())) {
      fprintf(nusmv_stderr, "file %s: ",
              get_output_order_file(OptsHandler_get_instance()));
    }
    else {
      fprintf(nusmv_stderr, "file stdin: ");
    }

    if (parser_ap_lineno) {
      fprintf(nusmv_stderr, "line %d: ", parser_ap_lineno);
    }

    fprintf(nusmv_stderr, "at token \"%s\": %s\n", parser_ap_text, s);
    if (opt_batch(OptsHandler_get_instance())) {
      /* exits the execution */
      fprintf(nusmv_stderr, "\n");
      print_io_atom_stack(nusmv_stderr);
      nusmv_exit(1);
    }
}

int parser_ap_wrap()  { return 1; }

extern void yyerror_lined(const char *s, int line);

/* this functions checks that the expression is formed syntactically correct.
   Takes the expression to be checked, the expected kind of the
   expression. Returns true if the expression is formed correctly, and
   false otherwise.
   See enum EXP_KIND for more info about syntactic well-formedness.
*/
static boolean isCorrectExp(node_ptr exp, enum EXP_KIND expectedKind)
{
  switch(node_get_type(exp))
    {
      /* atomic expression (or thier operands have been checked earlier) */
    case FAILURE:
    case FALSEEXP:
    case TRUEEXP:
    case NUMBER:
    case NUMBER_UNSIGNED_WORD:
    case NUMBER_SIGNED_WORD:
    case NUMBER_FRAC:
    case NUMBER_REAL:
    case NUMBER_EXP:
    case TWODOTS:
    case DOT:
    case ATOM:
    case SELF:
    case ARRAY:
    case BIT_SELECTION:
      return true;

      /* unary operators incompatible with LTL and CTL operator */
    case CAST_BOOL:
    case CAST_WORD1:
    case CAST_SIGNED:
    case CAST_UNSIGNED:
    case EXTEND:
      if (EXP_LTL == expectedKind || EXP_CTL == expectedKind) {
        return isCorrectExp(car(exp), EXP_SIMPLE);
      }
      /* unary operators compatible with LTL and CTL operator */
    case NOT:
    case UMINUS:
      return isCorrectExp(car(exp), expectedKind);

      /* binary opertors incompatible with LTL and CTL operator */
    case CASE: case COLON:
    case CONCATENATION:
    case TIMES: case DIVIDE: case PLUS :case MINUS: case MOD:
    case LSHIFT: case RSHIFT: case LROTATE: case RROTATE:
    case WAREAD: case WAWRITE: /* AC ADDED THESE */
    case UNION: case SETIN:
    case EQUAL: case NOTEQUAL: case LT: case GT: case LE: case GE:
      if (EXP_LTL == expectedKind || EXP_CTL == expectedKind) {
        return isCorrectExp(car(exp), EXP_SIMPLE)
          && isCorrectExp(cdr(exp), EXP_SIMPLE);
      }
      /* binary opertors compatible LTL and CTL operator */
    case AND: case OR: case XOR: case XNOR: case IFF: case IMPLIES:
      return isCorrectExp(car(exp), expectedKind)
        && isCorrectExp(cdr(exp), expectedKind);

      /* next expression */
    case NEXT:
      if (EXP_NEXT != expectedKind) {
        yyerror_lined("unexpected 'next' operator", node_get_lineno(exp));
        return false;
      }
      return isCorrectExp(car(exp), EXP_SIMPLE); /* NEXT cannot contain NEXT */

      /* CTL unary expressions */
    case EX: case AX: case EF: case AF: case EG: case AG:
    case ABU: case EBU:
    case EBF: case ABF: case EBG: case ABG:
      if (EXP_CTL != expectedKind) {
        yyerror_lined("unexpected CTL operator", node_get_lineno(exp));
        return false;
      }
      return isCorrectExp(car(exp), EXP_CTL); /* continue to check CTL */

      /* CTL binary expressions */
    case AU: case EU:
      if (EXP_CTL != expectedKind) {
        yyerror_lined("unexpected CTL operator", node_get_lineno(exp));
        return false;
      }
      return isCorrectExp(car(exp), EXP_CTL)
        && isCorrectExp(cdr(exp), EXP_CTL); /* continue to check CTL */


      /* LTL unary expressions */
    case OP_NEXT: case OP_PREC: case OP_NOTPRECNOT: case OP_GLOBAL:
    case OP_HISTORICAL: case OP_FUTURE: case OP_ONCE:
      if (EXP_LTL != expectedKind) {
        yyerror_lined("unexpected LTL operator", node_get_lineno(exp));
        return false;
      }
      return isCorrectExp(car(exp), EXP_LTL); /* continue to check LTL */


      /* LTL binary expressions */
    case UNTIL: case SINCE:
      if (EXP_LTL != expectedKind) {
        yyerror_lined("unexpected LTL operator", node_get_lineno(exp));
        return false;
      }
      return isCorrectExp(car(exp), EXP_LTL)
        && isCorrectExp(cdr(exp), EXP_LTL); /* continue to check LTL */

    default: nusmv_assert(false); /* unknown expression */
    }
  return false; /* should never be invoked */
}
