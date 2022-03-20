#include "util.h"
#include "vertex_list.h"
#include <stdlib.h>

static const size_t MINIMUM_CAPACITY = 16;

static size_t max(size_t a, size_t b) {
    return a < b ? b : a;
}

static size_t fix_initial_capacity(size_t initial_capacity)
{
    size_t ret = 1;

    initial_capacity = max(initial_capacity, MINIMUM_CAPACITY);

    while (ret < initial_capacity)
    {
        ret <<= 1;
    }

    return ret;
}

vertex_list* vertex_list_alloc(size_t initial_capacity)
{
    vertex_list* my_list = malloc(sizeof(*my_list));

    if (!my_list)
    {
        return NULL;
    }

    initial_capacity = fix_initial_capacity(initial_capacity);

    my_list->storage = malloc(sizeof(size_t) * initial_capacity);

    if (!my_list->storage)
    {
        free(my_list);
        return NULL;
    }

    my_list->capacity = initial_capacity;
    my_list->mask = initial_capacity - 1;
    my_list->head = 0;
    my_list->size = 0;

    return my_list;
}

static int ensure_capacity_before_add(vertex_list* my_list)
{
    size_t* new_table;
    size_t i;
    size_t new_capacity;

    if (my_list->size < my_list->capacity)
    {
        return TRUE;
    }

    new_capacity = 2 * my_list->capacity;
    new_table = malloc(sizeof(size_t) * new_capacity);

    if (!new_table)
    {
        return FALSE;
    }

    for (i = 0; i < my_list->size; ++i)
    {
        new_table[i] = my_list->storage[(my_list->head + i) & my_list->mask];
    }

    free(my_list->storage);

    my_list->storage = new_table;
    my_list->capacity = new_capacity;
    my_list->mask = new_capacity - 1;
    my_list->head = 0;

    return TRUE;
}

int vertex_list_push_front(vertex_list* my_list, size_t vertex_id)
{
    if (!ensure_capacity_before_add(my_list))
    {
        return RETURN_STATUS_NO_MEMORY;
    }

    my_list->head = (my_list->head - 1) & my_list->mask;
    my_list->storage[my_list->head] = vertex_id;
    my_list->size++;

    return RETURN_STATUS_OK;
}

int vertex_list_push_back(vertex_list* my_list, size_t vertex_id)
{
    if (!ensure_capacity_before_add(my_list))
    {
        return RETURN_STATUS_NO_MEMORY;
    }

    my_list->storage[(my_list->head + my_list->size) & my_list->mask] =
            vertex_id;

    my_list->size++;
    return RETURN_STATUS_OK;
}

size_t vertex_list_size(vertex_list* my_list)
{
    return my_list->size;
}

size_t vertex_list_get(vertex_list* my_list, size_t index)
{
    return my_list->storage[(my_list->head + index) & my_list->mask];
}

void vertex_list_clear(vertex_list* my_list)
{
    my_list->head = 0;
    my_list->size = 0;
}

void vertex_list_free(vertex_list* my_list)
{
    vertex_list_clear(my_list);
    free(my_list->storage);
    free(my_list);
}