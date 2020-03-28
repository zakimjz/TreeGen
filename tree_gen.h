#ifndef TREE_GEN_H
#define TREE_GEN_H

#define max_child 100

typedef struct node* nodeptr;
typedef struct nlink* linkptr;

struct nlink {
       float   probability;
       nodeptr next_level;
};

struct node {
	int  node_no;
        int  child_num;
        nlink links[max_child];
};


#endif
	
