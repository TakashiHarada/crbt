/* c1p.h */

#ifndef __TANAKALAB_CIRC1P_H__
#define __TANAKALAB_CIRC1P_H__

#ifndef __TANAKALAB_C1P_H__
#include "c1p.h"
#endif

#include <math.h>

/* each row r_i is a set of the column number j, s.t. M_ij = 0 */
vset_cell_list** make_row_set_with0(matrix* M, vset* E) {
  vset_cell_list** r = (vset_cell_list**)calloc(M->m, sizeof(vset_cell_list*));
  unsigned i, j;
  vset_cell** ptr = (vset_cell**)calloc(M->n, sizeof(vset_cell*));
  vset_cell* p;
  for (j = 0, p = E->head; NULL != p; ++j, p = p->next)
    ptr[j] = p;
  for (i = 0; i < M->m; ++i) {
    r[i] = (vset_cell_list*)calloc(1, sizeof(vset_cell_list));
    for (j = 0; j < M->n; ++j)
      if ('0' == M->b[i][j])
	vset_cell_list_add_rear(r[i], ptr[j]);
  }
  free(ptr);
  return r;
}

unsigned* decide_c0p_matrix(matrix* M) {
  unsigned* order = (unsigned*)calloc(M->n, sizeof(unsigned));

  vset* E = init_vertex_set(M);
  /* vset_print(E); putchar('\n'); */
  
  vset_cell_list** r = make_row_set_with0(M,E);
  /* row_set_print(r, M->m); */

  graph* G = make_overlap_graph(M, r);
  decomposing_to_connected_components(G);
  /* graph_print(G); */

  unsigned k;
  unsigned num_c = G->num_of_components;
  partition** P = (partition**)calloc(num_c, sizeof(partition*));

  /* printf("#component = %d\n", num_c); */
  for (k = 0; k < num_c; ++k) {
    if (1 < G->com[k]->size) {
      list_unsigned* L = spanning_tree(G, k);
      /* printf("ST[%d] : ", k); list_unsigned_print(L); printf("\n\n"); */

      list_unsigned_cell* p = L->head;
      P[k] = set_P1(r, E, p->key);
      /* partition_print(P[k]); */
      MAX_CLS_NUM = 1;
      /* printf("\n"); partition_print(P[k]); printf("\n"); */
      for (p = p->next; NULL != p; p = p->next) {
      	if (1 < r[p->key]->size) {
      	  lr lr = refine(P[k], E, r[p->key]);
	  /* if (NULL != lr.l && NULL != lr.r) { printf("l = %d, r = %d\n", lr.l->cls_num, lr.r->cls_num); } */
      	  /* if (!is_consecutive(lr, E)) { */
	  if (!is_consecutive(lr)) {
	    /* printf("Input Matrix has non-C1P\n"); */
	    graph_clear(G); row_set_clear(r, M->m); return NULL;
	  }
      	}
	/* printf("\n"); partition_print(P[k]); printf("\n"); */
	/* printf("P->head->head = %d, P->last->last = %d\n", P[k]->head->head->key, P[k]->last->last->key); */
	/* printf("P->head = P[%d], P->last = P[%d]\n", P[k]->head->cls_num, P[k]->last->cls_num); */
	/* vset_print(E); putchar('\n'); */
	reset_do_intersect_P(P[k]);
	reset_do_intersect_E(r[p->key]);
	reset_class_counter(P[k]);
      }
      list_unsigned_clear(L); L = NULL;
      reset_cls_pointer(E);
    }
  }
  /* vset_print(E); putchar('\n'); */
  vset_cell* q;
  for (k = 0, q = E->head; NULL != q; q = q->next, ++k)
    order[k] = q->key;

  partitions_clear(P, G->num_of_components);
  graph_clear(G);
  row_set_clear(r, M->m);
  vset_clear(E);
  
  return order;
}

bool check_c0p_matrix(matrix* M) {
  unsigned i;
  for (i = 0; i < M->m; ++i) {
    unsigned j;
    /* state = 0 means that M[i][0] .. M[i][j-1] are '1'
     * state = 1 means that for some k, M[i][k] .. M[i][j-1] are '0' */
    unsigned state = 0;
    for (j = 0; j < M->n; ++j) {
      if ('0' == M->b[i][j] && 0 == state) { ++state; }
      else if ('1' == M->b[i][j] && 1 == state) { ++state; }
      else if ('0' == M->b[i][j] && 2 == state) { return false; }
      /* printf("i = %d, j = %d, state = %d\n", i, j, state); */
    }
  }
  return true;
}

bool check_circ1p_matrix(matrix* M) {
  return (check_c1p_matrix(M) || check_c0p_matrix(M));
}

unsigned* decide_circ1p_matrix(matrix* M) {
  unsigned* order = decide_c1p_matrix(M);
  if (NULL != order)
    return order;
  return decide_c0p_matrix(M);
}

#endif
