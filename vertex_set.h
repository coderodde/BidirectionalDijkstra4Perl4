#ifndef COM_GITHUB_CODERODDE_BIDIR_SEARCH_VERTEX_SET_H
#define	COM_GITHUB_CODERODDE_BIDIR_SEARCH_VERTEX_SET_H

#include "util.h"
#include <stdlib.h>

typedef struct vertex_set_entry {
    size_t vertex_id;
    struct vertex_set_entry* chain_next;
    struct vertex_set_entry* prev;
    struct vertex_set_entry* next;
} vertex_set_entry;

typedef struct vertex_set {
    vertex_set_entry** table;
    vertex_set_entry* head;
    vertex_set_entry* tail;
    size_t mod_count;
    size_t table_capacity;
    size_t size;
    size_t mask;
    size_t max_allowed_size;
    float  load_factor;
} vertex_set;

vertex_set* vertex_set_alloc
        (size_t initial_capacity,
         float load_factor);

int vertex_set_add(vertex_set* p_set, size_t vertex_id);

int vertex_set_contains(vertex_set* p_set, size_t vertex_id);

size_t vertex_set_size(vertex_set* p_set);

void vertex_set_free(vertex_set* p_set);

#endif	/* COM_GITHUB_CODERODDE_BIDIR_SEARCH_VERTEX_SET_H */