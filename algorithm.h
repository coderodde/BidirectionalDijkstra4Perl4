#ifndef COM_GITHUB_CODERODDE_PERL_ALGORITHM_H
#define COM_GITHUB_CODERODDE_PERL_ALGORITHM_H

#include "graph.h"
#include "vertex_list.h"

vertex_list* find_shortest_path(Graph* p_graph,
                                size_t source_vertex_id,
                                size_t target_vertex_id,
                                int* p_return_status);

vertex_list* find_shortest_path_2(Graph* p_graph,
                                  size_t source_vertex_id,
                                  size_t target_vertex_id,
                                  int* p_return_status);

#endif /* COM_GITHUB_CODERODDE_PERL_ALGORITHM_H */