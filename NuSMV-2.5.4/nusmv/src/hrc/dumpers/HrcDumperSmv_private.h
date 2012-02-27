/**CHeaderFile*****************************************************************

  FileName    [HrcDumperSmv_private.h]

  PackageName [hrc.dumpers]

  Synopsis    [Private and protected interface of class 'HrcDumperSmv']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [HrcDumperSmv.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``hrc.dumpers'' package of NuSMV version 2. 
  Copyright (C) 2011 by FBK-irst. 

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

  Revision    [$Id: $]

******************************************************************************/


#ifndef __HRC_DUMPER_SMV_PRIVATE_H__
#define __HRC_DUMPER_SMV_PRIVATE_H__


#include "HrcDumperSmv.h" 
#include "HrcDumper.h"
#include "HrcDumper_private.h"

#include "utils/utils.h" 


/**Struct**********************************************************************

  Synopsis    [HrcDumperSmv class definition derived from
               class HrcDumper]

  Description []

  SeeAlso     [Base class HrcDumper]   
  
******************************************************************************/
typedef struct HrcDumperSmv_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(HrcDumper); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */


  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

} HrcDumperSmv;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only        */
/* ---------------------------------------------------------------------- */
EXTERN void hrc_dumper_smv_init ARGS((HrcDumperSmv_ptr self, FILE* fout));
EXTERN void hrc_dumper_smv_deinit ARGS((HrcDumperSmv_ptr self));

EXTERN void hrc_dumper_smv_dump_snippet ARGS((HrcDumper_ptr self,
                                              HrcDumperSnippet snippet,
                                              const HrcDumperInfo* info));

EXTERN void hrc_dumper_smv_dump_comment ARGS((HrcDumper_ptr self, 
                                              const char* msg));

EXTERN void hrc_dumper_smv_dump_header ARGS((HrcDumper_ptr self, 
                                             const char* msg));


#endif /* __HRC_DUMPER_SMV_PRIVATE_H__ */
