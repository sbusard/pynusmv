/**CHeaderFile*****************************************************************

  FileName    [avl.h]

  PackageName [utils]

  Synopsis    [AVL Trees]

  Author      [Originated from glu library of VIS]

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

  Revision    [$Id: avl.h,v 1.2.6.1.4.2 2005-11-16 12:04:45 nusmv Exp $]

******************************************************************************/
#ifndef AVL_INCLUDED
#define AVL_INCLUDED


#include "util.h"

typedef struct avl_node_struct avl_node;
struct avl_node_struct {
    avl_node *left, *right;
    char *key;
    char *value;
    int height;
};


typedef struct avl_tree_struct avl_tree;
struct avl_tree_struct {
    avl_node *root;
    int (*compar)();
    int num_entries;
    int modified;
};


typedef struct avl_generator_struct avl_generator;
struct avl_generator_struct {
    avl_tree *tree;
    avl_node **nodelist;
    int count;
};


#define AVL_FORWARD 	0
#define AVL_BACKWARD 	1


EXTERN avl_tree *avl_init_table ARGS((int (*)()));
EXTERN int avl_delete ARGS((avl_tree *, char **, char **));
EXTERN int avl_insert ARGS((avl_tree *, char *, char *));
EXTERN int avl_lookup ARGS((avl_tree *, char *, char **));
EXTERN int avl_first ARGS((avl_tree *, char **, char **));
EXTERN int avl_last ARGS((avl_tree *, char **, char **));
EXTERN int avl_find_or_add ARGS((avl_tree *, char *, char ***));
EXTERN int avl_count ARGS((avl_tree *));
EXTERN int avl_numcmp ARGS((char *, char *));
EXTERN int avl_gen ARGS((avl_generator *, char **, char **));
EXTERN void avl_foreach ARGS((avl_tree *, void (*)(), int));
EXTERN void avl_free_table ARGS((avl_tree *, void (*)(), void (*)()));
EXTERN void avl_free_gen ARGS((avl_generator *));
EXTERN avl_generator *avl_init_gen ARGS((avl_tree *, int));

#define avl_is_member(tree, key)	avl_lookup(tree, key, (char **) 0)

#define avl_foreach_item(table, gen, dir, key_p, value_p) 	\
    for(gen = avl_init_gen(table, dir); 			\
	    avl_gen(gen, key_p, value_p) || (avl_free_gen(gen),0);)

#endif
