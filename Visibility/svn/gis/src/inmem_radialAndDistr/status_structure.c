#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "status_structure.h"
#include "rbbst.h"

/* ------------------------------------------------------------ */
/* given a StatusNode, fill in its dist2vp and gradient. Note since
   sqrt are expensive, we compute in fact (signed) squares of
   distances, tans, etc. */
void calculate_dist_n_gradient(StatusNode * sn, Viewpoint * vp)
{

    assert(sn && vp);
    /*sqrt is expensive
       //sn->dist2vp = sqrt((float) ( pow(sn->row - vp->row,2.0) + 
       //               pow(sn->col - vp->col,2.0)));
       */
    sn->dist_to_vp = (sn->row - vp ->row) * (sn->row-vp->row) + 
      (sn->col-vp->col) * (sn->col-vp->col);
        
    sn->gradient = (sn->elev - vp->elev) * (sn->elev - vp->elev) / (sn->dist_to_vp);
    /*maintain sign */
    if (sn->elev < vp->elev)
	sn->gradient = -sn->gradient;
    return;
}




/* ------------------------------------------------------------ */
/*create an empty  status list */
StatusList *create_status_struct()
{
    StatusList *sl;

    sl = (StatusList *)malloc(sizeof(StatusList));
    assert(sl);

    TreeValue tv;

    tv.gradient = SMALLEST_GRADIENT;
    tv.key = 0;
    tv.maxGradient = SMALLEST_GRADIENT;


    sl->rbt = create_tree(tv);
    return sl;
}

/*delete a status structure */
void delete_status_structure(StatusList * sl)
{
    assert(sl);
    delete_tree(sl->rbt);

    free(sl);
    return;
}

/* ------------------------------------------------------------ */
/*insert the element into the status structure */
void insert_into_status_struct(StatusNode sn, StatusList * sl)
{
    assert(sl);
    TreeValue tv;

    tv.key = sn.dist_to_vp;
    tv.gradient = sn.gradient;
    tv.maxGradient = SMALLEST_GRADIENT;
    insert_into(sl->rbt, tv);
    return;
}


/* ------------------------------------------------------------ */
/*find the node with max Gradient within the distance (from viewpoint)
   given */
double find_max_gradient_in_status_struct(StatusList * sl, double dist)
{
    assert(sl);
    /*note: if there is nothing in the status struccture, it means this
       cell is VISIBLE */
    if (is_status_list_empty(sl))
	return SMALLEST_GRADIENT;
    /*it is also possible that the status structure is not empty, but
       there are no events with key < dist ---in this case it returns
       SMALLEST_GRADIENT; */
    return find_max_gradient_within_key(sl->rbt, dist);
}

/*returns true is it is empty */
int is_status_list_empty(StatusList * sl)
{
    assert(sl);
    return (is_rbbst_empty(sl->rbt) ||
	    sl->rbt->root->value.maxGradient == SMALLEST_GRADIENT);
}

/* ------------------------------------------------------------ */
/*delete the statusNode with the given key=dist2vp; check that we are
  actually deleting the right cell */
void delete_from_status_struct(StatusList * sl, double dist2vp, StatusNode sn)
{
    assert(sl);
    TreeNode deleted = delete_from(sl->rbt, dist2vp);
    //assert(sn.row == deleted->row);
    
    return;
}
