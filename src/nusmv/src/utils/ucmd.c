/**CFile***********************************************************************

  FileName    [ucmd.c]

  PackageName [utils]

  Synopsis    [Contains useful function and structure to be used when
  processing command line strings.]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2.
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
# include "nusmv-config.h"
#endif

#include "portability.h"
#include "utils/ucmd.h"
#include "utils/error.h"
/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Searches for a symbol in the given string, and
  and substitutes all its occurences with the specified element, using the
  given format.]

  Description        [The first parameter <I>subst</I> contains information
  about the symbol to be checked and about the element which substitutes every
  occurence of the symbol, and the format (as in printf) used to convert the
  element in a string. The element has a type (integer, string, float, etc.)
  and a statically assigned value. <BR>
  The second parameter <I>string</I> contains the string to be searched for,
  and the string finally returned too. So it is *very important* you supply
  a buffer large enought to contain the larger string between source and
  destination strings. Use the third parameter to fix the maximum buffer
  length. <BR><BR>
  The element can be built with a 2-passes procedure.
  The first pass consists in constructing the static instance of the element.
  Use the SYMBOL_CREATE macro to build it, and assign the result to a
  <I>SubstString</I> type variable.
  Then assign the substitution value to the created instance using the macro
  SYMBOL_ASSIGN. <BR>
  <I>Example of usage:</I><BR>
  <PRE>
  {
    char szBuffer[256];
    SubstString sb = SYMBOL_CREATE("$D");

    SYMBOL_ASSIGN(sb, integer, "%d", 10);

    strncpy(szBuffer, "Every symbol $D will be substituted with $D",
            sizeof(szBuffer));

    apply_string_macro_expansion(&sb, szBuffer, sizeof(szBuffer));
  }
  </PRE>
  ]

  SideEffects        [The given string will change]

  SeeAlso            [SYMBOL_CREATE, SYMBOL_ASSIGN, SubstString]

******************************************************************************/
void apply_string_macro_expansion(const SubstString* const subst,
				  char* string, size_t buf_len)
{
  static char szSubst[256];
  char* szSymbolPos = NULL;
  size_t string_len = strlen(string);
  int c = 0;

  nusmv_assert(string_len < buf_len); /* is buffer large enought? */

  /* substitutes all symbols in input string, forall symbols: */
  szSymbolPos = strstr(string, subst->symbol);
  while(szSymbolPos != NULL) {
    /* applies format: */
    switch(subst->value.type) {
    case sv_string:
      c = snprintf(szSubst, sizeof(szSubst), subst->format,
                   subst->value.assign.string);
      break;

    case sv_integer:
      c = snprintf(szSubst, sizeof(szSubst), subst->format,
                   subst->value.assign.integer);
      break;

    case sv_floating:
      c = snprintf(szSubst, sizeof(szSubst), subst->format,
                   subst->value.assign.floating);
      break;

    case sv_undef:
      error_unreachable_code(); /* did you use the SYMBOL_ASSIGN macro? */
      break;

    case sv_pointer:
    default:
      c = snprintf(szSubst, sizeof(szSubst), subst->format,
                   subst->value.assign.pointer);
      break;
    } /* switch end */

    SNPRINTF_CHECK(c, sizeof(szSubst));

    {
      /* now substitutes the symbol with the string: */
      size_t symbol_len = strlen(subst->symbol);
      size_t subst_len  = strlen(szSubst);
      char*  moveTo   = szSymbolPos+subst_len;
      char*  moveFrom = szSymbolPos+symbol_len;

      /* prepare space for subst: */
      memmove( moveTo, moveFrom, string_len - (moveFrom - string) + 1 );
      string_len += (subst_len-symbol_len);

      /* substitutes the symbol :*/
      memcpy(szSymbolPos, szSubst, subst_len);

      /* searches for the following reference to this symbol: */
      szSymbolPos = strstr(szSymbolPos + subst_len, subst->symbol);
    }
  } /* end of search for the single symbol */
}


/**Function********************************************************************

  Synopsis           [Converts a given string representing a number (base 10)
  to an integer with the same value]

  Description        [Returns zero (0) if conversion is carried out
  successfully, otherwise returns 1]

  SideEffects        ['value' parameter might change]

  SeeAlso            []

******************************************************************************/
int util_str2int(const char* str, int* value)
{
  int res = 1;
  char* error_pos = NIL(char);
  long dummy;

  dummy = strtol(str, &error_pos, 10);

  if ( (str != NIL(char)) && (error_pos == NULL ||
                              error_pos == (str+strlen(str)) ||
                              *error_pos == ' ' ||
                              *error_pos == '\t') ) {

    /* conversion has been succesfully carried out */
    res = 0;

    /* as an additional check, verify that the value can fit into an
       ordinary int */
    if ((dummy < INT_MIN) && (INT_MAX < dummy)) {
      res = 1;
    }

    *value = (int)dummy;
  }

  return res;
}


/**Function********************************************************************

  Synopsis    [An abstraction over BSD strtol for integers]

  Description [Parses an integer value from a string, performing
               error-checking on the parsed value. This function can
               be used to parse incrementally a complex string made of
               numbers and separators.

               Returns 0 iff no error was detected.

               Remarks:

               * Empty strings are allowed as a corner case. They are
                 interpreted as 0.]

  SideEffects [*endptr points to the next character in string, *out
               contains the integer value corresponding to the parsed
               string.]

******************************************************************************/
int util_str2int_incr(const char* str, char **endptr, int* out)
{
  long tmp; /* required by strtol */
  int res = 0; /* no error */

  errno = 0;    /* To distinguish success/failure after call */
  tmp = strtol(str, endptr, 10);

  /* Check for various possible errors */

  if ((errno == ERANGE && (tmp == LONG_MAX || tmp == LONG_MIN))
      || (errno != 0 && tmp == 0)) {
    res = 1; goto leave;
  }

  /* as an additional check, verify that the value can fit into an
     ordinary int */
  if ((tmp < INT_MIN) && (INT_MAX < tmp)) {
    res = 1; goto leave;
  }

  /* here tmp can be safely cast to int */
  (*out) = (int)(tmp);

 leave:
  return res;
} /* util_str2int */



/**Function********************************************************************

  Synopsis           [Checks if given string is NULL, "", or the converted
  string of NULL]

  Description        [Returns 1 if the string is equal to "", NULL or
  equal to the converted string of NULL (as sprintf does).
  Otherwise returns 0.
  ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int util_is_string_null(const char* string)
{
  int ret = 0;
  if (string == (char*)NULL) ret = 1;
  else {
    if (strcmp(string, "") == 0) ret = 1;
    else {
      if (strcmp(string, OPT_USER_POV_NULL_STRING) == 0) ret = 1;
    }
  }

  return ret;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/



