#include "outer_minds.h"
#include "yyjson.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUMBER_OF_LINES 32
#define BASE_LIST 16
#define MAX_STRING_ID_LEN 19

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

/* Outer_minds Types */

int graph_init(graph_t *current_graph) {
  if (!current_graph) {
    fprintf(stderr, "Error: Graph is NULL\n");
    return -1;
  }

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

int add_node(graph_t *current_graph, node_t node) {
  if (current_graph->num_of_nodes >= current_graph->nodes_limit) {
    int new_limit = current_graph->nodes_limit * 2;
    void *ptr = realloc(current_graph->nodes, new_limit * sizeof(node_t));
    if (!ptr) {
      perror("failed realloc nodes");
      return -1;
    }
    current_graph->nodes = ptr;
    current_graph->nodes_limit = new_limit;
  }

  current_graph->nodes[current_graph->num_of_nodes++] = node;
  return 0;
}

int add_line(graph_t *current_graph, line_t line) {
  if (current_graph->num_of_lines >= current_graph->lines_limit) {
    int new_limit = current_graph->lines_limit * 2;
    void *ptr = realloc(current_graph->lines, new_limit * sizeof(line_t));
    if (!ptr) {
      perror("failed realloc lines");
      return -1;
    }
    current_graph->lines = ptr;
    current_graph->lines_limit = new_limit;
  }

  current_graph->lines[current_graph->num_of_lines++] = line;
  return 0;
}

int graph_free(graph_t *current_graph) {
  free(current_graph->lines);
  free(current_graph->nodes);
  return 0;
}

char *graph_write_json(const graph_t *graph, yyjson_mut_doc *doc,
                       yyjson_mut_val *root) {
  // it requires doc and root to be already linked
  if (!graph || !doc || !root)
    return NULL;

  yyjson_mut_val *nodes_arr = yyjson_mut_arr(doc);
  if (!nodes_arr) {
    fprintf(
        stderr,
        "Error: yyjson_mut_arr failed to return data due to lack of memory\n");
    return NULL;
  }
  for (int i = 0; i < graph->num_of_nodes; i++) {
    // Create an object {} for node
    yyjson_mut_val *node_obj = yyjson_mut_obj(doc);
    const node_t *current_node = &graph->nodes[i];
    // node_t = {name, id, xpos, ypos, area}
    yyjson_mut_obj_add_str(doc, node_obj, "name", current_node->name);
    yyjson_mut_obj_add_int(doc, node_obj, "id", current_node->id);
    yyjson_mut_obj_add_double(doc, node_obj, "x_pos", current_node->x_pos);
    yyjson_mut_obj_add_double(doc, node_obj, "y_pos", current_node->y_pos);
    yyjson_mut_obj_add_double(doc, node_obj, "area", current_node->area);

    // append object to array
    yyjson_mut_arr_append(nodes_arr, node_obj);
  }

  yyjson_mut_val *lines_arr = yyjson_mut_arr(doc);
  if (!lines_arr) {
    fprintf(
        stderr,
        "Error: yyjson_mut_arr failed to return data due to lack of memory\n");
    return NULL;
  }

  for (int i = 0; i < graph->num_of_lines; i++) {
    yyjson_mut_val *line_obj = yyjson_mut_obj(doc);
    const line_t *current_line = &graph->lines[i];
    // line_t = {name, id, source, target}
    yyjson_mut_obj_add_str(doc, line_obj, "name", current_line->name);
    yyjson_mut_obj_add_int(doc, line_obj, "id", current_line->id);
    yyjson_mut_obj_add_int(doc, line_obj, "source_id",
                           current_line->source_node);
    yyjson_mut_obj_add_int(doc, line_obj, "target_id",
                           current_line->target_node);

    yyjson_mut_arr_append(lines_arr, line_obj);
  }

  // now we can add the object array to the root
  yyjson_mut_obj_add_val(doc, root, "nodes", nodes_arr);
  yyjson_mut_obj_add_val(doc, root, "lines", lines_arr);

  char *json = yyjson_mut_write(doc, YYJSON_WRITE_PRETTY, NULL);
  if (!json) {
    fprintf(stderr, "Error: yyjson_mut_write failed to generate JSON string\n");
    return NULL;
  }

  return json;
}

int node_get_data(node_t *current_node, yyjson_doc *doc, const int *id) {
  if (!doc) {
    fprintf(stderr, "Error: doc is NULL.\n");
    return -1;
  }

  char json_pointer[MAX_STRING_ID_LEN];

  int written = snprintf(json_pointer, MAX_STRING_ID_LEN, "/nodes/%d", *id);

  if (written < 0) {
    fprintf(stderr, "Error: formatting pointer failed.\n");
    return -1;
  }

  if ((size_t)written >= sizeof json_pointer) {
    fprintf(stderr, "Error: JSON pointer was trancated.\n");
    return -1;
  }

  // test
  printf("json_pointer = %s\n", json_pointer);

  yyjson_val *node_obj = yyjson_doc_ptr_get(doc, json_pointer);
  if (!node_obj) {
    fprintf(stderr, "Error: ether the file you have chosen not formatted "
                    "correctly, or out of index.\n");
    return -1;
  }

  yyjson_val *id_val = yyjson_obj_get(node_obj, "id");
  yyjson_val *name_val = yyjson_obj_get(node_obj, "name");
  yyjson_val *x_val = yyjson_obj_get(node_obj, "x_pos");
  yyjson_val *y_val = yyjson_obj_get(node_obj, "y_pos");
  yyjson_val *area_val = yyjson_obj_get(node_obj, "area");

  written = snprintf(current_node->name, MAX_NAME_SIZE, "%s",
                     yyjson_get_str(name_val));

  if (written < 0) {
    fprintf(stderr, "Error: copying name failed.\n");
    return -1;
  }

  if ((size_t)written >= sizeof current_node->name) {
    fprintf(stderr, "Error: node name was truncated.\n");
    return -1;
  }

  current_node->id = yyjson_get_int(id_val);
  current_node->x_pos = yyjson_get_real(x_val);
  current_node->y_pos = yyjson_get_real(y_val);
  current_node->area = yyjson_get_real(area_val);

  return 0;
}

int line_get_data(line_t *current_line, yyjson_doc *doc, const int *id) {
  if (!doc) {
    fprintf(stderr, "Error: lines_arr is NULL.\n");
    return -1;
  }

  char json_pointer[MAX_STRING_ID_LEN];

  int written = snprintf(json_pointer, MAX_STRING_ID_LEN, "/lines/%d", *id);

  if (written < 0) {
    fprintf(stderr, "Error: formatting pointer failed.\n");
    return -1;
  }

  if ((size_t)written >= sizeof json_pointer) {
    fprintf(stderr, "Error: JSON pointer was trancated.\n");
    return -1;
  }

  // test
  printf("json_pointer = %s\n", json_pointer);

  yyjson_val *line_obj = yyjson_doc_ptr_get(doc, json_pointer);
  if (!line_obj) {
    fprintf(stderr, "Error: ether the file you have chosen not formatted "
                    "correctly, or out of index.\n");
    return -1;
  }

  yyjson_val *id_val = yyjson_obj_get(line_obj, "id");
  yyjson_val *name_val = yyjson_obj_get(line_obj, "name");
  yyjson_val *source_val = yyjson_obj_get(line_obj, "source_id");
  yyjson_val *target_val = yyjson_obj_get(line_obj, "target_id");

  written = snprintf(current_line->name, MAX_NAME_SIZE, "%s",
                     yyjson_get_str(name_val));

  if (written < 0) {
    fprintf(stderr, "Error: copying name failed.\n");
    return -1;
  }

  if ((size_t)written >= sizeof current_line->name) {
    fprintf(stderr, "Error: line name was truncated.\n");
    return -1;
  }

  current_line->id = yyjson_get_int(id_val);
  current_line->source_node = yyjson_get_int(source_val);
  current_line->target_node = yyjson_get_int(target_val);

  return 0;
}

// test this if I didn't like it I will use the pointer method
int graph_read_json(graph_t *graph, const char *filepath, char **buffer) {
  if (!graph) {
    fprintf(stderr, "Error: graph is NULL\n");
    return -1;
  }
  yyjson_read_err err;

  yyjson_doc *doc = yyjson_read_file(filepath, 0, NULL, &err);
  if (!doc) {
    fprintf(stderr, "read error: %s, code: %u at byte position: %lu\n", err.msg,
            err.code, err.pos);
    return -1;
  }

  yyjson_val *root = yyjson_doc_get_root(doc);
  if (!root || !yyjson_is_obj(root)) {
    yyjson_doc_free(doc);
    return -1;
  }

  yyjson_val *nodes_arr = yyjson_obj_get(root, "nodes");
  if (!nodes_arr && !yyjson_is_arr(nodes_arr)) {
    yyjson_doc_free(doc);
    return -1;
  }

  yyjson_val *lines_arr = yyjson_obj_get(root, "lines");
  if (!lines_arr && !yyjson_is_arr(lines_arr)) {
    yyjson_doc_free(doc);
    return -1;
  }

  *buffer = yyjson_write(doc, YYJSON_WRITE_PRETTY, NULL);
  yyjson_doc_free(doc);
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
