#ifndef __CRBT_PARTITION_H__
#define __CRBT_PARTITION_H__

#ifndef __CRBT_RULE_H__
#include "rule.h"
#endif

#ifndef __TANAKALAB_C1P_H__
#include "c1p.h"
#endif

#ifndef __TANAKALAB_CIRC1P_H__
#include "circ1p.h"
#endif

/************************* list of Rule List **************************/
struct LIST_MRULELIST_CELL {
  list_mrule* key;
  struct LIST_MRULELIST_CELL* prev;
  struct LIST_MRULELIST_CELL* next;
};
typedef struct LIST_MRULELIST_CELL list_mrulelist_cell;

struct LIST_MRULELIST {
  list_mrulelist_cell* head;
  list_mrulelist_cell* last;
  unsigned size;
};
typedef struct LIST_MRULELIST list_mrulelist;

void list_mrulelist_clear(list_mrulelist* RR) {
  if (NULL == RR) { return ; }
  list_mrulelist_cell *p, *q;
  for (p = RR->head; NULL != p; ) {
    q = p;
    p = p->next;
    list_mrule_clear(q->key);
    free(q->key);
    free(q);
  }
}

void list_mrulelist_print(list_mrulelist* RR) {
  list_mrulelist_cell* p = RR->head;
  list_mrule_print(p->key);
  for (p = p->next; NULL != p; p = p->next) {
    list_mrule_print(p->key);
    putchar('\n');
  }
}

void list_mrulelist_print_with_order(list_mrulelist* RR) {
  list_mrulelist_cell* p = RR->head;
  list_mrule_print_with_order(p->key); putchar('\n');
  for (p = p->next; NULL != p; p = p->next) {
    list_mrule_print_with_order(p->key);
    putchar('\n');
  }
}

void list_mrulelist_add_rear_sub(list_mrulelist* RR, list_mrulelist_cell* x) {
  x->prev = RR->last;
  if (NULL != RR->last) { RR->last->next = x; }
  else { RR->head = x; }
  RR->last = x;
  x->next = NULL;
}

void list_mrulelist_add_rear(list_mrulelist* RR, list_mrule* R) {
  ++RR->size;
  list_mrulelist_cell* new = (list_mrulelist_cell*)malloc(sizeof(list_mrulelist_cell));
  new->key = list_mrule_copy(R);
  list_mrulelist_add_rear_sub(RR, new);
}

list_mrulelist* partition_list_mrule(list_mrule* R) {
  if (NULL == R || 0 == R->size) { return NULL; }
  list_mrulelist* RR = (list_mrulelist*)calloc(1, sizeof(list_mrulelist));
  if (R->size < 3) {
    matrix* M = list_mrule_to_matrix(R);
    /* matrix_print(M); */
    R->sigma = decide_c1p_matrix(M);
    /* R->sigma = decide_circ1p_matrix(M); */
    list_mrulelist_add_rear(RR, R);
    matrix_clear(M);
    return RR;
  }

  /* add the rul list containing the first and second rule of L to a list of rulelist LL */
  list_mrule* R_init = (list_mrule*)calloc(1, sizeof(list_mrule));
  list_mrule_cell* p;
  unsigned i;
  for (p = R->head, i = 0; i < 2; p = p->next, ++i)
    list_mrule_add_rear(R_init, p->key);
  /* printf("List Size = %d\n", R_init->size); */
  list_mrulelist_add_rear(RR, R_init);
  
  matrix* M0 = list_mrule_to_matrix(R);
  /* matrix_print(M0); */
  
  /* partition */
  matrix* tmpM = (matrix*)calloc(1, sizeof(matrix));
  tmpM->n = M0->n;
  tmpM->b = (char**)calloc(M0->m, sizeof(char*));
  for ( ; NULL != p; p = p->next) {
    list_mrulelist_cell* q;
    bool flag = false;
    for (q = RR->head; NULL != q; q = q->next) {
      tmpM->m = 0;
      list_mrule_cell* s;
      for (i = 0, s = q->key->head; NULL != s; ++i, s = s->next)
	tmpM->b[i] = M0->b[s->key->num];
      tmpM->b[i] = M0->b[p->key->num];
      tmpM->m = i+1;
      /* matrix_print(tmpM); putchar('\n'); */

      q->key->sigma = decide_c1p_matrix(tmpM);
      if (NULL != q->key->sigma) {
	/* matrix_print_with_order(tmpM, q->key->sigma); */
	/* putchar('\n'); */
	list_mrule_add_rear(q->key, p->key);
	flag = true;
	break;
      }
    }
    if (!flag) { /* for all rule list q, q union p is non-C1P */
      list_mrule* S = mk_new_list_mrule(p->key);
      list_mrulelist_add_rear(RR, S);
    }
  }

  list_mrulelist_cell* q;
  for (q = RR->head; NULL != q; q = q->next)
    if (NULL == q->key->sigma) {
      tmpM->m = 0;
      list_mrule_cell* s;
      for (i = 0, s = q->key->head; NULL != s; ++i, s = s->next)
	tmpM->b[i] = M0->b[s->key->num];
      tmpM->m = i;      
      q->key->sigma = decide_c1p_matrix(tmpM);
    }
  
  free(tmpM->b);
  matrix_clear(M0);
  
  return RR;
}

list_mrulelist* partition_list_mrule_circ1p(list_mrule* R) {
  if (NULL == R || 0 == R->size) { return NULL; }
  list_mrulelist* RR = (list_mrulelist*)calloc(1, sizeof(list_mrulelist));
  if (R->size < 3) {
    matrix* M = list_mrule_to_matrix(R);
    /* matrix_print(M); */
    R->sigma = decide_circ1p_matrix(M);
    list_mrulelist_add_rear(RR, R);
    matrix_clear(M);
    return RR;
  }

  /* add the rul list containing the first and second rule of L to a list of rulelist LL */
  list_mrule* R_init = (list_mrule*)calloc(1, sizeof(list_mrule));
  list_mrule_cell* p;
  unsigned i;
  for (p = R->head, i = 0; i < 2; p = p->next, ++i)
    list_mrule_add_rear(R_init, p->key);
  /* printf("List Size = %d\n", R_init->size); */
  list_mrulelist_add_rear(RR, R_init);
  
  matrix* M0 = list_mrule_to_matrix(R);
  /* matrix_print(M0); */
  
  /* partition */
  matrix* tmpM = (matrix*)calloc(1, sizeof(matrix));
  tmpM->n = M0->n;
  tmpM->b = (char**)calloc(M0->m, sizeof(char*));
  for ( ; NULL != p; p = p->next) {
    list_mrulelist_cell* q;
    bool flag = false;
    for (q = RR->head; NULL != q; q = q->next) {
      tmpM->m = 0;
      list_mrule_cell* s;
      for (i = 0, s = q->key->head; NULL != s; ++i, s = s->next)
	tmpM->b[i] = M0->b[s->key->num];
      tmpM->b[i] = M0->b[p->key->num];
      tmpM->m = i+1;
      /* matrix_print(tmpM); putchar('\n'); */

      q->key->sigma = decide_circ1p_matrix(tmpM);
      if (NULL != q->key->sigma) {
	/* matrix_print_with_order(tmpM, q->key->sigma); */
	/* putchar('\n'); */
	list_mrule_add_rear(q->key, p->key);
	flag = true;
	break;
      }
    }
    if (!flag) { /* for all rule list q, q union p is non-Circ1P */
      list_mrule* S = mk_new_list_mrule(p->key);
      list_mrulelist_add_rear(RR, S);
    }
  }

  list_mrulelist_cell* q;
  for (q = RR->head; NULL != q; q = q->next)
    if (NULL == q->key->sigma) {
      tmpM->m = 0;
      list_mrule_cell* s;
      for (i = 0, s = q->key->head; NULL != s; ++i, s = s->next)
	tmpM->b[i] = M0->b[s->key->num];
      tmpM->m = i;      
      q->key->sigma = decide_circ1p_matrix(tmpM);
    }
  
  free(tmpM->b);
  matrix_clear(M0);
  
  return RR;
}

#endif
