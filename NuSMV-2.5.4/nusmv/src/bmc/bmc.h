/**CHeaderFile*****************************************************************

  FileName    [bmc.h]

  PackageName [bmc]

  Synopsis    [The header file for the <tt>bmc</tt> package]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by FBK-irst and University of Trento.

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

  Revision    [$Id: bmc.h,v 1.22.4.2.2.1.2.3.6.5 2010-01-18 14:58:30 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_H
#define _BMC_H

#include "utils/utils.h" /* for EXTERN and ARGS */

/* all BMC modules: */
#include "bmcCmd.h"
#include "bmcBmc.h"
#include "bmcPkg.h"

#include "bmcGen.h"
#include "bmcDump.h"
#include "bmcTableau.h"
#include "bmcModel.h"
#include "bmcConv.h"
#include "bmcCheck.h"
#include "bmcUtils.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* BMC Option names */
#define BMC_OPT_INITIALIZED "__bmc_opt_initialized__"

#define BMC_MODE          "bmc_mode"
#define BMC_DIMACS_FILENAME "bmc_dimacs_filename"
#define BMC_INVAR_DIMACS_FILENAME "bmc_invar_dimacs_filename"
#define BMC_PB_LENGTH      "bmc_length"
#define BMC_PB_LOOP        "bmc_loopback"
#define BMC_INVAR_ALG        "bmc_invar_alg"

#if NUSMV_HAVE_INCREMENTAL_SAT
#define BMC_INC_INVAR_ALG        "bmc_inc_invar_alg"
#endif

#define BMC_OPTIMIZED_TABLEAU "bmc_optimized_tableau"
#define BMC_FORCE_PLTL_TABLEAU "bmc_force_pltl_tableau"
#define BMC_SBMC_IL_OPT "bmc_sbmc_il_opt"
#define BMC_SBMC_GF_FG_OPT "bmc_sbmc_gf_fg_opt"
#define BMC_SBMC_CACHE_OPT "bmc_sbmc_cache_opt"


/**Constant********************************************************************

  Synopsis [The names for INVAR solving algorithms (incremental and
  non-incremental).]

  Description        []

  SeeAlso            []

******************************************************************************/
#define BMC_INVAR_ALG_CLASSIC       "classic"
#define BMC_INVAR_ALG_EEN_SORENSSON "een-sorensson"
#define BMC_INVAR_ALG_FALSIFICATION "falsification"
#define BMC_INC_INVAR_ALG_DUAL      "dual"
#define BMC_INC_INVAR_ALG_ZIGZAG    "zigzag"
#define BMC_INC_INVAR_ALG_FALSIFICATION "falsification"
#define BMC_INC_INVAR_ALG_INTERP_SEQ "interp_seq"
#define BMC_INC_INVAR_ALG_INTERPOLANTS "interpolants"

/**Constant********************************************************************

  Synopsis           [The names for INVAR closure strategies.]

  Description        [Currently this applies to DUAL algorithm only]

  SeeAlso            []

******************************************************************************/
#define BMC_INVAR_BACKWARD "backward"
#define BMC_INVAR_FORWARD "forward"

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
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/


/* BMC Options */
EXTERN void    set_bmc_mode ARGS((OptsHandler_ptr));
EXTERN void    unset_bmc_mode ARGS((OptsHandler_ptr));
EXTERN boolean opt_bmc_mode ARGS((OptsHandler_ptr));
EXTERN char* get_bmc_dimacs_filename ARGS((OptsHandler_ptr));
EXTERN void set_bmc_dimacs_filename ARGS((OptsHandler_ptr, char *));
EXTERN char* get_bmc_invar_dimacs_filename ARGS((OptsHandler_ptr));
EXTERN void set_bmc_invar_dimacs_filename ARGS((OptsHandler_ptr, char *));
EXTERN void set_bmc_pb_length ARGS((OptsHandler_ptr opt, const int k));
EXTERN int get_bmc_pb_length ARGS((OptsHandler_ptr));
EXTERN void set_bmc_pb_loop  ARGS((OptsHandler_ptr opt, const char* loop));
EXTERN const char* get_bmc_pb_loop ARGS((OptsHandler_ptr));
EXTERN void set_bmc_invar_alg ARGS((OptsHandler_ptr opt, const char* loop));
EXTERN const char* get_bmc_invar_alg ARGS((OptsHandler_ptr));
#if NUSMV_HAVE_INCREMENTAL_SAT
EXTERN void set_bmc_inc_invar_alg ARGS((OptsHandler_ptr opt, const char* loop));
EXTERN const char* get_bmc_inc_invar_alg ARGS((OptsHandler_ptr));
#endif
EXTERN void set_bmc_optimized_tableau ARGS((OptsHandler_ptr));
EXTERN void unset_bmc_optimized_tableau ARGS((OptsHandler_ptr));
EXTERN boolean opt_bmc_optimized_tableau ARGS((OptsHandler_ptr));
EXTERN void    set_bmc_force_pltl_tableau   ARGS((OptsHandler_ptr));
EXTERN void    unset_bmc_force_pltl_tableau ARGS((OptsHandler_ptr));
EXTERN boolean opt_bmc_force_pltl_tableau   ARGS((OptsHandler_ptr));


/* SBMC Options */
EXTERN void set_bmc_sbmc_gf_fg_opt ARGS((OptsHandler_ptr opt));
EXTERN void unset_bmc_sbmc_gf_fg_opt ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_bmc_sbmc_gf_fg_opt ARGS((OptsHandler_ptr opt));
EXTERN void set_bmc_sbmc_il_opt ARGS((OptsHandler_ptr opt));
EXTERN void unset_bmc_sbmc_il_opt ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_bmc_sbmc_il_opt ARGS((OptsHandler_ptr opt));
EXTERN void set_bmc_sbmc_cache ARGS((OptsHandler_ptr opt));
EXTERN void unset_bmc_sbmc_cache ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_bmc_sbmc_cache ARGS((OptsHandler_ptr opt));


/**AutomaticEnd***************************************************************/

#endif /* _BMC_H */
