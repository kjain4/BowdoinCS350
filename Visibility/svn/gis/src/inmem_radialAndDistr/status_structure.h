#ifndef __STATUS_STRUCTURE_H
#define __STATUS_STRUCTURE_H

#include "rbbst.h"
#include "event.h"

typedef struct statusnode_ {
  int row, col;          /* position of the cell */
  float elev;            /* elevation of the cell */
  double dist_to_vp;     /* distance to the viewpoint */
  double gradient;       /*gradient of the Line of Sight */
}StatusNode;

typedef struct statuslist_ {
  RBTree *rbt;        /* pointer to the root of the bst */
} StatusList;

/* ------------------------------------------------------------ */
//given a StatusNode, fill in its distance to vp and gradient
void calculate_dist_n_gradient(StatusNode * sn, Viewpoint * vp);

/*create an empty status list. */
StatusList *create_status_struct();

void delete_status_structure(StatusList * sl);

/*returns true is it is empty */
int is_status_list_empty(StatusList * sl);

/*delete the statusNode with the given key */
void delete_from_status_struct(StatusList * sl, double dist2vp, StatusNode sn);

/*insert the element into the status structure */
void insert_into_status_struct(StatusNode sn, StatusList * sl);

/*find the node with max Gradient. The node must be
   //within the distance (from viewpoint) given */
double find_max_gradient_in_status_struct(StatusList * sl, double dist);

#endif
