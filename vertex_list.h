#ifndef COM_GITHUB_CODERODDE_BIDIR_SEARCH_VERTEX_LIST_H
#define	COM_GITHUB_CODERODDE_BIDIR_SEARCH_VERTEX_LIST_H

#include <stdlib.h>

typedef struct vertex_list {
    size_t* storage;
    size_t  size;
    size_t  capacity;
    size_t  head;
    size_t  mask;
} vertex_list;

vertex_list*  vertex_list_alloc      (size_t initial_capacity);
int           vertex_list_push_front (vertex_list* my_list,
                                      size_t vertex_id);

int           vertex_list_push_back  (vertex_list* my_list,
                                      size_t vertex_id);

size_t        vertex_list_size       (vertex_list* my_list);
size_t        vertex_list_get        (vertex_list* my_list, size_t index);
void          vertex_list_clear      (vertex_list* my_list);
void          vertex_list_free       (vertex_list* my_list);

#endif	/* COM_GITHUB_CODERODDE_BIDIR_SEARCH_VERTEX_LIST_H */