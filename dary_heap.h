#ifndef COM_GITHUB_CODERODDE_BIDIR_SEARCH_DARY_HEAP_H
#define	COM_GITHUB_CODERODDE_BIDIR_SEARCH_DARY_HEAP_H

#include <stdlib.h>

typedef struct dary_heap_node {
    size_t vertex_id;
    double priority;
    size_t index;
} dary_heap_node;

typedef struct dary_heap_node_map_entry {
    size_t                    vertex_id;
    dary_heap_node*           heap_node; /* ptr to the actual heap node */
    struct dary_heap_node_map_entry* chain_next;
    struct dary_heap_node_map_entry* prev;
    struct dary_heap_node_map_entry* next;
} dary_heap_node_map_entry;

typedef struct dary_heap_node_map {
    dary_heap_node_map_entry** table;
    dary_heap_node_map_entry* head;
    dary_heap_node_map_entry* tail;
    size_t                    table_capacity;
    size_t                    size;
    size_t                    max_allowed_size;
    size_t                    mask;
    float                     load_factor;
} dary_heap_node_map;

typedef struct dary_heap {
    dary_heap_node_map* node_map;
    dary_heap_node**    table;
    size_t              size;
    size_t              capacity;
    size_t              degree;
    size_t*             indices;
} dary_heap;

dary_heap* dary_heap_alloc(size_t degree,
                           size_t initial_capacity,
                           float  load_factor);

int dary_heap_add (dary_heap* heap,
                   size_t vertex_id,
                   double priority);

void   dary_heap_decrease_key (dary_heap* heap,
                               size_t vertex_id,
                               double priority);

size_t dary_heap_extract_min  (dary_heap* heap);
size_t dary_heap_min          (dary_heap* heap);
size_t dary_heap_size         (dary_heap* heap);
void   dary_heap_clear        (dary_heap* heap);
void   dary_heap_free         (dary_heap* heap);

#endif	/* COM_GITHUB_CODERODDE_BIDIR_SEARCH_DARY_HEAP_H */