#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "pqheap.h"


/* setting this enables printing pq debug info */
//#define PQ_DEBUG if(1)
#define PQ_DEBUG if(0)


const int  PQINITSIZE = 256;


/* arrays in C start at 0.  For simplicity the heap structure is slightly modified as:
   0
   |
   1
   /\
  2  3
 /\  /\
4 5 6  7

*/

/* The children of an element of the heap. */
static inline unsigned int heap_lchild(unsigned int index) {
  return 2 * index;
}

static inline unsigned int heap_rchild(unsigned int index) {
  return 2 * index + 1;
}

/* The parent of an element. */
static inline unsigned int heap_parent(unsigned int index) {
  return index >> 1;
}


/* return minimum of two integers */
static unsigned int mymin(unsigned int a, unsigned int b) {
  return (a<=b)? a:b;
}


static void heapify(PQueue* pq, unsigned int root) {

  unsigned int min_index = root;
  unsigned int lc = heap_lchild(root);
  unsigned int rc = heap_rchild(root);

  assert(pq && pq->elements);
  if ((lc < pq->cursize) &&
      (compare_element(pq->elements[lc], pq->elements[min_index]) < 0)) {
    min_index = lc;
  }
  if ((rc < pq->cursize) && 
      (compare_element(pq->elements[rc], pq->elements[min_index]) < 0)) {
    min_index = rc;
  }
  
  if (min_index != root) {
    elemType tmp_q = pq->elements[min_index];
    
    pq->elements[min_index] = pq->elements[root];
    pq->elements[root] = tmp_q;
    
    heapify(pq, min_index);
  }
}   


static void PQ_grow(PQueue* pq) {

  elemType* elements;
  unsigned int i;
  PQ_DEBUG {
  printf("PQ: doubling size to %d\n",pq->maxsize*2); fflush(stdout);
  }
  
  assert(pq && pq->elements);
  pq->maxsize *= 2; 
  elements = (elemType*)malloc(pq->maxsize*sizeof(elemType));
  if (!elements) {
    printf("PQ_grow: could not reallocate priority queue: insufficient memory..\n");
    exit(1);
  }
  /* should use realloc..*/
  for (i=0; i<pq->cursize; i++) {
    elements[i] = pq->elements[i];
  }
  free(pq->elements);
  pq->elements = elements;
}



/**************************************************************/
/* create and initialize a pqueue and return it */ 
PQueue* PQ_initialize() {
  PQueue *pq; 

  PQ_DEBUG { printf("PQ-initialize: initializing heap\n"); fflush(stdout); }
  pq = (PQueue*)malloc(sizeof(PQueue));
  assert(pq);
  pq->elements = (elemType*)malloc(PQINITSIZE*sizeof(elemType));
  if (!pq->elements) {
    printf("PQ_initialize: could not allocate priority queue: insufficient memory..\n");
    exit(1);
  }
  assert(pq->elements);
  
  pq->maxsize = PQINITSIZE;
  pq->cursize = 0;
  return pq;
}

  


/************************************************************/
/* delete the pqueue and free its space */
void PQ_delete(PQueue* pq) { 

  printf("PQ-delete: deleting heap\n"); fflush(stdout);
  assert(pq && pq->elements); 
  if (pq->elements) free(pq->elements);
}
 

/************************************************************/
/* Is it empty? */
int  PQ_isEmpty(PQueue* pq) {
  assert(pq && pq->elements); 
  return (pq->cursize == 0);
}


/************************************************************/
/* Return the nb of elements currently in the queue */
unsigned int PQ_size(PQueue* pq) {
  assert(pq && pq->elements);
  return pq->cursize;
}



/************************************************************/
/* Set *elt to the min element in the queue; 
   return value: 1 if exists a min, 0 if not   */
int PQ_min(PQueue* pq, elemType* elt) {

  assert(pq && pq->elements); 
  if (!pq->cursize) {
    return 0;
  }
  *elt = pq->elements[0];
  return 1;
}




/************************************************************/
/* Set *elt to the min element in the queue and delete it from queue; 
   return value: 1 if exists a min, 0 if not   */
int PQ_extractMin(PQueue* pq, elemType* elt) {

  assert(pq && pq->elements);
  if (!pq->cursize) {
    return 0;
  }
  *elt = pq->elements[0];
  pq->elements[0] = pq->elements[--pq->cursize];
  heapify(pq, 0);

  PQ_DEBUG {printf("PQ_extractMin: "); printElem(*elt); printf("\n"); fflush(stdout);}
  return 1;
}


/************************************************************/
/* Delete the min element; same as PQ_extractMin, but ignore the value extracted;
   return value: 1 if exists a min, 0 if not  */
int  PQ_deleteMin(PQueue* pq) {
    
  assert(pq && pq->elements); 
  elemType dummy;
  return PQ_extractMin(pq, &dummy);
}



/************************************************************/
/* Insert */
void PQ_insert(PQueue* pq, elemType elt) {
  
  unsigned int ii;
  assert(pq && pq->elements); 
 
  PQ_DEBUG {printf("PQ_insert: "); printElem(elt); printf("\n"); fflush(stdout);}
  if (pq->cursize==pq->maxsize) {
    PQ_grow(pq);
  }
  assert(pq->cursize < pq->maxsize);
  for (ii = pq->cursize++;
       ii && (compare_element(pq->elements[heap_parent(ii)], elt) > 0);
       ii = heap_parent(ii)) {
    pq->elements[ii] = pq->elements[heap_parent(ii)];
  }
  pq->elements[ii] = elt;
}                                       



/************************************************************/
/* Delete the min element and insert the new item x; by doing a delete
   and an insert together you can save a heapify() call */
void PQ_deleteMinAndInsert(PQueue* pq, elemType elt) {
  
  assert(pq && pq->elements); 
   PQ_DEBUG {printf("PQ_deleteMinAndinsert: "); printElem(elt); printf("\n"); fflush(stdout);}
  pq->elements[0] = elt;
  heapify(pq, 0);
}



/************************************************************/
/* print the elements in the queue */
void PQ_print(PQueue* pq) {
  printf("PQ: "); fflush(stdout);
  unsigned int i;
  for (i=0; i< mymin(10, pq->cursize); i++) {
    printElem(pq->elements[i]);
  }
  printf("\n");fflush(stdout);
}
   


