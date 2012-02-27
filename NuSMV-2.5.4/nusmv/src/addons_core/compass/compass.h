/**CHeaderFile*****************************************************************

  FileName    [compass.h]

  PackageName [addons.compass]

  Synopsis    [The header file of the <tt>compass</tt> addon.]

  Description [The <tt>compass</tt> implementation package]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compass'' package of NuSMV version 2. 
  Copyright (C) 2008 by FBK-irst. 

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

  Revision    [$Id: compass.h,v 1.1.2.7 2009-08-24 14:25:26 nusmv Exp $]

******************************************************************************/

#ifndef __COMPASS_H__
#define __COMPASS_H__

#include "utils/utils.h"
#include "enc/bdd/BddEnc.h"
#include "fsm/bdd/BddFsm.h"
#include "dd/dd.h"


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
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void Compass_init ARGS((void));
EXTERN void Compass_init_cmd ARGS((void));

EXTERN void Compass_reset ARGS((void));
EXTERN void Compass_quit ARGS((void));

EXTERN void Compass_write_sigref ARGS((FILE* file, BddFsm_ptr fsm, 
                                       NodeList_ptr probs_list, 
                                       Expr_ptr tau,
                                       NodeList_ptr ap_list,
                                       boolean do_indent));

EXTERN int 
Compass_write_language_sigref ARGS((BddEnc_ptr enc, FILE* file));

EXTERN int 
Compass_print_add_sigref_format ARGS((DdManager* dd, add_ptr add, FILE* file, 
                                      boolean do_indent));


#endif /* __COMPASS_H__ */
