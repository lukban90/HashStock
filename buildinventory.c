#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hashfile.h"
#include "tokenizer.h"


#define MAX_LINE_LENGTH 256
#define DELIM " \t\r\n\a"

int main(int argc, char *argv[]) {

  if(argc != 3) {
    printf("Wrong number of arguments, aborting...\n");
    exit(1);
  }

  FILE *inventory = fopen(argv[1], "r+");
  FILE *hashed = create_hashfile(argv[2]);

  if(inventory == 0) {
    printf("Could not open: %s\n", argv[1]);
    printf("Aborting...\n");
    exit(1);
  }

  char line[MAX_LINE_LENGTH];
  product_ptr p_ptr = (product_ptr)malloc(sizeof(product_t));

  while(fgets(line, MAX_LINE_LENGTH, inventory)) {

    // args = tokenize(line, DELIM);
    // index = make_index(args[0]);
    // printf("%d %d\n", line_num, index);
    // line_num++;

    read_product(line, p_ptr);
    add_item(hashed, *p_ptr);

  }

  display_hashfile(hashed);

  fclose(inventory);
  fclose(hashed);
  free(p_ptr);
  return 0;
}
