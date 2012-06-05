
#ifndef __IDLIST_INT_H__
#define __IDLIST_INT_H__

#include "ParserIdList.h"
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

EXTERN int parser_idlist_lineno;
EXTERN FILE* parser_idlist_in;


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void parser_idlist_set_global_parser ARGS((ParserIdList_ptr parser));
EXTERN void parser_idlist_reset_global_parser ARGS((ParserIdList_ptr parser));
EXTERN ParserIdList_ptr parser_idlist_get_global_parser ARGS((void));


/* from generated code: */
EXTERN int parser_idlist_lex ARGS((void));
EXTERN int parser_idlist_parse ARGS((void));
EXTERN void parser_idlist_restart ARGS((FILE* input_file));

EXTERN void 
parser_idlist__switch_to_buffer ARGS((YY_BUFFER_STATE new_buffer));

EXTERN YY_BUFFER_STATE 
parser_idlist__create_buffer ARGS((FILE* file, int size));

EXTERN void parser_idlist__delete_buffer ARGS((YY_BUFFER_STATE buf));

EXTERN YY_BUFFER_STATE 
parser_idlist__scan_string ARGS((const char* str));


#endif /* __IDLIST_INT_H__ */
