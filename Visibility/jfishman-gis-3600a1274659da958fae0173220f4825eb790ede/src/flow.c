
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
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include "rtimer.h"
#include "vector.h"
#include "runthreads.h"
#include "flow.h"


// enumeration for flow map orientations
//   see flow_direction() comments for explanation
enum flowdir {
  MM = 0,
  LM = 1,
  LR = 2,
  LL = 3,
  MR = 4,
  ML = 5,
  UR = 6,
  UL = 7,
  UM = 8,
  NO_DIR = 9
};

// A global timer
static Rtimer rt;


// Thread routine & closure definitions
//   flow_direction subroutine
void* flow_direction_sub(void *_closure);
//   flow_direction closure
typedef struct gridflow_thread_data { 
  int id;
  int nthread;
  Grid *elev;
  Grid *flow;
} gridflow_band;



/**
 * Calculate a matrix representing the flow direction for each point in an
 * elevation data grid.  Directions are coded as follows:
 *
 *   7 8 6
 *   5 0 4   NODATA = 9
 *   3 1 2
 *
 * This order was chosen tobe able to visualize the flow map directly,
 * similarly to a hill shading algorithm.  The southern slopes are darkest
 * and the slopes lighten as they shift to a northern flow, with sinks
 * appearing as black.  This should look somewhat like the sun is low on the
 * northern horizon.
 */
DataSet* flow_direction(DataSet *elev_set, int nthread)
{
  Grid *elev, *flow;
  DataSet *flow_set;
  Vector *gridflows;
  gridflow_band band;
  int i;

  rt_start(rt);

  // DataSet is not NULL
  assert(elev_set);
  // DataSet is float elevation data
  elev = &elev_set->grid;
  assert(elev->type == FLOAT);

  // Initialize output data grid
  flow_set = dInit(elev->nrow, elev->ncol, UCHAR);
  if (!flow_set)
    return NULL;
  flow = &flow_set->grid;
  flow->xllcorner = elev->xllcorner;
  flow->yllcorner = elev->yllcorner;
  flow->cellsize = elev->cellsize;
  flow->sNODATA = NO_DIR;

  // initialize closures
  gridflows = vinit2(sizeof(gridflow_band), nthread);
  assert(gridflows);
  i = -1;
  while (++i < nthread) {
    // set grid/flow data
    band.id = i;
    band.nthread = nthread;
    band.elev = elev;
    band.flow = flow;

    vappend(gridflows, &band);
  }

  // calcculate flows in parallel
  run_threads(nthread, flow_direction_sub, gridflows);

  // garbage collect
  vfree(gridflows);

  // print results
  rt_stop(rt);
  static char buf[256];
  rt_sprint(buf, rt);
  printf("flow_direction\t\t%s\n", buf);

  return flow_set;
}

/**
 * Thread subroutine for the flow_direction method.
 *
 * Given information about a grid, calculates the flow direction of for some
 * subset of the data.  The passed gridflow_t contains the data and flow grid
 * pointers, along with the size of the array, the NODATA value, and the number
 * of entries to skip while iterating through the array.  Other threads will
 * fill in the intervening values.
 */
void* flow_direction_sub(void *_closure)
{
  // try not to get confused by the variables here.  whereas the datagrid
  // objects use 'f', 'i', 's', etc. to indicate the type of the contained
  // data, here the 'e' and 'f' prefixes refer to 'elevation' and 'flow'
  gridflow_band band;
  index_t nrow, ncol, r, c;
  float *ep1, *ep2, *ep3;
  float min, eNODATA;
  unsigned char *fp, *fend;
  int skip, gskip;

  assert(_closure);
  band = *(gridflow_band*) _closure;
  assert(band.id >= 0);
  assert(band.nthread >= 1);
  assert(band.elev);
  assert(band.flow);
  assert(band.elev->nrow == band.flow->nrow);
  assert(band.elev->ncol == band.flow->ncol);

  nrow = band.elev->nrow;
  ncol = band.elev->ncol;
  eNODATA = band.elev->fNODATA;

  // we are thread %(id) of %(nthread) threads in total
  r = band.id / ncol;
  c = band.id % ncol;
  assert(r < nrow); assert(c < ncol);
  skip = band.nthread;
  gskip = skip - 2;

  // initialize pointers to 3 rows, in both arrays
  ep2 = band.elev->fData + r*ncol + c - 1;
  ep1 = ep2 - ncol;
  ep3 = ep2 + ncol;
  fp = band.flow->ucData + c;
  fend = band.flow->ucData + nrow * ncol;

#ifndef NDEBUG
  printf("Thread %d running from row " DGI_FMT ", col " DGI_FMT
         ", skip is %d, gskip is %d\n", band.id, r, c, skip, gskip);
  printf("Elevation band is " DGI_FMT "x" DGI_FMT
         ", flow band is " DGI_FMT "x" DGI_FMT "\n",
          band.elev->nrow, band.elev->ncol, band.flow->nrow, band.flow->ncol);
  printf("Elevation NODATA is %.f, flow NODATA is %hd\n", eNODATA, NO_DIR);
#endif

  // valid skip
  assert(skip > 0);

  // This code does not choose a neighbor at random if there is a tie for the
  // lowest elevation; it uses the first equal entry in the order UL, ML, LL,
  // UM, MM, LM, UR, MR, LR.  It does however default to itself when all there
  // are only equal or greater elevations.
  while (fp < fend) {
    min = SHRT_MAX;
    *fp = NO_DIR;

    while (c >= ncol) {
      c -= ncol;
      r++;
    }
    assert(r == (fp - band.flow->ucData) / ncol);
    assert(c == (fp - band.flow->ucData) % ncol);

    // first column of 3
    if (c > 0) {
      if (r > 0) {
        if (*ep1 != eNODATA && *ep1 < min) {
          min = *ep1;
          *fp = UL;
        }
      }
      if (*ep2 != eNODATA && *ep2 < min) {
        min = *ep2;
        *fp = ML;
      }
      if (r < nrow - 1) {
        if (*ep3 != eNODATA && *ep3 < min) {
          min = *ep3;
          *fp = LL;
        }
      }
    }
    ep1++; ep2++; ep3++;

    // second column of 3
    if (r > 0) {
      if (*ep1 != eNODATA && *ep1 < min) {
        min = *ep1;
        *fp = UM;
      }
    }
    if (*ep2 != eNODATA && *ep2 <= min) {
      min = *ep2;
      *fp = MM;
    }
    if (r < nrow - 1) {
      if (*ep3 != eNODATA && *ep3 < min) {
        min = *ep3;
        *fp = LM;
      }
    }
    ep1++; ep2++; ep3++;

    // third column of 3
    if (c < ncol - 1) {
      if (r > 0) {
        if (*ep1 != eNODATA && *ep1 < min) {
          min = *ep1;
          *fp = UR;
        }
      }
      if (*ep2 != eNODATA && *ep2 < min) {
        min = *ep2;
        *fp = MR;
      }
      if (r < ncol - 1) {
        if (*ep3 != eNODATA && *ep3 < min) {
          min = *ep3;
          *fp = LR;
        }
      }
    }

    // set value and continue to next entry
    fp += skip;
    c += skip;
    ep1 += gskip; ep2 += gskip; ep3 += gskip;
  }

  pthread_exit(NULL);
}


// Accumulation point structure
//   structure for holding a vector of neighbors in the reverse flow tree
//   constructed during the accumulation algorithm, as well as holding a
//   pointer to its location in the accumulation result matrix
typedef struct flowaccum_point_data {
  short *accu_ptr;
  Vector *sources;
} flowaccum_p;
// Thread band structure
//   holds information about the band to process to calculated the reverse flow
//   tree.  Each thread will fill data into the points in the flowpoint array,
//   and will append any sink points to the vector.
typedef struct flow_map_thread_data {
  int id;
  int nthread;
  Grid *flow;
  Grid *accu;
  flowaccum_p *flowpoints;
  Vector *sinks;
} flowaccum_band;
/**
 * Relatively complex macro used to determine whether there is flow _into_ a
 * node from a given direction.
 *
 * Takes a base pointer to the flow array, the current row and column values
 * (of type index_t), a pointer to the current location (node) in the flow
 * array, a pointer to the current location (node) in the accumulation array,
 * the direction (_from_ this node _to_ the 'source' node), and finally the
 * 'result' pointer, which is an accumulation pointer that will be set to the
 * accumulation node in the given direction.
 *
 * Since this is a macro, these are essentially all passed by reference.  It is
 * also inserted in-line, but since it is incased in an if statement it can be
 * called like any other function, i.e.
 *     if (foo)
 *         check_r_dir(...);
 * will perform as expected.
 */
#define check_r_dir(flow, r, c, fp, ap, flowdir, result) { \
  if ((r > 1             || flowdir < UR                     ) && \
      (r < flow.nrow - 1 || flowdir > MR                     ) && \
      (c > 1             || flowdir == LM || flowdir % 2 == 0) && \
      (c < flow.ncol - 1 || flowdir == UM || flowdir % 2 == 1)) { \
    switch (flowdir) { \
      case LM: \
        result = *(fp + flow.ncol    ) == UM ? (ap + flow.ncol    ) : NULL; \
        break; \
      case LR: \
        result = *(fp + flow.ncol + 1) == UL ? (ap + flow.ncol + 1) : NULL; \
        break; \
      case LL: \
        result = *(fp + flow.ncol - 1) == UR ? (ap + flow.ncol - 1) : NULL; \
        break; \
      case MR: \
        result = *(fp + 1            ) == ML ? (ap + 1            ) : NULL; \
        break; \
      case ML: \
        result = *(fp - 1            ) == MR ? (ap - 1            ) : NULL; \
        break; \
      case UR: \
        result = *(fp - flow.ncol + 1) == LL ? (ap - flow.ncol + 1) : NULL; \
        break; \
      case UL: \
        result = *(fp - flow.ncol - 1) == LR ? (ap - flow.ncol - 1) : NULL; \
        break; \
      case UM: \
        result = *(fp - flow.ncol    ) == LM ? (ap - flow.ncol    ) : NULL; \
        break; \
      default: \
        assert(FALSE); \
    } \
  } \
}

// helper threaded methods - forward references
void* zero_accum_array(void *_closure);
void* iter_accumulation(void *_closure);

/**
 * Calculate the flow accumulation among all forests of a flow map.
 *
 * Performs a depth-first traversal the of each reverse-flow tree rooted at a
 * sink node in the flow direction grid. Flow accumulates while backtracking
 * the traversal path.  
 *
 * This method instantiates the thread bands and begins the threaded
 * processing using run_threads().  Since all flow paths are trees, threads
 * are gauranteed not to conflict with other threads, given that the threads
 * begin accumulation on different sink nodes.  The banding assigns each
 * thread its own start node and a skip value such that the threads progress
 * together down the array, in order to limit conflicting page requests.  Some
 * threads may 'get ahead' of others, causing paging problems.
 */
DataSet* flow_accumulation_tree(DataSet *flow_set, int nthread)
{
  Vector *flowmap_bands;
  DataSet *accu_set;
  Grid *flow, *accu;
  flowaccum_band band;
  int i;

  rt_start(rt);

  // DataSet is not NULL
  assert(flow_set);
  // DataSet is short flow direction data
  flow = &flow_set->grid;
  assert(flow->type == SHRT);

  // Initialize output data grid
  accu_set = dInit(flow->nrow, flow->ncol, SHRT);
  if (!accu_set)
    return NULL;
  accu = &accu_set->grid;
  accu->xllcorner = flow->xllcorner;
  accu->yllcorner = flow->yllcorner;
  accu->cellsize = flow->cellsize;
  accu->sNODATA = 0;

  // allocate thread closure vectors
  flowmap_bands = vinit2(sizeof(flowaccum_band), nthread);
  assert(flowmap_bands);

  // initialize reverse mapping thread bands and sink vectors
  i = -1;
  while (++i < nthread) {
    band.id = i;
    band.nthread = nthread;
    band.flow = flow;
    band.accu = accu;
    // leave out flowpoints array and sinks vector

    vappend(flowmap_bands, &band);
  }

  // zero accumulation array to use as 'visited node' marker
  run_threads(nthread, zero_accum_array, flowmap_bands);

  // perform iterative accumulation in parallel threads
  run_threads(nthread, iter_accumulation, flowmap_bands);

  // garbage collect
  vfree(flowmap_bands);

  // print results
  rt_stop(rt);
  static char buf[1024];
  rt_sprint(buf, rt);
  printf("flow_accumulation\t%s\n", buf);

  return accu_set;
}

/**
 * Simply zero the entries in a thread band.
 */
void* zero_accum_array(void *_closure)
{
  flowaccum_band band;
  short *ap, *aend;

  assert(_closure);
  band = *(flowaccum_band*) _closure;
  assert(band.accu);

  ap = band.accu->sData + band.id;
  aend = band.accu->sData + band.accu->nrow * band.accu->ncol;

  while (ap < aend) {
    *ap = 0;
    ap += band.nthread;
  }

  pthread_exit(NULL);
}

/**
 * Holds pointer to specific entry in accumulation array, along with the last
 * direction checked in a depth-first reverse-flow traversal.  Used in a stack
 * to keep track of visited nodes and continue traversal after accumulating a
 * complete branch.
 */
typedef struct iter_accum_point_data {
  short *ap;
  enum flowdir dir;
} iteraccum_point;

/**
 * The meat of the flow_accumulation_tree algorithm.
 *
 * To reiterate, performs a depth-first traversal the of each reverse-flow
 * tree rooted at a sink node in the flow direction grid. Flow accumulates
 * while backtracking the traversal path.  
 *
 * This method performs the actual accumulation at all sink nodes in the
 * thread band passed in its closure.  Since all flow paths are trees, this
 * thread is gauranteed not to conflict with another thread, given that the
 * threads begin accumulation on different sink nodes.  The banding assigns
 * each thread its own start node and a skip value such that the threads
 * progress together down the array, in order to limit conflicting page
 * requests.  Some threads may 'get ahead' of others, causing paging problems.
 */
void* iter_accumulation(void *_closure)
{
  flowaccum_band band;
  Vector *stack;
  Grid flow, accu;
  unsigned char *fp, *fend, *p;
  short *ap, *aend;
  short *result, *child;
  index_t r, c;
  iteraccum_point point;

  assert(_closure);
  band = *(flowaccum_band*) _closure;
  assert(band.flow);
  assert(band.accu);
  flow = *band.flow;
  accu = *band.accu;
  assert(flow.nrow == accu.nrow);
  assert(flow.ncol == accu.ncol);

  // initialize the stack
  stack = vinit(sizeof(iteraccum_point));
  assert(stack);

  fp = flow.ucData + band.id;
  fend = flow.ucData + flow.nrow * flow.ncol;
  ap = accu.sData + band.id;
  aend = accu.sData + accu.nrow * accu.ncol;
  r = 0;
  c = band.id;

  // progress through the entire flow array, skipping other threads' entries,
  // and perform the accumulation on each sink node in our entries
  while (fp < fend) {

    if (*fp == MM) {
      // point is a sink

      // begin accumulation at this point
      point.ap = ap;
      point.dir = MM;
      child = NULL;

      do {
        // calculate important values
        r = (point.ap - accu.sData) / accu.ncol;
        c = (point.ap - accu.sData) % accu.ncol;
        p = flow.ucData + r*flow.ncol + c;
        result = NULL;
        // check the remaining directions for reverse flow children
        while (++point.dir < NO_DIR) {
          result = NULL;
          // check if the flow point in flow in the direction DIR from fp
          // (which is at [r][c]) is flowing into fp, and store the pointer to
          // the corresponding accumulation entry in result, or NULL
          check_r_dir(flow, r, c, p, point.ap, point.dir, result);

          if (result)
            break;
        }

        if (result) {
          // we found a branch, traverse down it
          vappend(stack, &point);
          point.ap = result;
          point.dir = MM;
          child = NULL;
        }else {
          // no children, begin accumulating flow
          if (child == NULL) 
            // leaf node
            *point.ap = 1;
          else
            // branch node
            *point.ap = *child + 1;

          // recurse backwards
          child = point.ap;
          if (stack->length > 0)
            // pop the latest point from the stack
            point = *(iteraccum_point*) vpop(stack);
          else
            // exit loop
            point.ap = NULL;
        }
      } while (point.ap);
    }

    fp += band.nthread;
    ap += band.nthread;
  }

  // garbage collect stack vector
  vfree(stack);

  pthread_exit(NULL);
}
