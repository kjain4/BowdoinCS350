
/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * Jeremy Fishman's Lexer program -- vector.h
 *
 *  Contains header information defining the vector_size_t, vector_data_size_t,
 *  vector_data_t, and Vector (struct vector_t) types, as well as all functions
 *  on or initializing vector.
 *
 *  Vectors are dynamically-allocated array data structures.  I have written
 *  these vector operations to make the rest of the code easier.
 *
 *  author: Jeremy Fishman
 *  date: February 2, 2007
 */

#ifndef VECTOR_H
#  define VECTOR_H 0

#include <string.h>
#include "misc.h"

/*  Vector types  */

/* void pointer holds any data */
typedef void* vector_data_t;
/* size of the data type being stored */
#ifndef vector_data_size_t
typedef unsigned short vector_data_size_t;
#endif
#ifndef vector_size_t
typedef unsigned int vector_size_t;
#endif
/* structure to hold data information and array pointer */
typedef struct vector_t {
  vector_size_t length, capacity;
  vector_data_size_t data_size;
  vector_data_t data;
} Vector;

/*  Vector initializers  */

/* initialize a vector to hold the given size type data, using the default
   initial capacity */
Vector * vinit(vector_data_size_t _data_size);
/* same, but use the given initial capacity */
Vector * vinit2(vector_data_size_t _data_size, vector_size_t _capacity);

/*  Vector operations  */

/* ensure that the given Vector has room for at least the given number of
   elements */
void vensure_capacity(Vector *v, vector_size_t _capacity);
/* append the given element to the end of the Vector.  vector will
   automatically self-scale it capacity if needed. */
void vappend(Vector *v, const vector_data_t e);
/* append all the items in the given Vector to the first one.  Vectors must be
   holding elements of the same size. */
void vappend_vector(Vector *v1, const Vector *v2);
/* retrive a pointer to the item at the given index in the Vector. */
vector_data_t vget(const Vector *v, vector_size_t index);
/* set the value of the given element index to the value at the given
   pointer */
void vset(const Vector *v, vector_size_t i, const vector_data_t e);
/* check whether there are any elements in the given Vector that bitwise
   compare equal to the passsed element */
vector_size_t vindex(const Vector *v, const vector_data_t e);
/* remove and return the item at the end of the vector.  like vremove, will
   scale-down the vector size as elements are removed */
vector_data_t vpop(Vector *v);
/* remove the element at the given index from the Vector.  will automatically
   scale-down the vector when there is too much allocated space. */
void vremove(Vector *v, vector_size_t index);

/* Free the memory allocated for the given vector */
void vfree(Vector *v);

/* Empty all items from the vector.  Simply sets vector length to 0. */
#define vclear(v) (v->length = 0)

/**
 * A macro to print out the elements of a vector.  Takes parameters of
 * the vector, a temporary integer variable, the start index, the number
 * of elements to print, the type of the elements, and a format string
 * for how to display the elements
 */
#define VPRINTF(v,var,start,amount,t,fmt) { \
  var = start; \
  printf("["); \
  while (var < start + amount && var < v->length ) { \
    ASSERT(vget(v, var) != NULL, "vector element is null") \
    printf(fmt, *(t*) vget(v, var++)); \
    if (var != start + amount && var != v->length) \
      printf(","); \
  } \
  printf("]"); \
}

/**
 * A macro to print out the elements of a vector.  Takes parameters of
 * the vector, a temporary integer variable, the start index, the number
 * of elements to print, the type of the elements, and a function that
 * takes a pointer to the given type and prints out the element however
 * it should be printed.
 */
#define VPRINTFNC(v,var,start,amount,fnc, sep) { \
  var = start; \
  printf("["); \
  ASSERT(v->data != NULL, "vector is null"); \
  while (var < start + amount && var <  v->length) { \
    ASSERT(vget(v, var) != NULL, "vector element is null") \
    fnc(vget(v, var++)); \
    if (var != start + amount && var != v->length ) \
      printf(sep); \
  } \
  printf("]"); \
}


#endif  /* VECTOR_H */

