# FileName	[ Makefile ]
#
# PackageName	[ NuSMV ]
#
# Synopsis	[ Package-wide Makefile ]
#
# Description	[ Makefile template for the NuSMV library ]
#
# Author	[ Roberto Cavada ]
#
# Copyright	[Copyright (C) 2004 by ITC-irst ]
#
# NuSMV version 2 is free software; you can redistribute it and/or 
# modify it under the terms of the GNU Lesser General Public 
# License as published by the Free Software Foundation; either 
# version 2 of the License, or (at your option) any later version.
#
# NuSMV version 2 is distributed in the hope that it will be useful, 
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public 
# License along with this library; if not, write to the Free Software 
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.
#
# For more information on NuSMV see <http://nusmv.fbk.eu>
# or email to <nusmv-users@fbk.eu>.
# Please report bugs to <nusmv-users@fbk.eu>.
#
# To contact the NuSMV development board, email to <nusmv@fbk.eu>.]
#
# Revision	[1.1.2.4 2003/08/04 11:29:51]

## parser must be the first package to be built

EXAMPLE_DIRS = abp example_irst prod-cons tcas guidance production-cell \
	   brp queue deadlock p-queue reactor example_cmu pci smv-dist \
	   msi bmc_tutorial m4 psl-samples

# ----------------------------------------------------------------------

bad_names = %/CVS %~ .%

example_files = $(filter-out $(bad_names), \
	$(foreach dir, $(EXAMPLE_DIRS), $(wildcard $(dir)/*)))


examplesdatadir =  $(datadir)/nusmv/examples
nobase_dist_examplesdata_DATA = $(example_files)

bad:
	echo $(example_files)