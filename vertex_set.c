#include "vertex_set.h"
#include <stdlib.h>

static vertex_set_entry* vertex_set_entry_alloc(size_t vertex_id)
{
    vertex_set_entry* entry = malloc(sizeof(*entry));

    if (!entry)
    {
        return NULL;
    }

    entry->vertex_id = vertex_id;
    entry->chain_next = NULL;
    entry->next = NULL;
    entry->prev = NULL;

    return entry;
}

static const float  MINIMUM_LOAD_FACTOR = 0.2f;
static const size_t MINIMUM_INITIAL_CAPACITY = 16;

static float maxf(float a, float b)
{
    return a < b ? b : a;
}

static int maxi(int a, int b)
{
    return a < b ? b : a;
}

/*******************************************************************************
* Makes sure that the load factor is no less than a minimum threshold.         *
*******************************************************************************/
static float fix_load_factor(float load_factor)
{
    return maxf(load_factor, MINIMUM_LOAD_FACTOR);
}

/*******************************************************************************
* Makes sure that the initial capacity is no less than a minimum allowed and   *
* is a power of two.                                                           *
*******************************************************************************/
static size_t fix_initial_capacity(size_t initial_capacity)
{
    size_t ret;

    initial_capacity = maxi(initial_capacity, MINIMUM_INITIAL_CAPACITY);
    ret = 1;

    while (ret < initial_capacity)
    {
        ret <<= 1;
    }

    return ret;
}

vertex_set* vertex_set_alloc(size_t initial_capacity,
                             float load_factor)
{
    vertex_set* set = malloc(sizeof(*set));

    if (!set)
    {
        return NULL;
    }

    load_factor = fix_load_factor(load_factor);
    initial_capacity = fix_initial_capacity(initial_capacity);

    set->load_factor = load_factor;
    set->table_capacity = initial_capacity;
    set->size = 0;
    set->mod_count = 0;
    set->head = NULL;
    set->tail = NULL;
    set->table = calloc(initial_capacity,
                        sizeof(vertex_set_entry*));

    set->mask = initial_capacity - 1;
    set->max_allowed_size = (size_t)(initial_capacity * load_factor);

    return set;
}

static int ensure_capacity(vertex_set* set)
{
    size_t new_capacity;
    size_t new_mask;
    size_t index;
    vertex_set_entry* entry;
    vertex_set_entry** new_table;

    if (set->size < set->max_allowed_size)
    {
        return RETURN_STATUS_OK;
    }

    new_capacity = 2 * set->table_capacity;
    new_mask = new_capacity - 1;
    new_table = calloc(new_capacity, sizeof(vertex_set_entry*));

    if (!new_table)
    {
        return RETURN_STATUS_NO_MEMORY;
    }

    /* Rehash the entries. */
    for (entry = set->head; entry; entry = entry->next)
    {
        index = entry->vertex_id & new_mask;
        entry->chain_next = new_table[index];
        new_table[index] = entry;
    }

    free(set->table);

    set->table = new_table;
    set->table_capacity = new_capacity;
    set->mask = new_mask;
    set->max_allowed_size = (size_t)(new_capacity * set->load_factor);

    return RETURN_STATUS_OK;
}

int vertex_set_add(vertex_set* set, size_t vertex_id)
{
    size_t index;
    size_t hash_value;
    int rs; /* return status */
    vertex_set_entry* entry;

    hash_value = vertex_id;
    index = hash_value & set->mask;

    for (entry = set->table[index]; entry; entry = entry->chain_next)
    {
        if (entry->vertex_id == vertex_id)
        {
            return RETURN_STATUS_OK;
        }
    }

    if ((rs = ensure_capacity(set)) != RETURN_STATUS_OK) {
        return rs;
    }

    /* Recompute the index since it is possibly changed by 'ensure_capacity' */
    index = hash_value & set->mask;
    entry = vertex_set_entry_alloc(vertex_id);
    entry->chain_next = set->table[index];
    set->table[index] = entry;

    /* Link the new entry to the tail of the list. */
    if (!set->tail)
    {
        set->head = entry;
        set->tail = entry;
    }
    else
    {
        set->tail->next = entry;
        entry->prev = set->tail;
        set->tail = entry;
    }

    set->size++;
    set->mod_count++;

    return RETURN_STATUS_OK;
}

int vertex_set_contains(vertex_set* set, size_t vertex_id)
{
    size_t index;
    vertex_set_entry* p_entry;
    index = vertex_id & set->mask;

    for (p_entry = set->table[index]; p_entry; p_entry = p_entry->chain_next)
    {
        if (vertex_id == p_entry->vertex_id)
        {
            return TRUE;
        }
    }

    return FALSE;
}

size_t vertex_set_size(vertex_set* p_set) {
    return p_set->size;
}

static void vertex_set_clear(vertex_set* set)
{
    vertex_set_entry* entry;
    vertex_set_entry* next_entry;
    size_t index;
    entry = set->head;

    while (entry)
    {
        index = entry->vertex_id & set->mask;
        next_entry = entry->next;
        free(entry);
        entry = next_entry;
        set->table[index] = NULL;
    }

    set->mod_count += set->size;
    set->size = 0;
    set->head = NULL;
    set->tail = NULL;
}

void vertex_set_free(vertex_set* set)
{
    if (!set)
    {
        return;
    }

    vertex_set_clear(set);
    free(set->table);
    free(set);
}