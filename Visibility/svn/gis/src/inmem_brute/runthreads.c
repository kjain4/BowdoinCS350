
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

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include "runthreads.h"
#include "misc.h"

// useful struct for the circle_threads() and run_on_semaphore() routines
typedef struct subthread_data_t {
  pthread_t thread;
  sem_t *sem;
  void* (*func)(void*);
  void *closure;
} subthread_t;
// function that using this struct to call the contained function with the
// contained second closure, and posts on the contained semaphore when complete
void* run_on_semaphore(void *_closure);

/**
 * Run several threads on input closures and wait for them to finish.
 *
 * For each of n thread instances, calls the function with the corresponding
 * closure supplied in a vector.  The closures vector may be NULL, in which
 * case the function will be passed NULL.  Only the first n closures will be
 * used, and exactly n thread will have run and exited before this method
 * returns.
 */
void run_threads(int n, void* (*func) (void*) , Vector *closures)
{
  pthread_t *threads;
  pthread_attr_t attr;
  int i, result;

  // at least one thread, no more than reasonable
  assert(n > 0);
  assert(n <= 32);
  // at least enough closures
  assert(closures == NULL || closures->length >= n);

  // allocate and initialize thread array and thread attributes
  threads = (pthread_t*) malloc(n * sizeof(pthread_t));
  assert(threads);
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);

  // start threads
  i = 0;
  while (i < n) {
    if (closures == NULL)
      result = pthread_create(threads + i, &attr, func, NULL);
    else
      result = pthread_create(threads + i, &attr, func, vget(closures, i));

    assert(result == 0);
    i++;
  }

  // wait for threads to complete
  i = 0;
  while (i < n) {
    result = pthread_join(*(threads + i), NULL);
    assert(result == 0);
    i++;
  }

  // garbage collect
  pthread_attr_destroy (&attr);
  free(threads);
}


/**
 * Run a set number series of threads on input closures until there are no more
 * closures and all threads have exited.
 *
 * Spawns at most n threads at a time, using the closures from the vector.
 * Threads will be initiated in order (0 to length-1), but there is no
 * gaurantee on their actual execution order.  If the closures vector is NULL
 * or of length 0, no threads will be executed.  A condition is used to await
 * for finished threads, instead of joining threads, to allow threads to exit
 * in any order without delaying spawning a thread for the next available
 * closure.
 */
void circle_threads(int n, void* (*func) (void*) , Vector *closures)
{
  subthread_t *sub;
  pthread_attr_t attr;
  sem_t *sem;
  int i, result;

  // at least one thread, no more than reasonable
  assert(n > 0);
  assert(n <= 32);
  // at least enough closures
  if (closures == NULL || closures->length == 0) {
    fprintf(stderr, "No threads to run!\n");
    return;
  }

  // allocate semaphore and initialize thread attributes
  sem = (sem_t*) malloc(sizeof(sem_t));
  assert(sem);
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

  // initiaze semaphore
  result = sem_init(sem, 0, n);
  assert(result == 0);

  // start threads
  i = 0;
  while (i < closures->length) {
    // wait for a thread to finish
    sem_wait(sem);
    // initialize a new subthread closure struct
    sub = (subthread_t*) malloc(sizeof(subthread_t));
    sub->sem = sem;
    sub->func = func;
    sub->closure = vget(closures, i++);
    // create thread
    result = pthread_create(&sub->thread, &attr, run_on_semaphore, sub);
  }

  // regain all the semaphore locks
  i = 0;
  while (i++ < n) {
    result = sem_wait(sem);
    assert(result == 0);
  }

  // cleanup
  pthread_attr_destroy (&attr);
  sem_destroy(sem);
  free(sem);
}

void* run_on_semaphore(void *_closure)
{
  subthread_t *info;
  void *result;
  int err;
    
  info = (subthread_t*) _closure;

  // call function
  result = (*info->func) (info->closure);
  // decrement semaphore
  err = sem_post(info->sem);
  assert(err == 0);
  // free thread data
  //free(info); // techinically, I don't this is safe, as we're still in the
              // thread, but the thread is detached so maybe it's okay
  // exit thread
  pthread_exit(result);
}
