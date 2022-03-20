#include "weight_map.h"
#include "util.h"
#include <stdlib.h>

static weight_map_entry*
weight_map_entry_alloc(size_t vertex_id,
                       double weight)
{
    weight_map_entry* entry = malloc(sizeof(*entry));

    if (!entry)
    {
        return NULL;
    }

    entry->vertex_id = vertex_id;
    entry->weight = weight;
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

weight_map* weight_map_alloc(
        size_t initial_capacity,
        float load_factor)
{
    weight_map* map = malloc(sizeof(*map));

    if (!map)
    {
        return NULL;
    }

    load_factor = fix_load_factor(load_factor);
    initial_capacity = fix_initial_capacity(initial_capacity);

    map->load_factor = load_factor;
    map->table_capacity = initial_capacity;
    map->size = 0;
    map->head = NULL;
    map->tail = NULL;
    map->table = calloc(initial_capacity,
                        sizeof(weight_map_entry*));

    map->mask = initial_capacity - 1;
    map->max_allowed_size = (size_t)(initial_capacity * load_factor);

    return map;
}

static int ensure_capacity(weight_map* map)
{
    size_t new_capacity;
    size_t new_mask;
    size_t index;
    weight_map_entry* entry;
    weight_map_entry** new_table;

    if (map->size < map->max_allowed_size)
    {
        return RETURN_STATUS_OK;
    }

    new_capacity = 2 * map->table_capacity;
    new_mask = new_capacity - 1;
    new_table = calloc(new_capacity, sizeof(weight_map_entry*));

    if (!new_table)
    {
        return RETURN_STATUS_NO_MAP;
    }

    /* Rehash the entries. */
    for (entry = map->head; entry; entry = entry->next)
    {
        index = entry->vertex_id & new_mask;
        entry->chain_next = new_table[index];
        new_table[index] = entry;
    }

    free(map->table);

    map->table = new_table;
    map->table_capacity = new_capacity;
    map->mask = new_mask;
    map->max_allowed_size = (size_t)(new_capacity * map->load_factor);
    return RETURN_STATUS_OK;
}

int weight_map_put(weight_map* map, size_t vertex_id, double weight)
{
    size_t index;
    size_t hash_value;
    weight_map_entry* entry;

    if (!map)
    {
        return RETURN_STATUS_NO_MAP;
    }

    hash_value = vertex_id;
    index = hash_value & map->mask;

    for (entry = map->table[index]; entry; entry = entry->chain_next)
    {
        if (entry->vertex_id == vertex_id)
        {
            entry->weight = weight;
            return RETURN_STATUS_OK;
        }
    }

    if (ensure_capacity(map) != RETURN_STATUS_OK) {
        return RETURN_STATUS_NO_MEMORY;
    }

    /* Recompute the index since it is possibly changed by 'ensure_capacity' */
    index = hash_value & map->mask;
    entry = weight_map_entry_alloc(vertex_id, weight);

    if (!entry) {
        return RETURN_STATUS_NO_MEMORY;
    }

    entry->chain_next = map->table[index];
    map->table[index] = entry;

    /* Link the new entry to the tail of the list. */
    if (!map->tail)
    {
        map->head = entry;
        map->tail = entry;
    }
    else
    {
        map->tail->next = entry;
        entry->prev = map->tail;
        map->tail = entry;
    }

    map->size++;
    return RETURN_STATUS_OK;
}

int weight_map_contains_key(weight_map* map, size_t vertex_id)
{
    size_t index;
    weight_map_entry* entry;

    if (!map)
    {
        return 0;
    }

    index = vertex_id & map->mask;

    for (entry = map->table[index]; entry; entry = entry->chain_next)
    {
        if (entry->vertex_id == vertex_id)
        {
            return 1;
        }
    }

    return 0;
}

double weight_map_get(weight_map* map, size_t vertex_id)
{
    size_t index;
    weight_map_entry* p_entry;

    if (!map)
    {
        abort();
    }

    index = vertex_id & map->mask;

    for (p_entry = map->table[index]; p_entry; p_entry = p_entry->chain_next)
    {
        if (p_entry->vertex_id == vertex_id)
        {
            return p_entry->weight;
        }
    }

    abort();
    return -1.0;
}

void weight_map_remove(weight_map* map, size_t vertex_id)
{
    size_t index;
    weight_map_entry* prev_entry;
    weight_map_entry* current_entry;

    if (!map)
    {
        return;
    }

    index = vertex_id & map->mask;
    prev_entry = NULL;

    for (current_entry = map->table[index];
         current_entry;
         current_entry = current_entry->chain_next)
    {
        if (current_entry->vertex_id == vertex_id) {
            if (prev_entry)
            {
                /* Omit the 'p_current_entry' in the collision chain. */
                prev_entry->chain_next = current_entry->chain_next;
            }
            else
            {
                map->table[index] = current_entry->chain_next;
            }

            /* Unlink from the global iteration chain. */
            if (current_entry->prev)
            {
                current_entry->prev->next = current_entry->next;
            }
            else
            {
                map->head = current_entry->next;
            }

            if (current_entry->next)
            {
                current_entry->next->prev = current_entry->prev;
            }
            else
            {
                map->tail = current_entry->prev;
            }

            map->size--;
            free(current_entry);
            return;
        }

        prev_entry = current_entry;
    }
}

void weight_map_clear(weight_map* map)
{
    weight_map_entry* entry;
    weight_map_entry* next_entry;
    size_t index;

    entry = map->head;

    while (entry)
    {
        index = entry->vertex_id & map->mask;
        next_entry = entry->next;
        free(entry);
        entry = next_entry;
        map->table[index] = NULL;
    }

    map->size = 0;
    map->head = NULL;
    map->tail = NULL;
}

size_t weight_map_size(weight_map* map)
{
    return map->size;
}

void weight_map_free(weight_map* map)
{
    if (!map)
    {
        return;
    }

    weight_map_clear(map);
    free(map->table);
    free(map);
}

weight_map_iterator*
weight_map_iterator_alloc(weight_map* map)
{
    weight_map_iterator* p_ret;

    if (!map)
    {
        return NULL;
    }

    p_ret = malloc(sizeof(*p_ret));

    if (!p_ret)
    {
        return NULL;
    }

    p_ret->map = map;
    p_ret->entry = map->head;

    return p_ret;
}

int weight_map_iterator_has_next(weight_map_iterator* iterator)
{
    return iterator && iterator->entry;
}

void weight_map_iterator_next(weight_map_iterator* iterator)
{
    iterator->entry = iterator->entry->next;
}

void weight_map_iterator_free(weight_map_iterator* iterator)
{
    if (!iterator)
    {
        return;
    }

    iterator->map = NULL;
    iterator->entry = NULL;
    free(iterator);
}

void weight_map_iterator_visit(weight_map_iterator* p_iterator,
                               size_t* p_vertex_id,
                               double* p_weight) {
    *p_vertex_id = p_iterator->entry->vertex_id;
    *p_weight = p_iterator->entry->weight;
}

void weight_map_iterator_remove(
        weight_map_iterator* p_iterator) {
    weight_map_entry* p_next_entry = p_iterator->entry->next;

    weight_map_remove(p_iterator->map,
                      p_iterator->entry->vertex_id);

    p_iterator->entry = p_next_entry;
}