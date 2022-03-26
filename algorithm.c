#include "algorithm.h"
#include "dary_heap.h"
#include "distance_map.h"
#include "graph.h"
#include "parent_map.h"
#include "util.h"
#include "vertex_list.h"
#include "vertex_set.h"
#include <float.h>
#include <stdlib.h>

#define TRY_REPORT_RETURN_STATUS(RETURN_STATUS) \
if (p_return_status) {                          \
    *p_return_status = RETURN_STATUS;           \
}

#define CLEAN_SEARCH_STATE   search_state_free(&search_state_)
#define CLEAN_SEARCH_STATE_2 search_state_2_free(&search_state_2_)

static const size_t INITIAL_MAP_CAPACITY = 1024;
static const float LOAD_FACTOR = 1.3f;
static const size_t DARY_HEAP_DEGREE = 4;

typedef struct search_state {
    dary_heap*     p_open_forward;
    dary_heap*     p_open_backward;
    vertex_set*    p_closed_forward;
    vertex_set*    p_closed_backward;
    distance_map*  p_distance_forward;
    distance_map*  p_distance_backward;
    parent_map*    p_parent_forward;
    parent_map*    p_parent_backward;
} search_state;

static void search_state_init(search_state* p_state) {
    p_state->p_open_forward =
            dary_heap_alloc(
                    DARY_HEAP_DEGREE,
                    INITIAL_MAP_CAPACITY,
                    LOAD_FACTOR);

    p_state->p_open_backward =
            dary_heap_alloc(
                    DARY_HEAP_DEGREE,
                    INITIAL_MAP_CAPACITY,
                    LOAD_FACTOR);

    p_state->p_closed_forward =
            vertex_set_alloc(
                    INITIAL_MAP_CAPACITY,
                    LOAD_FACTOR);

    p_state->p_closed_backward =
            vertex_set_alloc(
                    INITIAL_MAP_CAPACITY,
                    LOAD_FACTOR);

    p_state->p_distance_forward =
            distance_map_alloc(
                    INITIAL_MAP_CAPACITY,
                    LOAD_FACTOR);

    p_state->p_distance_backward =
            distance_map_alloc(
                    INITIAL_MAP_CAPACITY,
                    LOAD_FACTOR);

    p_state->p_parent_forward =
            parent_map_alloc(
                    INITIAL_MAP_CAPACITY,
                    LOAD_FACTOR);

    p_state->p_parent_backward =
            parent_map_alloc(
                    INITIAL_MAP_CAPACITY,
                    LOAD_FACTOR);
}

static int search_state_ok(search_state* p_search_state) {
    return p_search_state->p_open_forward &&
           p_search_state->p_open_backward &&
           p_search_state->p_closed_forward &&
           p_search_state->p_closed_backward &&
           p_search_state->p_distance_forward &&
           p_search_state->p_distance_backward &&
           p_search_state->p_parent_forward &&
           p_search_state->p_parent_backward;
}

static void search_state_free(search_state* p_search_state) {
    if (p_search_state->p_open_forward) {
        dary_heap_free(p_search_state->p_open_forward);
    }

    if (p_search_state->p_open_backward) {
        dary_heap_free(p_search_state->p_open_backward);
    }

    if (p_search_state->p_closed_forward) {
        vertex_set_free(p_search_state->p_closed_forward);
    }

    if (p_search_state->p_closed_backward) {
        vertex_set_free(p_search_state->p_closed_backward);
    }

    if (p_search_state->p_distance_forward) {
        distance_map_free(p_search_state->p_distance_forward);
    }

    if (p_search_state->p_distance_backward) {
        distance_map_free(p_search_state->p_distance_backward);
    }

    if (p_search_state->p_parent_forward) {
        parent_map_free(p_search_state->p_parent_forward);
    }

    if (p_search_state->p_parent_backward) {
        parent_map_free(p_search_state->p_parent_backward);
    }
}
typedef struct search_state_2 {
    dary_heap*     p_open;
    vertex_set*    p_closed;
    distance_map*  p_distance;
    parent_map*    p_parent;
} search_state_2;

static void search_state_2_init(search_state_2* p_state) {
    p_state->p_open =
            dary_heap_alloc(
                    DARY_HEAP_DEGREE,
                    INITIAL_MAP_CAPACITY,
                    LOAD_FACTOR);

    p_state->p_closed =
            vertex_set_alloc(
                    INITIAL_MAP_CAPACITY,
                    LOAD_FACTOR);

    p_state->p_distance =
            distance_map_alloc(
                    INITIAL_MAP_CAPACITY,
                    LOAD_FACTOR);

    p_state->p_parent =
            parent_map_alloc(
                    INITIAL_MAP_CAPACITY,
                    LOAD_FACTOR);
}

static int search_state_2_ok(search_state_2* p_search_state) {
    return p_search_state->p_open &&
           p_search_state->p_closed &&
           p_search_state->p_distance &&
           p_search_state->p_parent;
}

void search_state_2_free(search_state_2* p_search_state) {
    if (p_search_state->p_open) {
        dary_heap_free(p_search_state->p_open);
    }

    if (p_search_state->p_closed) {
        vertex_set_free(p_search_state->p_closed);
    }

    if (p_search_state->p_distance) {
        distance_map_free(p_search_state->p_distance);
    }

    if (p_search_state->p_parent) {
        parent_map_free(p_search_state->p_parent);
    }
}

/* Constructs a shortest path after bidirectional search: */
static vertex_list* traceback_path(size_t touch_vertex_id,
                                   parent_map * parent_forward,
                                   parent_map * parent_backward) {

    vertex_list* path = vertex_list_alloc(100);
    int rs; /* result status */
    size_t vertex_id = touch_vertex_id;
    size_t previous_vertex_id =
            parent_map_get(parent_forward,
                           touch_vertex_id);

    do {
        rs = vertex_list_push_front(path, vertex_id);

        if (rs != RETURN_STATUS_OK) {
            vertex_list_free(path);
            return NULL;
        }

        previous_vertex_id = vertex_id;
        vertex_id = parent_map_get(parent_forward, vertex_id);
    } while (vertex_id != previous_vertex_id);

    vertex_id = parent_map_get(parent_backward, touch_vertex_id);
    previous_vertex_id = touch_vertex_id;

    while (vertex_id != previous_vertex_id) {
        rs = vertex_list_push_back(path, vertex_id);

        if (rs != RETURN_STATUS_OK) {
            vertex_list_free(path);
            return NULL;
        }

        previous_vertex_id = vertex_id;
        vertex_id = parent_map_get(parent_backward, vertex_id);
    }

    return path;
}

/* Runs the bidirectional Dijkstra's algorithm: */
vertex_list* find_shortest_path(Graph * p_graph,
                                size_t source_vertex_id,
                                size_t target_vertex_id,
                                int* p_return_status) {

    search_state search_state_;
    double best_path_length = DBL_MAX;
    double temporary_path_length;
    double tentative_length;
    double weight;
    size_t* p_touch_vertex_id = NULL;
    int rs; /* return status */
    int updated;
    size_t current_vertex_id;
    size_t child_vertex_id;
    size_t parent_vertex_id;
    GraphVertex* p_graph_vertex;

    vertex_list*    p_path;
    dary_heap*      p_open_forward;
    dary_heap*      p_open_backward;
    vertex_set*     p_closed_forward;
    vertex_set*     p_closed_backward;
    distance_map*   p_distance_forward;
    distance_map*   p_distance_backward;
    parent_map*     p_parent_forward;
    parent_map*     p_parent_backward;

    weight_map_iterator* p_weight_map_children_iterator;
    weight_map_iterator* p_weight_map_parents_iterator;

    /* Begin: routine checks. */
    if (!p_graph) {
        TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_GRAPH);
        return NULL;
    }

    rs = 0;

    if (!hasVertex(p_graph, source_vertex_id)) {
        rs |= RETURN_STATUS_NO_SOURCE_VERTEX;
    }

    if (!hasVertex(p_graph, target_vertex_id)) {
        rs |= RETURN_STATUS_NO_TARGET_VERTEX;
    }

    if (rs) {
        CLEAN_SEARCH_STATE;
        TRY_REPORT_RETURN_STATUS(rs);
        return NULL;
    }
    /* End: routine checks. */

    /*
    Handle a case where the source and target vertices are the same.
    Otherwise, the algorithm may return a cycle containing the
    source/target vertex.
     */
    if (source_vertex_id == target_vertex_id) {
        p_path = vertex_list_alloc(1);

        if (p_path) {
            TRY_REPORT_RETURN_STATUS(RETURN_STATUS_OK);
        } else {
            TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);
            return NULL;
        }

        if ((rs = vertex_list_push_back(p_path, source_vertex_id)) != RETURN_STATUS_OK) {
            vertex_list_free(p_path);
            TRY_REPORT_RETURN_STATUS(rs);
            return NULL;
        }

        TRY_REPORT_RETURN_STATUS(RETURN_STATUS_OK);
        return p_path;
    }

    /* Begin: create data structures. */
    search_state_init(&search_state_);

    if (!search_state_ok(&search_state_)) {
        CLEAN_SEARCH_STATE;
        TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);
        return NULL;
    }

    p_open_forward      = search_state_.p_open_forward;
    p_open_backward     = search_state_.p_open_backward;
    p_closed_forward    = search_state_.p_closed_forward;
    p_closed_backward   = search_state_.p_closed_backward;
    p_distance_forward  = search_state_.p_distance_forward;
    p_distance_backward = search_state_.p_distance_backward;
    p_parent_forward    = search_state_.p_parent_forward;
    p_parent_backward   = search_state_.p_parent_backward;

    /* Begin: initialize the state: */
    if (dary_heap_add(p_open_forward,
                      source_vertex_id,
                      0.0) != RETURN_STATUS_OK) {
        CLEAN_SEARCH_STATE;
        TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);
        return NULL;
    }

    if (dary_heap_add(p_open_backward,
                      target_vertex_id,
                      0.0) != RETURN_STATUS_OK) {
        CLEAN_SEARCH_STATE;
        TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);
        return NULL;
    }

    if (distance_map_put(p_distance_forward,
                         source_vertex_id,
                         0.0) != RETURN_STATUS_OK) {
        CLEAN_SEARCH_STATE;
        TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);
        return NULL;
    }

    if (distance_map_put(p_distance_backward,
                         target_vertex_id,
                         0.0) != RETURN_STATUS_OK) {
        CLEAN_SEARCH_STATE;
        TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);
        return NULL;
    }

    if (parent_map_put(p_parent_forward,
                       source_vertex_id,
                       source_vertex_id) != RETURN_STATUS_OK) {

        CLEAN_SEARCH_STATE;
        TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);
        return NULL;
    }

    if (parent_map_put(p_parent_backward,
                       target_vertex_id,
                       target_vertex_id) != RETURN_STATUS_OK) {

        CLEAN_SEARCH_STATE;
        TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);
        return NULL;
    }
    /* End: initialize the state. */

    /* Main loop: */
    while (dary_heap_size(p_open_forward) > 0 &&
           dary_heap_size(p_open_backward) > 0) {

        if (p_touch_vertex_id) {
            /* There is somewhere a vertex at which both the search
            frontiers are meeting: */
            temporary_path_length =
                    distance_map_get(
                            p_distance_forward,
                            dary_heap_min(p_open_forward))
                    +
                    distance_map_get(
                            p_distance_backward,
                            dary_heap_min(p_open_backward));

            if (temporary_path_length > best_path_length) {
                /* Once here, we have a shortest path passing through
                '*p_touch_vertex_id'.
                '*/
                p_path = traceback_path(*p_touch_vertex_id,
                                        p_parent_forward,
                                        p_parent_backward);

                if (p_path) {
                    TRY_REPORT_RETURN_STATUS(RETURN_STATUS_OK);
                } else {
                    TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);
                }

                /* Clean up and return the path. The path may be NULL, which
                implies that there were no sufficient memory available for
                the path. */
                CLEAN_SEARCH_STATE;
                free(p_touch_vertex_id);
                return p_path;
            }
        }

        /* Choose the expansion direction. The smaller of the two search
        frontiers will be selected:
        */
        if (dary_heap_size(p_open_forward) +
            vertex_set_size(p_closed_forward)
            <=
            dary_heap_size(p_open_backward) +
            vertex_set_size(p_closed_backward)) {
            /* Once here, we expanding the forward search frontier generating
            the child vertices of the selected vertex: */

            current_vertex_id = dary_heap_extract_min(p_open_forward);

            /*  Mark that we know the shortest path to 'current_vertex_id': */
            if ((rs = vertex_set_add(p_closed_forward, current_vertex_id)) !=
                RETURN_STATUS_OK) {
                CLEAN_SEARCH_STATE;
                TRY_REPORT_RETURN_STATUS(rs);
                return NULL;
            }

            p_graph_vertex =
                    graph_vertex_map_get(p_graph->p_nodes,
                                         current_vertex_id);

            p_weight_map_children_iterator =
                    weight_map_iterator_alloc(
                            p_graph_vertex->p_children);

            if (!p_weight_map_children_iterator) {
                CLEAN_SEARCH_STATE;
                TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);

                if (p_touch_vertex_id) {
                    free(p_touch_vertex_id);
                }

                return NULL;
            }

            /* Expand the 'current_vertex_id'. In other words,
            iterate over all child nodes of 'current_vertex_id': */
            while (weight_map_iterator_has_next(
                    p_weight_map_children_iterator)) {

                updated = 0;

                weight_map_iterator_visit(p_weight_map_children_iterator,
                                          &child_vertex_id,
                                          &weight);

                weight_map_iterator_next(p_weight_map_children_iterator);

                if (vertex_set_contains(p_closed_forward, child_vertex_id)) {
                    /* Once here, the shortest path to 'child_vertex_id' is already known.
                    Omit it: */
                    continue;
                }

                tentative_length = distance_map_get(p_distance_forward,
                                                    current_vertex_id) +
                                   weight;

                if (!distance_map_contains_vertex_id(p_distance_forward,
                                                     child_vertex_id)) {
                    /* Once here, we reached 'child_vertex_id' for the first time! */
                    /* Add to the forward priority queue: */
                    if ((rs = dary_heap_add(
                            p_open_forward,
                            child_vertex_id,
                            tentative_length)) != RETURN_STATUS_OK) {

                        CLEAN_SEARCH_STATE;
                        TRY_REPORT_RETURN_STATUS(rs);

                        if (p_touch_vertex_id) {
                            free(p_touch_vertex_id);
                        }

                        return NULL;
                    }

                    /* Mark that the distance and the parent maps should
                    be updated: */
                    updated = 1;
                }
                else if (distance_map_get(p_distance_forward, child_vertex_id) >
                         tentative_length) {
                    /* Once here, we are reaching the 'child_vertex_id' for not the first time,
                    byt we can lower its shortest path estimate. For that reason, we lower
                    that estimate: */
                    dary_heap_decrease_key(
                            p_open_forward,
                            child_vertex_id,
                            tentative_length);

                    /* Mark that the distance and the parent maps should
                    be updated: */
                    updated = 1;
                }

                if (updated) {
                    /* Once here, we need to update the shortest path estimate
                    and the parent of the 'child_vertex_id':
                    */
                    if ((rs = distance_map_put(
                            p_distance_forward,
                            child_vertex_id,
                            tentative_length)) != RETURN_STATUS_OK) {

                        CLEAN_SEARCH_STATE;
                        TRY_REPORT_RETURN_STATUS(rs);
                        free(NULL);
                        free(p_touch_vertex_id);
                        return NULL;
                    }

                    if ((rs = parent_map_put(
                            p_parent_forward,
                            child_vertex_id,
                            current_vertex_id)) != RETURN_STATUS_OK) {

                        CLEAN_SEARCH_STATE;
                        TRY_REPORT_RETURN_STATUS(rs);
                        free(p_touch_vertex_id);
                        return NULL;
                    }

                    /* Checks whether we can find the meeting vertex: */
                    if (vertex_set_contains(p_closed_backward,
                                            child_vertex_id)) {

                        temporary_path_length =
                                tentative_length +
                                distance_map_get(p_distance_backward,
                                                 child_vertex_id);

                        /* Can we improve the cost of a shortest path via the
                        meeting point?
                        */
                        if (best_path_length > temporary_path_length) {
                            best_path_length = temporary_path_length;

                            if (!p_touch_vertex_id) {
                                p_touch_vertex_id = malloc(sizeof(size_t));
                            }

                            *p_touch_vertex_id = child_vertex_id;
                        }
                    }
                }
            }
        }
        else {
            /* Once here, we expand the backward search frontier generating
            all the parents of the selected of the selected vertex:
            */
            current_vertex_id = dary_heap_extract_min(p_open_backward);
            vertex_set_add(p_closed_backward, current_vertex_id);

            p_graph_vertex =
                    graph_vertex_map_get(p_graph->p_nodes,
                                         current_vertex_id);

            p_weight_map_parents_iterator =
                    weight_map_iterator_alloc(
                            p_graph_vertex->p_parents);

            /* Expand the 'current_vertex_id' in backward direction generating
            its parents: */
            while (weight_map_iterator_has_next(
                    p_weight_map_parents_iterator)) {

                updated = 0;

                weight_map_iterator_visit(
                        p_weight_map_parents_iterator,
                        &parent_vertex_id,
                        &weight);

                weight_map_iterator_next(p_weight_map_parents_iterator);

                if (vertex_set_contains(p_closed_backward,
                                        parent_vertex_id)) {
                    /* Once here, the shortest distance to 'parent_vertex_id'
                    is already known. Omit it!*/
                    continue;
                }

                tentative_length = distance_map_get(p_distance_backward,
                                                    current_vertex_id)
                                   + weight;

                if (!distance_map_contains_vertex_id(p_distance_backward,
                                                     parent_vertex_id)) {
                    /* Once here, 'parent_vertex_id' is reached for the
                    first time. Add it to the backward search frontier: */
                    if ((rs = dary_heap_add(
                            p_open_backward,
                            parent_vertex_id,
                            tentative_length)) != RETURN_STATUS_OK) {

                        CLEAN_SEARCH_STATE;
                        TRY_REPORT_RETURN_STATUS(rs);
                        free(p_touch_vertex_id);
                        return NULL;
                    }

                    updated = 1;
                }
                else if (distance_map_get(p_distance_backward,
                                          parent_vertex_id)
                         >
                         tentative_length) {
                    /* Once here, we can improve the distance to
                    'parent_vertex_id: */
                    dary_heap_decrease_key(
                            p_open_backward,
                            parent_vertex_id,
                            tentative_length);

                    updated = 1;
                }

                if (updated) {
                    /* Once here, we need to update the
                    auxiliary info: */
                    if ((rs = distance_map_put(
                            p_distance_backward,
                            parent_vertex_id,
                            tentative_length)) != RETURN_STATUS_OK) {

                        CLEAN_SEARCH_STATE;
                        TRY_REPORT_RETURN_STATUS(rs);
                        free(p_touch_vertex_id);
                        return NULL;
                    }

                    if ((rs = parent_map_put(
                            p_parent_backward,
                            parent_vertex_id,
                            current_vertex_id)) != RETURN_STATUS_OK) {

                        CLEAN_SEARCH_STATE;
                        TRY_REPORT_RETURN_STATUS(rs);
                        free(p_touch_vertex_id);
                        return NULL;
                    }

                    if (vertex_set_contains(p_closed_forward,
                                            parent_vertex_id)) {
                        /* Once here, there is a meeting vertex: */
                        temporary_path_length =
                                tentative_length +
                                distance_map_get(p_distance_forward,
                                                 parent_vertex_id);

                        /* Possibly update the meeting vertex and the cost of the
                        path passing through it: */
                        if (best_path_length > temporary_path_length) {
                            best_path_length = temporary_path_length;

                            if (!p_touch_vertex_id) {
                                p_touch_vertex_id = malloc(sizeof(size_t));
                            }

                            *p_touch_vertex_id = parent_vertex_id;
                        }
                    }
                }
            }
        }
    }

    if (p_touch_vertex_id) {
        free(p_touch_vertex_id);
    }

    /* Once here, there is no path from the source vertex to
    the target vertex: */
    CLEAN_SEARCH_STATE;
    TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_PATH);
    return NULL;
}

/* Constructs the shortest path after unidirectional search: */
static vertex_list* traceback_path_2(size_t target_vertex_id,
                                     parent_map* parent) {

    vertex_list* path = vertex_list_alloc(100);
    int rs; /* result status */
    size_t vertex_id = target_vertex_id;
    size_t previous_vertex_id =
            parent_map_get(parent,
                           target_vertex_id);

    do {
        rs = vertex_list_push_front(path, vertex_id);

        if (rs != RETURN_STATUS_OK) {
            vertex_list_free(path);
            return NULL;
        }

        previous_vertex_id = vertex_id;
        vertex_id = parent_map_get(parent, vertex_id);
    } while (vertex_id != previous_vertex_id);

    return path;
}

/* Runs the traditional (unidirectional) Dijkstra's algorithm: */
vertex_list* find_shortest_path_2(Graph* p_graph,
                                  size_t source_vertex_id,
                                  size_t target_vertex_id,
                                  int* p_return_status) {

    search_state_2 search_state_2_;
    size_t current_vertex_id;
    size_t child_vertex_id;
    double weight;
    double tentative_length;
    GraphVertex* p_graph_vertex;
    int rs; /* return status */
    int updated;

    vertex_list*  p_path;
    dary_heap*    p_open;
    vertex_set*   p_closed;
    distance_map* p_distance;
    parent_map*   p_parent;

    weight_map_iterator* p_weight_map_children_iterator;

    /* Begin: routing checks. */
    if (!p_graph) {
        TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_GRAPH);
        return NULL;
    }

    rs = 0;

    if (!hasVertex(p_graph, source_vertex_id)) {
        rs |= RETURN_STATUS_NO_SOURCE_VERTEX;
    }

    if (!hasVertex(p_graph, target_vertex_id)) {
        rs |= RETURN_STATUS_NO_TARGET_VERTEX;
    }

    if (rs) {
        CLEAN_SEARCH_STATE_2;
        TRY_REPORT_RETURN_STATUS(rs);
        return NULL;
    }
    /* End: routine checks. */

    search_state_2_init(&search_state_2_);

    if (!search_state_2_ok(&search_state_2_)) {
        CLEAN_SEARCH_STATE_2;
        TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);
        return NULL;
    }

    p_open     = search_state_2_.p_open;
    p_closed   = search_state_2_.p_closed;
    p_distance = search_state_2_.p_distance;
    p_parent   = search_state_2_.p_parent;

    /* Begin: initialize the state: */
    if ((rs = dary_heap_add(p_open,
                            source_vertex_id,
                            0.0)) != RETURN_STATUS_OK) {
        CLEAN_SEARCH_STATE_2;
        TRY_REPORT_RETURN_STATUS(rs);
        return NULL;
    }

    if ((rs = distance_map_put(p_distance,
                               source_vertex_id,
                               0.0)) != RETURN_STATUS_OK) {
        CLEAN_SEARCH_STATE_2;
        TRY_REPORT_RETURN_STATUS(rs);
        return NULL;
    }

    if ((rs = parent_map_put(p_parent,
                             source_vertex_id,
                             source_vertex_id)) != RETURN_STATUS_OK) {

        CLEAN_SEARCH_STATE_2;
        TRY_REPORT_RETURN_STATUS(rs);
        return NULL;
    }
    /* End: intitialize the state. */

    /* Main loop: */
    while (dary_heap_size(p_open) > 0) {
        current_vertex_id = dary_heap_extract_min(p_open);

        if (current_vertex_id == target_vertex_id) {
            /* Once here, the search has reached the target vertex. */
            p_path = traceback_path_2(target_vertex_id, p_parent);

            CLEAN_SEARCH_STATE_2;

            if (p_path) {
                TRY_REPORT_RETURN_STATUS(RETURN_STATUS_OK);
            }
            else {
                TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);
            }

            return p_path;
        }

        if (vertex_set_contains(p_closed, current_vertex_id)) {
            /* Once here, the shortest path distance to 'current_vertex_id'
            is already known. Omit it.*/
            continue;
        }

        if ((rs = vertex_set_add(p_closed, current_vertex_id))
            != RETURN_STATUS_OK) {
            /* Mark 'current_vertex_id' as settled. */
            CLEAN_SEARCH_STATE_2;
            TRY_REPORT_RETURN_STATUS(rs);
            return NULL;
        }

        p_graph_vertex =
                graph_vertex_map_get(p_graph->p_nodes,
                                     current_vertex_id);

        p_weight_map_children_iterator =
                weight_map_iterator_alloc(
                        p_graph_vertex->p_children);

        if (!p_weight_map_children_iterator) {
            CLEAN_SEARCH_STATE_2;
            TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_MEMORY);
            return NULL;
        }

        /* Iterate over all child vertices of the 'current_vertex_id': */
        while (weight_map_iterator_has_next(
                p_weight_map_children_iterator)) {

            updated = FALSE;

            weight_map_iterator_visit(p_weight_map_children_iterator,
                                      &child_vertex_id,
                                      &weight);

            weight_map_iterator_next(p_weight_map_children_iterator);

            if (vertex_set_contains(p_closed, child_vertex_id)) {
                /* The shortest path distance to 'child_vertex_id' is already
                known. Omit it. */
                continue;
            }

            tentative_length = distance_map_get(p_distance,
                                                current_vertex_id) +
                               weight;

            if (!distance_map_contains_vertex_id(p_distance,
                                                 child_vertex_id)) {
                /* Once here, 'child_vertex_id' is reached for the first
                time. Add it to the search frontier: */
                updated = TRUE;

                if ((rs = dary_heap_add(
                        p_open,
                        child_vertex_id,
                        tentative_length))
                    != RETURN_STATUS_OK) {

                    CLEAN_SEARCH_STATE_2;
                    TRY_REPORT_RETURN_STATUS(rs);
                    return NULL;
                }
            } else if (distance_map_get(p_distance, child_vertex_id) >
                       tentative_length) {
                /* Once here, we can improve the shortest path estimate to
                'child_vertex_id':
                */
                updated = TRUE;

                dary_heap_decrease_key(
                        p_open,
                        child_vertex_id,
                        tentative_length);
            }

            if (updated) {
                /* Once here, we need to update the state for the
                'child_vertex_id':
                */
                if ((rs = distance_map_put(
                        p_distance,
                        child_vertex_id,
                        tentative_length))
                    != RETURN_STATUS_OK) {

                    CLEAN_SEARCH_STATE_2;
                    TRY_REPORT_RETURN_STATUS(rs);
                    return NULL;
                }

                if ((rs = parent_map_put(
                        p_parent,
                        child_vertex_id,
                        current_vertex_id))
                    != RETURN_STATUS_OK) {
                    CLEAN_SEARCH_STATE_2;
                    TRY_REPORT_RETURN_STATUS(rs);
                    return NULL;
                }
            }
        }
    }

    /* Once here, there is no path from the source vertex
    to the target vertex:
    */
    CLEAN_SEARCH_STATE_2;
    TRY_REPORT_RETURN_STATUS(RETURN_STATUS_NO_PATH);
    return NULL;
}
