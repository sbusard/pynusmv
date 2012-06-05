/***************************************************************  -*-C-*-  ***/
%{
/**CFile***********************************************************************

  FileName    [prob_input.l]

  PackageName [compass.parser.prob]

  Synopsis    [Lexical analyzer for the probability input language]

  SeeAlso     [prob_grammar.y]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``compass.parser.prob'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and FBK-irst. 

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
#include "nusmv-config.h"
#endif

#include "utils/utils.h"
#include "utils/ustring.h"
#include "utils/error.h"

#include "prob_grammar.h"
#include "ParserProb.h"
#include "ParserProb_private.h"

static char rcsid[] UTIL_UNUSED = "$Id: prob_input.l,v 1.1.2.2 2009-02-04 20:02:12 nusmv Exp $";

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
static ParserProb_ptr _parser = PARSER_PROB(NULL);

void parser_prob_set_global_parser(ParserProb_ptr parser)
{
  nusmv_assert(_parser == PARSER_PROB(NULL));
  _parser = parser;
}

void parser_prob_reset_global_parser(ParserProb_ptr parser)
{
  PARSER_PROB_CHECK_INSTANCE(_parser);
  nusmv_assert(parser == _parser);
  _parser = PARSER_PROB(NULL);
}

ParserProb_ptr parser_prob_get_global_parser(void)
{
   PARSER_PROB_CHECK_INSTANCE(_parser);
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
%option prefix="parser_prob_"
%option outfile="lex.yy.c"


%%
[ \n\t\r\f]             ;
"--"                    nusmv_skip_comment();
"."                     return TK_DOT;
"["                     return TK_LB;
"]"                     return TK_RB;
"&"                     return TK_AND;
"="                     return TK_EQUAL;
":"                     return TK_COLON;
";"                     return TK_SEMI;

"TRUE"                  {
                              parser_prob_lval.node = parser_prob_mk_true(parser_prob_get_global_parser());
                              return TK_TRUE;
                        }

"FALSE"                 {
                              parser_prob_lval.node = parser_prob_mk_false(parser_prob_get_global_parser());
                              return TK_FALSE;
                        }

[A-Za-z_][A-Za-z0-9_\$#-]*  { 
                              parser_prob_lval.node = 
                                parser_prob_mk_atom(parser_prob_get_global_parser(), 
                                                   parser_prob_text);
                              return TK_ATOM;
                            }
\"[^\"]*\"                  {
                              parser_prob_lval.node =
                                parser_prob_mk_atom(parser_prob_get_global_parser(),
                                                   parser_prob_text);
                              return TK_ATOM;
                            }
[0-9]+"."[0-9]+             { 
                              parser_prob_lval.node =
                                 parser_prob_mk_real(parser_prob_get_global_parser(), 
                                                    parser_prob_text);
                              return TK_REAL;
                            }
[0-9]+                      { 
                              int i;
                              sscanf(parser_prob_text, "%d", &i);
                              parser_prob_lval.node = 
                                parser_prob_mk_num(parser_prob_get_global_parser(), 
                                                  i);
                              return TK_NUMBER;
                            }
.	  		                    {  
                             extern FILE* nusmv_stderr;
                             fprintf(nusmv_stderr, 
                                     "While parsing variable ordering: "\
                                     "illegal character\n");
                             exit(1);
                           }
%%

static int nusmv_skip_comment() 
{
  register int c;

  do {
    c = input();
  } while ( c != '\n' && c != EOF );

  return 0;
}
 
