/*******************************************************************************
 * Name        : bstree.c
 * Author      : Justin Chen
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System
 ******************************************************************************/
#include "bstree.h"

/**
 * Copies data from new_data_pointer into the node's data "section"
 * @param new_data_pointer Void pointer with new data to add
 * @param data_bytes Number of bytes inside the new chunk of data
 * @param node The node to copy data into
*/
void copy_data(void* new_data_pointer, size_t data_bytes, node_t* node)
{
    // IDK why C doesn't automatically set uninitialized items to NULL, but you need to explicitly set them apparently
    node->left = NULL;
    node->right = NULL;

    // First, let's malloc $data_bytes in node's data
    node->data = malloc(data_bytes);

    // We need to copy over one byte at a time. Hence, loop through the data
    for (size_t i = 0; i < data_bytes; i++)
    {
        // Find the two addresses we're trying to copy from/to. 
        // First, we use char* so our pointer arithmetic is dealing with 1 byte
        // Then, we'll add the offset
        char* new_data_addr = (char*)new_data_pointer + i;      // The * 1 is implicit because it's a char byte
        char* curr_data_addr = (char*)(node->data) + i;

        // Swap the data
        *curr_data_addr = *new_data_addr;
    }
}

/**
 * Appends a new node with the data inside new_data_pointer into the BST
 * @param new_data_pointer Void pointer with new data
 * @param data_bytes Number of bytes inside the new chunk of data
 * @param testing_tree The tree we're appending to
 * @param compare The generic function to compare two items
*/
void add_node(void* new_data_pointer, size_t data_bytes, tree_t* testing_tree, int (*compare)(void*,void*))
{
    // Check to see if the root of the testing_tree is null
    if (testing_tree->root == NULL)
    {
        // If it is, malloc a node and fill it with new_data_pointer
        node_t* root_node = malloc(24);
        copy_data(new_data_pointer, data_bytes, root_node);

        // Now, set that to root
        testing_tree->root = root_node;
    }
    else
    {
        // If it's not root, we need to perform a BST operation. We will have a pointer to the current node the BST
        // insertion operation is working on. We will then adjust the current node as needed.
        // We always start with root.
        node_t* curr_node = testing_tree->root;

        // Use a while loop to perform the BST insertion, since recursion would get messy
        while (1 == 1)
        {
            // Compare the data we are inserting with the current node's data
            int comparison_val = compare(new_data_pointer, curr_node->data);

            if (comparison_val == -1)       // If it's less than the current node
            {
                // Check the left node. If it's null, insert our new data as a node there
                if (curr_node->left == NULL)
                {
                    // Malloc a node and fill it with new_data_pointer
                    node_t* insert_node = malloc(24);
                    copy_data(new_data_pointer, data_bytes, insert_node);

                    // Make the new node the "left" of current node
                    curr_node->left = insert_node;

                    break;              // Break the loop
                }
                else  
                {
                    // If not, move to the left node.
                    curr_node = curr_node->left;
                }
            }
            else        // If it's greater than or equal to the current node
            {
                // Check the right node. If it's null, insert our new data as a node there
                if (curr_node->right == NULL)
                {
                    puts("right");
                    // Malloc a node and fill it with new_data_pointer
                    node_t* insert_node = malloc(24);
                    copy_data(new_data_pointer, data_bytes, insert_node);

                    // Make the new node the "right" of current node
                    curr_node->right = insert_node;

                    break;              // Break the loop
                }
                else  
                {
                    // If not, move to the right node.
                    curr_node = curr_node->right;
                }
            }
        }
    }
}

/**
 * Print the current tree (AKA nodes) via in-order traversal
 * @param curr_node Current node we're printing
 * @param print_item A function pointer to the function we'll use to print the current node in the BST
*/
void print_tree(node_t* curr_node, void (*print_item)(void*))
{
    // If we reach a null node, do nothing and just return. We are done here.
    if (curr_node == NULL)
    {
        return;
    }

    // Otherwise, print the left, print the current, and print the right
    print_tree(curr_node->left, print_item);

    // Printing the current node == printing the current data
    print_item(curr_node->data);

    print_tree(curr_node->right, print_item);
}

/**
 * Destroys and deallocates the current node in the BST, and all its children.
 * If I'm not wrong, free() takes in a void pointer. This means we don't need to check for pointer types when freeing anything.
 * @param curr_node Current node to free
*/
void destroy_node(node_t* curr_node)
{
    // If we reach a null node, do nothing. We are done here.
    if (curr_node == NULL)
    {
        return;
    }

    // Otherwise, destroy the left, destroy the right, and destroy the current node.
    destroy_node(curr_node->left);
    destroy_node(curr_node->right);

    // Now, we can destroy the current node. Remember we also need to deallocate the data inside the node
    free(curr_node->data);
    curr_node->data = NULL;
    free(curr_node);
}

/**
 * Destroy and deallocates the current BST.
 * @param curr_tree Tree to deallocate
*/
void destroy(tree_t* curr_tree)
{
    // Destroy all nodes, starting from the root
    destroy_node(curr_tree->root);

    // Free this tree
    curr_tree->root = NULL;    
}