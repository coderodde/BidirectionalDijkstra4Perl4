#ifndef COM_GITHUB_CODERODDE_BIDIR_SEARCH_WEIGHT_MAP_H
#define COM_GITHUB_CODERODDE_BIDIR_SEARCH_WEIGHT_MAP_H

#include <stdlib.h>

typedef struct weight_map_entry {
    size_t            vertex_id;
    double            weight;
    struct weight_map_entry* chain_next;
    struct weight_map_entry* prev;
    struct weight_map_entry* next;
} weight_map_entry;

typedef struct weight_map {
    weight_map_entry** table;
    weight_map_entry* head;
    weight_map_entry* tail;
    size_t            table_capacity;
    size_t            size;
    size_t            max_allowed_size;
    size_t            mask;
    float             load_factor;
} weight_map;

typedef struct weight_map_iterator {
    weight_map*       map;
    weight_map_entry* entry;
} weight_map_iterator;

weight_map* weight_map_alloc(size_t initial_capacity,
                             float load_factor);

int weight_map_put(weight_map* map, size_t vertex_id, double weight);

int weight_map_contains_key(weight_map* map, size_t vertex_id);

double weight_map_get(weight_map* map, size_t vertex_id);

void weight_map_remove(weight_map* map, size_t vertex_id);

void weight_map_clear(weight_map* map);

void weight_map_free(weight_map* map);

weight_map_iterator* weight_map_iterator_alloc
        (weight_map* map);

int weight_map_iterator_has_next
        (weight_map_iterator* iterator);

void weight_map_iterator_next(weight_map_iterator* iterator);

void weight_map_iterator_visit(weight_map_iterator* p_iterator,
                               size_t* p_vertex_id,
                               double* p_weight);

void weight_map_iterator_remove(weight_map_iterator* p_iterator);

#endif	/* COM_GITHUB_CODERODDE_BIDIR_SEARCH_WEIGHT_MAP_H */