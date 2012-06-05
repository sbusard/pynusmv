/**CSourceFile*****************************************************************

  FileName    [WordNumber.c]

  PackageName [utils]

  Synopsis    []

  Description []

  SeeAlso     [WordNumber.h]

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``enc.utils'' package of NuSMV version 2.
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

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "WordNumber.h"

#include "utils/assoc.h"
#include "node/node.h"
#include "utils/ustring.h"
#include "parser/symbols.h"
#include "opt/opt.h"
#include "utils/portability.h" /* for LLONG_MAX, errno */
#include "utils/error.h"

#include <stdlib.h> /* for strtol */

static char rcsid[] UTIL_UNUSED = "$Id: WordNumber.c,v 1.1.2.1.4.19 2010-01-21 19:54:29 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Types definition                                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [WordNumber struct.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
typedef struct WordNumber_TAG
{
  WordNumberValue value; /* Words are unsigned */
  int width;
  string_ptr parsedString;
} WordNumber;


/**Struct**********************************************************************

  Synopsis           [The memory manager of the WordNumber class.]

  Description        [the manager looks after all the created WordNumbers and
  destroys them during deinitialisation.
  There are two kinds of numbers: one kind contains just value and width,
  and the other one contains also a string obtained obtained during parsing.

  The former kind of numbers are stored in a hash table is used (and
  the memory is shared by the numbers). The later kind is stored in
  a list (and memory is not shared).
  ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
typedef struct MemoryManager_TAG
{
  /*
    - For internally generated word constants

        node_ptr(width, value(half_1), value(half_2)) -> -> WordNumber_ptr

    - For parsed word constants, to allow printing with the same
      format as parsed

          string_ptr("0bN_....") -> -> WordNumber_ptr
  */
  hash_ptr hashTable;
} MemoryManager;

/*---------------------------------------------------------------------------*/
/* Macro definition                                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable definition                                                       */
/*---------------------------------------------------------------------------*/
/* for print function */


/**Variable*********************************************************************

  Synopsis           [The instance of a global memory manager of the WordNumber
  class.]

  Description        []

  SideEffects        []

  SeeAlso            [WordNumberManager_TAG ]

******************************************************************************/
static MemoryManager memoryManager;


/**Variable*********************************************************************

  Synopsis           [The maximal width of a WordConstant, i.e.
  a maximal number of bits supported by the class ]

  Description        [This value is implementation-dependent.
  The constant is initialised in the WordNumber_init]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int wordNumberMaxWidth;

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static WordNumber_ptr word_number_create ARGS((WordNumberValue value,
                                               int width,
                                               char* parsedString));
static void word_number_destroy ARGS((WordNumber_ptr));

static WordNumberValue
word_number_to_signed_c_value ARGS((const WordNumber_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [initialiser of the class]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void WordNumber_init(void)
{
  /* compute the maximal size of Word constants we can handle */
  if (0 == wordNumberMaxWidth) {/* the constant has not been initialized*/
    int i = 1;
    WordNumberValue ull = 1;
    while ((ull << 1) > ull) {
      ull = ull << 1;
      ++i;
    }
    wordNumberMaxWidth = i; /* max number of bits in "unsigned long long" */

    /* two int must be enough to hold one Word constant (this is
       used in caching, see word_number_create). So, if required,
       decrease the number of bits to make it possible.
    */
    if (sizeof(int) * 2 * CHAR_BIT < wordNumberMaxWidth) {
      wordNumberMaxWidth = sizeof(int) * 2 * CHAR_BIT;
    }
  }

  /* inits the manager */
  memoryManager.hashTable = new_assoc();
  nusmv_assert((hash_ptr)NULL != memoryManager.hashTable);
}


/* hash table cleaner for the  WordNumber_quit */
static enum st_retval word_number_hashTableCleaner(char* key,
                                                   char* record, char* arg)
{
  WordNumber_ptr number = WORD_NUMBER(record);
  nusmv_assert(WORD_NUMBER(NULL) != number);
  word_number_destroy(number);
  return ST_DELETE;
}

/**Function********************************************************************

  Synopsis           [deinitialiser of the class]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void WordNumber_quit(void)
{
  /* the manager should have been initialized already */
  nusmv_assert((hash_ptr) NULL != memoryManager.hashTable);
  clear_assoc_and_free_entries(memoryManager.hashTable,
                               word_number_hashTableCleaner);
  free_assoc(memoryManager.hashTable);
  memoryManager.hashTable = (hash_ptr) NULL;
}


/**Function********************************************************************

  Synopsis           [The functions returns the maximal width a Word constant
  can have. This is implemenatation-dependent limit]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int WordNumber_max_width()
{
  return wordNumberMaxWidth;
}

/**Function********************************************************************

  Synopsis           [Constructs a Word number WordNumber_ptr from the string
  representation]

  Description        [The string and base should be proper for standard
  "strtoull" function.  The base can be 2, 8 or 16.  In the case of
  any problem NULL is returned.

  Note: base 10 is not allowed, because it does not provide enough info
  about the width of the Word number.

  NOTE: Memory sharing is used, i.e. given a string with the same
  value of WordNumber this constructor will return the same pointer
  (this is important for node_ptr hashing)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_from_string(char* str, int base)
{
  WordNumberValue value;
  int width;
  char* tmpStr;

#if NUSMV_HAVE_ERRNO_H
  errno = 0;
#endif

  value = strtoull(str, &tmpStr, base);

  /* an error happened */
  if (NULL == str ||

#if NUSMV_HAVE_ERRNO_H
      /* proper errno usage from the strtoull manpage */
      (((ERANGE == errno && (LLONG_MAX == value)) ||
        (0 != errno && 0 == value))) ||
#endif
      '\0' != *tmpStr) {

    return WORD_NUMBER(NULL);
  }

  /* calculate the width */
  width = tmpStr - str;
  switch (base) {
  case 2:  /* nothing */ break;
  case 8:  width *= 3; break;
  case 16: width *= 4; break;
  default: error_unreachable_code(); /* only 2,8 and 16 bits base are allowed */
  }

  return word_number_create(value, width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [Constructs a Word number WordNumber_ptr from the string
  representation]

  Description        [The string and base should be proper for standard
  "strtoull" function. The base can be 2, 8, 10 or 16. The number
  should be in the range supposed by the width. The provided width of
  the constant should be enough to hold the obtained number. In the
  case of any problem NULL is returned.

  NOTE: Memory sharing is used, i.e. given a string with the same
  value of WordNumber this constructor will return the same pointer
  (this is important for node_ptr hashing)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_from_sized_string(char* str, int base,
                                            int width)
{
  WordNumberValue value;
  char* tmpStr;

  /* only this bases are allowed here */
  nusmv_assert(2 == base || 8 == base || 10 == base || 16 == base);

#if NUSMV_HAVE_ERRNO_H
  errno = 0;
#endif
  value = strtoull(str, &tmpStr, base);

  /* an error happened */
  if (NULL == str ||

#if NUSMV_HAVE_ERRNO_H
      /* proper errno usage from the strtoull manpage */
      ((ERANGE == errno && (LLONG_MAX == value))
       || (0 != errno && 0 == value)) ||
#endif
      '\0' != *tmpStr) {
    return WORD_NUMBER(NULL);
  }

  /* the width is the number of digit multiplied by the base */
  return word_number_create(value, width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [Constructs a Word number WordNumber_ptr from the string
  representation obtained during parsing]

  Description        [The string is the string obtained during parsing. The
  string should correspond to the NuSMV lexer token "word constants",
  i.e. "0" character followed by the base, optional signed specifier,
  optional width (decimal number), "_" character and the value
  (binary, octal, decimal or hexadecimal number).  The base and the
  digits should correspond each other.

  The limit for width is implementation dependant.
  In the case of any problem NULL is returned, and if errorString is not NULL,
  it is set to a text string explaining the cause of the error.
  The returned error-string belongs to this function (it may change during next
  function invocation).

  NOTE: this constructor is NOT memory shared, i.e. given the same
  string twice different pointers may be returned. (Actually the
  returned pointers may be the same but different from
  pointers returned by memory shared constructors.)  ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_from_parsed_string(char* str,
                                             char** errorString)
{
  WordNumberValue value;
  long width;
  int base;
  boolean isSigned;

  char* currentStr;
  char* tmpStr;

  /* buffer for error messages. 200 chars should be enough to
     reprsent any number and any error message
  */
#define err_buf_size  200
  static char err_buf[err_buf_size];

  /* the buffer is used to get rid of '_' from the string */
  static char* buffer = (char*)NULL;
  static int bufferSize = 0;

  nusmv_assert((char*)NULL != str);
  currentStr = str;

  /* 1. remove the first "0", check check whether the number signed or not
     and get the base specifier */
  nusmv_assert('0' == *currentStr);/*first character of Word constant is "0"*/
  ++currentStr;

  if ('s' == *currentStr) {
    isSigned = true;
    ++currentStr;
  }
  else {
    isSigned = false;
    if ('u' == *currentStr) ++currentStr;
  }


  switch (toupper(*currentStr)) {
  case 'B': base = 2;  break;
  case 'O': base = 8;  break;
  case 'D': base = 10; break;
  case 'H': base = 16; break;
  default: error_unreachable_code();/* something wrong with the base specifier */
    /* this error should be impossible => assertion */
  } /* switch */
  ++currentStr; /* get past the base specifier */


  /* 2. calculate the explicit width of the Word constant*/
  if ('_' != *currentStr) { /* the width is explicitly specified */
    nusmv_assert(isdigit(*currentStr)); /* only a digit can go here */

#if NUSMV_HAVE_ERRNO_H
    errno = 0;
#endif

    width = strtol(currentStr, &tmpStr, 10);

#if NUSMV_HAVE_ERRNO_H /* proper errno usage from the strtol manpage */
    if ((ERANGE == errno && (LONG_MAX == width || LONG_MIN == width))
        || (0 != errno && 0 == width)) {
#else
    /* error : width specifier overflow or underflow*/
    if (width != (int) width) {
#endif
      if (NULL != errorString) {
        *errorString = "overflow or underflow in the width specifier of a Word constant";
      }
      return WORD_NUMBER(NULL);
    }

    /* error in the width specifier */
    if ('_' != *tmpStr) { /* 'underscore must go after the width specifier */
      error_unreachable_code();/* in current implementation this code
                             is impossible */
      if (NULL != errorString) {
        int i = snprintf(err_buf, err_buf_size,
                         "erroneous character '%c' in the width specifier "
                         "of a Word constant", *tmpStr);
        SNPRINTF_CHECK(i, err_buf_size); /* above print was successful */
        *errorString = err_buf;
      }
      return WORD_NUMBER(NULL);
    }

    /* 2.1 move past the optional width specifier */
    for ( ; isdigit(*currentStr); ++currentStr) {}
  }
  else width = -1; /* for sure incorrect value */


  /* 3. copy the value string into a buffer and remove '_' from the string */
  nusmv_assert('_' == *currentStr); /* underscore before the value part */

  if (bufferSize < strlen(currentStr)) { /* reallocate the buffer */
    bufferSize = strlen(currentStr);
    buffer = REALLOC(char, buffer, bufferSize);
  }

  for (tmpStr = buffer; '\0' != *currentStr; ++currentStr) {
    if ('_' != *currentStr) {
      *tmpStr = *currentStr; /* = toupper(*str); not needed at the moment */
      ++tmpStr;
    }
  } /* for */
  *tmpStr = '\0';

  /* 4. calculate the implicit width of the Word constant */
  if (-1 == width) {/* there was no width specifier => calculate it */
    /* calculate the number of digits */
    width = strlen(buffer);

    switch (base) {
    case 2: /* nothing */ break;
    case 8: width *= 3; break;
    case 10:  /* error */
      if (NULL != errorString) {
        *errorString = "decimal Word constant without width specifier";
      }
      return WORD_NUMBER(NULL);
    case 16:width *= 4; break;
    default: error_unreachable_code(); /* impossible error */
    } /* switch */
  }
  /* check the wellformedness of the width of the Word */
  if (width <= 0 || width > wordNumberMaxWidth) {
    if (NULL != errorString) {
      int i = snprintf(err_buf, err_buf_size,
                       "width of a Word constant is out of range 1..%i",
                       wordNumberMaxWidth);
      SNPRINTF_CHECK(i, err_buf_size); /* above print was successful */
      *errorString = err_buf;
    }
    return WORD_NUMBER(NULL);
  }

  /* 5. calculate the value */
#if NUSMV_HAVE_ERRNO_H
  errno = 0;
#endif
  value = strtoull(buffer, &tmpStr, base);

  /* error : value overflow or underflow*/
#if NUSMV_HAVE_ERRNO_H /* proper errno usage from the strtoull manpage */
  if  (((ERANGE == errno && (LLONG_MAX == value)))
       || (0 != errno && 0 == value)) {

    if (NULL != errorString) {
      *errorString = "overflow or underflow in the value of a Word constant";
    }
    return WORD_NUMBER(NULL);
  }
#endif

  /* error in the value */
  if (*tmpStr != '\0') {
    if (NULL != errorString) {
      int i = snprintf(err_buf, err_buf_size,
                       "erroneous character '%c' in a Word constant value",
                       *tmpStr);
      SNPRINTF_CHECK(i, err_buf_size); /* above print was successful */
      *errorString = err_buf;
    }
    return WORD_NUMBER(NULL);
  }

  /* here two shifts are performed because shift by the width of
     the type in C is illegal
  */
  if ( ((value >> (width-1)) >> 1) != 0) {
    if (NULL != errorString) {
      int i = snprintf(err_buf, err_buf_size,
                       "value of a Word constant %s is outside of its width",
                       str);
      SNPRINTF_CHECK(i, err_buf_size); /* above print was successful */
      *errorString = err_buf;
    }
    return WORD_NUMBER(NULL);
  }
  /* for signed decimal words values have to be in range 0 .. 2^(width-1) */
  if (isSigned && 10 == base && value > (1ULL << (width-1))) {
    /* NOTE: To represent negative constants value 2^(width-1) was
       allowed though positive constant cannot have such values.
       2^(width-1) == -2^(width-1) holds in this case. */
    if (NULL != errorString) {
      int i = snprintf(err_buf, err_buf_size,
                       "value of a decimal Signed Word constant %s is outside of its width",
                       str);
      SNPRINTF_CHECK(i, err_buf_size); /* above print was successful */
      *errorString = err_buf;
    }
    return WORD_NUMBER(NULL);
  }

  if (NULL != errorString) { /* potential error in the constructor */
    *errorString = "undefined error with a Word constant";
  }

  return word_number_create(value, width, str);
}


/**Function********************************************************************

  Synopsis           [returns a WordNumber]

  Description        [value and width should be correct, i.e. in a proper
  range. See WordNumber_from_signed_integer if original value is signed.

  NOTE: Memory sharing is used, i.e. given the same parameter this
  constructor will return the same pointer (this is important for
  node_ptr hashing)]

  SideEffects        []

  SeeAlso            [WordNumber_from_signed_integer]

******************************************************************************/
WordNumber_ptr WordNumber_from_integer(WordNumberValue value, int width)
{
  return word_number_create(value, width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [returns a WordNumber]

  Description [ This constructor is the same as
  WordNumber_from_integer except than value is interpreted as signed
  value casted to WordNumberValue.

  The difference is that signed negative value casted to WordNumberValue
  will have 1s at positions greater than width. These bits are ignored
  but in WordNumber_from_integer they cause assertion violation.

  For originally positive values both constructors behave the same.
  ]

  SideEffects        []

  SeeAlso            [WordNumber_from_integer]

******************************************************************************/
WordNumber_ptr WordNumber_from_signed_integer(WordNumberValue value, int width)
{
  nusmv_assert(width > 0 && width <= wordNumberMaxWidth); /* implementation limit */

  /* simply put 0s at positions [max-width-1, width] */
  WordNumberValue new_value= ((~ 0ULL) >> (wordNumberMaxWidth - width)) & value;

  /* DEBUGGING CODE:
     the value is representable with given width iff
     either all bits [max-width-1, width-1] are 0 (for positive signed values)
     or all are 1 (for negative signed values).
  */
  nusmv_assert((value >> (width-1)) == 0 ||
               (value >> (width-1)) == ((~ 0ULL) >> (width-1)));

  return word_number_create(new_value, width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [returns a memory shared WordNumber]

  Description [If a word number was created with a constructor
  WordNumber_from_parsed_string then such a WordNumber is NOT memory
  shared.
  This function takes such WordNumber and returns its memory shared
  analog, i.e. with the same width and value but without string
  information.
  If WordNumber was created with memory-shared constructor (for
  example, WordNumber_from_integer), there is no need to use this
  function since the returned value will be the same as input]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_normalize(const WordNumber_ptr number)
{
  /* Here NULL was passed, now passing the parsed string too, if
     any. Fix for issue 2220 */
  return word_number_create(WordNumber_get_unsigned_value(number),
                            WordNumber_get_width(number),
                            (NULL != number->parsedString ?
                             get_text(number->parsedString) : NULL));
}


/**Function********************************************************************

  Synopsis           [returns the value of a WordNumber, as unsigned word]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumberValue WordNumber_get_unsigned_value(WordNumber_ptr self)
{
  WORD_NUMBER_CHECK_INSTANCE(self);
  return self->value;
}


/**Function********************************************************************

  Synopsis [returns the value of a WordNumber, interpreted as a signed
  word]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumberValue WordNumber_get_signed_value(WordNumber_ptr self)
{
  WORD_NUMBER_CHECK_INSTANCE(self);
  return word_number_to_signed_c_value(self);
}


/**Function********************************************************************

  Synopsis           [returns the status (true or false) of the sign bit]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_get_sign(WordNumber_ptr self)
{
  WORD_NUMBER_CHECK_INSTANCE(self);
  return WordNumber_get_bit(self, WordNumber_get_width(self)-1);
}


/**Function********************************************************************

  Synopsis           [returns the width of a WordNumber]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int WordNumber_get_width(WordNumber_ptr self)
{
  WORD_NUMBER_CHECK_INSTANCE(self);
  return self->width;
}


/**Function********************************************************************

  Synopsis           [returns the status (true or false) of a particular bit]

  Description        [the bit number should be in the range \[0, width-1\].]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_get_bit(WordNumber_ptr self, int n)
{
  WORD_NUMBER_CHECK_INSTANCE(self);
  nusmv_assert(n >= 0 && n < self->width);
  return (self->value >> n) & 1;
}


/**Function********************************************************************

  Synopsis           [returns a string which was given
  to WordNumber_from_parsed_string constructor. If the number was created
  by any other constructor, NULL is returned.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
char* WordNumber_get_parsed_string(WordNumber_ptr self)
{
  WORD_NUMBER_CHECK_INSTANCE(self);
  if ((string_ptr)NULL != self->parsedString) {
    return get_text(self->parsedString);
  }
  else return (char*)NULL;
}


/**Function********************************************************************

  Synopsis           [returns a maximal value of unsigned word of given width]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumberValue WordNumber_max_unsigned_value(int width)
{
  nusmv_assert(width > 0 && width <= wordNumberMaxWidth); /* implementation limit */
  /* max unsigned value of given width consists of 1 at positions[width-1:0] */
  return (~ 0ULL) >> (wordNumberMaxWidth - width);
}


/**Function********************************************************************

  Synopsis           [returns a maximal value of signed word of given width.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumberValue WordNumber_max_signed_value(int width)
{
  nusmv_assert(width > 0 && width <= wordNumberMaxWidth); /* implementation limit */
  /* max signed value of given width is 1 at positions [width-2:0]
     and 0 everything else */
  return ((~ 0ULL) >> (wordNumberMaxWidth - width) >> 1);
}


/**Function********************************************************************

  Synopsis           [returns a minimal value of signed word of given width.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumberValue WordNumber_min_signed_value(int width)
{
  /* min signed value of given width consists of 0 at positions[width-1:0]
     and 1 everthing else */
  return (~ 0ULL) << (width - 1);
}


/**Function********************************************************************

  Synopsis           [prints a Word constant to a stream in a generic format.]

  Description        [Generic format means that the number is printed
  in the base specified by the system variable "output_word_format"
  with one exception -- if the number was created with
  WordNumber_from_parsed_string, then that string provided during
  construction will be printed.
  If base is 10 then isSigned is taken into account, i.e. if it is true then
  the number is ouput as signed word, and as unsigned word otherwise.]

  SideEffects        []

  SeeAlso            [WordNumber_based_print]

******************************************************************************/
int WordNumber_print(FILE* output_stream, WordNumber_ptr self, boolean isSigned)
{
  char* result = WordNumber_to_string(self, isSigned);
  if ((char*)NULL == result) return -1;
  return fprintf(output_stream, "%s", result);
}


/**Function********************************************************************

  Synopsis           [prints a Word constant in a provided base.]

  Description        [returns negative value in a case of error.
  Only 2, 8, 10, 16 bits bases are allowed.
  If base is 10 then isSigned is taken into account, i.e. if it is true then
  the number is ouput as signed word, and as unsigned word otherwise.]

  SideEffects        []

  SeeAlso            [WordNumber_print]

******************************************************************************/
int WordNumber_based_print(FILE* output_stream, WordNumber_ptr self, int base,
                           boolean isSigned)
{
  char* result = WordNumber_to_based_string(self, base, isSigned);
  if ((char*)NULL == result) return -1;
  return fprintf(output_stream, "%s", result);
}


/**Function********************************************************************

  Synopsis           [prints a Word constant to a char string.]

  Description        [It is the same as WordNumber_print
  but outputs to a string instead of a stream, i.e.:
    the number is printed in the base specified by the system variable
    "output_word_format".
  In case of any problem, NULL is returned.

  If base is 10 then isSigned is taken into account, i.e. if it is true then
  the number is ouput as signed word, and as unsigned word otherwise.

  Note: The returned string belongs to the funcion. Do not modify this
  string.
  Note: The next invocation of this function or WordNumber_to_based_string
  makes the previously returned string unusable]

  SideEffects        []

  SeeAlso            [WordNumber_print, WordNumber_to_based_string]

******************************************************************************/
char* WordNumber_to_string (WordNumber_ptr self, boolean isSigned)
{
  WORD_NUMBER_CHECK_INSTANCE(self);

  /* Commented out for issue 2220 */
  /* if ((string_ptr)NULL != self->parsedString) {/\*this is a string from parser*\/ */
  /*   return get_text(self->parsedString); */
  /* } */

  return WordNumber_to_based_string(self,
                     get_output_word_format(OptsHandler_get_instance()),
                     isSigned);
}


/**Function********************************************************************

  Synopsis           [prints a Word constant in a provided base to a string.]

  Description        [This function is the same as WordNumber_based_print,
  except this function outputs to a string, not a stream.
  Only 2, 8, 10, 16 bits bases are allowed.
  If base is 10 then isSigned is taken into account, i.e. if it is true then
  the number is ouput as signed word, and as unsigned word otherwise.
  In case of any problem, NULL is returned.

  Note: The returned string belongs to the funcion. Do not modify this
  string.
  Note: The next invocation of this function or WordNumber_to_string
  makes the previously returned string unusable]

  SideEffects        []

  SeeAlso            [WordNumber_based_print, WordNumber_to_string]

******************************************************************************/
char* WordNumber_to_based_string(WordNumber_ptr self, int base, boolean isSigned)
{
  static char* buffer = (char*)NULL;
  static int buffer_size = 0;

  int printed;

  WORD_NUMBER_CHECK_INSTANCE(self);

  /* allocate a buffer. The requires string is very likely less than width+20*/
  if (buffer_size < (self->width + 20)) {
    buffer_size = self->width + 20;
    buffer = REALLOC(char, buffer, buffer_size);
  }


  /* print the "0", the base, the width and the value */
  switch (base) {

  case 2: {
    int n = self->width - 1;
    WordNumberValue v = self->value;
    char* str = buffer;
    nusmv_assert(v >= 0);

    printed = snprintf(str, buffer_size, "0%sb%i_", isSigned ? "s" : "u", self->width);
    SNPRINTF_CHECK(printed, buffer_size);

    if (printed <= 0) return (char*)NULL; /* error in printing */
    else str += printed;

    /* while (((v>>n)&1) == 0 && n > 0) --n; */ /* skip initial zeros */

    while (n >= 0) {
      printed = snprintf(str, buffer_size, "%u", (int)((v>>n)&1));
      SNPRINTF_CHECK(printed, buffer_size);

      if ( printed <= 0) return (char*)NULL; /* error in printing */
      else str += printed;
      --n;
    }
    nusmv_assert((str - buffer) < buffer_size);
    return buffer;
  }

  case 8: 
    printed = snprintf(buffer, buffer_size, "0%so%d_%"LLO,
                       isSigned ? "s" : "u", self->width, self->value);
    SNPRINTF_CHECK(printed, buffer_size);
    break;
  case 16:
    printed = snprintf(buffer, buffer_size, "0%sh%d_%"LLX,
                      isSigned ? "s" : "u" , self->width, self->value);
    SNPRINTF_CHECK(printed, buffer_size);
    break;
  case 10: {
    WordNumberValue value = self->value;
    int sign = 0;

    if (isSigned) { /* the word is signed => check the sign */
      sign = value >> (self->width-1);
      nusmv_assert(0 == sign || 1 == sign); /* one the highest bit only */
      if (sign) { /* get positive number */
        value = -((((~0ULL) >> (self->width-1)) << (self->width-1)) | value);
      }
    }
    printed = snprintf(buffer, buffer_size, "%s0%sd%d_%"LLU, sign ? "-" : "",
                       isSigned ? "s" : "u", self->width, value);
    SNPRINTF_CHECK(printed, buffer_size);
    break;
  }
  default: error_unreachable_code(); /* unknown base */
  }/* switch */

  if (printed <= 0) return (char*)NULL; /* error in printing */
  nusmv_assert(printed < buffer_size);
  return buffer;
}


/**Function********************************************************************

  Synopsis           [perform the negation operation]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_unary_minus(WordNumber_ptr v)
{
  WordNumberValue l;
  WORD_NUMBER_CHECK_INSTANCE(v);

  /* create a constant of 'width' number of 1 bits.
     The left shifts are used because in C shift by a full width is not allowed
  */
  l = ~ (((~ 0ULL) << (v->width - 1)) << 1);

  return word_number_create((- v->value)&l, v->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [perform summation operation]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_plus(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WordNumberValue l;
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  /* create a constant of 'width' number of 1 bits.
     The left shifts are used because in C shift by a full width is not allowed
  */
  l = ~ (((~ 0ULL) << (v1->width - 1)) << 1);

  return word_number_create((v1->value + v2->value)&l, v1->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [perform subtraction operation on Words]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_minus(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WordNumberValue l;

  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  /* create a constant of 'width' number of 1 bits.
     The left shifts are used because in C shift by a full width is not allowed
  */
  l = ~ (((~ 0ULL) << (v1->width - 1)) << 1);

  return word_number_create((v1->value - v2->value)&l, v1->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [perform multiplidation operation on Words]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_times(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WordNumberValue l;

  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);

  /* create a constant of 'width' number of 1 bits.
     The left shifts are used because in C shift by a full width is not allowed
  */
  l = ~ (((~ 0ULL) << (v1->width - 1)) << 1);

  nusmv_assert(v1->width == v2->width);
  return word_number_create((v1->value * v2->value)&l, v1->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [perform unsigned division operation on Words]

  Description        [the width of operands should be equal. The
  right operand should not be 0.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_unsigned_divide(WordNumber_ptr v1,  WordNumber_ptr v2)
{
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  nusmv_assert(0 != v2->value);
  /* division is rounded DOWN (as in evaluation procedure).
   Since 'value' is unsigned, there is no need to worry about it.
  */
  return word_number_create(v1->value / v2->value, v1->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [perform signed division operation on Words]

  Description        [the width of operands should be equal. The
  right operand should not be 0]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_signed_divide(WordNumber_ptr v1,  WordNumber_ptr v2)
{
  signed long long int _v1;
  signed long long int _v2;
  signed long long int _res;
  WordNumberValue l;

  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);
  nusmv_assert(0 != v2->value);

  _v1 = word_number_to_signed_c_value(v1);
  _v2 = word_number_to_signed_c_value(v2);
  _res = (_v1 / _v2);

  l = ~ (((~ 0ULL) << (v1->width - 1)) << 1);

  return word_number_create(_res & l, v1->width, (char*) NULL);
}


/**Function********************************************************************

  Synopsis           [perform remainder unsigned operation on Words]

  Description        [the width of operands should be equal. The right
  operand should not be 0.
  Note: numbers are considered as unsigned.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_unsigned_mod(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  nusmv_assert(0 != v2->value);
  /* C's division is rounded towards 0, but NuSMV rounds down.
   Since 'value' is unsigned, there is no need to worry about it.
  */
  return word_number_create(v1->value % v2->value, v1->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [perform remainder signed operation on Words]

  Description        [the width of operands should be equal. The right
  operand should not be 0]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_signed_mod(WordNumber_ptr v1,  WordNumber_ptr v2)
{
  signed long long int _v1;
  signed long long int _v2;
  signed long long int _res;
  WordNumberValue l;

  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);
  nusmv_assert(0 != v2->value);

  _v1 = word_number_to_signed_c_value(v1);
  _v2 = word_number_to_signed_c_value(v2);
  _res = (_v1 % _v2);

  l = ~ (((~ 0ULL) << (v1->width - 1)) << 1);

  return word_number_create(_res & l, v1->width, (char*) NULL);
}


/**Function********************************************************************

  Synopsis           [Checks wether the word is the constant word of
  all bit set to zero]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_is_zero(WordNumber_ptr v)
{
  WORD_NUMBER_CHECK_INSTANCE(v);

  return (WordNumber_get_unsigned_value(v) == 0LL);
}


/**Function********************************************************************

  Synopsis           [returns TRUE if operands are equal]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_equal(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  return v1->value == v2->value;
}


/**Function********************************************************************

  Synopsis           [returns TRUE if operands are NOT equal]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_not_equal(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  return v1->value != v2->value;
}


/**Function********************************************************************

  Synopsis           [returns TRUE if left operand is less than
  the right one (numbers are considered as unsigned)]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_unsigned_less(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  return v1->value < v2->value;
}


/**Function********************************************************************

  Synopsis           [returns TRUE if left operand is less than, or equal to,
  the right one (numbers are considered as unsigned)]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_unsigned_less_or_equal(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  return v1->value <= v2->value;
}


/**Function********************************************************************

  Synopsis           [returns TRUE if left operand is greater than
  the right one (numbers are considered as unsigned)]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_unsigned_greater(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  return v1->value > v2->value;
}


/**Function********************************************************************

  Synopsis           [returns TRUE if left operand is greate than, or eqaul to,
  the right one (numbers are considered as unsigned)]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_unsigned_greater_or_equal(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  return v1->value >= v2->value;
}


/**Function********************************************************************

  Synopsis           [returns TRUE if left operand is signed less than
  the right one (numbers are considered as signed)]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_signed_less(WordNumber_ptr v1, WordNumber_ptr v2)
{
  boolean bit1, bit2;

  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  bit1 = WordNumber_get_bit(v1, v1->width - 1);
  bit2 = WordNumber_get_bit(v2, v1->width - 1);

  return bit1 > bit2 || (bit1 == bit2 && v1->value < v2->value);
}


/**Function********************************************************************

  Synopsis           [returns TRUE if left operand is signed less than,
  or equal to, the right one (numbers are considered as signed)]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_signed_less_or_equal(WordNumber_ptr v1, WordNumber_ptr v2)
{
  boolean bit1, bit2;

  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  bit1 = WordNumber_get_bit(v1, v1->width - 1);
  bit2 = WordNumber_get_bit(v2, v1->width - 1);

  return bit1 > bit2 || (bit1 == bit2 && v1->value <= v2->value);
}


/**Function********************************************************************

  Synopsis           [returns TRUE if left operand is signed greater than
  the right one (numbers are considered as signed)]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_signed_greater(WordNumber_ptr v1, WordNumber_ptr v2)
{
  boolean bit1, bit2;

  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  bit1 = WordNumber_get_bit(v1, v1->width - 1);
  bit2 = WordNumber_get_bit(v2, v1->width - 1);

  return bit1 < bit2 || (bit1 == bit2 && v1->value > v2->value);
}


/**Function********************************************************************

  Synopsis           [returns TRUE if left operand is signed greate than,
  or eqaul to, the right one (numbers are considered as signed)]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean WordNumber_signed_greater_or_equal(WordNumber_ptr v1, WordNumber_ptr v2)
{
  boolean bit1, bit2;

  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  bit1 = WordNumber_get_bit(v1, v1->width - 1);
  bit2 = WordNumber_get_bit(v2, v1->width - 1);

  return bit1 < bit2 || (bit1 == bit2 && v1->value >= v2->value);
}


/**Function********************************************************************

  Synopsis           [returns bitwise NOT of a Word number]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_not(WordNumber_ptr v)
{
  WordNumberValue l;
  WORD_NUMBER_CHECK_INSTANCE(v);

  /* create a constant of 'width' number of 1 bits.
     The left shifts are used because in C shift by a full width is not allowed
  */
  l = ~ (((~ 0ULL) << (v->width - 1)) << 1);

  return word_number_create((~ v->value) & l, v->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [returns bitwise AND of two Word numbers]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_and(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  return word_number_create(v1->value & v2->value, v1->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [returns bitwise OR of two Word numbers]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_or(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  return word_number_create(v1->value | v2->value, v1->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [returns bitwise XOR of two Word numbers]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_xor(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  return word_number_create(v1->value ^ v2->value, v1->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [returns bitwise XNOR(or IFF) of two Word numbers]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_xnor(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WordNumberValue l;

  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  /* create a constant of 'width' number of 1 bits.
     The left shifts are used because in C shift by a full width is not allowed
  */
  l = ~ (((~ 0ULL) << (v1->width - 1)) << 1);

  return word_number_create((~ (v1->value ^ v2->value)) & l,
                            v1->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [returns bitwise IMPLIES of two Word numbers]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_implies(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WordNumberValue l;

  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width == v2->width);

  /* create a constant of 'width' number of 1 bits.
     The left shifts are used because in C shift by a full width is not allowed
  */
  l = ~ (((~ 0ULL) << (v1->width - 1)) << 1);

  return word_number_create(((~ v1->value) | v2->value) & l,
                            v1->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [returns bitwise IFF(or XNOR) of two Word numbers]

  Description        [the width of operands should be equal]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_iff(WordNumber_ptr v1, WordNumber_ptr v2)
{
  return WordNumber_xnor(v1, v2);
}


/**Function********************************************************************

  Synopsis           [returns a concatenation of two Word numbers]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_concatenate(WordNumber_ptr v1, WordNumber_ptr v2)
{
  WORD_NUMBER_CHECK_INSTANCE(v1);
  WORD_NUMBER_CHECK_INSTANCE(v2);
  nusmv_assert(v1->width + v2->width <= wordNumberMaxWidth);

  return word_number_create((v1->value << v2->width) | v2->value,
                            v1->width + v2->width,  (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [returns a Word number consisting of the
  bits [highBit .. lowBit] from a given Word number]

  Description        [highBit should be less than the Word width and greater or
  equal to lowBit. lowBit should be greater or equal to 0.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_bit_select(WordNumber_ptr v, int highBit, int lowBit)
{
  WordNumberValue l;
  int newWidth = highBit - lowBit + 1;

  WORD_NUMBER_CHECK_INSTANCE(v);
  nusmv_assert(v->width > highBit && highBit >= lowBit && lowBit >= 0);

  /* create a constant of 'width' number of 1 bits.
     Two left shift are used because in C shift by a full width is not allowed
  */
  l = ~ (((~ 0ULL) << (newWidth - 1)) << 1);

  return word_number_create((v->value >> lowBit) & l, newWidth, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [perform right shift on a Word numbers]

  Description        [the number of shifted bits should be in the range
  \[0, width\]. The word is padded with zeros.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr
WordNumber_unsigned_right_shift(WordNumber_ptr v, int numberOfBits)
{
  WORD_NUMBER_CHECK_INSTANCE(v);
  nusmv_assert(v->width >= numberOfBits && numberOfBits >= 0);

  return word_number_create((v->value >> (numberOfBits-1)) >> 1, v->width,
                            (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [perform right shift on a Word numbers]

  Description        [the number of shifted bits should be in the range
  \[0, width\]. The word is padded with zeros.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr
WordNumber_signed_right_shift(WordNumber_ptr v, int numberOfBits)
{
  WordNumberValue l;
  WORD_NUMBER_CHECK_INSTANCE(v);
  nusmv_assert(v->width >= numberOfBits && numberOfBits >= 0);

  if (numberOfBits == v->width) {
    numberOfBits -= 1;
  }

  /* prepares a mask for sign bits if sign is set */
  if (((1ULL << (v->width - 1)) & v->value) != 0) {
    l = (~((~0ULL) << (numberOfBits))) << (v->width - numberOfBits);
  }
  else l = 0; /* no sign bit */

  return word_number_create((v->value >> numberOfBits) | l,
                            v->width, (char*) NULL);
}


/**Function********************************************************************

  Synopsis           [perform left shift on a Word numbers]

  Description        [the number of shifted bits should be in the range
  \[0, width\]. The word is padded with zeros.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_left_shift(WordNumber_ptr v, int numberOfBits)
{
  WordNumberValue l;

  WORD_NUMBER_CHECK_INSTANCE(v);
  nusmv_assert(v->width >= numberOfBits && numberOfBits >= 0);

  if (v->width == numberOfBits) {
    return word_number_create(0, v->width, (char*)NULL);
  }

  /* create a constant of 'width' number of 1 bits.  The left
     shifts are used because in C shift by a full width is not
     allowed */
  l = ~ (((~ 0ULL) << (v->width - 1)) << 1);

  return word_number_create((v->value << numberOfBits) & l,
                            v->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [perform right rotate on a Word numbers]

  Description        [the number of rotated bits should be in the range
  \[0, width\].]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_right_rotate(WordNumber_ptr v, int numberOfBits)
{
  WordNumberValue l;

  WORD_NUMBER_CHECK_INSTANCE(v);
  nusmv_assert(v->width >= numberOfBits && numberOfBits >= 0);

  if (v->width == numberOfBits) {
    return word_number_create(v->value, v->width, (char*)NULL);
  }

  /* create a constant of 'width' number of 1 bits.  The left
     shifts are used because in C shift by a full width is not
     allowed */
  l = ~ (((~ 0ULL) << (v->width - 1)) << 1);

  return word_number_create(( (v->value >> numberOfBits)
                            | (v->value << (v->width - numberOfBits))
                            ) & l,
                            v->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [perform left rotate on a Word numbers]

  Description        [the number of rotated bits should be in the range
  \[0, width\].]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_left_rotate(WordNumber_ptr v, int numberOfBits)
{
  WordNumberValue l;

  WORD_NUMBER_CHECK_INSTANCE(v);
  nusmv_assert(v->width >= numberOfBits && numberOfBits >= 0);

  if (v->width == numberOfBits) {
    return word_number_create(v->value, v->width, (char*)NULL);
  }

  /* create a constant of 'width' number of 1 bits.  The left
     shifts are used because in C shift by a full width is not
     allowed */
  l = ~ (((~ 0ULL) << (v->width - 1)) << 1);

  return word_number_create(( (v->value << numberOfBits)
                            | (v->value >> (v->width - numberOfBits))
                            ) & l,
                            v->width, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [performs sign extend, i.e. concatenates 'numberOfTimes'
  number of times the highest bit of v with v.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_signed_extend(WordNumber_ptr v, int numberOfTimes)
{
  WordNumberValue highestBit;
  WordNumberValue value;
  int newWidth;

  WORD_NUMBER_CHECK_INSTANCE(v);
  nusmv_assert(v->width + numberOfTimes <= wordNumberMaxWidth);

  /* optimisation */
  if (0 == numberOfTimes) return v;

  highestBit = WordNumber_get_bit(v, v->width - 1);
  highestBit <<= v->width;

  newWidth = v->width + numberOfTimes;

  for (value = v->value; numberOfTimes > 0; --numberOfTimes) {
    value |= highestBit;
    highestBit <<= 1;
  }

  return word_number_create(value, newWidth, (char*)NULL);
}


/**Function********************************************************************

  Synopsis           [performs unsign extend]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
WordNumber_ptr WordNumber_unsigned_extend(WordNumber_ptr v, int numberOfTimes)
{
  return word_number_create(WordNumber_get_unsigned_value(v),
                            WordNumber_get_width(v)+numberOfTimes,
                            (char*) NULL);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The constructor of WordNumber_ptr. Also adds the created
  number to the memory manager]

  Description        [In case of any errors, returns NULL.
  This function can be invoked only by constructor-converter functions.
  The function creates a copy of the parameter parsedString.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static WordNumber_ptr word_number_create(WordNumberValue value, int width,
                                         char* parsedString)
{
  WordNumber_ptr word = (WordNumber_ptr)NULL;
  node_ptr key = (node_ptr)NULL;
  string_ptr ps = (string_ptr)NULL;

  /* implementation limit */
  nusmv_assert(width > 0 && width <= wordNumberMaxWidth);

  /* the number is out of the range for a given width.

     NB: Two shifts are performed since shift by the full width isn't
     allowed in C
  */
  nusmv_assert(((value >> (width - 1)) >> 1) == 0);

  /* there is a string from parser => we use the string as key, as to
     allow printing the same model read, otherwise we use the width
     and value */
  if (NULL != parsedString) {
    ps = find_string(parsedString);
    key = (node_ptr)ps;
  }
  else {
    key = find_node(width,
                   NODE_FROM_INT((int)value),
                   NODE_FROM_INT((int)(value>>(wordNumberMaxWidth/2))));
  }

  word = WORD_NUMBER(find_assoc(memoryManager.hashTable, key));

  if (WORD_NUMBER(NULL) != word) return word;

  word = ALLOC(WordNumber, 1);
  if (WORD_NUMBER(NULL) == word) return WORD_NUMBER(NULL);

  word->value = value;
  word->width = width;
  word->parsedString = ps;

  insert_assoc(memoryManager.hashTable, key, (node_ptr)word);

  return word;
}


/**Function********************************************************************

  Synopsis           [Destructor of a WordNumber_ptr]

  Description        [Destructor can be invoked only by the class
  deinitializer, when all the WordNumber_ptr numbers are destroyed.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void word_number_destroy(WordNumber_ptr word)
{
  FREE(word);
}


/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static WordNumberValue
word_number_to_signed_c_value(const WordNumber_ptr self)
{
  WordNumberValue uv = WordNumber_get_unsigned_value(self);
  int sign = uv >> (self->width-1);
  WordNumberValue l;
  nusmv_assert(0 == sign || 1 == sign); /* one the highest bit only */

  if (sign == 0) return uv;
  l = (((~ 0ULL) << (self->width - 1)) << 1);
  return uv | l;
}
