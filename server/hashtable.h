#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <cstdlib>

struct HNode {
    struct HNode *next;
    uint64_t hcode;
};

struct HTab {
    struct HNode **tab;
    size_t mask;
    size_t size;
};

struct HMap {
    struct HTab ht1;
    struct HTab ht2;
    size_t resizing_pos;
};

struct HNode *hm_lookup(struct HMap *hmap, struct HNode *key, bool (*cmp)(struct HNode *, struct HNode *));
void hm_insert(struct HMap *hmap, struct HNode *node);
struct HNode *hm_pop(struct HMap *hmap, struct HNode *key, bool (*cmp)(struct HNode *, struct HNode *));
size_t hm_size(struct HMap *hmap);
void hm_destroy(struct HMap *hmap);
