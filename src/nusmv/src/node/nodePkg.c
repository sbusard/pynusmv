/**CFile***********************************************************************

  FileName    [nodePkg.c]

  PackageName [node]

  Synopsis    [Initialization and deinitialization for package node and 
  subpackages]

  Description [Initialization and deinitialization for package node and 
  subpackages]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``node'' package of NuSMV version 2. 
  Copyright (C) 2006 by FBK-irst.

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

#include "node.h"
#include "nodeInt.h" 

#include "node/printers/MasterPrinter.h"
#include "node/printers/PrinterWffCore.h"
#include "node/printers/PrinterIWffCore.h"
#include "node/printers/PrinterPsl.h"
#include "node/printers/PrinterSexpCore.h"

#include "node/normalizers/MasterNormalizer.h"
#include "node/normalizers/NormalizerBase.h"
#include "node/normalizers/NormalizerCore.h"
#include "node/normalizers/NormalizerPsl.h"

#include "utils/error.h" /* for CATCH */


static char rcsid[] UTIL_UNUSED = "$Id: nodePkg.c,v 1.1.2.4.4.2 2009-03-23 18:13:22 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

  Synopsis    [The node master printers]

  Description [Formerly there was only one global Master Printer. This
  is called master_wff_printer. Now another Master Printer is
  available (master_iwff_printer). This is used to provide support to
  the newer, indenting printout functions.]

******************************************************************************/
MasterPrinter_ptr master_wff_printer = MASTER_PRINTER(NULL);
MasterPrinter_ptr master_iwff_printer = MASTER_PRINTER(NULL);
MasterPrinter_ptr master_sexp_printer = MASTER_PRINTER(NULL);


MasterNormalizer_ptr master_normalizer = MASTER_NORMALIZER(NULL);

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the node package]

  Description        [Creates master and printers, and initializes the node 
  structures]

  SideEffects        []

  SeeAlso            [node_pkg_quit]

******************************************************************************/
void node_pkg_init()
{
  node_init();

  if (master_wff_printer == MASTER_PRINTER(NULL)) {

    /* Core printer (legacy) */
    master_wff_printer = MasterPrinter_create();

    CATCH {

      PrinterBase_ptr printer;
      printer = PRINTER_BASE(PrinterWffCore_create("Core Wff Printer"));
      MasterNodeWalker_register_walker(
                                       MASTER_NODE_WALKER(master_wff_printer), 
                                       NODE_WALKER(printer));

      /* printer for PSL: */
      printer = PRINTER_BASE(PrinterPsl_create("PSL Printer"));
      MasterNodeWalker_register_walker(
                                       MASTER_NODE_WALKER(master_wff_printer), 
                                       NODE_WALKER(printer));
    }

    FAIL {
      node_pkg_quit();
      nusmv_exit(1);
    }   
  }

  if (master_iwff_printer == MASTER_PRINTER(NULL)) {

    /* Core printer (indenting) */
    master_iwff_printer = MasterPrinter_create();

    CATCH {

      PrinterBase_ptr printer;
      printer = PRINTER_BASE(PrinterIWffCore_create("Core IWff Printer"));
      MasterNodeWalker_register_walker(
                                       MASTER_NODE_WALKER(master_iwff_printer), 
                                       NODE_WALKER(printer));
      /* printer for PSL: */
      printer = PRINTER_BASE(PrinterPsl_create("PSL Printer"));
      MasterNodeWalker_register_walker(
                                       MASTER_NODE_WALKER(master_iwff_printer), 
                                       NODE_WALKER(printer));
    }

    FAIL {
      node_pkg_quit();
      nusmv_exit(1);
    }
  }

  if (master_sexp_printer == MASTER_PRINTER(NULL)) {

    /* Core printer (sexp) */
    master_sexp_printer = MasterPrinter_create();

    CATCH {

      PrinterBase_ptr printer;
      printer = PRINTER_BASE(PrinterSexpCore_create("Core Sexp Printer"));
      MasterNodeWalker_register_walker(
                                       MASTER_NODE_WALKER(master_sexp_printer), 
                                       NODE_WALKER(printer));
    }

    FAIL {
      node_pkg_quit();
      nusmv_exit(1);
    }
  }

  if (master_normalizer == MASTER_NORMALIZER(NULL)) {
    master_normalizer = MasterNormalizer_create();

    CATCH {

      NormalizerBase_ptr normalizer;
      normalizer = NORMALIZER_BASE(NormalizerCore_create("Core Normalizer"));
      MasterNodeWalker_register_walker(
                                       MASTER_NODE_WALKER(master_normalizer),
                                       NODE_WALKER(normalizer));

      normalizer = NORMALIZER_BASE(NormalizerPsl_create("Psl Normalizer"));
      MasterNodeWalker_register_walker(
                                       MASTER_NODE_WALKER(master_normalizer),
                                       NODE_WALKER(normalizer));
    }

    FAIL {
      node_pkg_quit();
      nusmv_exit(1);
    }
  }
}


/**Function********************************************************************

  Synopsis [Deinitializes the packages, finalizing all internal
  structures]

  Description        []

  SideEffects        []

  SeeAlso            [node_pkg_init]

******************************************************************************/
void node_pkg_quit()
{
  if (master_wff_printer != MASTER_PRINTER(NULL)) {
    MasterNodeWalker_destroy(MASTER_NODE_WALKER(master_wff_printer));
    master_wff_printer = MASTER_PRINTER(NULL);
  }

  if (master_iwff_printer != MASTER_PRINTER(NULL)) {
    MasterNodeWalker_destroy(MASTER_NODE_WALKER(master_iwff_printer));
    master_iwff_printer = MASTER_PRINTER(NULL);
  }

  if (master_sexp_printer != MASTER_PRINTER(NULL)) {
    MasterNodeWalker_destroy(MASTER_NODE_WALKER(master_sexp_printer));
    master_sexp_printer = MASTER_PRINTER(NULL);
  }

  if (master_normalizer != MASTER_NORMALIZER(NULL)) {
    MasterNodeWalker_destroy(MASTER_NODE_WALKER(master_normalizer));
    master_normalizer = MASTER_NORMALIZER(NULL);
  }

  node_quit();
}

/**Function********************************************************************

  Synopsis           [Returns the global master normalizer]

  Description        [Returns the global master normalizer]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
MasterNormalizer_ptr node_pkg_get_global_master_normalizer()
{
  nusmv_assert(master_normalizer != MASTER_NORMALIZER(NULL)); /* initialized */
  return master_normalizer;
}

/**Function********************************************************************

  Synopsis           [Returns the global master wff printer]

  Description        [Returns the global master wff printer.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
MasterPrinter_ptr node_pkg_get_global_master_wff_printer()
{
  nusmv_assert(master_wff_printer != MASTER_PRINTER(NULL)); /* initialized */
  return master_wff_printer;
}

/**Function********************************************************************

  Synopsis           [Returns the global master wff printer]

  Description        [Returns the global master wff printer.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
MasterPrinter_ptr node_pkg_get_global_master_sexp_printer()
{
  nusmv_assert(master_sexp_printer != MASTER_PRINTER(NULL)); /* initialized */
  return master_sexp_printer;
}

/**Function********************************************************************

  Synopsis           [Returns the indenting master wff printer]

  Description        [Returns the indenting master wff printer.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
MasterPrinter_ptr node_pkg_get_indenting_master_wff_printer()
{
  nusmv_assert(master_iwff_printer != MASTER_PRINTER(NULL)); /* initialized */
  return master_iwff_printer;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

