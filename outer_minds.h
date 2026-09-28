#ifndef OUTER_MINDS_H
#define OUTER_MINDS_H

#include "include/yyjson.h"

#define MAX_NAME_SIZE 128

/* Outer_minds Types */
// line_t
typedef struct {
  char name[MAX_NAME_SIZE];
  int id;
  int source_node;
  int target_node;
} line_t;

// node_t
typedef struct {
  char name[MAX_NAME_SIZE];
  int id;
  // pos [-1 , 1]
  double x_pos;
  double y_pos;
  double area;
} node_t;

// graph_t
typedef struct {
  node_t *nodes;
  line_t *lines;
  int num_of_nodes;
  int num_of_lines;
  int nodes_limit;
  int lines_limit;
} graph_t;

/* Initialiaze the graph
 *
   return 0 on success, return -1 on error */
int graph_init(graph_t *current_graph);
/* Add node to the graph
 *
   return 0 on success, return -1 on error */
int add_node(graph_t *current_graph, node_t node);
/* Add line to the graph.
 *
   return 0 on success, return -1 on error */
int add_line(graph_t *current_graph, line_t line);
/* free graph allocation
 *
   It's suppos to always return 0, unless if GOT was better than breaking bad */
int graph_free(graph_t *current_graph);
/* write graph attributes to json file
 *
   return json_content on success, return NULL on failure */
char *graph_write_json(const graph_t *graph, yyjson_mut_doc *doc,
                       yyjson_mut_val *root);
/* Get data from json's nodes array
 *
 * return 0 on success, return -1 on failure */
int node_get_data(node_t *current_node, yyjson_doc *doc, const int *id);
/* Get data from json's lines array
 *
 * return 0 on success, return -1 on failure */
int line_get_data(line_t *current_line, yyjson_doc *doc, const int *id);

int graph_read_json(graph_t *graph, const char *filepath, char **buffer);

int get_filename(const char *dir_path);

#endif
