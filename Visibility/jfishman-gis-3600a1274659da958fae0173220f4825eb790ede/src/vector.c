
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.


/**
 * Jeremy Fishman's Lexer program -- vector.c
 *
 * Source for a simple Vector operations.  Vectors are dynamically
 * allocated scalable containers. They can hold any type of object, so
 * long aas the size of the elements being stored are constant, and are
 * specified on initialization.
 */

#ifdef __APPLE__
#  include <stdlib.h>
#else
#  include "malloc.h"
#endif
#include "vector.h"

// The default capacity for newly-initialized Vectors
static vector_size_t DEF_CAPACITY = 8;

/**
** Allocate a new Vector of elements of the given data size, with
** default initial capacity.
**/
Vector *
vinit(vector_data_size_t _data_size) {
  return vinit2(_data_size, DEF_CAPACITY);
}

/**
** Allocate a new vector of elements of the given data size, with the
** given initial capacity.
**/
Vector *
vinit2(vector_data_size_t _data_size, vector_size_t _capacity) {
  Vector *v;
  assert(_capacity >= 0);
  assert(_capacity < 1000);

  v = (Vector*)malloc(sizeof(Vector));
  assert(v);
  v->length = 0;
  v->capacity = _capacity;
  v->data_size = _data_size;
  v->data = (vector_data_t)calloc(_capacity,  _data_size);
  assert(v->data);
  return v;
}

/**
** Ensures that the given Vector has enough memory allocated to hold
** the given number of elements.  Will dynamically reallocate if the
** capacity is not large enough.
**/
void 
vensure_capacity(Vector *v, vector_size_t _capacity) {
  vector_data_t result;
  assert(v); assert(v->data);
  assert(_capacity > 0);

  if (v->capacity >= _capacity)
    return;
  if (v->capacity == 0)
    v->capacity = 1;
  while (v->capacity <= _capacity)
    v->capacity *= 2;
  result = realloc(v->data, v->capacity * v->data_size);
  assert(result);
  v->data = result;
}

/**
 * Ensures that the given Vector takes no more memory than the smallest power
 * of 2 greater than the current length.
 */
void 
vless_capacity(Vector *v, vector_size_t _capacity) {
  vector_data_t result;
  assert(v); assert(v->data);
  assert(_capacity > 0);

  if (_capacity < v->capacity / 2 && v->capacity > 1) {
    v->capacity /= 2;
    result = realloc(v->data, v->capacity * v->data_size);
    assert(result);
    v->data = result;
  }
}

/**
 * Append the given element onto the Vector.  When capacity is reached,
 * the Vector will automatically grow exponentially, so this method will
 * acheive O(n) running time over linear appends in the long run.
 */
void 
vappend(Vector *v, const vector_data_t e) {
  assert(v); assert(v->data);
  assert(e);

  vensure_capacity(v, v->length + 1);
  memcpy((v->data + v->length * v->data_size), e, v->data_size);
  v->length++;
}

/**
** Appends all the elements of the second Vector to the first.
**/
void
vappend_vector(Vector *v1, const Vector *v2) {
  assert(v1); assert(v1->data);
  assert(v2); assert(v2->data);
  assert(v1->data_size == v2->data_size);

  if (v2->length == 0)
    return;
  vensure_capacity(v1, v1->length + v2->length);
  memcpy(v1->data + v1->length * v1->data_size, v2->data,
         v2->length * v1->data_size);
  v1->length += v2->length;
}

/**
** Retrieves a pointer to the element stored at the given index in the
** Vector.
**/
vector_data_t 
vget(const Vector *v, vector_size_t index) {
  assert(v); assert(v->data);
  assert(index >= 0 && index < v->length);

  return v->data + index*v->data_size;
}

/**
** Sets the element at the given index of the given Vector to the given element.
**/
void
vset(const Vector *v, vector_size_t index, const vector_data_t e) {
  assert(v); assert(v->data);
  assert(e);
  assert(index >= 0 && index < v->length);

  memmove(v->data + index * v->data_size, e, v->data_size);
}

/**
** Locates and retrieves the index of the first occurance of the given
** element in the given Vector, or the length of the vector if the element
** isn't in the Vector.
**/
vector_size_t
vindex(const Vector *v, const vector_data_t e) {
  vector_data_t ptr, end;
  assert(v); assert(v->data);
  assert(e);

  ptr = v->data;
  end = v->data + v->length * v->data_size;
  while (ptr < end) {
    if (memcmp(ptr, e, v->data_size) == 0)
      return (ptr - v->data) / v->data_size;
    ptr += v->data_size;
  }
  return v->length;
}

/**
** Removes the element at the given index from the Vector.  If
** sufficient space has been freed from successive calls to vremove() or
** vpop(), the Vector will shrink itself to avoid clogging space.
**/
void 
vremove(Vector *v, vector_size_t index) {
  assert(v); assert(v->data);
  assert(index >= 0 && index < v->length);

  // move data over removed item
  if (index < v->length - 1)
    memmove(v->data + index * v->data_size,
            v->data + (index + 1) * v->data_size,
            (v->length - index - 1) * v->data_size);
  v->length--;
  // decrease capacity, if needed
  vless_capacity(v, v->length);
}

/**
 * Removes the element at the index (v->length - 1) from the Vector.  If
 * sufficient space has been freed from successive calls to vremove() or
 * vpop(), the Vector will shrink itself to avoid clogging space.  This is done
 * before the element is removed, in order to maintain the data where the item
 * is stored.  Make sure to access the data in the vector_data_t pointer only
 * BEFORE any additional calls to vpop() or vremove().
 */
vector_data_t
vpop(Vector *v) {
  assert(v); assert(v->data);
  assert(v->length > 0);

  // shrink vector if capacity is more than double the length
  vless_capacity(v, v->length);
  // no need to move data, it's at the end
  v->length--;
  return v->data + v->length * v->data_size;
}

/**
** Free the memory allocated for the given Vecotr, including the
** Vector's data, as well as the Vector itself.
**/
void
vfree(Vector *v) {
  if (v->data)
    free(v->data);
  free(v);
}

