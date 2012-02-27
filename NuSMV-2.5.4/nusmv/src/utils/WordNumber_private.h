/**CHeaderFile*****************************************************************

  FileName    [WordNumber_private.h]

  PackageName [enc.utils]

  Synopsis    [Private interface of the class WordNumber]

  Description [The private integeface contains the initialisation
  and deinitialisation of the class WordNumber, i.e. the memory manager
  of the class.]
                                               
  SeeAlso     [WordNumber.h]

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``enc.utils'' package of NuSMV
  version 2.  Copyright (C) 2005 by FBK-irst.

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

  Revision    [$Id: WordNumber_private.h,v 1.1.2.1 2006-06-01 18:18:40 nusmv Exp $]

******************************************************************************/

#ifndef __WORD_NUMBER_PRIVATE_H__
#define __WORD_NUMBER_PRIVATE_H__


/* ---------------------------------------------------------------------- */
/*     Private methods                                                    */
/* ---------------------------------------------------------------------- */

/* Initialisation and de-initialisation of the WordNumber class (the manager) */
EXTERN void WordNumber_init ARGS((void));
EXTERN void WordNumber_quit ARGS((void));


#endif /* __WORD_NUMBER_PRIVATE_H__ */
