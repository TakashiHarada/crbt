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

void mrule_print(mrule* r) {
  printf("%d %s\n", r->num, r->cond);
}

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
  p = R->last;
  // return R->size + 1;
  return R->last->key->num + 1;
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
  if (NULL == R->sigma) {
    R2->sigma = NULL;
  } else {
    unsigned const l = strlen(R->head->key->cond);
    R2->sigma = (unsigned*)calloc(l, sizeof(unsigned));
    for (unsigned i = 0; i < l; ++i) {
      R2->sigma[i] = R->sigma[i];
    }
  }
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

void set_protocol(mrule* r, int p1, int p2) {
  /* printf("%d/%d\n", p1, p2); */
  int LEN = 104;
  for (int i = 0; i < 8; ++i) {
    /* printf("%d", p1 & 1); */
    r->cond[LEN-i-1] = '0' + (p1 & 1);
    p1 = p1 >> 1;
  }
  /* putchar('\n'); */
  for (int i = 0; i < 8; ++i) {
    /* printf("%d", p2 & 1); */
    if ((p2 & 1) == 0) {
      r->cond[LEN-i-1] = '*';
    }
    p2 = p2 >> 1;
  }
  /* putchar('\n'); */
}

void convert(int l, int r, int a, int b, list_pair_unsigned* L) {
  if (l == a && r == b) {
    list_pair_unsigned_insert(L, a, b);
  } else {
    int m = (a+b-1)/2;
    int n = (a+b+1)/2;
    if (r <= m) {
      convert(l, r, a, m, L);
    } else if (l <= m && n <= r) {
      convert(l, m, a, m, L);
      convert(n, r, n, b, L);
    } else {
      convert(l, r, n, b, L);
    }
  }
}

list_string* prepare_port_string(int low, int high) {
  list_string* S = (list_string*)calloc(1, sizeof(list_string));
  list_pair_unsigned* L = (list_pair_unsigned*)calloc(1, sizeof(list_pair_unsigned));
  convert(low, high, 0, (1 << 16)-1, L);
  /* list_pair_unsigned_print(L); */
  /* putchar('\n'); */
  int LEN = 16;
  char buf[LEN+1];
  buf[LEN] = '\0';
  for (list_pair_unsigned_cell* p = L->head; p != NULL; p = p->next) {
    int num = p->pu.first;
    for (int i = 0; i < LEN; ++i) {
      buf[LEN-i-1] = '0' + (num & 1);
      num = (num >> 1);
    }
    int diff = p->pu.second - p->pu.first + 1;
    int mask = 0;
    while (diff > 1) {
      mask += 1;
      diff = (diff >> 1);
    }
    for (int i = 0; i < mask; ++i) {
      buf[LEN-i-1] = '*';
    }
    list_string_insert(S, buf);
    /* printf("(%u, %u): %s\n", p->pu.first, p->pu.second, buf); */
  }
  
  list_pair_unsigned_clear(L); L = NULL;
  return S;
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
    int s1, s2, s3, s4, sp, d1, d2, d3, d4, dp, spl, spr, dpl, dpr, p1, p2, flag, flag_mask;
    sscanf(buf, "@%d.%d.%d.%d/%d\t%d.%d.%d.%d/%d\t%d : %d\t%d : %d\t%x/%x\t%x/%x",
	   &s1, &s2, &s3, &s4, &sp, &d1, &d2, &d3, &d4, &dp, &spl, &spr, &dpl, &dpr, &p1, &p2, &flag, &flag_mask);

    /* printf("%s", buf); */
    
    mrule* r = (mrule*)calloc(1, sizeof(mrule));
    r->num = i;
    r->cond = (char*)malloc((w+1)*sizeof(char));
    r->cond[w] = '\0';
	
    set_address(r, s1, s2, s3, s4, sp, 0);
    set_address(r, d1, d2, d3, d4, dp, 1);
    set_protocol(r, p1, p2);

    /* for (int i = 64; i < 96; ++i) r->cond[i] = '?'; */
    /* printf("%s\n", r->cond); */
    
    list_string* PS = prepare_port_string(spl, spr);
    list_string* PD = prepare_port_string(dpl, dpr);

    for (list_string_cell* ps = PS->head; ps != NULL; ps = ps->next) {
      for (list_string_cell* pd = PD->head; pd != NULL; pd = pd->next) {
	for (int j = 0; j < 16; ++j) {
	  r->cond[64+j] = ps->key[j];
	}
	for (int j = 0; j < 16; ++j) {
	  r->cond[80+j] = pd->key[j];
	}
	/* printf("%s\n", r->cond); */
	list_mrule_add_rear(R, r);
      }
    }
    
    list_string_clear(PD);
    list_string_clear(PS);
  }

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

  /* printf("%ld\n", strlen(R->head->key->cond)); */
  /* for (unsigned i = 0; i < strlen(R->head->key->cond); ++i) { */
  /*   printf("%d ", R->sigma[i]); */
  /* } */
  /* putchar('\n'); */
  
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
