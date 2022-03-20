#include "dary_heap.h"
#include "util.h"
#include <stdbool.h>

static const float  MINIMUM_LOAD_FACTOR = 0.2f;
static const size_t MINIMUM_INITIAL_CAPACITY = 4;

static const size_t fix_degree(size_t degree) {
    return degree < 2 ? 2 : degree;
}

static const size_t fix_initial_capacity(size_t initial_capacity) {
    return initial_capacity < MINIMUM_INITIAL_CAPACITY ?
           MINIMUM_INITIAL_CAPACITY :
           initial_capacity;
}

static float fix_load_factor(float load_factor) {
    return load_factor < MINIMUM_LOAD_FACTOR ?
           MINIMUM_LOAD_FACTOR :
           load_factor;
}

/* Internal workings functions: */
static dary_heap_node_map* dary_heap_node_map_alloc(
        size_t initial_capacity,
        float load_factor) {

    dary_heap_node_map* map = malloc(sizeof(*map));

    if (!map) {
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
                        sizeof(dary_heap_node_map_entry*));

    map->mask = initial_capacity - 1;
    map->max_allowed_size = (size_t)(initial_capacity * load_factor);

    return map;
}

static dary_heap_node_map_entry*
dary_heap_node_map_entry_alloc(size_t vertex_id,
                               dary_heap_node* heap_node)
{
    dary_heap_node_map_entry* p_ret = malloc(sizeof(*p_ret));

    if (!p_ret)
    {
        return NULL;
    }

    p_ret->vertex_id = vertex_id;
    p_ret->heap_node = heap_node;
    p_ret->chain_next = NULL;
    p_ret->next = NULL;
    p_ret->prev = NULL;
    return p_ret;
}

static int ensure_capacity(dary_heap_node_map* map)
{
    size_t new_capacity;
    size_t new_mask;
    size_t index;
    dary_heap_node_map_entry* entry;
    dary_heap_node_map_entry** new_table;

    if (map->size < map->max_allowed_size)
    {
        return RETURN_STATUS_OK;
    }

    new_capacity = 2 * map->table_capacity;
    new_mask = new_capacity - 1;
    new_table = calloc(new_capacity,
                       sizeof(dary_heap_node_map_entry*));

    if (!new_table)
    {
        return RETURN_STATUS_NO_MEMORY;
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

static int dary_heap_node_map_put(
        dary_heap_node_map* map,
        size_t vertex_id,
        dary_heap_node* heap_node)
{
    size_t index;
    size_t hash_value;
    dary_heap_node_map_entry* entry;

    hash_value = vertex_id;
    index = hash_value & map->mask;

    for (entry = map->table[index]; entry; entry = entry->chain_next)
    {
        if (entry->vertex_id == vertex_id)
        {
            entry->heap_node = heap_node;
            return TRUE;
        }
    }

    if (ensure_capacity(map) != RETURN_STATUS_OK) {
        return FALSE;
    }

    /* Recompute the index since it is possibly changed by 'ensure_capacity' */
    index = hash_value & map->mask;
    entry = dary_heap_node_map_entry_alloc(vertex_id, heap_node);

    if (!entry) {
        return FALSE;
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
    return TRUE;
}

static int dary_heap_node_map_contains_vertex(
        dary_heap_node_map* map,
        size_t vertex_id)
{
    size_t index;
    dary_heap_node_map_entry* entry;

    index = vertex_id & map->mask;

    for (entry = map->table[index]; entry; entry = entry->chain_next)
    {
        if (vertex_id == entry->vertex_id)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static dary_heap_node*
dary_heap_node_alloc(size_t vertex_id,
                     double priority) {
    dary_heap_node* node = malloc(sizeof(*node));

    if (!node)
    {
        return NULL;
    }

    node->vertex_id = vertex_id;
    node->priority = priority;
    return node;
}


static dary_heap_node*
dary_heap_node_map_get(dary_heap_node_map* map,
                       size_t vertex_id)
{
    size_t index;
    dary_heap_node_map_entry* p_entry;

    index = vertex_id & map->mask;

    for (p_entry = map->table[index]; p_entry; p_entry = p_entry->chain_next)
    {
        if (vertex_id == p_entry->vertex_id)
        {
            return p_entry->heap_node;
        }
    }

    return NULL;
}

static void dary_heap_node_map_clear(dary_heap_node_map* map)
{
    dary_heap_node_map_entry* entry;
    dary_heap_node_map_entry* next_entry;
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

void dary_heap_node_map_remove(dary_heap_node_map* map,
                               size_t vertex_id)
{
    size_t index;
    dary_heap_node_map_entry* prev_entry;
    dary_heap_node_map_entry* current_entry;

    index = vertex_id & map->mask;
    prev_entry = NULL;

    for (current_entry = map->table[index];
         current_entry;
         current_entry = current_entry->chain_next)
    {
        if (vertex_id == current_entry->vertex_id)
        {
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

void dary_heap_node_map_free(dary_heap_node_map* map)
{
    dary_heap_node_map_clear(map);
    free(map->table);
    free(map);
}

dary_heap* dary_heap_alloc(size_t degree,
                           size_t initial_capacity,
                           float  load_factor)
{
    dary_heap* my_heap;
    dary_heap_node_map* p_map;

    my_heap = malloc(sizeof(*my_heap));

    if (!my_heap)
    {
        return NULL;
    }

    p_map = dary_heap_node_map_alloc(initial_capacity,
                                     load_factor);

    if (!p_map)
    {
        free(my_heap);
        return NULL;
    }

    degree = fix_degree(degree);
    initial_capacity = fix_initial_capacity(initial_capacity);

    my_heap->table = malloc(sizeof(dary_heap_node*) * initial_capacity);

    if (!my_heap->table)
    {
        dary_heap_node_map_free(p_map);
        free(my_heap);
        return NULL;
    }

    my_heap->indices = malloc(sizeof(size_t) * degree);

    if (!my_heap->indices)
    {
        dary_heap_node_map_free(p_map);
        free(my_heap->table);
        free(my_heap);
        return NULL;
    }

    my_heap->node_map = p_map;
    my_heap->capacity = initial_capacity;
    my_heap->size = 0;
    my_heap->degree = degree;

    return my_heap;
}

static size_t get_parent_index(dary_heap* my_heap, size_t child_index)
{
    return (child_index - 1) / my_heap->degree;
}

static void sift_up(dary_heap* my_heap, size_t index)
{
    size_t parent_index;
    dary_heap_node* p_target_node;
    dary_heap_node* p_parent_node;

    if (index == 0)
    {
        return;
    }

    parent_index = get_parent_index(my_heap, index);
    p_target_node = my_heap->table[index];

    while (true)
    {
        p_parent_node = my_heap->table[parent_index];

        if (p_parent_node->priority > p_target_node->priority) {
            my_heap->table[index] = p_parent_node;
            p_parent_node->index = index;
            index = parent_index;
            parent_index = get_parent_index(my_heap, index);
        }
        else
        {
            break;
        }

        if (index == 0)
        {
            break;
        }
    }

    my_heap->table[index] = p_target_node;
    p_target_node->index = index;
}

static void compute_children_indices(dary_heap* my_heap, size_t index)
{
    size_t degree = my_heap->degree;
    size_t i;

    for (i = 0; i < degree; ++i)
    {
        my_heap->indices[i] = degree * index + i + 1;

        if (my_heap->indices[i] >= my_heap->size)
        {
            my_heap->indices[i] = (size_t)-1;
            return;
        }
    }
}

static void sift_down_root(dary_heap* my_heap)
{
    dary_heap_node* target = my_heap->table[0];
    double priority = target->priority;
    double min_child_priority;
    double tentative_priority;
    size_t     min_child_index;
    size_t     i;
    size_t     degree = my_heap->degree;
    size_t     index = 0;

    for (;;)
    {
        min_child_priority = priority;
        min_child_index = -1; /* Very large value to denote "no children". */
        compute_children_indices(my_heap, index);

        for (i = 0; i < degree; ++i)
        {
            if (my_heap->indices[i] == (size_t)-1)
            {
                break;
            }

            tentative_priority = my_heap->table[my_heap->indices[i]]
                    ->priority;

            if (min_child_priority > tentative_priority) {
                min_child_priority = tentative_priority;
                min_child_index = my_heap->indices[i];
            }
        }

        if (min_child_index == (size_t)-1)
        {
            my_heap->table[index] = target;
            target->index = index;
            return;
        }

        my_heap->table[index] = my_heap->table[min_child_index];
        my_heap->table[index]->index = index;

        index = min_child_index;
    }
}

static int ensure_capacity_before_add(dary_heap* my_heap)
{
    dary_heap_node** new_table;
    size_t new_capacity;
    size_t i;

    if (my_heap->size < my_heap->capacity)
    {
        return TRUE;
    }

    new_capacity = 3 * my_heap->capacity / 2;
    new_table = malloc(sizeof(dary_heap_node*) * new_capacity);

    if (!new_table) {
        return FALSE;
    }

    for (i = 0; i < my_heap->size; ++i)
    {
        new_table[i] = my_heap->table[i];
    }

    free(my_heap->table);
    my_heap->table = new_table;
    my_heap->capacity = new_capacity;
    return TRUE;
}

int dary_heap_add(dary_heap* my_heap, size_t vertex_id, double priority)
{
    dary_heap_node* node;

    /* Already in the heap? */
    if (dary_heap_node_map_contains_vertex(my_heap->node_map, vertex_id)) {
        return RETURN_STATUS_ADDING_DUPLICATE_VERTEX;
    }

    node = dary_heap_node_alloc(vertex_id, priority);

    if (!node) {
        return RETURN_STATUS_NO_MEMORY;
    }

    if (!ensure_capacity_before_add(my_heap))
    {
        return RETURN_STATUS_NO_MEMORY;
    }

    node->index = my_heap->size;
    my_heap->table[my_heap->size] = node;

    if (!dary_heap_node_map_put(my_heap->node_map, vertex_id, node)) {
        free(node);
        return RETURN_STATUS_NO_MEMORY;
    }

    sift_up(my_heap, my_heap->size);
    my_heap->size++;
    return RETURN_STATUS_OK;
}

void dary_heap_decrease_key(dary_heap* my_heap,
                            size_t vertex_id,
                            double priority)
{
    dary_heap_node* node =
            dary_heap_node_map_get(
                    my_heap->node_map,
                    vertex_id);

    if (priority < node->priority)
    {
        node->priority = priority;
        sift_up(my_heap, node->index);
    }
}

size_t dary_heap_extract_min(dary_heap* my_heap)
{
    size_t vertex_id;
    dary_heap_node* node = my_heap->table[0];
    vertex_id = node->vertex_id;
    my_heap->size--;
    my_heap->table[0] = my_heap->table[my_heap->size];
    dary_heap_node_map_remove(my_heap->node_map, vertex_id);
    sift_down_root(my_heap);
    free(node);
    return vertex_id;
}

size_t dary_heap_min(dary_heap* my_heap)
{
    return my_heap->table[0]->vertex_id;
}

size_t dary_heap_size(dary_heap* my_heap)
{
    return my_heap->size;
}

void dary_heap_clear(dary_heap* my_heap)
{
    size_t i;

    dary_heap_node_map_clear(my_heap->node_map);

    for (i = 0; i < my_heap->size; ++i)
    {
        free(my_heap->table[i]);
    }

    my_heap->size = 0;
}

void dary_heap_free(dary_heap* my_heap)
{
    dary_heap_clear(my_heap);
    dary_heap_node_map_free(my_heap->node_map);
    free(my_heap->indices);
    free(my_heap->table);
}
