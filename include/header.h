/* header.h */

#ifndef __CRBT_HEADER_H__
#define __CRBT_HEADER_H__

#include <string.h>

#ifndef __TANAKALAB_BOOL_H__
#include "bool.h"
#endif

#define NUMFIELDS 5
#define HEADER_LENGTH 104

struct HEADER {
  unsigned num;
  char* string;
};
typedef struct HEADER header;

unsigned num_of_headers;

void free_header_list(header* H) {
  unsigned i;
  for (i = 0; i < num_of_headers; ++i) { free(H[i].string); }
  free(H);
}

void setstring(header* h, int counter, long num) {
  if (counter == 1 || counter == 2) {
    // Addresses
    for (unsigned i = 0; i < 32; ++i) {
      h->string[32*counter-i-1] = '0' + (num & 1);
      num = num >> 1;
    }
  } else if (counter == 3 || counter == 4) {
    // Ports
    for (unsigned i = 0; i < 16; ++i) {
      h->string[80 + 16*(counter-3)-i-1] = '0' + (num & 1);
      num = num >> 1;
    }
  } else if (counter == 5) {
    // Protocol
    for (unsigned i = 0; i < 8; ++i) {
      h->string[104-i-1] = '0' + (num & 1);
      num = num >> 1;
    }
  }
}

header* read_classbench_header_list(char* header_file_name) {
  FILE* fp;
  if (NULL == (fp = fopen(header_file_name, "r"))) {
    fprintf(stderr, "ERROR: Can't open the header list file.\n");
    exit(1);
  }

  int LEN = 255;
  char buf[LEN];

  while (NULL != fgets(buf, LEN, fp)) ++num_of_headers;

  header* H = (header*)calloc(num_of_headers, sizeof(header));

  /* reader headers */
  rewind(fp);
  for (unsigned i = 0; NULL != fgets(buf, LEN, fp); ++i) {
    H[i].num = i;
    H[i].string = (char*)malloc((HEADER_LENGTH+1)*sizeof(char));
    char s[LEN];
    int counter = 0;
    for (unsigned j = 0; buf[j] != '\n'; ++j) {
      unsigned k;
      /* 7th field implies the Filter Number that is not used to this program */
      for (k = 0; buf[j] != '\t' && buf[j] != '\n'; ++j, ++k) {
	s[k] = buf[j];
      }
      s[k] = '\0';
      long num = atol(s);
      ++counter;
      setstring(&(H[i]), counter, num);
      
      if (counter == NUMFIELDS) break;
    }
    /* strcpy(H[i].string, line); */
    H[i].string[HEADER_LENGTH] = '\0';
    /* printf("%s\n", H[i].string); */
  }

  /* fclose(fp); */
  return H;
}

header* read_header_list(char* header_file_name) {
  FILE* fp;
  if (NULL == (fp = fopen(header_file_name, "r"))) {
    fprintf(stderr, "ERROR: Can't open the header list file.\n");
    exit(1);
  }

  char* line = NULL;
  size_t len = 0;

  int w = getline(&line, &len, fp);
  unsigned l;
  if (-1 == w) {
    fprintf(stderr, "ERROR: Can't read the header list file.\n");
    exit(1);
  }

  /* set the length of the header */
  l = strlen(line);
  
  /* count the number of headers */
  num_of_headers = 1; /* already read the first header */
  while (-1 != getline(&line, &len, fp)) { ++num_of_headers; }

  header* H = (header*)calloc(num_of_headers, sizeof(header));

  /* reader headers */
  rewind(fp);
  unsigned i;
  for (i = 0; -1 != getline(&line, &len, fp); ++i) {
    /* printf("l = %d\n", l); */
    H[i].num = i;
    H[i].string = (char*)malloc(l*sizeof(char));
    strcpy(H[i].string, line);
    H[i].string[l-1] = '\0';
  }
  
  return H;
}

void print_header_list(header* H) {
  unsigned i, d = (unsigned)floor(log10(num_of_headers)) + 1;
  for (i = 0; i < num_of_headers; ++i)
    printf("h[%*d] = %s\n", d, H[i].num, H[i].string);
}

#endif
