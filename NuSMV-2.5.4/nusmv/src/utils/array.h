/**CHeaderFile*****************************************************************

  FileName    [array.h]

  PackageName [utils]

  Synopsis    [The header of the generic array manipulator.]

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

  Revision    [$Id: array.h,v 1.2.16.2.6.1 2007-06-18 15:23:45 nusmv Exp $]

******************************************************************************/
#ifndef ARRAY_H
#define ARRAY_H

/* Return value when memory allocation fails */
#define ARRAY_OUT_OF_MEM -10000


typedef struct array_t {
    char *space;
    int	 num;		/* number of array elements.		*/
    int	 n_size;	/* size of 'data' array (in objects)	*/
    int	 obj_size;	/* size of each array object.		*/
    int	 index;		/* combined index and locking flag.	*/
} array_t;

EXTERN array_t *array_do_alloc ARGS((int, int));
EXTERN array_t *array_dup ARGS((array_t *));
EXTERN array_t *array_join ARGS((array_t *, array_t *));
EXTERN void array_free ARGS((array_t *));
EXTERN int array_append ARGS((array_t *, array_t *));
EXTERN void array_sort ARGS((array_t *, int (*)()));
EXTERN void array_uniq ARGS((array_t *, int (*)(), void (*)()));
EXTERN int array_abort ARGS((const array_t *, int));
EXTERN int array_resize ARGS((array_t *, int));
EXTERN char *array_do_data ARGS((array_t *));

extern unsigned int array_global_index;
extern int array_global_insert;

/* allocates an array of 'number' elements of the type 'type' */
#define array_alloc(type, number)		\
    array_do_alloc(sizeof(type), number)

#define array_insert(type, a, i, datum)         \
    (  -(a)->index != sizeof(type) ? array_abort((a),4) : 0,\
        (a)->index = (i),\
        (a)->index < 0 ? array_abort((a),0) : 0,\
        (a)->index >= (a)->n_size ?\
	array_global_insert = array_resize(a, (a)->index + 1) : 0,\
        array_global_insert != ARRAY_OUT_OF_MEM ?\
        *((type *) ((a)->space + (a)->index * (a)->obj_size)) = datum : datum,\
        array_global_insert != ARRAY_OUT_OF_MEM ?\
        ((a)->index >= (a)->num ? (a)->num = (a)->index + 1 : 0) : 0,\
        array_global_insert != ARRAY_OUT_OF_MEM ?\
        ((a)->index = -(int)sizeof(type)) : ARRAY_OUT_OF_MEM )

#define array_insert_last(type, array, datum)	\
    array_insert(type, array, (array)->num, datum)

#define array_fetch(type, a, i)			\
    (array_global_index = (i),				\
      (array_global_index >= (a)->num) ? array_abort((a),1) : 0,\
      *((type *) ((a)->space + array_global_index * (a)->obj_size)))

#define array_fetch_p(type, a, i)                       \
    (array_global_index = (i),                             \
      (array_global_index >= (a)->num) ? array_abort((a),1) : 0,\
      ((type *) ((a)->space + array_global_index * (a)->obj_size)))

#define array_fetch_last(type, array)		\
    array_fetch(type, array, ((array)->num)-1)

#define array_fetch_last_p(type, array)		\
    array_fetch_p(type, array, ((array)->num)-1)

#define array_n(array)				\
    (array)->num

#define array_data(type, array)			\
    (type *) array_do_data(array)

#define arrayForEachItem(                                      \
  type,  /* type of object stored in array */                  \
  array, /* array to iterate */                                \
  i,     /* int, local variable for iterator */                \
  data   /* object of type */                                  \
)                                                              \
  for((i) = 0;                                                 \
      (((i) < array_n((array)))                                \
       && (((data) = array_fetch(type, (array), (i))), 1));    \
      (i)++)

#endif






