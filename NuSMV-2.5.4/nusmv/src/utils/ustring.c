/**CFile***********************************************************************

  FileName    [ustring.c]

  PackageName [utils]

  Synopsis    [Routines to handle with strings.]

  Description [Routines to handle with strings, in order to maintain
  an unique instance of each string.]

  Author      [Marco Roveri]

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

#include "util.h"
#include "utils/utils.h" /* for nusmv_assert */
#include "utils/ustring.h"

static char rcsid[] UTIL_UNUSED = "$Id: ustring.c,v 1.5.6.1.4.2 2005-03-03 12:32:24 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define STRING_HASH_SIZE 511
#define STRING_MEM_CHUNK 1022

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct string_mgr_ {
  long allocated;             /* Number of string struct allocated */
  long memused;               /* Total memory allocated by the string mgr */
  string_ptr * memoryList;    /* memory manager */
  string_ptr nextFree;        /* list of free strings */
  string_ptr * string_hash;   /* the string hash table */
} string_mgr_;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
static string_mgr_ *string_mgr;

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int string_hash_fun ARGS((string_ptr string));
static int string_eq_fun ARGS((string_ptr a1, string_ptr a2));
static string_ptr string_alloc ARGS((void));
static void string_free ARGS((string_ptr str));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

void init_string()
{
  string_mgr = (string_mgr_ *)ALLOC(string_mgr_, 1);
  if (string_mgr == (string_mgr_ *)NULL) {
    fprintf(stderr, "init_string: Out of Memory in allocating the string manager\n");
    exit(1);
  }
  string_mgr->allocated  = 0;
  string_mgr->memused    = 0;
  string_mgr->memoryList = (string_ptr *)NULL;
  string_mgr->nextFree   = (string_ptr)NULL;

  string_mgr->string_hash = (string_ptr *)ALLOC(string_ptr, STRING_HASH_SIZE);
  if (string_mgr->string_hash == (string_ptr *)NULL) {
    fprintf(stderr, "init_string: Out of Memory in allocating the string hash.\n");
    exit(1);
  }
  { /* Initializes the node cache */
    int i;

    for(i = 0; i < STRING_HASH_SIZE; i++) string_mgr->string_hash[i] = (string_ptr)NULL;
  }
}

void quit_string()
{
  /* Shut down the string manager */
  int i;

  /* Free the hash table and all string copies. */
  for (i = 0; i < STRING_HASH_SIZE; ++i) {
    string_ptr curr = string_mgr->string_hash[i];
    while ((string_ptr)NULL != curr) {
      string_ptr next = curr->link;
      string_free(curr);
      curr = next;
    }
  }

  FREE(string_mgr->string_hash);

  /* Free memory chunks */
  {
    string_ptr * curr = string_mgr->memoryList;
    while ((string_ptr*)NULL != curr) {
      string_ptr * next = (string_ptr *) curr[0];
      FREE(curr);
      curr = next;
    }
  }

  FREE(string_mgr);
}

char * get_text(string_ptr str) {
  nusmv_assert(str != (string_ptr)0);
  return(str->text);
}

string_ptr find_string(char* text)
{
  string_rec str;
  string_ptr * string_hash;
  string_ptr looking;
  int pos;
  
  str.text = text;
  string_hash = string_mgr->string_hash;
  pos = string_hash_fun(&str);
  looking = string_hash[pos];

  while (looking != (string_ptr)NULL) {
    if (string_eq_fun(&str, looking)) return(looking);
    looking = looking->link;
  }
  /* The string is not in the hash, it is created and then inserted in the hash */
  looking = string_alloc();
  if (looking == (string_ptr)NULL) {
    fprintf(stderr, "find_string: Out of Memory\n");
    return((string_ptr)NULL);
  }
  looking->text = (char *)ALLOC(char, strlen(text) + 1);
  looking->text = (char *)strcpy(looking->text, text);
  looking->link = string_hash[pos];
  string_hash[pos] = looking;
  return(looking);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

static int string_hash_fun(string_ptr string)
{
  char *p = string->text;
  unsigned h = 0;

  while (*p) h = (h<<1) + *(p++);
  return(h % STRING_HASH_SIZE);
}

static int string_eq_fun(string_ptr a1, string_ptr a2)
{ return(strcmp(a1->text,a2->text)==0);}

static string_ptr string_alloc()
{
  int i;
  string_ptr str;
  
  if(string_mgr->nextFree == (string_ptr)NULL) { /* Memory is full */
    string_ptr list;
    string_ptr * mem = (string_ptr *)ALLOC(string_rec, STRING_MEM_CHUNK + 1);

    if (mem == (string_ptr *)NULL) { /* out of memory */
      fprintf(stderr, "string_alloc: out of memory");
      fprintf(stderr, "Memory in use for ustring = %ld\n",
                     string_mgr->memused);
      return((string_ptr)NULL);
    }
    else { /* Adjust manager data structure */
      string_mgr->memused += (STRING_MEM_CHUNK + 1)* sizeof(string_rec);
      mem[0] = (string_ptr)string_mgr->memoryList;
      string_mgr->memoryList = mem;
      list = (string_ptr)mem;
      
      /* Link the new set of allocated strings together */
      i = 1;
      do {
        list[i].link = &list[i+1];
      } while (++i < STRING_MEM_CHUNK);
      list[STRING_MEM_CHUNK].link = (string_ptr)NULL;

      string_mgr->nextFree = &list[1];
    }
  }
  /* Now the list of nextFree is not empty */
  string_mgr->allocated++;
  str = string_mgr->nextFree; /* Takes the first free available string */
  string_mgr->nextFree = str->link;
  str->link = (string_ptr)NULL;
  return(str);
}


static void string_free(string_ptr str)
{
  FREE(str->text);
}
