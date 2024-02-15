
#include <stdio.h>
#include <stdlib.h>

// In total, this is 24 bytes (because of 3 pointers)
typedef struct node {
	void*  data;
	struct node* left;
	struct node* right;
} node_t;

// In total, this is 32 bytes (because of 4 pointers)
typedef struct tree {
	node_t* root;
	void (*add_node)(void*, size_t, struct tree*, int (*)(void*,void*));
	void (*print_tree)(node_t*, void (*)(void*));
	void (*destroy)(struct tree*);
} tree_t;


void add_node(void* , size_t, tree_t*, int (*)(void*,void*));

void print_tree(node_t*, void (*)(void*));

void destroy(tree_t*);