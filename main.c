#include "algorithm.h"
#include "graph.h"
#include "vertex_list.h"
#include "vertex_set.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static clock_t milliseconds()
{
    return clock() / (CLOCKS_PER_SEC / 1000);
}

static const size_t NODES = 100 * 1000;
static const size_t EDGES = 500 * 1000;

static int paths_are_equal(vertex_list* path_1,
                           vertex_list* path_2) {
    size_t i;

    if (vertex_list_size(path_1) != vertex_list_size(path_2)) {
        return FALSE;
    }

    for (i = 0; i < vertex_list_size(path_1); ++i) {
        if (vertex_list_get(path_1, i) != vertex_list_get(path_2, i)) {
            return FALSE;
        }
    }

    return TRUE;
}

static double randfrom(double min, double max)
{
    double range = (max - min);
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

static size_t intrand() {
    size_t a = rand();
    size_t b = rand();
    return ((a << 15) | b);
}

double get_path_length(vertex_list* path,
                       Graph* graph) {
    size_t i;
    size_t vertex_id_1;
    size_t vertex_id_2;
    double length = 0.0;

    for (i = 0; i < vertex_list_size(path) - 1; ++i) {
        vertex_id_1 = vertex_list_get(path, i);
        vertex_id_2 = vertex_list_get(path, i + 1);

        length += getEdgeWeight(graph,
                                vertex_id_1,
                                vertex_id_2);
    }

    return length;
}

Graph* buildGraph() {
    Graph* p_graph = allocGraph();

    size_t i;
    size_t id1;
    size_t id2;
    size_t edge;
    double weight;
    size_t source_vertex_id = 0;
    size_t target_vertex_id = 0;
    clock_t milliseconds_a;
    clock_t milliseconds_b;
    vertex_list* path;
    vertex_list* path_2;
    int rs = -1;
    unsigned random_seed;
    initGraph(p_graph);

    random_seed = (unsigned) time(NULL);
    srand(random_seed);
    printf("Seed = %d\n\n", random_seed);

    milliseconds_a = milliseconds();

    for (edge = 0; edge < EDGES; ++edge)
    {
        id1 = intrand() % NODES;
        id2 = intrand() % NODES;
        weight = randfrom(0.0, 10.0);
        addEdge(p_graph, id1, id2, weight);

        if (edge == 0) {
            source_vertex_id = id1;
        }
        else if (edge == NODES / 2) {
            target_vertex_id = id2;
        }
    }

    milliseconds_b = milliseconds();
    printf("Built the graph in %ld milliseconds.\n",
           (milliseconds_b - milliseconds_a));

    printf("Source node: %d\n", (int) source_vertex_id);
    printf("Target node: %d\n", (int) target_vertex_id);

    puts("--- Bidirectional Dijkstra:");

    milliseconds_a = milliseconds();
    path = find_shortest_path(p_graph,
                              source_vertex_id,
                              target_vertex_id,
                              &rs);

    milliseconds_b = milliseconds();

    for (i = 0; i < vertex_list_size(path); ++i) {
        printf("%d\n", (int) vertex_list_get(path, i));
    }

    printf("Path length: %f\n", get_path_length(path, p_graph));
    printf("Duration: %ld milliseconds.\n",
           (milliseconds_b - milliseconds_a));

    printf("Result status: %d\n\n", rs);
    puts("--- Original Dijkstra:");

    milliseconds_a = milliseconds();
    path_2 = find_shortest_path_2(p_graph,
                                  source_vertex_id,
                                  target_vertex_id,
                                  &rs);

    milliseconds_b = milliseconds();

    for (i = 0; i < vertex_list_size(path_2); ++i) {
        printf("%d\n", (int) vertex_list_get(path_2, i));
    }

    printf("Path length: %f\n", get_path_length(path_2, p_graph));
    printf("Duration: %ld milliseconds.\n",
           (milliseconds_b - milliseconds_a));

    printf("Result status: %d\n", rs);

    printf("Algorithms agree: %d\n", paths_are_equal(path, path_2));

    vertex_list_free(path);
    vertex_list_free(path_2);

    freeGraph(p_graph);
    return p_graph;
}

void testSameVertex() {
    size_t i;
    vertex_list* path;
    Graph* graph = allocGraph();
    int rs = -1;

    addEdge(graph, 0, 1, 1.0);
    addEdge(graph, 1, 2, 2.0);
    addEdge(graph, 2, 3, 3.0);
    addEdge(graph, 3, 0, 4.0);

    path = find_shortest_path(graph, 0, 0, &rs);

    printf("size: %d\n", (int) vertex_list_size(path));

    graph = allocGraph();

    addEdge(graph, 0, 1, 10.0);
    addEdge(graph, 0, 2, 2.0);
    addEdge(graph, 2, 1, 3.0);
    addEdge(graph, 0, 3, 3.0);
    addEdge(graph, 3, 1, 5.0);

    path = find_shortest_path(graph, 0, 1, &rs);

    for (i = 0; i < vertex_list_size(path); ++i) {
        printf("%d\n", (int) vertex_list_get(path, i));
    }
}

int main(int argc, char* argv[])
{
    buildGraph();
    return 0;
}