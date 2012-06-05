
#ifndef __ORD_INT_H__
#define __ORD_INT_H__

#include "ParserOrd.h"
#include "utils/utils.h"
#include "opt/opt.h"


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct yy_buffer_state* YY_BUFFER_STATE;


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;
extern FILE* nusmv_stdin;

EXTERN int parser_ord_lineno;
EXTERN FILE* parser_ord_in;


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void parser_ord_set_global_parser ARGS((ParserOrd_ptr parser));
EXTERN void parser_ord_reset_global_parser ARGS((ParserOrd_ptr parser));
EXTERN ParserOrd_ptr parser_ord_get_global_parser ARGS((void));


/* from generated code: */
EXTERN int parser_ord_lex ARGS((void));
EXTERN int parser_ord_parse ARGS((void));
EXTERN void parser_ord_restart ARGS((FILE* input_file));

EXTERN void 
parser_ord__switch_to_buffer ARGS((YY_BUFFER_STATE new_buffer));

EXTERN YY_BUFFER_STATE 
parser_ord__create_buffer ARGS((FILE* file, int size));

EXTERN void parser_ord__delete_buffer ARGS((YY_BUFFER_STATE buf));

EXTERN YY_BUFFER_STATE 
parser_ord__scan_string ARGS((const char* str));


#endif /* __ORD_INT_H__ */
