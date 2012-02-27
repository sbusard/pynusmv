%{
/**CFile***********************************************************************

  FileName    [psl_grammar.y]

  PackageName [parser.psl]

  Synopsis [Grammar (for Yacc and Bison) of PSL specification input
  language]

  SeeAlso     [psl_input.l]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``parser.psl'' package of NuSMV version 2. 
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

******************************************************************************/

#include "pslExpr.h"
#include "utils/error.h"

EXTERN int yylex ARGS((void));
EXTERN void Parser_switch_to_smv ARGS(());

EXTERN void psl_yyerror ARGS((char* s, ...));
EXTERN int psl_error ARGS((void));

  node_ptr psl_parsed_tree; /* the returned value of parsing */
  node_ptr psl_property_name = Nil; /* the property name, Nil if none */
%}

/*%debug*/
/* %locations creates a structure storing line and column plus other
   info. Useful for printing error messages. */
/*%locations*/
/*%defines*/


%union {
  node_ptr node; 
  int lineno;

  /* these are news */
  int ival;
  char* wval;
  char* fval;
  char* baseval;
  char* idname;
  PslExpr psl_expr;
  PslOp operator;
};

%token TKEOF            "eof"
%token TKSTRING         "string"
%token TKERROR          
%token TKSTRUCT         "struct"

%token <ival> TKNUMBER         "number"
%token <fval> TKREALNUMBER     "real number"
%token <wval> TKWORDNUMBER     "word number"
%token <baseval> TKBASENUMBER  "based number"

%token TKTRUE TKFALSE

%token TKUNSIGNEDWORDNUMBER
%token TKSIGNEDWORDNUMBER

%token TKINCONTEXT
%token TKEQDEF TKNAME

/* ---------------------------------------------------------------------- */
/* These are used to build specific nodes that can be recognized later    */
%token TKSERE TKSERECONCAT TKSEREFUSION TKSERECOMPOUND TKSEREREPEATED
%token TKCONS
%token TKCONCATENATION
%token TKREPLPROP
%token TKARRAY TKCONTEXT 
%token <idname> TKATOM
%token TKFAILURE
%token TKITE
/* ---------------------------------------------------------------------- */

%token TKVUNIT          "vunit"
%token TKVMODE          "vmode"
%token TKVPROP          "vprop"
%token TKSTRONG         "strong"
%token TKDEFPARAM       "defparam"

%token TKINHERIT        "inherit"

%token TKFAIRNESS       "fairness"
%token TKCONST          "const"
%token TKBEGIN          "begin"
%token TKEND            "end"
%token TKPARAMETER      "parameter"
%token TKTASK           "task"
%token TKENDTASK        "endtask"
%token TKFORK           "fork"
%token TKJOIN           "join"
%token TKSUPPLY0        "supply0"
%token TKSUPPLY1        "supply1"
%token TKSTRONG0        "strong0"
%token TKPULL0          "pull0"
%token TKWEAK0          "weak0"
%token TKHIGHZ0         "highz0"
%token TKSTRONG1        "strong1"
%token TKPULL1          "pull1"
%token TKWEAK1          "weak1"
%token TKHIGHZ1         "highz1"

%token TKINPUT          "input"
%token TKOUTPUT         "output"
%token TKINOUT          "inout"
%token TKDEFAULT_CLOCK  "default clock"
%token TKDEFAULT_COLON  "default"
%token TKDEASSIGN       "deassign"
%token TKDISABLE        "disable"
%token TKENDSPECIFY     "endspecify"
%token TKFOR            "for"
%token TKINITIAL        "initial"
%token TKSPECIFY        "specify"
%token TKWAIT           "wait"
%token TKFOREVER        "forever"
%token TKREPEAT         "repeat"
%token TKWHILE          "while"
%token TKENDMODULE      "endmodule"
%token TKENDFUNCTION    "endfunction"
%token TKWIRE           "wire"
%token TKTRI            "tri"
%token TKTRI1           "try1"
%token TKWAND           "wand"
%token TKTRIAND         "triand"
%token TKTRI0           "tri0"
%token TKWOR            "wor"
%token TKTRIOR          "trior"
%token TKTRIREG         "trireg"
%token TKREG            "reg"
%token TKINTEGER        "integer"

%token TKINF            "inf"

%token TKDOT            "dot"
%token TKENDPOINT       "endpoint"
%token TKASSIGN         "assign"
%token TKFORCE          "force"
%token TKRELEASE        "release"
%token TKPROPERTY       "property"
%token TKSEQUENCE       "sequence"
%token TKMODULE         "module"
%token TKFUNCTION       "function"
%token TKRESTRICT       "restrict"
%token TKRESTRICT_GUARANTEE "restrict_guarantee"

%token <operator> TKFORALL         "forall"
%token <operator> TKFORANY         "forany"

%token TKASSERT         "assert"
%token TKASSUME         "assume"
%token TKASSUME_GUARANTEE "assume_guarantee"
%token TKCOVER          "cover"
%token TKBOOLEAN        "boolean"
%token TKCASE           "case"
%token TKCASEX          "casex"
%token TKCASEZ          "casez"
%token TKELSE           "else"
%token TKENDCASE        "esac"
%token TKIF             "if"

%token TKNONDET         "$nondet"
%token TKNONDET_VECTOR  "$nondet_vector"
%token TKNONDET_RANGE   "$nondet_range"
%token TKWNONDET        "$wnondet"

%token TKBASE           
%token TKDOTDOT         ".."
%token <operator> TKPIPEMINUSGT    "|->"
%token <operator> TKPIPEEQGT       "|=>"
%token TKIDENTIFIER     "identifier"
%token TKHIERARCHICALID 
%token TKLP             "("
%token TKRP             ")"
%token TKLC             "{"
%token TKRC             "}"
%token TKLB             "["
%token TKRB              "]"
%token TKCOMMA          ","

%token TKDIEZ           "#"

%token TKTRANS          "trans"
%token TKHINT           "hint"
%token TKTEST_PINS      "test_pins"


%token <operator> TKALWAYS         "always"
%token <operator> TKNEVER          "never"
%token <operator> TKEVENTUALLYBANG "eventually!"
%token <operator> TKWITHINBANG     "within!"
%token <operator> TKWITHIN         "within"
%token <operator> TKWITHINBANG_    "within!_"
%token <operator> TKWITHIN_        "within_"
%token <operator> TKWHILENOTBANG   "whilenot!"
%token <operator> TKWHILENOT       "whilenot"
%token <operator> TKWHILENOTBANG_  "whilenot!_"
%token <operator> TKWHILENOT_      "whilenot_"
%token <operator> TKNEXT_EVENT_ABANG "next_event_a!"
%token <operator> TKNEXT_EVENT_A   "next_event_a"
%token <operator> TKNEXT_EVENT_EBANG "next_event_e!"
%token <operator> TKNEXT_EVENT_E   "next_event_e"
%token <operator> TKNEXT_EVENTBANG "next_event!"
%token <operator> TKNEXT_EVENT     "next_event"
%token <operator> TKNEXT_ABANG     "next_a!"
%token <operator> TKNEXT_EBANG     "next_e!"
%token <operator> TKNEXT_A         "next_a"
%token <operator> TKNEXT_E         "next_e"
%token <operator> TKNEXTBANG       "next!"
%token <operator> TKNEXT           "next"
%token <operator> TKNEXTfunc       "next (function)"

%token <operator> TKBEFOREBANG     "before!"
%token <operator> TKBEFORE         "before"
%token <operator> TKBEFOREBANG_    "before!_"
%token <operator> TKBEFORE_        "before_"
%token <operator> TKUNTILBANG      "until!"
%token <operator> TKUNTIL          "until"
%token <operator> TKUNTILBANG_     "until!_"
%token <operator> TKUNTIL_         "until_"

%token TKABORT          "abort"

%token TKROSE           "rose"
%token TKFELL           "fell"
%token TKPREV           "prev"

%token <operator> TKG              "G"
%token <operator> TKXBANG          "X!"
%token <operator> TKX              "X"
%token <operator> TKF              "F"
%token <operator> TKU              "U"
%token <operator> TKW              "W"
%token <operator> TKEG             "EG"
%token <operator> TKEX             "EX"
%token <operator> TKEF             "EF"
%token <operator> TKAG             "AG"
%token <operator> TKAX             "AX"
%token <operator> TKAF             "AF"
%token <operator> TKA              "A"
%token <operator> TKE              "E"
%token <operator> TKIN             "in"
%token <operator> TKUNION          "union"
%token TKQUESTIONMARK   "?"
%token TKCOLON          ":"
%token TKSEMI           ";"
%token <operator> TKPIPEPIPE       "||"
%token <operator> TKAMPERSANDAMPERSAND "&&"
%token <operator> TKMINUSGT        "->"
%token <operator> TKLTMINUSGT      "<->"
%token <operator> TKPIPE           "|"
%token <operator> TKTILDEPIPE      "~|"
%token TKOR             "or"
%token TKPOSEDGE        "posedge"
%token TKNEGEDGE        "negedge"
%token <operator> TKCARET          "^"
%token <operator> TKXOR            "xor"
%token <operator> TKXNOR            "xnor"
%token <operator> TKCARETTILDE     "^~"
%token <operator> TKTILDECARET     "~^"
%token <operator> TKAMPERSAND      "&"
%token <operator> TKTILDEAMPERSAND "~&"
%token <operator> TKEQEQ           "=="
%token <operator> TKBANGEQ         "!="
%token <operator> TKEQEQEQ         "==="
%token <operator> TKBANGEQEQ       "!=="
%token <operator> TKEQ             "="
%token <operator> TKGT             ">"
%token <operator> TKGE             ">="
%token <operator> TKLT             "<"
%token <operator> TKLE             "<="
%token <operator> TKLTLT           "<<"
%token <operator> TKGTGT           ">>"
%token <operator> TKWSELECT        "select"
%token <operator> TKGTGTGT         ">>>"
%token <operator> TKLTLTLT         "<<<"
%token <operator> TKPLUS           "+"
%token <operator> TKMINUS          "-"
%token <operator> TKSPLAT          "*"
%token <operator> TKSLASH          "/"
%token <operator> TKPERCENT        "mod"
%token <operator> TKSPLATSPLAT     "**"

%token <operator> TKBANG           "!"
%token <operator> TKTILDE          "~"
%token <operator> TKLBSPLAT        "[*"
%token <operator> TKLBEQ           "[="
%token <operator> TKLBMINUSGT      "[->"
%token <operator> TKLBPLUSRB       "[+]"


 /* Word operation tokens */
%token <operator> TKWCONCATENATION  "::"
%token <operator> TKBOOL           "bool"
%token <operator> TKWRESIZE        "resize"
%token <operator> TKWSIZEOF        "sizeof"
%token <operator> TKWTOINT         "toint"
%token <operator> TKUWCONST        "uwconst"
%token <operator> TKBITSELECTION   "bit selection"
%token <operator> TKUMINUS         "unary minus"
%token <operator> TKSWCONST        "swconst"
%token <operator> TKWORD1          "word1"
%token <operator> TKSIGNED         "signed"
%token <operator> TKUNSIGNED       "unsigned"
%token <operator> TKEXTEND         "extend"


%token TKSTRUDLE        "@"
%token TKSEREFORGR      "grsequence"
%token TKPSLSPEC        "PSLSPEC"

/* PRECEDENCE and ASSOCIATIVITY */
/* note:  priorities should be checked carefully for consistency with verilog,
   psl documentation
 */

%left TKALWAYS TKNEVER TKG

%left TKMINUSGT TKLTMINUSGT

%left TKEVENTUALLYBANG TKWITHINBANG TKWITHIN TKWITHINBANG_
 TKWITHIN_ TKWHILENOTBANG TKWHILENOT TKWHILENOTBANG_ TKWHILENOT_
 TKNEXT_EVENT_ABANG TKNEXT_EVENT_A TKNEXT_EVENT_EBANG TKNEXT_EVENT_E
 TKNEXT_EVENTBANG TKNEXT_EVENT
 TKNEXT_ABANG TKNEXT_EBANG TKNEXT_A TKNEXT_E
 TKNEXTBANG TKNEXT TKNEXTfunc

%left TKBEFOREBANG TKBEFORE TKBEFOREBANG_ TKBEFORE_
%left TKUNTILBANG TKUNTIL TKUNTILBANG_ TKUNTIL_
%left TKABORT

%left TKROSE TKFELL TKPREV

%left TKXBANG TKX TKF TKU TKW TKEG TKEX TKEF TKAG TKAX TKAF
 TKA TKE
%left TKIN TKUNION
%right TKQUESTIONMARK TKCOLON TKSEMI
%left TKPIPEPIPE
%left TKAMPERSANDAMPERSAND
%left TKPIPE TKTILDEPIPE TKOR
%left TKPOSEDGE TKNEGEDGE
%left TKCARET TKXOR TKXNOR TKCARETTILDE TKTILDECARET
%left TKAMPERSAND TKTILDEAMPERSAND
%left TKEQEQ TKBANGEQ TKEQEQEQ TKBANGEQEQ TKEQ
%left TKGT TKGE TKLT TKLE
%left TKLTLT TKGTGT
%left TKGTGTGT TKLTLTLT
%left TKPLUS TKMINUS 
%left TKSPLAT TKSLASH TKPERCENT TKSPLATSPLAT TKWCONCATENATION

%left TKLB TKRB

%nonassoc TKBANG TKTILDE TKLBSPLAT TKLBEQ TKLBMINUSGT
%nonassoc TKLBPLUSRB
%nonassoc TKSTRUDLE

%type <operator> AndOrOp

%type <operator> ForStar

%type <psl_expr> Property
%type <psl_expr> Additional_Binary_Operators FL_Property OBE_Property
%type <psl_expr> Opt_IndexRange IndexRange ValueSet ValueRange_List ValueRange Range 

%type <psl_expr> Primary Unary_Expression Binary_Expression Conditional_Expression 
%type <psl_expr> Case_Expression case_list
%type <psl_expr> Sequence Sequence_or_Expression Sere

%type <psl_expr> Expression  Mintypmax_Expression word_operators
%type <psl_expr> Concatenation  Expression_List Multiple_Concatenation 

%type <psl_expr> Opt_Count  Count  LowBound  HighBound
%type <psl_expr> boolean number word_number
%type <psl_expr> hierarchical_identifier 
%type <psl_expr> _PslSpec
%type <psl_expr> PslSpec


%start PslSpecSemi

%%

/* Currently there is a 1 shift/reduce conflict and 1 reduce/reduce
  conflict.  The reduce/reduce conflict can be removed by imposing
  that a PSL expr is preceeded by a token like e.g. PSLSPEC. The
  shift/reduce not yet clear how to remove it. It should be not a real
  problem (I believe) */


PslSpecSemi: 
TKPSLSPEC PslSpec TKSEMI TKEOF 
{ psl_parsed_tree = $2.psl_node; YYACCEPT;} 
|
TKSEREFORGR Sequence TKSEMI TKEOF 
{ psl_parsed_tree = $2.psl_node; YYACCEPT;} 
|
PslSpec TKSEMI 
{ psl_parsed_tree = $1.psl_node; Parser_switch_to_smv(); YYACCEPT;} 
|
PslSpec TKEOF
{ 
  psl_yyerror("Unexpected end of file (did you forget a semicolon ';' ?)\n"); 
  Parser_switch_to_smv(); YYABORT; 
}
;

PslSpec : _PslSpec 
        | TKNAME hierarchical_identifier TKEQDEF _PslSpec { psl_property_name = $2.psl_node; $$ = $4;}
        ;

_PslSpec: 
Expression { }
| Expression TKINCONTEXT hierarchical_identifier  
  { $$ = psl_expr_make_context($3, $1); }
| TKSEQUENCE Sequence { $$ = $2; }
; 

Property :
ForStar TKATOM Opt_IndexRange TKIN ValueSet TKCOLON Expression 
{ 
  $$ = psl_expr_make_replicated_property(
     psl_expr_make_replicator($1, psl_expr_make_atom($2), $3, $5), $7); 
  free($2);  /* TKATOM must be freed */
}

| Additional_Binary_Operators
| FL_Property  
| OBE_Property 
;


ForStar :
    TKFORALL 
  | TKFORANY
  ;

Opt_IndexRange :
/* empty */  { $$ = psl_expr_make_empty(); }
| IndexRange 
 ;
 
IndexRange :
TKLB Range TKRB { $$ = $2; }
| TKLB error TKRB { psl_error(); }
;
 
ValueSet :
TKLC ValueRange_List ValueRange TKRC { $$ = psl_expr_make_cons($3, $2); }
| TKLC ValueRange_List error TKRC { psl_error(); }
| TKBOOLEAN { $$ = psl_expr_make_boolean_type(); }
;

ValueRange_List :
ValueRange_List ValueRange TKCOMMA { $$ = psl_expr_make_cons($2, $1); }
| ValueRange_List error TKCOMMA { psl_error(); }
| /* empty */ { $$ = psl_expr_make_empty(); }
;
 
ValueRange :
Expression
| Range
;
 
FL_Property :
/* we should add here Sequence for making it possible to next G and sere, e.g. G {a;b} */
/* Primitive LTL Operators */
TKX Expression                        { PSL_EXPR_MAKE_F2F_OP($$, $2, $1); }
| TKXBANG Expression                  { PSL_EXPR_MAKE_F2F_OP($$, $2, $1); }
| TKF Expression                      { PSL_EXPR_MAKE_F2F_OP($$, $2, $1); }
| TKG Expression                      { PSL_EXPR_MAKE_F2F_OP($$, $2, $1); }
| TKLB Expression TKU Expression TKRB { PSL_EXPR_MAKE_F_F2F_OP($$, $2, $3, $4); } 
| TKLB Expression TKW Expression TKRB { PSL_EXPR_MAKE_F_F2F_OP($$, $2, $3, $4); } 

/* Simple Temporal Operators */

| TKALWAYS Expression            { PSL_EXPR_MAKE_F2F_OP($$, $2, $1); }
| TKNEVER  Expression            { PSL_EXPR_MAKE_F2F_OP($$, $2, $1); }
| TKNEXT   Expression            { PSL_EXPR_MAKE_F2F_OP($$, $2, $1); }
| TKNEXTBANG Expression          { PSL_EXPR_MAKE_F2F_OP($$, $2, $1); }
| TKEVENTUALLYBANG Expression    { PSL_EXPR_MAKE_F2F_OP($$, $2, $1); }


| Expression TKUNTILBANG Expression   { PSL_EXPR_MAKE_F_F2F_OP($$, $1, $2, $3); } 
| Expression TKUNTIL Expression       { PSL_EXPR_MAKE_F_F2F_OP($$, $1, $2, $3); } 
| Expression TKUNTILBANG_ Expression  { PSL_EXPR_MAKE_F_F2F_OP($$, $1, $2, $3); } 
| Expression TKUNTIL_ Expression      { PSL_EXPR_MAKE_F_F2F_OP($$, $1, $2, $3); } 

| Expression TKBEFOREBANG Expression  { PSL_EXPR_MAKE_F_F2F_OP($$, $1, $2, $3); } 
| Expression TKBEFORE Expression      { PSL_EXPR_MAKE_F_F2F_OP($$, $1, $2, $3); } 
| Expression TKBEFOREBANG_ Expression { PSL_EXPR_MAKE_F_F2F_OP($$, $1, $2, $3); } 
| Expression TKBEFORE_ Expression     { PSL_EXPR_MAKE_F_F2F_OP($$, $1, $2, $3); } 

 /* Extended Next (Event) Operators */
| TKX TKLB Expression TKRB TKLP Expression TKRP
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN($$, $1, $6, $3); }

| TKXBANG TKLB Expression TKRB TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN($$, $1, $6, $3); }

| TKNEXT TKLB Expression TKRB TKLP Expression TKRP
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN($$, $1, $6, $3); }

| TKNEXTBANG TKLB Expression TKRB TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN($$, $1, $6, $3); }


| TKNEXT_A TKLB Range TKRB TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN($$, $1, $6, $3); }

| TKNEXT_ABANG TKLB Range TKRB TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN($$, $1, $6, $3); }

| TKNEXT_E TKLB Range TKRB TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN($$, $1, $6, $3); }

| TKNEXT_EBANG TKLB Range TKRB TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN($$, $1, $6, $3); }


| TKNEXT_EVENTBANG TKLP Expression TKRP TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_BOOL($$, $1, $6, $3); }

| TKNEXT_EVENT TKLP Expression TKRP TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_BOOL($$, $1, $6, $3); }

| TKNEXT_EVENTBANG TKLP Expression TKRP TKLB Expression TKRB TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN_BOOL($$, $1, $9, $6, $3); }

| TKNEXT_EVENT TKLP Expression TKRP TKLB Expression TKRB TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN_BOOL($$, $1, $9, $6, $3); }

| TKNEXT_EVENT_ABANG TKLP Expression TKRP TKLB Range TKRB TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN_BOOL($$, $1, $9, $6, $3); }

| TKNEXT_EVENT_A TKLP Expression TKRP TKLB Range TKRB TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN_BOOL($$, $1, $9, $6, $3); }

| TKNEXT_EVENT_EBANG TKLP Expression TKRP TKLB Range TKRB TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN_BOOL($$, $1, $9, $6, $3); }

| TKNEXT_EVENT_E TKLP Expression TKRP TKLB Range TKRB TKLP Expression TKRP 
{ PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN_BOOL($$, $1, $9, $6, $3); }


 /* Operators on Sequence */
| Sequence TKLP Expression TKRP 
{ $$ = psl_expr_make_suffix_implication_strong($1, TKPIPEMINUSGT, $3); } 
| Sequence TKPIPEMINUSGT Sequence TKBANG  
{ $$ = psl_expr_make_suffix_implication_strong($1, $2, $3); }
| Sequence TKPIPEMINUSGT Sequence 
{ $$ = psl_expr_make_suffix_implication_weak($1, $2, $3); }
| Sequence TKPIPEEQGT Sequence TKBANG 
{ $$ = psl_expr_make_suffix_implication_strong($1, $2, $3); }
| Sequence TKPIPEEQGT Sequence 
{ $$ = psl_expr_make_suffix_implication_weak($1, $2, $3); }

| TKALWAYS Sequence { PSL_EXPR_MAKE_F2F_OP($$, $2, $1); }
| TKNEVER  Sequence { PSL_EXPR_MAKE_F2F_OP($$, $2, $1); }
| TKEVENTUALLYBANG Sequence { PSL_EXPR_MAKE_F2F_OP($$, $2, $1); }

| TKWITHINBANG TKLP Sequence_or_Expression TKCOMMA Expression TKRP Sequence 
{ $$ = psl_expr_make_within($1, $3, $5, $7); }
| TKWITHIN TKLP Sequence_or_Expression TKCOMMA Expression TKRP Sequence 
{ $$ = psl_expr_make_within($1, $3, $5, $7); }
| TKWITHINBANG_ TKLP Sequence_or_Expression TKCOMMA Expression TKRP Sequence 
{ $$ = psl_expr_make_within($1, $3, $5, $7); }
| TKWITHIN_ TKLP Sequence_or_Expression TKCOMMA Expression TKRP Sequence 
{ $$ = psl_expr_make_within($1, $3, $5, $7); }

| TKWHILENOTBANG TKLP Expression TKRP Sequence 
{ $$ = psl_expr_make_whilenot($1, $3, $5); }
| TKWHILENOT TKLP Expression TKRP Sequence 
{ $$ = psl_expr_make_whilenot($1, $3, $5); }
| TKWHILENOTBANG_ TKLP Expression TKRP Sequence 
{ $$ = psl_expr_make_whilenot($1, $3, $5); }
| TKWHILENOT_ TKLP Expression TKRP Sequence 
{ $$ = psl_expr_make_whilenot($1, $3, $5); }
;								   	    

Sequence_or_Expression :
Sequence
| Expression
;


Additional_Binary_Operators : 
Expression TKABORT Expression 
{ $$ = psl_expr_make_abort($1, $3); }

| Expression TKLTMINUSGT Expression { PSL_EXPR_MAKE_BW_BW2BW_OP($$, $1, $2, $3); }
| Expression TKMINUSGT Expression   { PSL_EXPR_MAKE_BW_BW2BW_OP($$, $1, $2, $3); }
;

Sequence :
TKLC Sere TKRC { $$ = $2; }
;

Sere :
Expression { $$ = psl_expr_make_sere($1); }
| Sequence { $$ = psl_expr_make_sere($1); }

/* Composition Operators */
| Sere TKSEMI Sere          { $$ = psl_expr_make_sere_concat($1, $3); }
| Sequence TKCOLON Sequence { $$ = psl_expr_make_sere_fusion($1, $3); }
| Sequence AndOrOp Sequence { $$ = psl_expr_make_sere_compound_binary_op($1, $2, $3); }

/* RegExpr Qualifiers */
| Sere TKLBSPLAT Opt_Count TKRB { $$ = psl_expr_make_repeated_sere($2, $1, $3); }
| TKLBSPLAT Opt_Count TKRB      { $$ = psl_expr_make_repeated_sere($1, 
					   psl_expr_make_empty(), $2); }
| Sere TKLBPLUSRB               { $$ = psl_expr_make_repeated_sere($2, 
					   $1, psl_expr_make_empty()); }
| TKLBPLUSRB                     { $$ = psl_expr_make_repeated_sere($1, 
					   psl_expr_make_empty(), 
				           psl_expr_make_empty()); }

 | Expression TKLBEQ Count TKRB  { $$ = psl_expr_make_repeated_sere($2, $1, $3); }
 | Expression TKLBMINUSGT Opt_Count TKRB 
                                 { $$ = psl_expr_make_repeated_sere($2, $1, $3); }
 ;

AndOrOp :
   TKAMPERSANDAMPERSAND 
 | TKAMPERSAND 
 | TKPIPE 
 ;
 
Opt_Count :
/* empty */ { $$ = psl_expr_make_empty(); }
| Count     
;
 
Count :
TKNUMBER        { $$ = psl_expr_make_number($1); }
| TKBASENUMBER  { $$ = psl_expr_make_base_number($1); }
| Range         
| hierarchical_identifier 
;
 
Range :
   LowBound TKCOLON HighBound  { $$ = psl_expr_make_range($1, $3); }
 ;
 
LowBound :
TKNUMBER        { $$ = psl_expr_make_number($1); }
| TKMINUS TKNUMBER { $$ = psl_expr_make_number(-$2); }
| TKBASENUMBER  { $$ = psl_expr_make_base_number($1); }
| hierarchical_identifier 
;
 
HighBound :
TKNUMBER        { $$ = psl_expr_make_number($1); }
| TKMINUS TKNUMBER { $$ = psl_expr_make_number(-$2); }
| TKBASENUMBER  { $$ = psl_expr_make_base_number($1); }
| TKINF         { $$ = psl_expr_make_inf(); }
| hierarchical_identifier 
;
 
OBE_Property :
/* Universal Operators */
TKAX Expression { $$ = psl_expr_make_obe_unary($1, $2); }
| TKAG Expression { $$ = psl_expr_make_obe_unary($1, $2); }
| TKAF Expression { $$ = psl_expr_make_obe_unary($1, $2); }
| TKA TKLB Expression TKU Expression TKRB { $$ = psl_expr_make_obe_binary($3, $1, $5); }

/* Existential Operators */
| TKEX Expression { $$ = psl_expr_make_obe_unary($1, $2); }
| TKEG Expression { $$ = psl_expr_make_obe_unary($1, $2); }
| TKEF Expression { $$ = psl_expr_make_obe_unary($1, $2); }
| TKE TKLB Expression TKU Expression TKRB { $$ = psl_expr_make_obe_binary($3, $1, $5); }
;								   	    

/* SMV Rules */
Expression :
   Primary
 | Unary_Expression
 | Binary_Expression
 | Conditional_Expression
 | Case_Expression
 | Property
 ;

Unary_Expression :
TKPLUS Primary     { PSL_EXPR_MAKE_NW2NW_OP($$, $2, $1); }
| TKMINUS Primary  { PSL_EXPR_MAKE_NW2NW_OP($$, $2, $1); }
| TKBANG Primary   { PSL_EXPR_MAKE_BW2BW_OP($$, $2, $1); }
| TKBOOL TKLP Expression TKRP  { PSL_EXPR_MAKE_W2B_OP($$, $3, $1); }
| TKWORD1 TKLP Expression TKRP { PSL_EXPR_MAKE_B2W_OP($$, $3, $1);  }
| TKSIGNED TKLP Expression TKRP { PSL_EXPR_MAKE_W2W_OP($$, $3, $1); }
| TKUNSIGNED TKLP Expression TKRP { PSL_EXPR_MAKE_W2W_OP($$, $3, $1); }
| TKEXTEND TKLP Expression TKCOMMA Primary TKRP 
                               { PSL_EXPR_MAKE_W_N2W_OP($$, $3, $1, $5); }
| TKWRESIZE TKLP Expression TKCOMMA Primary TKRP 
                               { PSL_EXPR_MAKE_W_N2W_OP($$, $3, $1, $5); }
;

Binary_Expression :
Expression TKPLUS Expression      { PSL_EXPR_MAKE_NW_NW2NW_OP($$, $1, $2, $3); }
| Expression TKUNION Expression   { PSL_EXPR_MAKE_T_T2T_OP($$, $1, $2, $3); }
| Expression TKIN Expression      { PSL_EXPR_MAKE_NB_NB2B_OP($$, $1, $2, $3); }
| Expression TKMINUS Expression   { PSL_EXPR_MAKE_NW_NW2NW_OP($$, $1, $2, $3); }
| Expression TKSPLAT Expression   { PSL_EXPR_MAKE_NW_NW2NW_OP($$, $1, $2, $3); }
| Expression TKSLASH Expression   { PSL_EXPR_MAKE_NW_NW2NW_OP($$, $1, $2, $3); }
| Expression TKPERCENT Expression { PSL_EXPR_MAKE_NW_NW2NW_OP($$, $1, $2, $3); }
| Expression TKEQ Expression      { PSL_EXPR_MAKE_NBW_NBW2B_OP($$, $1, $2, $3); }
| Expression TKEQEQ Expression    { PSL_EXPR_MAKE_NBW_NBW2B_OP($$, $1, $2, $3); }
| Expression TKBANGEQ Expression  { PSL_EXPR_MAKE_NBW_NBW2B_OP($$, $1, $2, $3); }

| Expression TKLT Expression       { PSL_EXPR_MAKE_NW_NW2B_OP($$, $1, $2, $3); }
| Expression TKLE Expression       { PSL_EXPR_MAKE_NW_NW2B_OP($$, $1, $2, $3); }
| Expression TKGT Expression       { PSL_EXPR_MAKE_NW_NW2B_OP($$, $1, $2, $3); }
| Expression TKGE Expression       { PSL_EXPR_MAKE_NW_NW2B_OP($$, $1, $2, $3); }

| Expression TKAMPERSAND Expression { PSL_EXPR_MAKE_BW_BW2BW_OP($$, $1, $2, $3); } 
| Expression TKAMPERSANDAMPERSAND Expression { PSL_EXPR_MAKE_B_B2B_OP($$, $1, TKAMPERSAND, $3); } 

| Expression TKPIPE Expression      { PSL_EXPR_MAKE_BW_BW2BW_OP($$, $1, $2, $3); } 
| Expression TKPIPEPIPE Expression  { PSL_EXPR_MAKE_B_B2B_OP($$, $1, TKPIPE, $3); } 

| Expression TKCARET Expression     { PSL_EXPR_MAKE_B_B2B_OP($$, $1, $2, $3); } 
| Expression TKXOR Expression       { PSL_EXPR_MAKE_BW_BW2BW_OP($$, $1, $2, $3); } 
| Expression TKXNOR Expression       { PSL_EXPR_MAKE_BW_BW2BW_OP($$, $1, $2, $3); } 
| Expression TKLTLT Expression       { PSL_EXPR_MAKE_W_NW2W_OP($$, $1, $2, $3); } 
| Expression TKGTGT Expression       { PSL_EXPR_MAKE_W_NW2W_OP($$, $1, $2, $3); } 

/* PSL BIT SELECTION  */
| TKWSELECT TKLP Expression TKCOMMA Expression TKCOMMA Expression TKRP
                          { $$ = psl_expr_make_bit_selection($3, $5, $7); }

| Expression TKWCONCATENATION Expression  
                          { $$ = psl_expr_make_word_concatenation($1, $3); }
;

Conditional_Expression :
Expression TKQUESTIONMARK Expression TKCOLON Expression 
{ $$ = psl_expr_make_ite($1, $3, $5); }
;

Case_Expression :
TKCASE case_list TKENDCASE { $$ = $2; }
;

case_list :
/* empty */    { $$ = psl_expr_make_failure("case conditions are not exhaustive", 
					    FAILURE_CASE_NOT_EXHAUSTIVE); }
| Expression TKCOLON Expression TKSEMI case_list { $$ = psl_expr_make_case($1, $3, $5); }
;

boolean :
  TKTRUE     { $$ = psl_expr_make_boolean_value(1); }
| TKFALSE    { $$ = psl_expr_make_boolean_value(0); }
;

number :
TKBASENUMBER   { $$ = psl_expr_make_base_number($1); }
| TKNUMBER     { $$ = psl_expr_make_number($1); }
| TKREALNUMBER { $$ = psl_expr_make_real_number($1); }
;

word_number:
TKUWCONST TKLP Expression TKCOMMA Expression TKRP
                    { PSL_EXPR_MAKE_N_N2W_OP($$, $3, $1, $5); }
| TKSWCONST TKLP Expression TKCOMMA Expression TKRP 
                    { PSL_EXPR_MAKE_N_N2W_OP($$, $3, $1, $5); }
| TKWORDNUMBER { $$ = psl_expr_make_word_number($1); }
;

word_operators :
TKWSIZEOF TKLP Expression TKRP { PSL_EXPR_MAKE_W2N_OP($$, $3, $1); }
| TKWTOINT  TKLP Expression TKRP { PSL_EXPR_MAKE_W2N_OP($$, $3, $1);  }
;

Primary :
number
| boolean
| word_number
| word_operators
| hierarchical_identifier 
| Concatenation 
| Multiple_Concatenation
| TKLP Mintypmax_Expression TKRP { $$ = $2; }
;

hierarchical_identifier :
TKATOM   { $$ = psl_expr_make_atom($1); free($1); }

| hierarchical_identifier TKDOT TKATOM
  { $$ = psl_expr_make_id($1, psl_expr_make_atom($3)); free($3); }

| hierarchical_identifier TKDOT TKNUMBER
  { $$ = psl_expr_make_id($1, psl_expr_make_number($3)); }

| hierarchical_identifier TKLB Expression TKRB
  { $$ = psl_expr_make_id_array($1, $3); }
;

Mintypmax_Expression :
Expression
;


Concatenation :
TKLC Expression_List TKRC { $$ = psl_expr_make_concatenation($2); }
 ;

/* This will be built in reversed order */
Expression_List :
  Expression_List TKCOMMA Expression { $$ = psl_expr_make_cons($3, $1); }
| Expression  { $$ = psl_expr_make_cons($1, psl_expr_make_empty()); }
;

Multiple_Concatenation :
TKLC Expression TKLC Expression_List TKRC TKRC 
{ $$ = psl_expr_make_multiple_concatenation($2, $4); }
;


%%

#include <stdarg.h>
#include <stdio.h>

extern FILE* nusmv_stderr;
extern int yylineno;
extern int psl_yylineno;

void psl_yyerror(char* s, ...)
{
  va_list args;

  va_start(args, s);
  fprintf(nusmv_stderr, "PSL parse error line %d: ", 
	  yylineno+psl_yylineno-1);
  vfprintf(nusmv_stderr, s, args);
  fprintf(nusmv_stderr, "\n");
  va_end(args);
}

int psl_error()
{
  fprintf(nusmv_stderr, "PSL parse error line %d: \n", 
	  yylineno+psl_yylineno-1);
  return 1;
}
