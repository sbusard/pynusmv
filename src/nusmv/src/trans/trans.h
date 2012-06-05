/**CHeaderFile*****************************************************************

  FileName    [trans.h]

  PackageName [trans]

  Synopsis    [ The public interface of the <tt>trans</tt> package.]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``trans'' package of NuSMV version 2. 
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

  Revision    [$Id: ]

******************************************************************************/

#ifndef __TRANS_H__
#define __TRANS_H__

#include "utils/utils.h" /* for EXTERN and ARGS */


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Type***********************************************************************

  Synopsis    [Use one of this id when creating a trans]

  Description [If you modify this type, also modify the corresponding 
  string definition in transTrans.c]

******************************************************************************/
typedef enum TransType_TAG { 
  TRANS_TYPE_INVALID = -1, 
  TRANS_TYPE_MONOLITHIC = 0, 
  TRANS_TYPE_THRESHOLD, 
  TRANS_TYPE_IWLS95 
} TransType; 

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/*  Public methods:                                                          */
/*---------------------------------------------------------------------------*/

EXTERN TransType TransType_from_string ARGS((const char* name));

EXTERN const char* TransType_to_string ARGS((const TransType self));



#endif /* __TRANS_H__ */
