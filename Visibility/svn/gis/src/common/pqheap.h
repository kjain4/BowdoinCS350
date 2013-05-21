#ifndef _PQHEAP_H
#define _PQHEAP_H


#include "visevent.h"

/* the definition of a pqueue element */
typedef SweepEvent elemType;
#define compare_element compare_SweepEvent
#define getPriority(e) ((e).tangle)
#define printElem(e)   (printf("[%.3f, %.3f, %d, (%d,%d)] ", (e).angle, \
                        (e).dist, (e).type, (e).p.r, (e).p.c))



/**************************************************************
Priority queue of elements of type elemType.  

elemType assumed to have defined a function getPriority(elemType x)
***************************************************************/


typedef struct {
  /* A pointer to an array of elements */
  elemType* elements;
  
  /* The number of elements currently in the queue */
  unsigned int cursize;
  
  /* The maximum number of elements the queue can currently hold */
  unsigned int maxsize;

} PQueue; 



/* create and initialize a pqueue and return it */ 
PQueue* PQ_initialize(void);
  
/* delete the pqueue and free its space */
void PQ_delete(PQueue* pq); 

/* Is it empty? */
int  PQ_isEmpty(PQueue* pq);

/* Return the nb of elements currently in the queue */
unsigned int PQ_size(PQueue* pq);

/* Set *elt to the min element in the queue   */
int PQ_min(PQueue* pq, elemType* elt);

/* Set *elt to the min element in the queue and delete it from queue  */
int PQ_extractMin(PQueue* pq, elemType* elt);

/* Delete the min element; same as PQ_extractMin, but ignore the value extracted */
int  PQ_deleteMin(PQueue* pq);

/* Insert */
void PQ_insert(PQueue* pq, elemType elt);


/* Delete the min element and insert the new item x; by doing a delete
   and an insert together you can save a heapify() call */
void PQ_deleteMinAndInsert(PQueue* pq, elemType x);

/* print the elements in the queue */
void PQ_print(PQueue* pq);




#endif // _PQUEUE_HEAP_H 
