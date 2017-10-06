#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "product.h"

#define BUFLEN 1024
#define MONTHS 12
#define HASH_SIZE 13

// creates and initializes a new hashfile for product_t data
// opens for read/write
FILE* create_hashfile(char *name) {

  FILE *fp = fopen(name, "w+");

  if(fp == NULL) {
    printf("ERROR: opening file\n");
    printf("Aborting...\n");
    exit(1);
  }

  product_ptr dummy = (product_ptr)malloc(sizeof(product_t));

  char *d_code = "WL000000";
  char *d_desc = "PRODUCT_DUMMY";
  strcpy(dummy->code, d_code);
  strcpy(dummy->desc, d_desc);

  fseek(fp, 0, SEEK_SET);
  for(int i = 0; i < HASH_SIZE; i++) {
    fwrite(dummy, 1, sizeof(product_t), fp);
  }
  free(dummy);
  return fp;

}

// opens an existing hashfile of product_t data for read/write
FILE* open_hashfile(char *name) {

  FILE *fp = fopen(name, "rb+");

  if(fp == NULL) {
    printf("ERROR: opening file\n");
    printf("Aborting...\n");
    exit(1);
  }
  return fp;
}

// generates index from WLxxxxxx
int make_index(char *arg) {

  int code;
  if(sscanf(arg, "WL%d\n", &code) == 1)
    return code % HASH_SIZE;

  return 0;
}

// checks if added item is a duplicate
int isDuplicate(char *str1, char *str2) {

  if(strcmp(str1, str2) == 0) {
    // printf("Error: Duplicate Product\n");
    // printf("Code %s was not added\n", str2);
    return 1;
  }
  return 0;
}
/*
* function chaining
* params: FILE *fp, product_ptr col_prod, product_ptr add_prod
* attempts to chain the added product, if not a duplicate
*/
int chaining(FILE *fp, product_ptr col_prod, product_ptr add_prod) {

  long location;
  long offset;

  /*move down the chain till eol*/
  location = ftell(fp);
  while (col_prod->next != 0) {
    if(isDuplicate(col_prod->code, add_prod->code)) {
      printf("DUPLICATE CODE: %s. NOT ADDED TO INVENTORY\n", col_prod->code);
      return 0;
    }

    fseek(fp, col_prod->next, SEEK_SET);
    location = ftell(fp);
    fread(col_prod, sizeof(product_t), 1, fp);
  }

    /*seek to eof, and obtain the offset*/
    fseek(fp, 0, SEEK_END);
    offset = ftell(fp);

    /*seek to the location, read the collided product
    * change collided's next, seek back to location,
    * finally write the collided product */
    fseek(fp, location, SEEK_SET);
    fread(col_prod, sizeof(product_t), 1, fp);
    col_prod->next = offset;
    fseek(fp, location, SEEK_SET);
    fwrite(col_prod, sizeof(product_t), 1, fp);

    /*seek to the eof, then write the added product*/
    fseek(fp, 0, SEEK_END);
    fwrite(add_prod, sizeof(product_t), 1, fp);
    return 1;
  }

// add a new product_t data item to the file
// return 0 on error, 1 if OK
int add_item(FILE *fp, product_t product) {

  product_ptr in_prod = (product_ptr)malloc(sizeof(product_t));
  product_ptr add_p = (product_ptr)malloc(sizeof(product_t));
  add_p = &product;


  /*generate the index*/
  int index = make_index(product.code);

  fseek(fp, sizeof(product_t)*index, SEEK_SET);
  fread(in_prod, sizeof(product_t), 1, fp);
  fseek(fp, -sizeof(product_t), SEEK_CUR);

  /*check if product is dummy*/
  if(strcmp(in_prod->code, "WL000000") == 0) {
    fwrite(&product, sizeof(product), 1, fp);
    return 1;
  }
  /*try to chain the product*/
  else {

    //if(isDuplicate(in_prod->code, add_p->code)) return 0;
    if(isDuplicate(in_prod->code, add_p->code)) {
      printf("DUPLICATE CODE: %s. NOT ADDED TO INVENTORY\n", in_prod->code);
      return 0;
    }
    if(chaining(fp, in_prod, add_p)) return 1;
  }
  return 0;
}

// display the entire contents of the file in text
// one line per product_t data item
void display_hashfile(FILE *fp) {
  int line_num = 0;
  fseek(fp, 0, SEEK_SET);
  if (fp == NULL) {
    perror("");
  }
  else {
    product_ptr p_ptr = (product_ptr)malloc(sizeof(product_t));
    while (fread(p_ptr, sizeof(product_t), 1, fp) == 1) {
      printf("%d", line_num);
      write_product(stdout, " ", *p_ptr, "\n");
      line_num++;
    }
    free(p_ptr);
  }
}


long find_item(FILE *fp, char *key) {

  if(fp == NULL) {
    printf("ERROR: opening file\n");
    printf("Aborting...\n");
    exit(1);
  }

  product_ptr read_prod = (product_ptr)malloc(sizeof(product_t));

  long location = 0;
  int index = make_index(key);

  fseek(fp, sizeof(product_t)*index, SEEK_SET);
  fread(read_prod, sizeof(product_t), 1, fp);

  /*check if item is in the beginning of the chain*/
  if(isDuplicate(read_prod->code, key)) {
    fseek(fp, sizeof(product_t)*index, SEEK_SET);
    location = ftell(fp);
    free(read_prod);
    return location;

  } else {
    /*locate item down the chain*/
    fseek(fp, sizeof(product_t)*index, SEEK_SET);
    location = ftell(fp);
    while(read_prod->next != 0 && !isDuplicate(read_prod->code, key)) {
    //while(!isDuplicate(read_prod->code, key))
    //while(read_prod->next != 0)
      fseek(fp, read_prod->next, SEEK_SET);
      location = ftell(fp);
      fread(read_prod, sizeof(product_t), 1, fp);

      if(isDuplicate(read_prod->code, key))
        {free(read_prod); return location;}
    }
  }

  printf("ITEM NOT FOUND: %s\n", key);
  return 0;
}

int delete_item(FILE *fp, char *key) {

  if(fp == NULL) {
    printf("ERROR: opening file\n");
    printf("Aborting...\n");
    exit(1);
  }

  long loc_dprod = 0;
  long offset = 0;

  char *d_desc = "PRODUCT_DELETE";
  product_ptr read_prod = (product_ptr)malloc(sizeof(product_t));
  product_ptr d_prod = (product_ptr)malloc(sizeof(product_t));

  if((loc_dprod = find_item(fp, key))) {

    /*seek to soon-to-be deleted item*/
    fseek(fp, loc_dprod, SEEK_SET);
    fread(read_prod, sizeof(product_t), 1, fp);

    /*save the offset of soon-to-be deleted item, and put that for d_prod*/
    offset = read_prod->next;
    d_prod->next = offset;
    /*put PRODUCT_DELETE for desc*/
    strcpy(d_prod->desc, d_desc);

    /*seek back to location, overwrite the product*/
    fseek(fp, loc_dprod, SEEK_SET);
    fwrite(d_prod, sizeof(product_t), 1, fp);
    return 1;
  }
  free(read_prod);
  free(d_prod);
  return 0;
}


// update the sales figures for the product_t data item matching the key
// return 1 on success, 0 on failure
int update_sales(FILE *fp, char *key, int quantity) {
  //printf("Updating Sales: %s with quantity %d\n", key, quantity);

  product_ptr temp_prod = (product_ptr)malloc(sizeof(product_t));
  long location;
  if((location = find_item(fp, key))) {
    fseek(fp, location, SEEK_SET);
    fread(temp_prod, sizeof(product_t), 1, fp);

    /*if flag is not turned on,
    * shift all elements to the right
    * add the quantity for the new month
    * decrease the stock*/
    temp_prod->stock -= quantity;
    if(temp_prod->sales_updated == 0) {
      for(int i=MONTHS-1; i>0; i--)
        temp_prod->sales[i] = temp_prod->sales[i-1];
      temp_prod->sales[0] = quantity;
      temp_prod->sales_updated = 1;
    /*flag is turned on, don't shift,
    * just increase the quantity for the new month*/
    } else if(temp_prod->sales_updated == 1){
      temp_prod->sales[0] += quantity;
    }
    fseek(fp, location, SEEK_SET);
    fwrite(temp_prod, sizeof(product_t), 1, fp);
  }

  free(temp_prod);
  return 0;
}

// update the stock for the product_t data item matching the key
// return 1 on success, 0 on failure
int update_stock(FILE *fp, char *key, int quantity) {
  //printf("Updating Stock: %s with quantity %d\n", key, quantity);

  /*locate the item and update the stock*/
  product_ptr temp_prod = (product_ptr)malloc(sizeof(product_t));
  long location;
  if((location = find_item(fp, key))) {
    fseek(fp, location, SEEK_SET);
    fread(temp_prod, sizeof(product_t), 1, fp);
    temp_prod->stock += quantity;
    fseek(fp, location, SEEK_SET);
    fwrite(temp_prod, sizeof(product_t), 1, fp);
  }
  return 0;
}

// update the price for the product_t data item matching the key
// return 1 on success, 0 on failure
int update_price(FILE *fp, char *key, price_t newprice) {
  // printf("Updating Price: Code %s with %d.%d\n",
  // key, newprice.dollars, newprice.cents);

  product_ptr temp_prod = (product_ptr)malloc(sizeof(product_t));
  long location;
  if((location = find_item(fp, key))) {
    fseek(fp, location, SEEK_SET);
    fread(temp_prod, sizeof(product_t), 1, fp);
    temp_prod->price = newprice;
    fseek(fp, location, SEEK_SET);
    fwrite(temp_prod, sizeof(product_t), 1, fp);
  }

  free(temp_prod);
  return 0;
}

// check the sales for each product in the file
// if sales not updated, assume 0 sales for the month
void check_sales(FILE *fp) {

  fseek(fp, 0, SEEK_SET);
  product_ptr p_ptr = (product_ptr)malloc(sizeof(product_t));

  while(fread(p_ptr, sizeof(product_t), 1, fp) == 1) {

    fseek(fp, -sizeof(product_t), SEEK_CUR);
    if(p_ptr->sales_updated == 0) {
      for(int i=MONTHS-1; i>0; i--)
        p_ptr->sales[i] = p_ptr->sales[i-1];
      p_ptr->sales[0] = 0;
      fwrite(p_ptr, sizeof(product_t), 1, fp);
    } else {
      fseek(fp, sizeof(product_t), SEEK_CUR);
    }
  }
  free(p_ptr);
}
