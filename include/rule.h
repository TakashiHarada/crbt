/* rule.h */
#ifndef __CRBT_RULE_H__
#define __CRBT_RULE_H__

#ifndef __TANAKALAB_BOOL_H__
#include "bool.h"
#endif

#ifndef __TANAKALAB_LIST_H__
#include "list.h"
#endif

#ifndef __TANAKALAB_PAIR_H__
#include "pair.h"
#endif

#ifndef __CRBT_HEADER_H__
#include "header.h"
#endif

#ifndef __TANAKALAB_C1P_H__
#include "c1p.h"
#endif


/* Arbitrary Mask Rule */
struct MRULE {
  unsigned num;
  char* cond;
};
typedef struct MRULE mrule;

mrule* mrule_copy(mrule* r) {
  mrule* copy = (mrule*)calloc(1, sizeof(mrule));
  copy->num = r->num;
  copy->cond = (char*)calloc(strlen(r->cond)+1, sizeof(char));
  strcpy(copy->cond, r->cond);
  copy->cond[strlen(r->cond)] = '\0';
  return copy;
}

bool does_match_header(mrule* r, header h) {
  /* printf("%lu, r = %s\n", strlen(r->cond), r->cond); */
  /* printf("%lu, h = %s\n", strlen(h.string), h.string); */
  const unsigned l = strlen(h.string);
  unsigned i;
  for (i = 0; i < l; ++i)
    if (r->cond[i] != '*' && r->cond[i] != h.string[i])
      return false;
  return true;
}

struct LIST_MRULE_CELL {
  mrule* key;
  struct LIST_MRULE_CELL* prev;
  struct LIST_MRULE_CELL* next;
};
typedef struct LIST_MRULE_CELL list_mrule_cell;

struct LIST_MRULE {
  unsigned* sigma; /* order of column */
  list_mrule_cell* head;
  list_mrule_cell* last;
  unsigned size;
};
typedef struct LIST_MRULE list_mrule;

matrix* list_mrule_to_matrix(list_mrule* R) {
  if (NULL == R) { return NULL; }
  matrix* M = (matrix*)calloc(1,sizeof(matrix));
  M->m = R->size;
  M->n = strlen(R->head->key->cond);
  /* printf("M->m = %d, M->n = %d\n", M->m, M->n); */
  M->b = (char**)calloc(M->m, sizeof(char*));
  unsigned i;
  for (i = 0; i < M->m; ++i)
    M->b[i] = (char*)calloc(M->n+1, sizeof(char));
    
  list_mrule_cell* p;
  for (i = 0, p = R->head; NULL != p; ++i, p = p->next) {
    unsigned j;
    for (j = 0; j < M->n; ++j)
      if ('*' == p->key->cond[j]) M->b[i][j] = '0';
      else M->b[i][j] = '1';
    M->b[i][j] = '\0';
  }
  
  return M;
}

unsigned linear_search(list_mrule* R, header h) {
  list_mrule_cell* p;
  for (p = R->head; NULL != p; p = p->next)
    if (does_match_header(p->key, h))
      return p->key->num;
  return R->size + 1;
}

void do_linear_search(list_mrule* R, header* H) {
  unsigned i;
  printf("==================== Linear Search ====================\n");
  /* for (i = 0; i < num_of_headers; ++i) */
  /*   printf("h[%d] = %s ---> %d\n", H[i].num, H[i].string, linear_search(R, H[i])); */
  for (i = 0; i < num_of_headers; ++i)
    linear_search(R, H[i]);
}

unsigned* do_linear_search_with_results(list_mrule* R, header* H) {
  unsigned* results = (unsigned*)calloc(num_of_headers, sizeof(unsigned));
  unsigned i;
  printf("==================== Linear Search ====================\n");
  for (i = 0; i < num_of_headers; ++i)
    results[i] = linear_search(R, H[i]);
  return results;
}

void list_mrule_add_rear_sub(list_mrule* R, list_mrule_cell* x) {
  x->prev = R->last;
  /* L is empty */
  if (NULL != R->last) { R->last->next = x; }
  else { R->head = x; }
  R->last = x;
  x->next = NULL;
}

void list_mrule_add_rear(list_mrule* R, mrule* r) {
  list_mrule_cell* new = (list_mrule_cell*)malloc(sizeof(list_mrule_cell));
  R->size = R->size + 1;
  new->key = mrule_copy(r);
  list_mrule_add_rear_sub(R, new);
}

list_mrule* mk_new_list_mrule(mrule* r) {
  list_mrule* R = (list_mrule*)calloc(1, sizeof(list_mrule));
  mrule* copy = mrule_copy(r);
  list_mrule_add_rear(R, copy);
  R->sigma = NULL;
  return R;
}

list_mrule* list_mrule_copy(list_mrule* R) {
  list_mrule* R2 = (list_mrule*)calloc(1, sizeof(list_mrule));
  list_mrule_cell* p;
  for (p = R->head; NULL != p; p = p->next) { list_mrule_add_rear(R2, p->key); }
  R2->size = R->size;
  return R2;
}

void set_address(mrule* r, int x1, int x2, int x3, int x4, int xp, int flag) {
  for (int i = 0; i < 8; ++i) {
    r->cond[8 + 32*flag - i - 1] = '0' + (x1 & 1);
    x1 = x1 >> 1;
  }
  for (int i = 0; i < 8; ++i) {
    r->cond[16 + 32*flag - i - 1] = '0' + (x2 & 1);
    x2 = x2 >> 1;
  }
  for (int i = 0; i < 8; ++i) {
    r->cond[24 + 32*flag - i - 1] = '0' + (x3 & 1);
    x3 = x3 >> 1;
  }
  for (int i = 0; i < 8; ++i) {
    r->cond[32 + 32*flag - i - 1] = '0' + (x4 & 1);
    x4 = x4 >> 1;
  }
  for (int i = 32*flag+xp; i < 32*flag+32; ++i) {
    r->cond[i] = '*';
  }
}

list_mrule* read_classbench_rule_list(char* rule_file_name) {
  FILE* fp;
  if (NULL == (fp = fopen(rule_file_name, "r"))) {
    fprintf(stderr, "ERROR: Can't open the rule file.\n");
    exit(1);
  }

  list_mrule* R = (list_mrule*)calloc(1, sizeof(list_mrule));
  int w = 104;
  int LENGTH = 255;
  char buf[LENGTH];

  for (unsigned i = 1; NULL != fgets(buf, LENGTH, fp); ++i) {
    int s1, s2, s3, s4, sp, d1, d2, d3, d4, dp, spl, spr, dpl, dpr, p1, p2;
    sscanf(buf, "@%d.%d.%d.%d/%d\t%d.%d.%d.%d/%d\t%d : %d\t%d : %d\t%d/%d",
	   &s1, &s2, &s3, &s4, &sp, &d1, &d2, &d3, &d4, &dp, &spl, &spr, &dpl, &dpr, &p1, &p2);

    mrule* r = (mrule*)calloc(1, sizeof(mrule));
    r->num = i;
    r->cond = (char*)malloc((w+1)*sizeof(char));
    set_address(r, s1, s2, s3, s4, sp, 0);
    set_address(r, d1, d2, d3, d4, dp, 1);
    
    r->cond[w] = '\0';
    list_mrule_add_rear(R, r);
    printf("%s", buf);
    printf("%s\n", r->cond);
  }


  /* unsigned i; */
  /* for (i = 1; -1 != (read = getline(&line, &len, fp)); ++i) { */
  /*   r = (mrule*)calloc(1, sizeof(mrule)); */
  /*   r->num = i; */
  /*   r->cond = (char*)malloc(w*sizeof(char)); */
  /*   strcpy(r->cond, line); */
  /*   r->cond[w-1] = '\0'; */
  /*   list_mrule_add_rear(R, r); */
  /* } */

  fclose(fp);
  
  return R;
}

list_mrule* read_mrule_list(char* rule_file_name) {
  FILE* fp;
  if (NULL == (fp = fopen(rule_file_name, "r"))) {
    fprintf(stderr, "ERROR: Can't open the rule file.\n");
    exit(1);
  }

  list_mrule* R = (list_mrule*)calloc(1, sizeof(list_mrule));

  char* line = NULL;
  size_t len = 0;
  ssize_t read;
  if (-1 == (read = getline(&line, &len, fp))) {
    fprintf(stderr, "ERROR: Can't read the rule list file.\n");
    exit(1);
  }

  unsigned w = strlen(line);
  mrule* r = (mrule*)calloc(1, sizeof(mrule));
  r->num = 0;
  r->cond = (char*)malloc(w*sizeof(char));
  strcpy(r->cond, line);
  r->cond[w-1] = '\0';
  list_mrule_add_rear(R, r);

  unsigned i;
  for (i = 1; -1 != (read = getline(&line, &len, fp)); ++i) {
    r = (mrule*)calloc(1, sizeof(mrule));
    r->num = i;
    r->cond = (char*)malloc(w*sizeof(char));
    strcpy(r->cond, line);
    r->cond[w-1] = '\0';
    list_mrule_add_rear(R, r);
  }
  
  return R;
}

void list_mrule_print(list_mrule* R) {
  list_mrule_cell* p;
  const unsigned d = (unsigned)floor(log10(R->size)) + 1;
  for (p = R->head; NULL != p; p = p->next)
    printf("r[%*d] = %s\n", d, p->key->num, p->key->cond);
}

void list_mrule_print_with_order(list_mrule* R) {
  if (NULL == R->sigma) { list_mrule_print(R); return ; }
  list_mrule_cell* p;
  const unsigned d = (unsigned)floor(log10(R->size)) + 1;
  for (p = R->head; NULL != p; p = p->next) {
    printf("r[%*d] = ", d, p->key->num);
    unsigned j;
    for (j = 0; j < strlen(p->key->cond); ++j)
      putchar(p->key->cond[R->sigma[j]]);
    putchar('\n');
  }
}

void list_mrule_clear(list_mrule* R) {
  if (NULL == R) { return; }
  list_mrule_cell *p, *q;
  for (p = R->head; NULL != p; ) {
    q = p;
    p = p->next;
    free(q->key->cond);
    free(q->key);
    free(q);
  }
  if (NULL != R->sigma) { free(R->sigma); R->sigma = NULL; }
  R->size = 0;
}

#endif
