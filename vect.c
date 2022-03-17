/**
 * Vector implementation.
 *
 * - Implement each of the functions to create a working growable array (vector).
 * - Do not change any of the structs
 * - When submitting, You should not have any 'printf' statements in your vector 
 *   functions.
 *
 * IMPORTANT: The initial capacity and the vector's growth factor should be 
 * expressed in terms of the configuration constants in vect.h
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vect.h"

/** Main data structure for the vector. */
struct vect {
  char **data;             /* Array containing the actual data. */
  unsigned int size;       /* Number of items currently in the vector. */
  unsigned int capacity;   /* Maximum number of items the vector can hold before growing. */
};

/** Construct a new empty vector. */
vect_t *vect_new() {
  // allocate memory for new vector, set initial capacity, size, and
  // memory for data
  vect_t *v = malloc(sizeof(vect_t));
  v->data = malloc(VECT_INITIAL_CAPACITY * sizeof(char*));
  v->size = 0;
  v->capacity = VECT_INITIAL_CAPACITY;
  
  return v;
}

/** Delete the vector, freeing all memory it occupies. */
void vect_delete(vect_t *v) {
  assert(v != NULL);
  
  for (int i = 0; i < v->size; i++) {
    free(v->data[i]);
  }
  
  free(v->data);
  free(v);
}

/** Get the element at the given index. */
const char *vect_get(vect_t *v, unsigned int idx) {
  assert(v != NULL);
  assert(idx < v->size);
  
  return v->data[idx];
}

/** Get a copy of the element at the given index. The caller is responsible
 *  for freeing the memory occupied by the copy. */
char *vect_get_copy(vect_t *v, unsigned int idx) {
  assert(v != NULL);
  assert(idx < v->size);

  char *elem_copy = malloc(strlen(v->data[idx]) + 1);
  strncpy(elem_copy, v->data[idx], strlen(v->data[idx]) + 1);
  
  return elem_copy;
}

/** Set the element at the given index. */
void vect_set(vect_t *v, unsigned int idx, const char *elt) {
  assert(v != NULL);
  assert(idx < v->size);

  // free and set the given element at the given index
  free(v->data[idx]);
  v->data[idx] = malloc(strlen(elt) + 1);
  strncpy(v->data[idx], elt, strlen(elt) + 1);
}

/** Add an element to the back of the vector. */
void vect_add(vect_t *v, const char *elt) {
  assert(v != NULL);
  
  // check to see if current vector would need to be resized
  // resize if necessary
  if (v->size >= v->capacity) {
    v->capacity = v->capacity * VECT_GROWTH_FACTOR;
    v->data = realloc(v->data, v->capacity * sizeof(char*));
  }

  v->data[v->size] = malloc(strlen(elt) + 1);
  strncpy(v->data[v->size], elt, strlen(elt) + 1);
  v->size++;
}

/** Remove the last element from the vector. */
void vect_remove_last(vect_t *v) {
  assert(v != NULL);
  
  free(v->data[v->size - 1]);
  v->size--;
}

/** The number of items currently in the vector. */
unsigned int vect_size(vect_t *v) {
  assert(v != NULL);
  
  return v->size;
}

/** The maximum number of items the vector can hold before it has to grow. */
unsigned int vect_current_capacity(vect_t *v) {
  assert(v != NULL);
  
  return v->capacity;
}