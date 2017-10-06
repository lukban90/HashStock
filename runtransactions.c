#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hashfile.h"
#include "tokenizer.h"

#define MAX_LINE_LENGTH 256
#define DELIM " \t\r\n\a"

void execute_transaction(FILE *hashfile, char *line) {

  char *line_cpy = (char*)malloc(strlen(line)*sizeof(char)+1);
  strcpy(line_cpy, line);

  char **args = tokenize(line, DELIM);
  /*args[0] = type
  * args[1] = code
  * args[2] = quantity (SALE, DELIVERY) or = desc (NEWPRODUCT)
  * args[3] = price (NEWPRODUCT)
  * args[4] = category (NEWPRODUCT)*/

  if(strcasecmp("SALE", args[0]) == 0) {

    if(find_item(hashfile, args[1]))
      update_sales(hashfile, args[1], atoi(args[2]));

  } else if(strcasecmp("DELIVERY", args[0]) == 0) {

      if(find_item(hashfile, args[1]))
        update_stock(hashfile, args[1], atoi(args[2]));

  } else if(strcasecmp("NEWPRODUCT", args[0]) == 0) {

      product_ptr new_prod = (product_ptr)malloc(sizeof(product_t));
      int size = strlen(line_cpy) - (strlen("NEWPRODUCT")+1);
      line_cpy += (strlen("NEWPRODUCT")+1);

      char *prod_info = (char*)malloc(strlen(line_cpy) * sizeof(char));
      strncpy(prod_info, line_cpy, size);

      if(read_new_product(prod_info, new_prod))
        add_item(hashfile, *new_prod);
      free(new_prod);

  } else if(strcasecmp("DELETE", args[0]) == 0) {

      if(!delete_item(hashfile, args[1]))
        printf("CANNOT DELETE %s. NOT IN INVENTORY\n", args[1]);

  } else if(strcasecmp("PRICE", args[0]) == 0) {

      if(find_item(hashfile, args[1])) {
        int dollars, cents;
        sscanf(args[2], "%d.%d\n", &dollars, &cents);
        price_t newprice = new_price(dollars, cents, 0); //why a neg value?
        update_price(hashfile, args[1], newprice);
      }
  }
  free(args);
}

int main(int argc, char *argv[]) {

  if(argc != 3) {
    printf("Wrong number of arguments, aborting...\n");
    exit(1);
  }

  FILE *transactions = fopen(argv[1], "r");
  FILE *hashed = open_hashfile(argv[2]);

  if(transactions == 0 || hashed == 0) {
    printf("Could not open: %s\n", argv[1]);
    printf("Aborting...\n");
    exit(1);
  }

  char line[MAX_LINE_LENGTH] = {0};
  while(fgets(line, MAX_LINE_LENGTH, transactions)) {
    execute_transaction(hashed, line);
  }

  check_sales(hashed);
  display_hashfile(hashed);
  return 0;
}
