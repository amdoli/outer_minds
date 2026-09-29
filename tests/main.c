#include "outer_minds.h"
#include "yyjson.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* READ TEST */

#ifdef TEST_READ_ALL
#define READ_MODE 1
#define READ_NODE 1
#define READ_LINE 1

#elif TEST_READ_NODE
#define READ_MODE 1
#define READ_NODE 1

#elif TEST_READ_LINE
#define READ_MODE 1
#define READ_LINE 1
#endif

#ifndef READ_MODE
#define READ_MODE 0
#define READ_NODE 0
#define READ_LINE 0
#endif

int main(int argc, char **argv) {

#if READ_MODE

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <filename> <id>\n", argv[0]);
    return 1;
  }

  yyjson_read_err err;

  yyjson_doc *doc = yyjson_read_file(argv[1], 0, NULL, &err);
  if (!doc) {
    fprintf(stderr, "read error: %s, code: %u at byte position: %lu\n", err.msg,
            err.code, err.pos);
    return 1;
  }

  errno = 0;
  char *endptr;
  int id = (int)strtol(argv[2], &endptr, 0);
  if (endptr == argv[2] || *argv[2] == '\0') {

    fprintf(stderr, "Error: Argument %d ('%s') contains no valid digits.\n", 2,
            argv[2]);
    yyjson_doc_free(doc);
    return 1;
  }
  if (*endptr != '\0') {
    fprintf(stderr, "Error: Argument %d ('%s') is junk value.\n", 2, argv[2]);
    yyjson_doc_free(doc);
    return 1;
  }

  if (errno == ERANGE) {
    fprintf(stderr, "Error: Argument %d ('%s') is out of numerical range.\n", 2,
            argv[2]);
    yyjson_doc_free(doc);
    return 1;
  }

#if READ_NODE
  printf("== NODE READ TEST ==\n\n");

  node_t n_test;

  int result = node_get_data(&n_test, doc, &id);
  if (result < 0) {
    yyjson_doc_free(doc);
    return 1;
  }

  printf("input = %d\n\n", id);

  printf("id = %d, name = %s, x = %f, y = %f \n", n_test.id, n_test.name,
         n_test.x_pos, n_test.y_pos);

#endif

#if READ_LINE
  printf("== LINE READ TEST ==\n\n");

  line_t l_test;

  int result_line = line_get_data(&l_test, doc, &id);
  if (result_line < 0) {
    yyjson_doc_free(doc);
    return 1;
  }

  printf("input = %d\n\n", id);

  printf("id = %d, name = %s, src_id = %d, target_id = %d\n", l_test.id,
         l_test.name, l_test.source_node, l_test.target_node);
#endif

  yyjson_doc_free(doc);
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
