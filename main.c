#include "outer_minds.h"
#include "yyjson.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

#ifdef TEST_READ
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  graph_t *graph = malloc(sizeof(graph_t));
  if (graph_init(graph) != 0)
    return 1;

  char *buffer = malloc(sizeof(char) * 1024);

  int result = graph_read_json(graph, argv[1], &buffer);
  if (result != 0) {
    free(graph);
    return 1;
  }

  printf("%s\n", buffer);

  /*for (int i = 0; i < graph->num_of_nodes; i++) {
    printf("%d- xpos = %f | ypos = %f\n", i, graph->nodes[i].x_pos,
           graph->nodes[i].y_pos);
  } */
  free(buffer);
  free(graph);
  printf("SUCCEED\n");
  return 0;
#endif

#ifdef TEST_GRAPH
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <loop size>", argv[0]);
    return 1;
  }

  int i = 0;
  int j = 0;
  node_t n = {"node_test", i++, -2, 5, 10};
  line_t l = {"line_test", j++, j - 1, j};

  graph_t graph;
  if (graph_init(&graph) != 0) {
    return 1;
  }

  int iteration = (int)strtol(argv[1], NULL, 0);
  for (int ii = 0; ii < iteration; ii++) {
    int result = add_node(&graph, n);
    if (result != 0)
      return 1;
    result = add_line(&graph, l);
    if (result != 0)
      return 1;
  }
  printf("graph lines = %d | graph_lim = %d\n", graph.num_of_lines,
         graph.lines_limit);
  printf("graph nodes = %d | graph_lim = %d\n", graph.num_of_nodes,
         graph.nodes_limit);

  yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
  if (!doc) {
    graph_free(&graph);
    return 1;
  }

  yyjson_mut_val *root = yyjson_mut_obj(doc);
  if (!root) {
    graph_free(&graph);
    yyjson_mut_doc_free(doc);
    return 1;
  }

  yyjson_mut_doc_set_root(doc, root);

  char *json = graph_write_json(&graph, doc, root);
  if (!json) {
    graph_free(&graph);
    yyjson_mut_doc_free(doc);
    return 1;
  }
  FILE *file = fopen("test.json", "w");
  if (!file) {
    perror("Error opening a file");
    graph_free(&graph);
    yyjson_mut_doc_free(doc);
    return 1;
  }

  fputs(json, file);

  fclose(file);
  graph_free(&graph);
  yyjson_mut_doc_free(doc);
  free((void *)json);
  return 0;
#endif

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <filename.json>\n", argv[0]);
    return 1;
  }

#ifdef TEST
  int res = get_filename(argv[1]);
  if (res != 0)
    return EXIT_FAILURE;
  return EXIT_SUCCESS;
#endif

  return 0;
}
