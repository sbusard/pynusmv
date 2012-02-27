/**CHeaderFile*****************************************************************

  FileName    [GenericTrans.h]

  PackageName [trans.generic]

  Synopsis    [The public interface of the GenericTrans class]

  Description [Declares the public interface to manipulate generic trans 
  objects, to be used as base class from more specific derived transition 
  relation objects]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``trans.generic'' package of NuSMV version 2. 
  Copyright (C) 2003 by FBK-irst. 

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

  Revision    [$Id: GenericTrans.h,v 1.1.2.2.4.1 2005-03-03 12:32:23 nusmv Exp $]

******************************************************************************/

#ifndef __TRANS_GENERIC_GENERIC_TRANS_H__
#define __TRANS_GENERIC_GENERIC_TRANS_H__

#include "utils/utils.h" /* for EXTERN and ARGS */
#include "trans/trans.h" /* for TransType */


/**Type***********************************************************************

  Synopsis    [The structure used to represent the transition relation.]

  Description []

******************************************************************************/
typedef struct GenericTrans_TAG* GenericTrans_ptr;

#define GENERIC_TRANS(x)  \
        ((GenericTrans_ptr) x)

#define GENERIC_TRANS_CHECK_INSTANCE(x)  \
        (nusmv_assert(GENERIC_TRANS(x) != GENERIC_TRANS(NULL)))


/* ---------------------------------------------------------------------- */
/*     Public methods                                                     */
/* ---------------------------------------------------------------------- */

EXTERN GenericTrans_ptr 
GenericTrans_create ARGS((const TransType trans_type));

EXTERN TransType GenericTrans_get_type ARGS((const GenericTrans_ptr self));


#endif /* __TRANS_GENERIC_GENERIC_TRANS_H__ */
