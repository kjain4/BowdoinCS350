/*
  Yi Zhuang
  June 05, 2006
  A Red Black binary search tree.
  Version 1.0.6
*/


#ifndef __RB_BINARY_SEARCH_TREE__
#define __RB_BINARY_SEARCH_TREE__

#include <limits.h>

#define SMALLEST_GRADIENT (INT_MIN)
//this value is returned by findMaxValueWithinDist() is there is no
//key within that distance

#define RB_RED (0)
#define RB_BLACK (1)

//<===========================================>
//public:
//The value that's stored in the tree
//Change this structure to avoid type casting at run time
typedef struct tree_value_ {
  //this field is mandatory and cannot be removed.
  //the tree is indexed by this "key".
  double key;
  
  //anything else below this line is optional 
  double gradient;
  double maxGradient;
} TreeValue;


//The node of a tree
typedef struct tree_node_ {
  TreeValue value;
  
  char color;
  
  struct tree_node_ * left;
  struct tree_node_ * right;
  struct tree_node_ * parent; 
  
} TreeNode;

typedef struct rbtree_ {
  TreeNode* root;
} RBTree;



RBTree* createTree(TreeValue tv);
void deleteTree(RBTree* t); 
void destroySubTree(TreeNode* node);
void insertInto(RBTree* rbt, TreeValue value);
void deleteFrom(RBTree* rbt, double key);
TreeNode* searchForNodeWithKey(RBTree* rbt, double key);


//------------The following is designed for kreveld's algorithm-------
double findMaxGradientWithinKey(RBTree* rbt, double key);

//LT: not sure if this is correct
int isEmpty(RBTree* t);





//<================================================>
//private:
//The below are private functions you should not 
//call directly when using the Tree

//<--------------------------------->
//for RB tree only

//in RB TREE, used to replace NULL
void initNILnode(void);
int  notNIL(TreeNode* node);


//Left and Right Rotation
//the root of the tree may be modified during the rotations
//so TreeNode** is passed into the functions
void leftRotate(TreeNode** root, TreeNode* x);
void rightRotate(TreeNode** root, TreeNode* y);
void rbInsertFixup(TreeNode** root, TreeNode* z);
void rbDeleteFixup(TreeNode** root, TreeNode* x);
//<------------------------------------>


//compare function used by findMaxValue
//-1: v1 < v2
//0:  v1 = v2
//2:  v1 > v2
char compareValues(TreeValue* v1, TreeValue* v2);

//a function used to compare two doubles
//char comparedouble(double a, double b);
#define comparedouble(a, b) (fabs(a-b) < EPSILON ? 0 : (a < b ? -1 : 1))


//create a tree node
TreeNode* createTreeNode(TreeValue value);

//create node with its value set to the value given
//and insert the node into the tree
void insertIntoTree(TreeNode** root, TreeValue value);

//delete the node out of the tree
void deleteFromTree(TreeNode** root, double key);

//search for a node with the given key
TreeNode * searchForNode(TreeNode* root, double key);

//find the max value in the given tree
//you need to provide a compare function to compare the nodes
double findMaxValue(TreeNode* root);

//find max within the max key
double findMaxValueWithinKey(TreeNode* root, double maxKey);

#endif

