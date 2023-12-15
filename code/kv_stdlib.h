/*
  Common library for throwable code that uses the standard library.
 */

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "kv-utils.h"

internal char *
readEntireFile(char *file_name)
{
  FILE *file = fopen(file_name, "r");
  if (file == 0)
  {
    assert(file);
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *result = (char *)malloc(file_size+1);
    fread(result, file_size, 1, file);
    result[file_size] = 0;
    fclose(file);
    return (result);
  }
  else
  {
    fprintf(stderr, "Cannot open file %s! \n", file_name);
    return(0);
  }
}

struct FileList {
  i32    count;
  char **files;
};

internal FileList
findFilesByExtension(char *directory, char *extension)
{
  FileList out = {};

  DIR *dir = opendir(directory);
  if (dir == NULL) {
    perror("Error opening directory");
    exit(EXIT_FAILURE);
  }
  defer(closedir(dir));

  while (dirent *entry = readdir(dir)) {
    if (entry->d_type == DT_REG) {
      char *dot = strrchr(entry->d_name, '.');
      if (dot && (strcmp(dot + 1, extension) == 0)) {
        char *filename = (char *)malloc(entry->d_namlen);
        out.files[out.count++] = filename;
      }
    }
  }

  return out;
}
