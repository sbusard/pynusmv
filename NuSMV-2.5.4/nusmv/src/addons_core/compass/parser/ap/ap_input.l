/***************************************************************  -*-C-*-  ***/
%{
/**CFile***********************************************************************

  FileName    [ap_input.l]

  PackageName [compass.parser.ap]

  Synopsis    [Lexical analyzer for the atomic proposition input language]

  SeeAlso     [ap_grammar.y]

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
  Please report bugs to <nusmv-users@fbk.eu>.]

******************************************************************************/

#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "utils/utils.h"
#include "utils/ustring.h"
#include "utils/error.h"
#include "utils/WordNumber.h"
#include "parser/symbols.h"

#include "ap_grammar.h" /* For yacc/lex interaction */
#include "ParserAp.h"
#include "ParserAp_private.h"

static char rcsid[] UTIL_UNUSED = "$Id: ap_input.l,v 1.1.2.3 2009-09-30 15:34:34 nusmv Exp $";

/* The below code is only meaningful for flex version 2.5.31 which is
   buggy since it does not define the yy_prev_more_offset static
   variable */
#if defined YY_FLEX_SUBMINOR_VERSION &&         \
  defined YY_FLEX_MAJOR_VERSION &&              \
  defined YY_FLEX_MINOR_VERSION
#if YY_FLEX_SUBMINOR_VERSION == 31 &&           \
  YY_FLEX_MINOR_VERSION == 5 &&                 \
  YY_FLEX_MAJOR_VERSION == 2
static int yy_prev_more_offset = 0;
#endif
#endif


#if ! NUSMV_HAVE_ISATTY
static inline int isatty(int d) { return 0; }
#endif

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/* The parser instance used by flex/bison generated code */
static ParserAp_ptr _parser = PARSER_AP(NULL);

void parser_ap_set_global_parser(ParserAp_ptr parser)
{
  nusmv_assert(_parser == PARSER_AP(NULL));
  _parser = parser;
}

void parser_ap_reset_global_parser(ParserAp_ptr parser)
{
  PARSER_AP_CHECK_INSTANCE(_parser);
  nusmv_assert(parser == _parser);
  _parser = PARSER_AP(NULL);
}

ParserAp_ptr parser_ap_get_global_parser(void)
{
   PARSER_AP_CHECK_INSTANCE(_parser);
   return _parser;
}


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int nusmv_skip_comment ARGS((void));

/* The following are needed in order to allow parsing of long
   comments, that otherwise will cause errors since flex thinks that
   it is using REJECT even if it isn't.  The "%option noreject" seems
   to be ignored in the latest (and previous) versions of flex.
*/
#ifdef YY_USES_REJECT
#undef YY_USES_REJECT
#endif

%}
%a	15000
%o	25000

%option pointer
%option yylineno
%option noreject
%option prefix="parser_ap_"
%option outfile="lex.yy.c"


%%
[ \n\t\r\f]             ;
"--"                    nusmv_skip_comment();
"("                     return(TOK_LP);
")"                     return(TOK_RP);
"["                     return(TOK_LB);
"]"                     return(TOK_RB);
"{"                     return(TOK_LCB);
"}"                     return(TOK_RCB);
"FALSE"                 {
                                parser_ap_lval.node = new_node(FALSEEXP,Nil,Nil);
                                return(TOK_FALSEEXP);
                        }
"TRUE"                  {
                                parser_ap_lval.node = new_node(TRUEEXP,Nil,Nil);
                                return(TOK_TRUEEXP);
                        }
"case"                  return(TOK_CASE);
"esac"                  return(TOK_ESAC);
"+"                     return(TOK_PLUS);
"-"                     return(TOK_MINUS);
"*"                     return(TOK_TIMES);
"/"                     return(TOK_DIVIDE);
"mod"                   return(TOK_MOD);
"<<"                    return(TOK_LSHIFT);
">>"                    return(TOK_RSHIFT);
"<<<"                   return(TOK_LROTATE);
">>>"                   return(TOK_RROTATE);
"="                     return(TOK_EQUAL);
"!="                    return(TOK_NOTEQUAL);
"<="                    return(TOK_LE);
">="                    return(TOK_GE);
"<"                     return(TOK_LT);
">"                     return(TOK_GT);
"next"                  return(TOK_NEXT);
"init"                  return(TOK_SMALLINIT);
"self"                  return(TOK_SELF);
"union"                 return(TOK_UNION);
"in"                    return(TOK_SETIN);
".."                    return(TOK_TWODOTS);
"."                     return(TOK_DOT);
"->"                    return(TOK_IMPLIES);
"<->"                   return(TOK_IFF);
"|"                     return(TOK_OR);
"&"                     return(TOK_AND);
"xor"                   return(TOK_XOR);
"xnor"                  return(TOK_XNOR);
"!"                     return(TOK_NOT);
","                     return(TOK_COMMA);
":"                     return(TOK_COLON);
";"                     return(TOK_SEMI);
"::"                    return(TOK_CONCATENATION);
 /* word constants */
"0"([us])?[bBoOdDhH][0-9]*"_"[0-9a-fA-F][0-9a-fA-F_]* {
                 char* errorString;
                 WordNumber_ptr wordConstant
                        = WordNumber_from_parsed_string(parser_ap_text, &errorString);
                 /* check for errors */
                 if (WORD_NUMBER(NULL) == wordConstant) rpterr("%s", errorString);
                 parser_ap_lval.node = new_node('s' == parser_ap_text[1]
                                        ? NUMBER_SIGNED_WORD : NUMBER_UNSIGNED_WORD,
                                        (node_ptr)wordConstant,Nil);
                 return(TOK_NUMBER_WORD);
               }

 /* real, fractional and exponential constants */
[fF]"'"[0-9]+"/"[0-9]+  {
                  parser_ap_text[0] = 'f';
                  parser_ap_lval.node = new_node(NUMBER_FRAC,
                                         (node_ptr)find_string(parser_ap_text),Nil);
                  return(TOK_NUMBER_FRAC);
                 }

[0-9]+("."[0-9]+)?[eE][+-]?[0-9]+ {
                  parser_ap_lval.node = new_node(NUMBER_EXP,
                                         (node_ptr)find_string(parser_ap_text),Nil);
                  return(TOK_NUMBER_EXP);
                 }

[0-9]+"."[0-9]+  {
                    parser_ap_lval.node = new_node(NUMBER_REAL,
                                           (node_ptr)find_string(parser_ap_text),Nil);
                    return(TOK_NUMBER_REAL);
                 }

 /* integer number */
[0-9]+           {
                  int i;
                  sscanf(parser_ap_text,"%d",&i);
                  parser_ap_lval.node = new_node(NUMBER, NODE_FROM_INT(i), Nil);
                  return(TOK_NUMBER);
                 }

 /* identifier */
[A-Za-z_][A-Za-z0-9_\$#-]*  {
                  parser_ap_lval.node = new_node(ATOM,
                                                 (node_ptr)find_string(parser_ap_text),Nil);
                  return(TOK_ATOM);
                 }
\"[^\"]*\"       {
                  parser_ap_lval.node = new_node(ATOM,(node_ptr)find_string(parser_ap_text),Nil);
                  return(TOK_ATOM);
                 } /* Unused token */

.                 rpterr("illegal character : \"%s\"", parser_ap_text);
%%

static int nusmv_skip_comment()
{
  register int c;

  do {
    c = input();
  } while ( c != '\n' && c != EOF );

  return 0;
}

