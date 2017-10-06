#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
* function tokenize(char *line)
* tokenizes the string with delimeter
*/
char **tokenize(char *line, char *delimiters) {

  char *line_cpy = (char *)malloc(strlen(line) * sizeof(char));
  strncpy(line_cpy, line, strlen(line));
  char *token = strtok(line_cpy, delimiters);

  int n = 0; //number of strings
  while(token != NULL) {
    token = strtok(NULL, delimiters);
    n++;
  }
  free(line_cpy);

  char **p = (char **)malloc((n+1) * sizeof(char*)+1);
  token = strtok(line, delimiters);

  int ptr = 0;
  while(token != NULL) {
    p[ptr] = (char *)malloc(strlen(token)*sizeof(char)+1);
    p[ptr] = token;
    if(token[strlen(token)-1] == '\n') p[ptr][strlen(token)-1] = '\0';
    token = strtok(NULL, delimiters);
    ptr++;
  }
  p[ptr] = '\0'; //mark the end of the array
  return p;
}

int token_count(char **tokens) {

  int ptr = 0;
  while(tokens[ptr] != NULL) {
    ptr++;
  }
  return ptr;
}
