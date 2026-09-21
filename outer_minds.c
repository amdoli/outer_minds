#include "include/yyjson.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_SIZE 128

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
  // pos [-1 , 1]
  double x_pos;
  double y_pos;
} node;

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

int write_to_json(const node n, const char *filename) {

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

  node n1;

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
