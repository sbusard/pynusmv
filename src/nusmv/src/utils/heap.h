#ifndef _HEAP_H
#define _HEAP_H

#define HEAP_MAXLENGTH_INIT 31

typedef struct heap_ * heap;

heap heap_create();
void heap_destroy(heap h);
void heap_add(heap h, float val, void * el);
int heap_isempty(heap h);
void * heap_getmax(heap h);

#endif
