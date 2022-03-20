cmake_minimum_required(VERSION 3.21)
project(untitled C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -ansi -pedantic -fmax-errors=1")
add_executable(untitled main.c algorithm.h algorithm.c dary_heap.c dary_heap.h distance_map.h distance_map.c graph.c graph.h graph_vertex_map.c graph_vertex_map.h parent_map.c parent_map.h util.h vertex_list.c vertex_list.h vertex_set.c vertex_set.h weight_map.c weight_map.h)
