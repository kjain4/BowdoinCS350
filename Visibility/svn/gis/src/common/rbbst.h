
/****************************************************************************
 *
 * MODULE:       r.viewshed
 * AUTHOR(S):    Laura Toma, Bowdoin College - ltoma@bowdion.edu
 *               Yi Zhuang - yzhuang@bowdoin.edu
 *              
 *               Ported to GRASS and cleaned up by William Richard -
 *               willster3021@gmail.com

 * PURPOSE: To calculate a viewpoint for the passed in point.  The
 * line of sight analysis is calculated using an algorithm outlined in
 * "Computing Visibility on Terrains in External Memory" by Herman
 * Haverkort, Laura Toma and Yi Zhuang.  COPYRIGHT: (C) 2008 by the
 * GRASS Development Team
 *
 *               This program is free software under the GNU General
 *               Public License (>=v2). Read the file COPYING that
 *               comes with GRASS for details.
 *
 *****************************************************************************/


/*
   Yi Zhuang
   June 05, 2006
   A Red Black binary search tree.
   Version 1.0.6
 */


#ifndef __RB_BINARY_SEARCH_TREE__
#define __RB_BINARY_SEARCH_TREE__

#define SMALLEST_GRADIENT (- 9999999999999999999999.0)
/*this value is returned by findMaxValueWithinDist() is there is no
  key within that distance.  The largest double value is 1.7 E 308*/

#define RB_RED (0)
#define RB_BLACK (1)

typedef struct tree_value_ {
  /* this field is mandatory and cannot be removed.  The tree is indexed by this "key". */
  double key;

  /* anything below this line is optional */
  double gradient;
  double maxGradient;
} TreeValue;

/* The node of a tree */
typedef struct tree_node_ {
  TreeValue value;

  char color;

  struct tree_node_ * left;
  struct tree_node_ *right;
  struct tree_node_ *parent;
} TreeNode;

typedef struct rbtree_ {
  TreeNode* root;
} RBTree;

RBTree* create_tree(TreeValue tv);
void delete_tree(RBTree* t);
void distroy_sub_tree(TreeNode* node);
void insert_into(RBTree* rbt, TreeValue value);
TreeNode  delete_from(RBTree* rbt, double key);
TreeNode* search_for_node_with_key(RBTree* rbt, double key);
void destroy_sub_tree(TreeNode * node);

/*------------The following is designed for kreveld's algorithm-------*/
double find_max_gradient_within_key(RBTree * rbt, double key);
double find_max_gradient_within_node(TreeNode *node);

int is_rbbst_empty(RBTree * t);
int notNIL(TreeNode *node);


/*<================================================>
   //private:
   //The below are private functions you should not 
   //call directly when using the Tree

   //<--------------------------------->
   //for RB tree only

   //in RB TREE, used to replace NULL */
void init_nil_node(void);


/*Left and Right Rotation
   //the root of the tree may be modified during the rotations
   //so TreeNode** is passed into the functions */
void left_rotate(TreeNode ** root, TreeNode * x);
void right_rotate(TreeNode ** root, TreeNode * y);
void rb_insert_fixup(TreeNode ** root, TreeNode * z);
void rb_delete_fixup(TreeNode ** root, TreeNode * x);

/*<------------------------------------> */

/*compare function used by findMaxValue
   //-1: v1 < v2
   //0:  v1 = v2
   //2:  v1 > v2 */
char compare_values(TreeValue * v1, TreeValue * v2);

/*a function used to compare two doubles */
char compare_double(double a, double b);

/*create a tree node */
TreeNode *create_tree_node(TreeValue value);

/*create node with its value set to the value given
   //and insert the node into the tree */
void insert_into_tree(TreeNode ** root, TreeValue value);

/*delete the node out of the tree */
TreeNode delete_from_tree(TreeNode ** root, double key);

/*search for a node with the given key */
TreeNode *search_for_node(TreeNode * root, double key);

/*find the max value in the given tree
   //you need to provide a compare function to compare the nodes */
double find_max_value(TreeNode * root);

/*find max within the max key */
double find_max_value_within_key(TreeNode * root, double maxKey);


#endif
