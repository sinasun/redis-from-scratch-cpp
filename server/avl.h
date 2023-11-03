
#pragma once

#include <stddef.h>
#include <stdint.h>

struct AVLNode {
    uint32_t depth;
    uint32_t count;
    AVLNode* left;
    AVLNode* right;
    AVLNode* parent;
};

inline void avl_init(AVLNode* node) {
    node->depth = 1;
    node->count = 1;
    node->left = node->right = node->parent = NULL;
}

AVLNode* avl_fix(AVLNode* node);
AVLNode* avl_delete(AVLNode* node);
AVLNode* avl_offset(AVLNode* node, int64_t offset);
