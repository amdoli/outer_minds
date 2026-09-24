#include "include/yyjson.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_SIZE 128
#define MAX_NUMBER_OF_LINES 32
#define BASE_LIST 16

#if defined(__linux__) || defined(__gnu_linux__)
#define OUTER_MINDS_PLATFORM_LINUX 1
#define OUTER_MINDS_HAS_DIRENT 1
#elif defined(__APPLE__) || defined(__MACH__)
#define OUTER_MINDS_PLATFORM_MAC 1
#define OUTER_MINDS_HAS_DIRENT 1
#elif defined(_WIN32) || defined(_WIN64)
#define OUTER_MINDS_PLATFORM_WINDOWS 1
#elif defined(__EMSCRIPTEN__)
#define OUTER_MINDS_PLATFORM_WASM 1
#define OUTER_MINDS_HAS_DIRENT 1
#else
#define OUTER_MINDS_PLATFORM_UNKNOWN 1
#endif

#ifndef OUTER_MINDS_HAS_DIRENT
#define OUTER_MINDS_HAS_DIRENT 0
#endif

#if OUTER_MINDS_HAS_DIRENT
#include <dirent.h>
#else
#include <windows.h>
#endif

typedef struct {
  char name[MAX_NAME_SIZE];
  int id;
  int source_node;
  int target_node;
} line_t;

typedef struct {
  char name[MAX_NAME_SIZE];
  int id;
  // pos [-1 , 1]
  double x_pos;
  double y_pos;
  double area;
} node_t;

typedef struct {
  node_t *nodes;
  line_t *lines;
  int num_of_nodes;
  int num_of_lines;
  int nodes_limit;
  int lines_limit;
} graph;

int graph_init(graph *current_graph) {
  current_graph->nodes = malloc(sizeof(node_t) * BASE_LIST);
  if (!current_graph->nodes) {
    perror("nodes malloc failed");
    return -1;
  }

  current_graph->lines = malloc(sizeof(line_t) * BASE_LIST);
  if (!current_graph->lines) {
    perror("lines malloc failed");
    free(current_graph->nodes);
    return -1;
  }
  current_graph->num_of_nodes = 0;
  current_graph->num_of_lines = 0;
  current_graph->nodes_limit = BASE_LIST;
  current_graph->lines_limit = BASE_LIST;
  return 0;
}

int add_node(graph *current_graph, node_t node) {
  if (current_graph->num_of_nodes >= current_graph->nodes_limit) {
    current_graph->nodes_limit *= 2;
    void *ptr = realloc(current_graph->nodes,
                        current_graph->nodes_limit * sizeof(node_t));
    if (!ptr) {
      perror("failed realloc nodes");
      return -1;
    }
    current_graph->nodes = ptr;
  }

  current_graph->nodes[current_graph->num_of_nodes++] = node;
  return 0;
}

/* getting the file name */
int get_filename(const char *dir_path) {
#if OUTER_MINDS_HAS_DIRENT
  DIR *dir = opendir(dir_path);
  if (!dir) {
    perror("Error opening directory");
    return -1;
  }

  struct dirent *de;
  errno = 0;
  printf("=====================\n");

  while ((de = readdir(dir)) != NULL) {
    if (errno != 0) {
      perror("Error reading directory");
      closedir(dir);
      return -1;
    }

    printf("'%s' ", de->d_name);
  }

  printf("\n=====================\n");
  closedir(dir);
  return 0;
#else
  fprintf(stderr, "Currently not supporting windows.\n");
  fprintf(stderr, "I will work on it as soon I'm free");
  return -1;
#endif
}

/* writing to a file */

int write_to_json(const node_t n, const char *filename) {

  yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
  if (!doc) {
    perror("Failed to create JSON document");
    return -1;
  }

  yyjson_mut_val *root = yyjson_mut_obj(doc);
  if (!root) {
    perror("Failed to create JSON root");
    yyjson_mut_doc_free(doc);
    return -1;
  }

  yyjson_mut_doc_set_root(doc, root);

  yyjson_mut_obj_add_str(doc, root, "name", n.name);
  yyjson_mut_obj_add_int(doc, root, "id", n.id);
  yyjson_mut_obj_add_double(doc, root, "x_pos", n.x_pos);
  yyjson_mut_obj_add_double(doc, root, "y_pos", n.y_pos);

  const char *json = yyjson_mut_write(doc, 0, NULL);
  if (!json) {
    fprintf(stderr, "Error: json variable failed to recive data.\n");
    yyjson_mut_doc_free(doc);
    return -1;
  }

  FILE *file = fopen(filename, "w");
  if (!file) {
    perror("Error opening json file");
    free((void *)json);
    yyjson_mut_doc_free(doc);
    return -1;
  }

  fputs(json, file);

  fclose(file);
  free((void *)json);
  yyjson_mut_doc_free(doc);

  return 0;
}

int main(int argc, char **argv) {

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

  node_t n1;

  printf("Enter the the name of the node:");
  if (fgets(n1.name, MAX_NAME_SIZE, stdin) == NULL) {
    fprintf(stderr, "Error: fgets didnt read any chars\n");
    return 1;
  }
  char *pnewline = strchr(n1.name, '\n');
  if (pnewline)
    *pnewline = '\0';

  printf("Enter the x_pos: ");
  scanf("%lf", &n1.x_pos);

  printf("Enter the y_pos: ");
  scanf("%lf", &n1.y_pos);

  n1.id = 1;

  int result = write_to_json(n1, argv[1]);
  if (result < 0) {
    return 1;
  }
  return 0;
}
