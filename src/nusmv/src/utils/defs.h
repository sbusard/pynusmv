/**CHeaderFile*****************************************************************

  FileName    [defs.h]

  PackageName [utils]

  Synopsis    [Some low-level definitions]

  Description [Some low-level definitions]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 2007 by FBK-irst. 

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

  Revision    [$Id: defs.h,v 1.1.2.4 2009-10-19 14:39:43 nusmv Exp $]

******************************************************************************/

#ifndef __UTILS_DEFS_H__
#define __UTILS_DEFS_H__

#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#if NUSMV_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <assert.h>
#include "util.h"


/* These are potential duplicates. */
#ifndef EXTERN
#   ifdef __cplusplus
#	define EXTERN extern "C"
#   else
#	define EXTERN extern
#   endif
#endif
#ifndef ARGS
#   if defined(__STDC__) || defined(__cplusplus)
#	define ARGS(protos)	protos		/* ANSI C */
#   else /* !(__STDC__ || __cplusplus) */
#	define ARGS(protos)	()		/* K&R C */
#   endif /* !(__STDC__ || __cplusplus) */
#endif
#ifndef NORETURN
#   if defined __GNUC__
#       define NORETURN __attribute__ ((__noreturn__))
#   else
#       define NORETURN
#   endif
#endif

#define nusmv_assert(expr) \
    assert(expr)

#define NIL_PTR(ptr_type)       \
    ((ptr_type) NULL)


/* use whenever you assign an integer to or from a pointer */
typedef util_ptrint  nusmv_ptrint;
typedef util_ptruint nusmv_ptruint;



/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#ifndef max
#define max(_a_, _b_) ((_a_ < _b_) ? _b_ : _a_)
#endif

#ifndef min
#define min(_a_, _b_) ((_a_ < _b_) ? _a_ : _b_)
#endif


/**Macro**********************************************************************
  Synopsis     [Casts the given pointer (address) to an int]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define PTR_TO_INT(x) \
         ((int) (nusmv_ptrint) (x))

/**Macro**********************************************************************
  Synopsis     [Casts the given int to the given pointer type]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define PTR_FROM_INT(ptr, x) \
         ((ptr) (nusmv_ptrint) (x))


/**Macro**********************************************************************
  Synopsis     [Casts the given int to void *]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define VOIDPTR_FROM_INT(x) \
         ((void *) (nusmv_ptrint) (x))


/* These are used to "stringize" x after its macro-evaluation: */
/* WARNING: these macro are not safe. If the macro x contains sequences
   corresponding to an element of the build triplet (architecture-vendor-os)
   they will be expanded to 1, because they are internal macro of
   cpp. Examples os such string are:
   i386, linux, unix.
   See issue 2838. */

#define MACRO_STRINGIZE_2nd_LEVEL(x) \
   #x

#define MACRO_STRINGIZE(x) \
    MACRO_STRINGIZE_2nd_LEVEL(x)

/**Macro**********************************************************************
  Synopsis     [This is a portable prefix to print size_t valus with printf]
  Description  [Use this prefix when printinf size_t values with printf.
  Warning! This macro is not prefixed with '%']
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#if !NUSMV_HAVE_INTTYPES_H
# if (defined __MINGW32__) || (defined __CYGWIN__)
#  ifdef _WIN64
#   define PRIuPTR "I64u"
#   define PRIdPTR "I64d"
#  else
#   define PRIuPTR "u"
#   define PRIdPTR "d"
#  endif
# else 
#  if __WORDSIZE == 64
#   define PRIuPTR "lu"
#   define PRIdPTR "ld"
#  else
#   define PRIuPTR "u"
#   define PRIdPTR "d"
#  endif
# endif
#else
# include <inttypes.h>
# endif /* NUSMV_HAVE_INTTYPES_H */

#if (defined __MINGW32__) || (defined __CYGWIN__)
#define LLU "I64u"
#define LLO "I64o"
#define LLX "I64X"
#else
#define LLU "llu"
#define LLO "llo"
#define LLX "llX"
#endif


#if NUSMV_HAVE_SRANDOM
#  if NUSMV_HAVE_GETPID
#    define utils_random_set_seed() \
       srandom((unsigned int)getpid())
#  else
#include <time.h>
#    define utils_random_set_seed() \
       srandom((unsigned int)time(NULL))
#  endif
#else
#  if NUSMV_HAVE_GETPID
#    define utils_random_set_seed() \
       srand((unsigned int)getpid())
#  else
#include <time.h>
#    define utils_random_set_seed() \
       srand((unsigned int)time(NULL))
#  endif
#endif

#if NUSMV_HAVE_RANDOM
#  define utils_random() \
    random()
#else
#  define utils_random() \
    rand()
#endif


#if NUSMV_HAVE_STDBOOL_H
#include <stdbool.h>
typedef bool boolean;
#else
#ifdef __cplusplus
typedef bool boolean;
#else
typedef enum {false=0, true=1} boolean;
#endif
#endif

typedef enum Outcome_TAG
{
  OUTCOME_GENERIC_ERROR, 
  OUTCOME_PARSER_ERROR,
  OUTCOME_SYNTAX_ERROR, 
  OUTCOME_FILE_ERROR, 
  OUTCOME_SUCCESS_REQUIRED_HELP, 
  OUTCOME_SUCCESS
} Outcome;


#endif /* __UTILS_DEFS_H__ */
