/**CHeaderFile*****************************************************************

  FileName    [ddInt.h]

  PackageName [dd]

  Synopsis    [Internal declaration of Decision Diagram interface Package]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``dd'' package of NuSMV version 2. 
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

  Revision    [$Id: ddInt.h,v 1.2.6.2.4.2.6.1 2006-10-31 13:39:30 nusmv Exp $]

******************************************************************************/

#ifndef _ddINT
#define _ddINT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "util.h"
#include "utils/utils.h"
#include "utils/array.h"
#include "utils/avl.h"
#include "node/node.h"
#include "cuddInt.h"
#include "utils/avl.h"
#include "st.h"
#include "dd/dd.h"
#include "dd/VarsHandler.h"
#include "utils/error.h"
#include "parser/symbols.h"
#include "cmd/cmd.h"
#include "opt/opt.h"

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern FILE* nusmv_stderr;
extern FILE* nusmv_stdout;
extern DdManager* dd_manager;

#endif /* _ddINT */
