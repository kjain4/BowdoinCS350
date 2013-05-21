/*

A R/B BST. Always call initNILnode() before using the tree.
Version 0.0.0

Version 0.0.1
Rewrote BST Deletion to improve efficiency

Version 0.0.2
Bug fixed in deletion.
CLRS pseudocode forgot to make sure that x is not NIL before
calling rbDeleteFixup(root,x).

Version 0.0.3
Some Cleanup. Separated the public portion and the 
private porthion of the interface in the header


=================================
This is based on BST 1.0.4
BST change log
<---------------->
find max is implemented in this version.
Version 1.0.2

Version 1.0.4 
Major bug fix in deletion (when the node has two children, 
one of them has a wrong parent pointer after the rotation in the deletion.)
<----------------->
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "rbbst.h"


//TreeNode *NIL;
static TreeNode staticNIL =
  { {0, SMALLEST_GRADIENT, SMALLEST_GRADIENT}, RB_BLACK, NULL, NULL, NULL};
static TreeNode *NIL = &staticNIL;

#define EPSILON 0.0000001


//public:---------------------------------
RBTree* createTree(TreeValue tv){
  RBTree* rbt = (RBTree*)malloc(sizeof(RBTree));
  TreeNode* root = (TreeNode*)malloc(sizeof(TreeNode));
  
  rbt->root = root;
  rbt->root->value = tv;
  rbt->root->left = NIL;
  rbt->root->right = NIL;
  rbt->root->parent = NIL;
  rbt->root->color = RB_BLACK;

  return rbt;
}

//LT: not sure if this is correct
int isEmpty(RBTree* t) {
  assert(t); 
  return (t->root == NIL);
}

void deleteTree(RBTree* t) {
  destroySubTree(t->root); 
} 

void destroySubTree(TreeNode* node){
  if(node == NIL)
    return;

  destroySubTree(node->left);
  destroySubTree(node->right);
  free(node);
}

void insertInto(RBTree* rbt, TreeValue value){
  insertIntoTree(&(rbt->root), value);
}

void deleteFrom(RBTree* rbt, double key){
  deleteFromTree(&(rbt->root), key);
}

TreeNode* searchForNodeWithKey(RBTree* rbt, double key){
  return searchForNode(rbt->root, key);
}

//------------The following is designed for kreveld's algorithm-------
double findMaxGradientWithinKey(RBTree* rbt, double key){
  return findMaxValueWithinKey(rbt->root, key);
}

int notNIL(TreeNode* node){
  return node != NIL;
}

//<--------------------------------->
//Private below this line

//you can write change this compare function, depending on your TreeValue struct
//compare function used by findMaxValue
//-1: v1 < v2
//0:  v1 = v2
//2:  v1 > v2
char compareValues(TreeValue* v1, TreeValue* v2){
  if(v1->gradient  >  v2->gradient)  return 1;
  if(v1->gradient  <  v2->gradient)  return -1;

  return 0;
}


#ifndef comparedouble
//a function used to compare two doubles
char comparedouble(double a, double b){
  if ( fabs(a-b) < EPSILON) return 0;
  if ( a-b < 0 ) return -1;

  return 1;
}
#endif



//create a tree node
TreeNode* createTreeNode(TreeValue value){
  TreeNode * ret;
  ret = (TreeNode*) malloc(sizeof(TreeNode));
	
  ret->color = RB_RED;
	
  ret->left  = NIL;
  ret->right = NIL;
  ret->parent= NIL;
		
  ret->value = value;
  ret->value.maxGradient = SMALLEST_GRADIENT;
  return ret;
}

//create node with its value set to the value given
//and insert the node into the tree
//rbInsertFixup may change the root pointer, so TreeNode** is passed in
void insertIntoTree(TreeNode** root, TreeValue value){
  TreeNode * curNode;
  TreeNode * nextNode;

  curNode = *root;
	
  if(  comparedouble(value.key, curNode->value.key) == -1 ){
    nextNode = curNode->left;
  }else{
    nextNode = curNode->right;
  }


  while(nextNode != NIL){
    curNode = nextNode;
	
    if(  comparedouble(value.key, curNode->value.key) == -1  ){
      nextNode = curNode->left;
    }else{
      nextNode = curNode->right;
    }
  }

  //create a new node 
  //and place it at the right place
  //created node is RED by default
  nextNode = createTreeNode(value); 

  nextNode->parent = curNode;

  if( comparedouble(value.key, curNode->value.key) == -1){
    curNode->left  = nextNode;
  }else{
    curNode->right = nextNode;
  }
	
  TreeNode * inserted = nextNode;

  //update augmented maxGradient
  nextNode->value.maxGradient = nextNode->value.gradient;
  while(nextNode->parent != NIL){
    if( nextNode->parent->value.maxGradient < nextNode->value.maxGradient)
      nextNode->parent->value.maxGradient = nextNode->value.maxGradient;
		
    if(nextNode->parent->value.maxGradient > nextNode->value.maxGradient)
      break;
    nextNode = nextNode->parent;
  }
	
  //fix rb tree after insertion
  rbInsertFixup(root, inserted);
}

void rbInsertFixup(TreeNode** root, TreeNode* z){
  //see pseudocode on page 281 in CLRS
  TreeNode * y;
  while (z->parent->color == RB_RED){
    if(z->parent == z->parent->parent->left){
      y= z->parent->parent->right;
      if(y->color == RB_RED){          //case 1
		z->parent->color = RB_BLACK;
		y->color = RB_BLACK;
		z->parent->parent->color = RB_RED;
		z = z->parent->parent;
      }else{
		if(z == z->parent->right){       //case 2
		  z = z->parent;
		  leftRotate(root,z);                 //convert case 2 to case 3
		}
		z->parent->color = RB_BLACK;  //case 3
		z->parent->parent->color = RB_RED;
		rightRotate(root, z->parent->parent);
      }
      
    }else{ //(z->parent == z->parent->parent->right)
      y= z->parent->parent->left;
      if(y->color == RB_RED){          //case 1
		z->parent->color = RB_BLACK;
		y->color = RB_BLACK;
		z->parent->parent->color = RB_RED;
		z = z->parent->parent;
      }else{ 
		if(z == z->parent->left){       //case 2
		  z = z->parent;
		  rightRotate(root,z);                 //convert case 2 to case 3
		}
		z->parent->color = RB_BLACK;  //case 3
		z->parent->parent->color = RB_RED;
		leftRotate(root, z->parent->parent);
      }
	}
  }
  (*root)->color = RB_BLACK;
}




//search for a node with the given key
TreeNode * searchForNode(TreeNode* root, double key){
  TreeNode* curNode = root;
  int cmp;

  while( curNode != NIL &&
	    (cmp = comparedouble(key, curNode->value.key)) != 0 )
	curNode = cmp == -1 ? curNode->left : curNode->right;

  return curNode;
}

//function used by treeSuccessor
TreeNode* treeMinimum(TreeNode* x){
  while(x->left != NIL)
    x = x->left;

  return x;
}

//function used by deletion
TreeNode* treeSuccessor(TreeNode* x){
  if(x->right != NIL)
    return treeMinimum(x->right);
  TreeNode* y = x->parent;
  while(y!=NIL && x==y->right){
    x = y;
    y = y->parent;
  }
  return y;
}


//delete the node out of the tree
void deleteFromTree(TreeNode** root, double key){
  double tmpMax;
  TreeNode* z;
  TreeNode* x;
  TreeNode* y;
  TreeNode* toFix;
  z = searchForNode(*root,key);
	
  if(z==NIL){
    printf("ATTEMPT to delete key=%f failed\n", key);
    fprintf(stderr,  "Node not found. Deletion fails.\n");
	exit(1);
    return; //node to delete is not found		
  }
  
  //1-3
  if(z->left == NIL || z->right == NIL)
    y = z;
  else
    y = treeSuccessor(z);

  //4-6
  if(y->left != NIL)
    x = y->left;
  else
    x = y->right;
  
  //7
  x->parent = y->parent;

  //8-12
  if(y->parent == NIL){
    *root = x;

    toFix = *root; //augmentation to be fixed
  }else{
    if(y==y->parent->left)
      y->parent->left = x;
    else
      y->parent->right = x;

    toFix = y->parent; //augmentation to be fixed
  }
  
  //fix augmentation for removing y
  TreeNode* curNode = y;
  double left, right;
  while(curNode->parent != NIL){
    if(	curNode->parent->value.maxGradient == y->value.gradient){
      left = findMaxValue(curNode->parent->left);
      right = findMaxValue(curNode->parent->right);
      
      if(left>right)
	curNode->parent->value.maxGradient=left;
      else
	curNode->parent->value.maxGradient=right;
      
      if(curNode->parent->value.gradient > curNode->parent->value.maxGradient)
	curNode->parent->value.maxGradient = curNode->parent->value.gradient;
    }else{
      break;
    }
    curNode = curNode->parent;
  }		
  
  
  //fix augmentation for x
  tmpMax = toFix->left->value.maxGradient > toFix->right->value.maxGradient ?
    toFix->left->value.maxGradient : toFix->right->value.maxGradient;
  if(tmpMax > toFix->value.gradient)
    toFix->value.maxGradient = tmpMax;
  else
    toFix->value.maxGradient = toFix->value.gradient;

  //13-15
  if (y!=z){
    double zGradient = z->value.gradient;  
            
    z->value.key = y->value.key;
    z->value.gradient = y->value.gradient;


    toFix = z;
    //fix augmentation
    tmpMax = toFix->left->value.maxGradient > toFix->right->value.maxGradient ?
      toFix->left->value.maxGradient : toFix->right->value.maxGradient;
    if(tmpMax > toFix->value.gradient)
      toFix->value.maxGradient = tmpMax;
    else
      toFix->value.maxGradient = toFix->value.gradient;

    while(z->parent!=NIL){
      if(z->parent->value.maxGradient == zGradient){ 
	if(z->parent->value.gradient != zGradient && 
	   (!(z->parent->left->value.maxGradient == zGradient && 
	      z->parent->right->value.maxGradient== zGradient ) )){
	  
	  left = findMaxValue(z->parent->left);
	  right = findMaxValue(z->parent->right);
	  
	  if(left>right)
	    z->parent->value.maxGradient=left;
	  else
	    z->parent->value.maxGradient=right;
	  
	  if(z->parent->value.gradient > z->parent->value.maxGradient)
	    z->parent->value.maxGradient = z->parent->value.gradient;
	  
	}
	
      }else{
	if(z->value.maxGradient > z->parent->value.maxGradient)
	  z->parent->value.maxGradient = z->value.maxGradient;
      }
      z = z->parent;
    }

  }
  
  //16-17
  if(y->color == RB_BLACK && x!=NIL)
    rbDeleteFixup(root, x);

  //18
  free(y);
}

//fix the rb tree after deletion
void rbDeleteFixup(TreeNode** root, TreeNode* x){
  TreeNode * w;
  while(x != *root  &&  x->color==RB_BLACK ){
    if(x == x->parent->left){
      w = x->parent->right;
      if(w->color == RB_RED){
	w->color = RB_BLACK;
	x->parent->color = RB_RED;
	leftRotate(root, x->parent);
	w = x->parent->right;
      }
      
      if(w==NIL){
	x = x->parent;
	continue;
      }
      
      if(w->left->color == RB_BLACK && w->right->color == RB_BLACK){
	w->color = RB_RED;
	x = x->parent;
      }else{
	if(w->right->color == RB_BLACK){
	  w->left->color = RB_BLACK;
	  w->color = RB_RED;
	  rightRotate(root, w);
	  w = x->parent->right;
	}

	w->color = x->parent->color;
	x->parent->color = RB_BLACK;
	w->right->color = RB_BLACK;
	leftRotate(root, x->parent);
	x = *root;
      }

    }else{  //(x==x->parent->right)
      w = x->parent->left;
      if(w->color == RB_RED){
	w->color = RB_BLACK;
	x->parent->color = RB_RED;
	rightRotate(root, x->parent);
	w = x->parent->left;
      }
      
      if(w==NIL){
      	x = x->parent;
	continue;
      }
	      
      if(w->right->color == RB_BLACK && w->left->color == RB_BLACK){
	w->color = RB_RED;
	x = x->parent;
      }else{
	if(w->left->color == RB_BLACK){
	  w->right->color = RB_BLACK;
	  w->color = RB_RED;
	  leftRotate(root, w);
	  w = x->parent->left;
	}

	w->color = x->parent->color;
	x->parent->color = RB_BLACK;
	w->left->color = RB_BLACK;
	rightRotate(root, x->parent);
	x = *root;
      }
    
    }
  }
  x->color = RB_BLACK;
}

//find the max value in the given tree
//you need to provide a compare function to compare the nodes
double 
findMaxValue(TreeNode* root){
  if(!root) 
    return SMALLEST_GRADIENT;
  assert(root); 
  //assert(root->value.maxGradient != SMALLEST_GRADIENT);
  //LT: this shoudl be fixed
  //if (root->value.maxGradient != SMALLEST_GRADIENT)
	return root->value.maxGradient;
}



//find max within the max key
double 
findMaxValueWithinKey(TreeNode* root, double maxKey){
  TreeNode* keyNode = searchForNode(root, maxKey);
  if(keyNode==NIL){
    //fprintf(stderr, "key node not found. error occured!\n");
    //there is no point in the structure with key < maxKey
    return SMALLEST_GRADIENT;
    exit(1);
  }
  
  double max = findMaxValue(keyNode->left);
  double tmpMax;
  while(keyNode->parent != NIL){
    if(keyNode == keyNode->parent->right){ //its the right node of its parent;
      tmpMax = findMaxValue(keyNode->parent->left);
      if(tmpMax > max) 
	max = tmpMax;
      if(keyNode->parent->value.gradient > max)
	max = keyNode->parent->value.gradient;
    }
    keyNode = keyNode->parent;
  }
   	
  return max;
}


void leftRotate(TreeNode** root, TreeNode* x){
  TreeNode * y;
  y = x->right;

  //maintain augmentation
  double tmpMax;
  //fix x
  tmpMax = x->left->value.maxGradient > y->left->value.maxGradient ? 
    x->left->value.maxGradient : y->left->value.maxGradient;
  
  if(tmpMax > x->value.gradient)
    x->value.maxGradient = tmpMax;
  else
    x->value.maxGradient = x->value.gradient;


  //fix y
  tmpMax = x->value.maxGradient > y->right->value.maxGradient ?
    x->value.maxGradient : y->right->value.maxGradient;

  if(tmpMax > y->value.gradient)
    y->value.maxGradient = tmpMax;
  else
    y->value.maxGradient = y->value.gradient;

  //left rotation
  //see pseudocode on page 278 in CLRS
  
  x->right = y->left;    //turn y's left subtree into x's right subtree
  y->left->parent = x;

  y->parent = x->parent; //link x's parent to y

  if (x->parent == NIL){
    *root = y;
  }else{
    if(x == x->parent->left)
      x->parent->left = y;
    else
      x->parent->right = y;
  }

  y->left = x;
  x->parent = y;  
}

void rightRotate(TreeNode** root, TreeNode* y){
  TreeNode * x;
  x = y->left;

  //maintain augmentation
  //fix y
  double tmpMax;
  tmpMax = x->right->value.maxGradient > y->right->value.maxGradient ?
    x->right->value.maxGradient : y->right->value.maxGradient;

  if(tmpMax > y->value.gradient)
    y->value.maxGradient = tmpMax;
  else
    y->value.maxGradient = y->value.gradient;

  //fix x
  tmpMax = x->left->value.maxGradient > y->value.maxGradient ?
    x->left->value.maxGradient : y->value.maxGradient;

  if(tmpMax > x->value.gradient)
    x->value.maxGradient = tmpMax;
  else
    x->value.maxGradient = x->value.gradient;

  //ratation
  y->left = x->right;
  x->right->parent = y;

  x->parent = y->parent;

  if(y->parent == NIL){
    *root = x;
  }else{
    if(y->parent->left == y)
      y->parent->left = x;
    else
      y->parent->right = x;
  }

  x->right = y;
  y->parent = x;
}
