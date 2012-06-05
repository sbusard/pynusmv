/**CHeaderFile*****************************************************************

  FileName    [ustring.h]

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

  Revision    [$Id: ustring.h,v 1.2.16.2 2005-03-10 15:38:36 nusmv Exp $]

******************************************************************************/
#ifndef _U_STRING_H
#define _U_STRING_H

#include "utils/utils.h"
/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct string_ {
  struct string_ *link;
  char *text;
} string_rec;

typedef struct string_ *string_ptr;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define str_get_text(_s_) _s_->text

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void init_string ARGS((void));
EXTERN void quit_string ARGS((void));
EXTERN string_ptr find_string ARGS((char *));
EXTERN char * get_text ARGS((string_ptr str));

#endif /* _U_STRING_H */
