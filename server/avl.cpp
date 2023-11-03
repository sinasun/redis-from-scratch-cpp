#include "avl.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static uint32_t avl_get_depth(AVLNode *node) {
    return node ? node->depth : 0;
}

static uint32_t avl_get_count(AVLNode *node) {
    return node ? node->count : 0;
}

// Update depth and count fields
static void avl_update(AVLNode *node) {
    node->depth = 1 + MAX(avl_get_depth(node->left), avl_get_depth(node->right));
    node->count = 1 + avl_get_count(node->left) + avl_get_count(node->right);
}

// Left rotation
static AVLNode *rotate_left(AVLNode *node) {
    AVLNode *new_node = node->right;
    if (new_node->left) {
        new_node->left->parent = node;
    }
    node->right = new_node->left;
    new_node->left = node;
    new_node->parent = node->parent;
    node->parent = new_node;
    avl_update(node);
    avl_update(new_node);
    return new_node;
}

// Right rotation
static AVLNode *rotate_right(AVLNode *node) {
    AVLNode *new_node = node->left;
    if (new_node->right) {
        new_node->right->parent = node;
    }
    node->left = new_node->right;
    new_node->right = node;
    new_node->parent = node->parent;
    node->parent = new_node;
    avl_update(node);
    avl_update(new_node);
    return new_node;
}

// Fix left subtree imbalance
static AVLNode *avl_fix_left(AVLNode *root) {
    if (avl_get_depth(root->left->left) < avl_get_depth(root->left->right)) {
        root->left = rotate_left(root->left);
    }
    return rotate_right(root);
}

// Fix right subtree imbalance
static AVLNode *avl_fix_right(AVLNode *root) {
    if (avl_get_depth(root->right->right) < avl_get_depth(root->right->left)) {
        root->right = rotate_right(root->right);
    }
    return rotate_left(root);
}

// Fix imbalanced nodes and maintain invariants until the root is reached
AVLNode *avl_fix(AVLNode *node) {
    while (true) {
        avl_update(node);
        uint32_t left_depth = avl_get_depth(node->left);
        uint32_t right_depth = avl_get_depth(node->right);
        AVLNode **from = NULL;
        if (node->parent) {
            from = (node->parent->left == node) ? &node->parent->left : &node->parent->right;
        }
        if (left_depth == right_depth + 2) {
            node = avl_fix_left(node);
        } else if (left_depth + 2 == right_depth) {
            node = avl_fix_right(node);
        }
        if (!from) {
            return node;
        }
        *from = node;
        node = node->parent;
    }
}

// Delete a node and return the new root of the tree
AVLNode *avl_delete(AVLNode *node) {
    if (node->right == NULL) {
        AVLNode *parent = node->parent;
        if (node->left) {
            node->left->parent = parent;
        }
        if (parent) {
            (parent->left == node ? parent->left : parent->right) = node->left;
            return avl_fix(parent);
        } else {
            return node->left;
        }
    } else {
        AVLNode *successor = node->right;
        while (successor->left) {
            successor = successor->left;
        }
        AVLNode *root = avl_delete(successor);

        *successor = *node;
        if (successor->left) {
            successor->left->parent = successor;
        }
        if (successor->right) {
            successor->right->parent = successor;
        }
        AVLNode *parent = node->parent;
        if (parent) {
            (parent->left == node ? parent->left : parent->right) = successor;
            return root;
        } else {
            return successor;
        }
    }
}

AVLNode *avl_offset(AVLNode *node, int64_t offset) {
    int64_t position = 0;
    while (offset != position) {
        if (position < offset && position + avl_get_count(node->right) >= offset) {
            node = node->right;
            position += avl_get_count(node->left) + 1;
        } else if (position > offset && position - avl_get_count(node->left) <= offset) {
            node = node->left;
            position -= avl_get_count(node->right) + 1;
        } else {
            AVLNode *parent = node->parent;
            if (!parent) {
                return NULL;
            }
            if (parent->right == node) {
                position -= avl_get_count(node->left) + 1;
            } else {
                position += avl_get_count(node->right) + 1;
            }
            node = parent;
        }
    }
    return node;
}
