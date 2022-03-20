#ifndef COM_GITHUB_CODERODDE_BIDIR_SEARCH_GRAPH_H
#define COM_GITHUB_CODERODDE_BIDIR_SEARCH_GRAPH_H

#include "graph_vertex_map.h"
#include "weight_map.h"
#include <stdlib.h>

typedef struct GraphVertex {
    size_t id;
    weight_map* p_children; /* Maps a child to the edge weight. */
    weight_map* p_parents;  /* Maps a parent to the edge weight. */
} GraphVertex;

typedef struct Graph {
    /* Maps each node ID to a vertex: */
    struct graph_vertex_map* p_nodes;
} Graph;

void initGraphVertex(GraphVertex* p_graph_vertex, size_t id);
void freeGraphVertex(GraphVertex* p_graph_vertex);

Graph* allocGraph();

void initGraph(Graph* p_graph);
void freeGraph(Graph* p_graph);

GraphVertex* addVertex (Graph* p_graph, size_t vertex_id);
void removeVertex      (Graph* p_graph, size_t vertex_id);
int hasVertex          (Graph* p_graph, size_t vertex_id);
GraphVertex* getVertex (Graph* p_graph, size_t vertex_id);

int addEdge(
        Graph* p_graph,
        size_t tail_vertex_id,
        size_t head_vertex_id,
        double weight);

void removeEdge(Graph* graph,
                size_t tail_vertex_id,
                size_t head_vertex_id);

int hasEdge(Graph* p_graph,
            size_t tail_vertex_id,
            size_t head_vertex_id);

double getEdgeWeight(Graph* p_graph,
                     size_t tail_vertex_id,
                     size_t head_vertex_id);

#endif /* COM_GITHUB_CODERODDE_BIDIR_SEARCH_GRAPH_H */