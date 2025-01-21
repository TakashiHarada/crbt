/* c1p.h */

#ifndef __TANAKALAB_C1P_H__
#define __TANAKALAB_C1P_H__

#ifndef __TANAKALAB_LIST_H__
#include "list.h"
#endif

#include <math.h>

unsigned MAX_CLS_NUM;

struct MATRIX {
  unsigned m; // number of row
  unsigned n; // number of column
  char**   b;
};
typedef struct MATRIX matrix;

bool check_c1p_matrix(matrix*);
void matrix_print(matrix*);

struct GRAPH {
  unsigned size;              /* number of vertices */
  list_unsigned** al;         /* adjacency list representing edges */
  unsigned num_of_components; /* number of connected components */
  int *cn;                    /* connected component number (id) for each vertex */
  list_unsigned** com;
};
typedef struct GRAPH graph;

struct CLASS;

struct LEFT_RIGHT {
  struct CLASS* l;
  struct CLASS* r;
};
typedef struct LEFT_RIGHT lr;

struct VSET_CELL {
  unsigned key;           /* vertex number */
  struct CLASS* cls;      /* pointer to a class */
  struct VSET_CELL* prev; /* pointer to the previous cell */
  struct VSET_CELL* next; /* pointer to the next cell */
  bool do_intersect_T;    /* for refine */
};
typedef struct VSET_CELL vset_cell;

struct VSET {
  vset_cell* head;
  vset_cell* last;
  unsigned size;
};
typedef struct VSET vset;

struct VSET_CELL_LIST_CELL {
  vset_cell* key;
  struct VSET_CELL_LIST_CELL *prev;
  struct VSET_CELL_LIST_CELL *next;
};
typedef struct VSET_CELL_LIST_CELL vset_cell_list_cell;

struct VSET_CELL_LIST {
  vset_cell_list_cell* head;
  vset_cell_list_cell* last;
  unsigned size;
};
typedef struct VSET_CELL_LIST vset_cell_list;

/* class is called "part" in other paper */
struct CLASS {
  unsigned cls_num;   // class number;
  unsigned size;
  vset_cell* head;    // pointer to the first element of the class
  vset_cell* last;    // pointer to the last element of the class
  struct CLASS* prev;
  struct CLASS* next;
  unsigned counter;   // for refine routine
  bool do_intersect;  // for refine routine
};
typedef struct CLASS class;

/* partition is a list of class */
struct PARTITION {
  class* head;   // pointer to the first element of the partition
  class* last;   // pointer to the last element of the partition
  unsigned size; // the number of classes
};
typedef struct PARTITION partition;

void partition_delete_sub(partition* P, class* x) {
  P->size = P->size-1;
  if (NULL != x->prev) { x->prev->next = x->next; }
  else { P->head = x->next; }
  if (NULL != x->next) { x->next->prev = x->prev; }
  else { P->last = x->prev; }
  free(x);
}

void partition_clear(partition* P) {
  if (NULL == P) { return; }
  class *C, *D;
  for (C = P->head; NULL != C; ) {
    D = C;
    C = C->next;
    free(D);
  }
  free(P);
}

void partitions_clear(partition** P, unsigned n) {
  if (NULL == P) { return ; }
  unsigned i;
  for (i = 0; i < n; ++i) { partition_clear(P[i]); P[i] = NULL; }
}

void partition_print(partition* P) {
  if (NULL == P) { return; }
  class* C;
  vset_cell* v;
  for (C = P->head; NULL != C; C = C->next) {
    printf("P[%d] : ", C->cls_num);
    v = C->head;
    if (NULL == v) { putchar('\n'); continue; }
    if (NULL != v) {
      if (C->size == 1) { printf("%d\n", v->key); } // |C| = 1
      else {      // |C| > 1
	printf("%d", v->key);
	for (v = v->next; C->last->next != v; v = v->next)
	  printf(", %d", v->key);
	putchar('\n');
      }
    }
  }
}


matrix* read_matrix(char* filename) {
   FILE* fp;
  if (NULL == (fp = fopen(filename,"r"))) {
    fprintf(stderr,"ERROR: Can't read the graph file.\n");
    exit(1);
  }
  char* line = NULL;
  size_t len = 0;
  unsigned m = 0;
  unsigned n = 0;
  if (-1 == getline(&line, &len, fp)) { exit(1); }
  ++m, n = strlen(line)-1;
  while (-1 != getline(&line, &len, fp)) { ++m; }

  matrix* M = (matrix*)calloc(1, sizeof(matrix));
  M->b = (char**)calloc(m, sizeof(char*));
  M->m = m;
  M->n = n;
  
  unsigned i, j;
  rewind(fp);
  for (i = 0; getline(&line, &len, fp) != -1; ++i) {
    M->b[i] = (char*)calloc(n+1, sizeof(char));
    for (j = 0; j < n; ++j) { M->b[i][j] = line[j]; }
    M->b[i][n] = '\0';
  }
  fclose(fp);
  return M;
}

void ordprint(unsigned* o, unsigned n) {
  printf("%d", o[0]);
  unsigned i;
  for (i = 1; i < n; ++i) { printf(", %d", o[i]); }
  putchar('\n');
}

void matrix_permutate(matrix* M, unsigned* si) {
  unsigned* s = (unsigned*)calloc(M->n, sizeof(unsigned));
  unsigned* t = (unsigned*)calloc(M->n, sizeof(unsigned));
  unsigned* ti = (unsigned*)calloc(M->n, sizeof(unsigned));
  
  unsigned j;
  for (j = 0; j < M->n; ++j) {
    s[si[j]] = j;
    t[j] = ti[j] = j;
  }
    
  for (j = 0; j < M->n; ++j) {
    unsigned i;
    char tmp;
    for (i = 0; i < M->m; ++i) {
      tmp = M->b[i][j];
      M->b[i][j] = M->b[i][t[si[j]]];
      M->b[i][t[si[j]]] = tmp;
    }
    unsigned k, h;
    k = ti[j];
    ti[j] = si[j];
    ti[t[si[j]]] = k;

    h = t[k];
    t[k] = t[si[j]];
    t[si[j]] = h;
    /* printf("t  = "); ordprint(t, M->n); */
    /* printf("ti = "); ordprint(ti, M->n); */
    /* matrix_print(M); putchar('\n'); */
  }

  free(ti);
  free(t);
  free(s);
}

void matrix_clear(matrix* M) {
  unsigned i;
  for (i = 0; i < M->m; ++i)
    free(M->b[i]);
  free(M->b);
  free(M);
}

void matrix_print(matrix* M) {
  unsigned i, j;
  for (i = 0; i < M->m; ++i) {
    for (j = 0; j < M->n; ++j) putchar(M->b[i][j]);
    putchar('\n');
  }
}

void matrix_print_with_order(matrix* M, unsigned* s) {
  unsigned i;
  for (i = 0; i < M->m; ++i) {
    unsigned j;
    for (j = 0; j < M->n; ++j) putchar(M->b[i][s[j]]);
    putchar('\n');
  }
}

void vset_cell_list_add_rear_sub(vset_cell_list* E, vset_cell_list_cell* x) {
  x->prev = E->last;
  if (NULL != E->last) { E->last->next = x; }
  else { E->head = x; }
  E->last = x;
  x->next = NULL;
}

void vset_cell_list_add_rear(vset_cell_list* E, vset_cell* v) {
  vset_cell_list_cell* new = (vset_cell_list_cell*)calloc(1, sizeof(vset_cell_list_cell));
  E->size = E->size + 1;
  new->key = v;
  vset_cell_list_add_rear_sub(E, new);
}

void vset_cell_list_clear(vset_cell_list* L) {
  if (NULL == L) { return; }
  vset_cell_list_cell *p, *q;
  for (p = L->head; NULL != p; ) {
    q = p;
    p = p->next;
    free(q);
  }
  free(L);
}

void vset_cell_list_print(vset_cell_list* L) {
  vset_cell_list_cell* p = L->head;
  if (NULL != p) {
    /* printf("pointed cell's key : %d", p->key->key); */
    printf("%d", p->key->key);
    for (p = p->next; NULL != p; p = p->next) { printf(", %d", p->key->key); }
  }
}

/* each row r_i is a set of the column number j, s.t. M_ij = 1 */
vset_cell_list** make_row_set(matrix* M, vset* E) {
  vset_cell_list** r = (vset_cell_list**)calloc(M->m, sizeof(vset_cell_list*));
  unsigned i, j;
  vset_cell** ptr = (vset_cell**)calloc(M->n, sizeof(vset_cell*));
  vset_cell* p;
  for (j = 0, p = E->head; NULL != p; ++j, p = p->next) { ptr[j] = p; }
  for (i = 0; i < M->m; ++i) {
    r[i] = (vset_cell_list*)calloc(1, sizeof(vset_cell_list));
    for (j = 0; j < M->n; ++j) { if ('1' == M->b[i][j]) { vset_cell_list_add_rear(r[i], ptr[j]); } }
  }
  free(ptr);
  return r;
}

void row_set_clear(vset_cell_list** r, const unsigned m) {
  unsigned i;
  for (i = 0; i < m; ++i) { vset_cell_list_clear(r[i]); }
  free(r);
}

void row_set_print(vset_cell_list** r, const unsigned m) {
  unsigned i;
  for (i = 0; i < m; ++i) { printf("r[%d] : ", i); vset_cell_list_print(r[i]); putchar('\n'); } 
}

void vset_add_rear_sub(vset* E, vset_cell* x) {
  x->prev = E->last;
  if (NULL != E->last) { E->last->next = x; }
  else { E->head = x; }
  E->last = x;
  x->next = NULL;
}

void vset_add_rear(vset* E, unsigned v) {
  vset_cell* new = (vset_cell*)calloc(1, sizeof(vset_cell));
  E->size = E->size + 1;
  new->key = v;
  new->do_intersect_T = false;
  new->cls = NULL;
  vset_add_rear_sub(E, new);
}

void vset_print(vset* E) {
  vset_cell* p = E->head;
  if (NULL != p) {
    printf("E : %d", p->key);
    for (p = p->next; NULL != p; p = p->next) { printf(", %d", p->key); }
  }
}

void vset_clear(vset* E) {
  if (NULL == E) { return; }
  vset_cell *p, *q;
  for (p = E->head; NULL != p; ) {
    q = p;
    p = p->next;
    free(q);
  }
  free(E);
}

/* move the cell pointed by x to the previous of the cell y */
void vset_move(vset* E, vset_cell* x, vset_cell* y) {
  if (x == y) { return; }
  /* remove x from E */
  if (NULL == x->prev) { E->head = x->next; } /* x is the first element of E */
  else { x->prev->next = x->next; }
  if (NULL == x->next) {
    E->last = x->prev;
    E->last->next = NULL;
  } /* x is the last element of E */
  else { x->next->prev = x->prev; }
  /* insert x to the previous of y */
  if (NULL == y->prev) { /* y is the first element of E */
    E->head = x;
    x->prev = NULL;
  }
  else { x->prev = y->prev; x->prev->next = x; }
  y->prev = x;
  x->next = y;
}

/* move the cell pointed by x to the next of the cell y */
void vset_insert_next(vset* E, vset_cell* x, vset_cell* y) {
  if (x == y) { return; }
  /* remove x from E */
  if (NULL == x->prev) { E->head = x->next; } /* x is the first element of E */
  else { x->prev->next = x->next; }
  if (NULL == x->next) { E->last = x->prev; } /* x is the last element of E */
  else { x->next->prev = x->prev; }
  /* insert x to the next of y */
  if (NULL == y->next) { E->last = x; } /* y is the last element of E */
  else { x->next = y->next; x->next->prev = x; }
  y->next = x;
  x->prev = y;
}

vset* init_vertex_set(matrix* M) {
  vset* E = (vset*)calloc(1, sizeof(vset));
  E->head = E->last = NULL;
  unsigned i;
  for (i = 0; i < M->n; ++i) { vset_add_rear(E, i); }
  return E;
}

void dfs(graph* G, unsigned v, int k) {
  G->cn[v] = k;
  list_unsigned_cell* p;
  for (p = G->al[v]->head; NULL != p; p = p->next) { if (-1 == G->cn[p->key]) { dfs(G, p->key, k); } }
}

void decomposing_to_connected_components(graph* G) {
  G->cn = (int*)calloc(G->size, sizeof(unsigned));
  unsigned v;
  for (v = 0; v < G->size; ++v) { G->cn[v] = -1; }
  int k = 0;
  for (v = 0; v < G->size; ++v) { if (-1 == G->cn[v]) { dfs(G, v, k); ++k; } }
  G->num_of_components = k;
  G->com = (list_unsigned**)calloc(G->num_of_components, sizeof(list_unsigned*));
  for (v = 0; v < G->num_of_components; ++v) { G->com[v] = (list_unsigned*)calloc(1, sizeof(list_unsigned)); }
  for (v = 0; v < G->size; ++v) { list_unsigned_add_rear(G->com[G->cn[v]], v); }
}

void graph_print(graph* G) {
  const unsigned d = floor(log10(G->size)) + 1;
  unsigned i;
  for (i = 0; i < G->size; ++i) { printf("al[%*d] : ", d, i); list_unsigned_print(G->al[i]); putchar('\n'); }
  if (NULL != G->com) { for (i = 0; i < G->num_of_components; ++i) { printf("cn[%*d] : ", d, i);  list_unsigned_print(G->com[i]); putchar('\n'); } }
}

void graph_clear(graph* G) {
  unsigned i;
  for (i = 0; i < G->size; ++i) { list_unsigned_clear(G->al[i]); }
  free(G->al);
  if (NULL != G->cn) { free(G->cn); }
  if (NULL != G->com) {
    for (i = 0; i < G->num_of_components; ++i) { list_unsigned_clear(G->com[i]); }
    free(G->com);
  }
  free(G);
}

graph* make_overlap_graph(matrix* M, vset_cell_list** r) {
  graph* g = (graph*)calloc(1,sizeof(graph));
  g->size = M->m; /* the number of vertices (g->size) is M->m */
  g->al = (list_unsigned**)calloc(M->m, sizeof(list_unsigned*));

  unsigned i, j;
  for (i = 0; i < M->m; ++i) { g->al[i] = (list_unsigned*)calloc(1, sizeof(list_unsigned)); }
  for (i = 0; i < M->m; ++i)
    for (j = i+1; j < M->m; ++j) {
      unsigned count = 0, min = r[i]->size, min_idx = i, max_idx = j;
      if (r[j]->size < min) { min = r[j]->size; min_idx = j; max_idx = i; }
      vset_cell_list_cell* p;
      for (p = r[min_idx]->head; NULL != p; p = p->next) { if ('1' == M->b[max_idx][p->key->key]) { ++count; } }
      if (0 < count && count < r[min_idx]->size) { list_unsigned_add_rear(g->al[i], j); list_unsigned_add_rear(g->al[j], i); }
    }
  return g;
}

void dfs_span_tree(graph* G, unsigned C, list_unsigned* L, unsigned v, unsigned k, int* cn) {
  cn[v] = k;
  /* printf("v = %d\n", v); */
  list_unsigned_add_rear(L, v);
  list_unsigned_cell* p;
  for (p = G->al[v]->head; NULL != p; p = p->next) { if (-1 == cn[p->key]) { dfs_span_tree(G, C, L, p->key, k, cn); } }
}

/* C is a component number */
list_unsigned* spanning_tree(graph* G, unsigned C) {
  list_unsigned* L = (list_unsigned*)calloc(1, sizeof(list_unsigned));
  L->head = L->last = NULL;
  int* cn = (int*)calloc(G->size, sizeof(int));
  list_unsigned_cell* p;
  for (p = G->com[C]->head; NULL != p; p = p->next) { cn[p->key] = -1; }
  unsigned k = 0;
  for (p = G->com[C]->head; NULL != p; p = p->next) { if (-1 == cn[p->key]) { dfs_span_tree(G, C, L, p->key, k, cn); ++k; } }
  /* for (p = G->com[C]->head; NULL != p; p = p->next) { if (0 == cn[p->key]) { dfs_span_tree(G, C, L, p->key, k, cn); ++k; } } */

  free(cn); cn = NULL;
  return L;
}

partition* set_P1(vset_cell_list** r, vset* E, unsigned v) {
  partition* P1 = (partition*)calloc(1, sizeof(partition));
  class* C = (class*)calloc(1, sizeof(class));
  C->cls_num = 0;
  C->counter = 0;
  C->prev = C->next = NULL;  // pointers for classes (parts)
  C->head = r[v]->head->key; // pointer for vertex cell
  C->last = NULL;            // pointer for vertex cell
  C->do_intersect = false;   // for refine routine
  P1->head = P1->last = C;
  P1->size = 1;
  
  vset_cell_list_cell* p;
  vset_cell* q = r[v]->head->key;
  for (p = r[v]->head; NULL != p; p = p->next) {
    /* printf("p points %d cell, q points %d cell\n", p->key->key, q->key); */
    if (p->key != q) { vset_move(E, p->key, q); }
    else { q = q->next; }
    p->key->cls = C;
    C->last = p->key;
  }
  C->size = r[v]->size;
  
  return P1;
}

/* bool is_consecutive(lr lr, vset* P) { */
bool is_consecutive(lr lr) {
  class* p;
  for (p = lr.l; p != lr.r->next; p = p->next) {
    if (!p->do_intersect) { return false; }
    /* printf("p[%d] : ", p->cls_num); */
    /* if (p->do_intersect) { printf("intersect\n"); } */
    /* else { printf("not intersect\n"); } */
  }
  return true;
}

void reset_do_intersect_P(partition* P) {
  class* p;
  for (p = P->head; NULL != p; p = p->next) { p->do_intersect = false; }
}

void reset_do_intersect_E(vset_cell_list* T) {
  vset_cell_list_cell* r;
  for (r = T->head; NULL != r; r = r->next) { r->key->do_intersect_T = false; }
}

void reset_class_counter(partition* P) {
  class* p;
  for (p = P->head; NULL != p; p = p->next) { p->counter = 0; }
}

void reset_cls_pointer(vset* E) {
  vset_cell* p;
  for (p = E->head; NULL != p; p = p->next)
    p->cls = NULL;
}

lr refine_case_1(partition* P, vset* E, vset_cell_list* T) {
  vset_cell* s = P->last->last->next;
  class* new_class = (class*)calloc(1, sizeof(class));
  new_class->cls_num = MAX_CLS_NUM; ++ MAX_CLS_NUM;
  new_class->counter = 0;
  new_class->prev = P->last; 
  new_class->next = NULL;
  new_class->head = T->head->key;
  new_class->do_intersect = true;
  P->last->next = new_class;
  P->last = new_class;
  ++(P->size);

  vset_cell_list_cell* r;
  for (r = T->head; NULL != r; r = r->next) {
    if (r->key != s) { vset_move(E, r->key, s); }
    else { s = s->next; }
    r->key->cls = new_class;
    new_class->last = s;
  }
  new_class->size = T->size;

  lr lr;
  lr.l = new_class;
  lr.r = new_class;

  return lr;
}

lr refine_case_2(partition* P, vset* E, vset_cell_list* T) {
  class* C;
  vset_cell_list_cell* r;

  lr lr = { NULL, NULL };
  for (C = P->head; NULL != C; ) {
    /* printf("C->cls_num = %d", C->cls_num); */
    /* if (NULL != C->prev) printf(", prev->cls_num = %d\n", C->prev->cls_num); */
    /* else printf(" oops prev pointer NULL !!\n"); */
    if (0 != C->counter && C->counter < C->size) { // to be refined
      if (P->head == C) { // (a)
	/* printf("(a)\n"); */
	/* P->head (p1) is used as p1_intersect_T */
	class* p1_minus_T = (class*)calloc(1, sizeof(class));
	p1_minus_T->cls_num = MAX_CLS_NUM; ++MAX_CLS_NUM;
	p1_minus_T->counter = 0;
	p1_minus_T->prev = NULL;
	p1_minus_T->next = C;
	p1_minus_T->do_intersect = false;
	P->head = p1_minus_T;
	C->prev = p1_minus_T;
	++(P->size);

	p1_minus_T->size = 0;
	p1_minus_T->head = p1_minus_T->last = NULL;
	vset_cell* c;
	for (c = C->head; C->last->next != c; c = c->next) {
	  if (!c->do_intersect_T) {
	    if (NULL == p1_minus_T->head) { p1_minus_T->head = c; }
	    if (c == C->head) { C->head = C->head->next; }
	    if (c == C->last) { C->last = C->last->prev; }
	    vset_move(E, c, C->head);
	    c->cls = p1_minus_T;
	    --(C->size);
	    ++(p1_minus_T->size);
	    p1_minus_T->last = c;
	  }
	}
	lr.l = lr.r = C;
	C->do_intersect = true;
	C = C->next;
      }
      else if (!C->prev->do_intersect) { // (b)
	/* printf("(b)\n"); */
	class* p1_minus_T = (class*)calloc(1, sizeof(class));
	p1_minus_T->cls_num = MAX_CLS_NUM; ++MAX_CLS_NUM;
	p1_minus_T->counter = 0;
	p1_minus_T->prev = C->prev;
	p1_minus_T->next = C;
	C->prev->next = p1_minus_T;
	C->prev = p1_minus_T;
	p1_minus_T->do_intersect = false;
	++(P->size);

	p1_minus_T->size = 0;
	p1_minus_T->head = p1_minus_T->last = NULL;
	vset_cell* c;
	for (c = C->head; C->last->next != c; c = c->next)
	  if (!c->do_intersect_T) {
	    if (NULL == p1_minus_T->head) { p1_minus_T->head = c; }
	    if (c == C->head) { C->head = C->head->next; }
	    if (c == C->last) { C->last = C->last->prev; }
	    vset_move(E, c, C->head);
	    c->cls = p1_minus_T;
	    --(C->size);
	    ++(p1_minus_T->size);
	    p1_minus_T->last = c;
	  }
	if (NULL == lr.l) { lr.l = C; }
	lr.r = C;
	C->do_intersect = true;
	C = C->next;
      }
      else { // (c)
	/* printf("(c)\n"); */
	class* p1_intersect_T = (class*)calloc(1, sizeof(class));
	p1_intersect_T->cls_num = MAX_CLS_NUM; ++MAX_CLS_NUM;
	p1_intersect_T->counter = 0;
	p1_intersect_T->prev = C->prev;
	p1_intersect_T->next = C;
	C->prev->next = p1_intersect_T;
	C->prev = p1_intersect_T;
	p1_intersect_T->do_intersect = true;
	++(P->size);
	if (NULL == lr.l) { lr.l = p1_intersect_T; }
	lr.r = p1_intersect_T;

	p1_intersect_T->size = 0;
	p1_intersect_T->head = p1_intersect_T->last = NULL;
	for (r = T->head; NULL != r; r = r->next)
	  if (r->key->cls == C) {
	    if (NULL == p1_intersect_T->head) { p1_intersect_T->head = r->key; }
	    if (r->key == C->head) { C->head = C->head->next; }
	    if (r->key == C->last) { C->last = C->last->prev; }
	    vset_move(E, r->key, C->head);
	    r->key->cls = p1_intersect_T;
	    --(C->size);
	    ++(p1_intersect_T->size);
	    p1_intersect_T->last = r->key;
	  }
	C->do_intersect = false;
	C = C->next;
      }
    }
    else {
      /* printf("cls_num = %d\n", C->cls_num); */
      if (0 != C->counter) {
	if (NULL == lr.l) { lr.l = C; }
	lr.r = C;
	C->do_intersect = true;
      }
      C = C->next;
    }
  }

  return lr;
}

lr refine_case_3(partition* P, vset* E, vset_cell_list* T) {
  /* check whether case (a) or (b) */
  bool flag = false; // false denotes (a), true denotes (b)
  vset_cell_list_cell* r;
  reset_class_counter(P);
  reset_do_intersect_P(P);
  reset_do_intersect_E(T);

  lr lr = { NULL, NULL };
  /* printf("======================================= CHECK =====================================\n"); */
  /* class* hoge; */
  /* for (hoge = P->head; hoge != NULL; hoge = hoge->next) { */
  /*   printf("p[%d] : ", hoge->cls_num); */
  /*   if (hoge->do_intersect) { printf("intersect\n"); } */
  /*   else { printf("not intersect\n"); } */
  /* } */
  
  for (r = T->head; NULL != r; r = r->next) {
    if (NULL != r->key->cls && P->head != r->key->cls) { flag = true; }
    if (NULL != r->key->cls) {
      r->key->do_intersect_T = true; 
      ++r->key->cls->counter; 
    }
  }

  if (!flag) { // (a)
    /* printf("(a)\n"); */
    /* printf("P[0] counter = %d\n", P->head->counter); */
    class* p0 = P->head;
    class* T_minus_S = (class*)calloc(1, sizeof(class));
    T_minus_S->prev = NULL;
    T_minus_S->head = T_minus_S->last = NULL;
    T_minus_S->size = 0;
    T_minus_S->cls_num = MAX_CLS_NUM; ++MAX_CLS_NUM;
    P->size += 1;
    T_minus_S->do_intersect = true;
    lr.l = T_minus_S;
    
    if (p0->counter == p0->size) {
      /* T_intersect_p0 is unnecessary
       * p0 is used as T_intersect_p0 and p0_minus_T
       */
      T_minus_S->next = p0;
      p0->prev = T_minus_S;
      p0->do_intersect = true;
      lr.r = p0;
      for (r = T->head; NULL != r; r = r->next) {
	vset_cell* ptr = NULL;
	if (NULL == r->key->cls) {
	  if (NULL == T_minus_S->head) { ptr = p0->head; }
	  else { ptr = T_minus_S->head; }
	  vset_move(E, r->key, ptr);
	  r->key->cls = T_minus_S;
	  T_minus_S->size += 1;
	  if (NULL == T_minus_S->last) { T_minus_S->last = r->key; }
	  T_minus_S->head = r->key;
	}
      }
    }
    else {
      /* p0 is used as p0_minus_T */
      class* T_intersect_p0 = (class*)calloc(1, sizeof(class));
      T_intersect_p0->size = 0;
      T_intersect_p0->cls_num = MAX_CLS_NUM; ++MAX_CLS_NUM;
      T_minus_S->next = T_intersect_p0;
      T_intersect_p0->prev = T_minus_S;
      T_intersect_p0->next = p0;
      p0->prev = T_intersect_p0;
      T_intersect_p0->head = T_intersect_p0->last = NULL;
      P->size += 1;
      T_intersect_p0->do_intersect = true;
      lr.r = T_intersect_p0;
      
      for (r = T->head; NULL != r; r = r->next) {
	/* printf("r->key->key = %d\n", r->key->key); */
	vset_cell* ptr = NULL;
	if (p0 == r->key->cls) {
	  if (NULL == T_intersect_p0->head) { ptr = p0->head; }
	  else { ptr = T_intersect_p0->head; }
	  if (r->key == p0->head) { p0->head = p0->head->next; }
	  if (r->key == p0->last) { p0->last = p0->last->prev; }
	  vset_move(E, r->key, ptr);
	  
	  r->key->cls = T_intersect_p0;
	  T_intersect_p0->size += 1;
	  p0->size -= 1;
	  /* printf("T ∩ p0\n"); */
	  if (NULL == T_intersect_p0->last) { T_intersect_p0->last = r->key; }
	  T_intersect_p0->head = r->key;
	}
	if (NULL == r->key->cls) {
	  if (NULL == T_minus_S->head) {
	    if (NULL == T_intersect_p0->head) { ptr = p0->head; }
	    else { ptr = T_intersect_p0->head; }
	  }
	  else { ptr = T_minus_S->head; }
	  vset_move(E, r->key, ptr);
	  r->key->cls = T_minus_S;
	  T_minus_S->size += 1;
	  /* printf("T \\ S\n"); */
	  if (NULL == T_minus_S->last) { T_minus_S->last = r->key; }
	  T_minus_S->head = r->key;
	}
      }
    }
    P->head = T_minus_S;
    /* vset_print(E); putchar('\n'); printf("P->head->cls_num = %d\n", P->head->cls_num); */
  }
  else { // (b)
    /* printf("(b)\n"); */
    vset_cell_list* TS = (vset_cell_list*)calloc(1, sizeof(vset_cell_list));
    for (r = T->head; NULL != r; r = r->next)
      if (NULL != r->key->cls) { vset_cell_list_add_rear(TS, r->key); }

    /* printf("  refine by {"); vset_cell_list_print(TS); printf("}\n"); */
    lr = refine_case_2(P, E, TS);
    /* partition_print(P); */

    /* check refine part of head or last */
    bool flag = false;
    /* false denotes to refine last, true denotes to refine head */
    for (r = T->head; NULL != r; r = r->next) {
      /* if (NULL != r->key->cls) { printf("r->key->cls = %d, P->head->cls = %d \n", r->key->cls->cls_num, P->head->cls_num); } */
      if (r->key->cls == P->head) { flag = true; break; } 
    }

    /* recompute the couters for each classes */
    reset_class_counter(P);
    for (r = T->head; NULL != r; r = r->next) {
      /* if (NULL != r->key->cls && P->head != r->key->cls) { flag = true; } */
      if (NULL != r->key->cls) { ++r->key->cls->counter; }
    }
    
    if (flag) { // refine p_0
      /* printf("refine p0\n"); */
      class* T_minus_S = (class*)calloc(1, sizeof(class));
      T_minus_S->cls_num = MAX_CLS_NUM; ++MAX_CLS_NUM;
      T_minus_S->head = T_minus_S->last = NULL;
      T_minus_S->size = 0;
      T_minus_S->counter = 0;
      T_minus_S->do_intersect = true;
      P->size += 1;

      class* p0 = P->head;
      if (p0->counter == p0->size) {
	/* T_intersect_p0 is unnecessary */
	/* p0 is used as p0_intersect_T and p0_minus_T */
	T_minus_S->next = p0;
	p0->do_intersect = true;
	lr.l = T_minus_S;
	for (r = T->head; NULL != r; r = r->next) {
	  vset_cell* ptr = NULL;
	  if (NULL == r->key->cls) {
	    if (NULL == T_minus_S->head) { ptr = p0->head; }
	    else { ptr = T_minus_S->head; }
	    vset_move(E, r->key, ptr);
	    r->key->cls = T_minus_S;
	    T_minus_S->size += 1;
	    if (NULL == T_minus_S->last) { T_minus_S->last = r->key; }
	    T_minus_S->head = r->key;
	  }
	}
	p0->prev = T_minus_S;
      }
      else {
	/* p0 is used as p0_minus_T */
	class* p0_intersect_T = (class*)calloc(1, sizeof(class));
	p0_intersect_T->cls_num = MAX_CLS_NUM; ++MAX_CLS_NUM;
	p0_intersect_T->counter = 0;
	T_minus_S->next = p0_intersect_T;
	p0_intersect_T->prev = T_minus_S;
	p0_intersect_T->next = p0;
	p0_intersect_T->head = p0_intersect_T->last = NULL;
	p0_intersect_T->do_intersect = true;
	p0->prev = p0_intersect_T;
	P->size += 1;

	for (r = T->head; NULL != r; r = r->next) {
	  vset_cell* ptr = NULL;
	  if (p0 == r->key->cls) {
	    if (NULL == p0_intersect_T->head) { ptr = p0->head; }
	    else { ptr = p0_intersect_T->head; }
	    if (r->key == p0->head) { p0->head = p0->head->next; }
	    if (r->key == p0->last) { p0->last = p0->last->prev; }
	    vset_move(E, r->key, ptr);
	    if (ptr == p0->head) { p0->head = p0->head->next; }

	    r->key->cls = p0_intersect_T;
	    p0_intersect_T->size += 1;
	    p0->size -= 1;
	    if (NULL == p0_intersect_T->last) {
	      p0_intersect_T->last = r->key;
	      p0->head = p0->head->next;
	    }
	    p0_intersect_T->head = r->key;
	  }
	  if (NULL == r->key->cls) {
	    if (NULL == T_minus_S->head) {
	      if (NULL == p0_intersect_T->head) { ptr = p0->head; }
	      else { ptr = p0_intersect_T->head; }
	    }
	    else { ptr = T_minus_S->head; }
	    vset_move(E, r->key, ptr);
	    r->key->cls = T_minus_S;
	    T_minus_S->size += 1;
	    if (NULL == T_minus_S->last) { T_minus_S->last = r->key; }
	    T_minus_S->head = r->key;
	  }
	}
      }
      P->head = T_minus_S;
      /* vset_print(E); putchar('\n'); printf("P->head->cls_num = %d\n", P->head->cls_num); */
    }
    else { // refine p_v
      /* printf("refine p_v(%d)\n", P->last->cls_num); */
      /* partition_print(P); */
      /* vset_print(E); putchar('\n'); */
      class* T_minus_S = (class*)calloc(1, sizeof(class));
      T_minus_S->cls_num = MAX_CLS_NUM; ++MAX_CLS_NUM;
      T_minus_S->head = T_minus_S->last = NULL;
      T_minus_S->size = 0;
      T_minus_S->counter = 0;
      P->size += 1;
      T_minus_S->next = NULL;
      T_minus_S->do_intersect = true;
      lr.r = T_minus_S;
      class* pv = P->last;      
      if (pv->counter == pv->size) {
	/* pv_intersect_T is unnecessary
	 * pv is used as pv_minus_T and pv_intersect_T
	 */
	/* printf("pv ∩ T is unnecessary\n"); */
	/* partition_print(P); */
	T_minus_S->prev = pv;
	pv->next = T_minus_S;
	for (r = T->head; NULL != r; r = r->next) {
	  vset_cell* ptr = NULL;
	  if (NULL == r->key->cls) {
	    if (NULL == T_minus_S->head) { ptr = pv->last; }
	    else { ptr = T_minus_S->last; }
	    /* printf("elm = %d\n", ptr->key); */
	    vset_insert_next(E, r->key, ptr);
	    r->key->cls = T_minus_S;
	    T_minus_S->size += 1;
	    if (NULL == T_minus_S->head) { T_minus_S->head = r->key; }
	    T_minus_S->last = r->key;
	  }
	}
      }
      else {
	/* pv is used as pv_minus_T */
	class* pv_intersect_T = (class*)calloc(1, sizeof(class));
	pv_intersect_T->cls_num = MAX_CLS_NUM; ++MAX_CLS_NUM;
	pv_intersect_T->counter = 0;
	T_minus_S->prev = pv_intersect_T;
	pv_intersect_T->prev = pv;
	pv_intersect_T->next = pv_intersect_T;
	pv_intersect_T->head = pv_intersect_T->last = NULL;
	pv->next = pv_intersect_T;
	pv_intersect_T->do_intersect = true;
	P->size += 1;

	for (r = T->head; NULL != r; r = r->next) {
	  vset_cell* ptr = NULL;
	  if (pv == r->key->cls) {
	    if (NULL == pv_intersect_T->last) { ptr = pv->last; }
	    else { ptr = pv_intersect_T->last; }
	    vset_insert_next(E, r->key, ptr);
	    if (ptr == pv->last) { pv->last = pv->last->prev; }

	    r->key->cls = pv_intersect_T;
	    pv_intersect_T->size += 1;
	    pv->size -= 1;
	    if (NULL == pv_intersect_T->head) {
	      pv_intersect_T->head = r->key;
	      pv->last = pv->last->prev;
	    }
	    pv_intersect_T->last = r->key;
	  }
	  if (NULL == r->key->cls) {
	    if (NULL == T_minus_S->last) {
	      if (NULL == pv_intersect_T->last) { ptr = pv->last; }
	      else { ptr = pv_intersect_T->last; }
	    }
	    else { ptr = T_minus_S->last; }
	    vset_insert_next(E, r->key, ptr);
	    r->key->cls = T_minus_S;
	    T_minus_S->size += 1;
	    if (NULL == T_minus_S->head) { T_minus_S->head = r->key; }
	    T_minus_S->last = r->key;
	  }
	}
      }
      P->last = T_minus_S;
    }
  }
  return lr;
}

lr refine(partition* P, vset* E, vset_cell_list* T) {
  /* printf("refine by {"); vset_cell_list_print(T); printf("}\n"); */

  /* collect the class C_i to be refined i.e., C_i is not a subset of T and intersection of C_i and T is not the empty */
  /* check whether case 1 or 2 (a), (b), (3) or 3 (a) (b) */
  vset_cell_list_cell* r;
  unsigned is_not_S = 0;
  unsigned is_S = 0;
  for (r = T->head; NULL != r; r = r->next) {
    if (NULL != r->key->cls) {
      r->key->do_intersect_T = true;
      ++r->key->cls->counter;
      ++is_S;
    }
    else { ++is_not_S; }
  }

  /* refine */
  if (0 == is_S) { // case 1.
    /* printf(" case 1\n"); */
    return refine_case_1(P, E, T);
  }
  else if (0 == is_not_S) { // case 2.
    /* printf(" case 2 "); */
    return refine_case_2(P, E, T);
  }
  else { // case 3.
    /* printf(" case 3 "); */
    /* partition_print(P); */
    /* vset_print(E); putchar('\n'); */
    return refine_case_3(P, E, T);
  }
}

unsigned* decide_c1p_matrix(matrix* M) {
  unsigned* order = (unsigned*)calloc(M->n, sizeof(unsigned));

  vset* E = init_vertex_set(M);
  /* vset_print(E); putchar('\n'); */
  
  vset_cell_list** r = make_row_set(M,E);
  /* row_set_print(r, M->m); */

  graph* G = make_overlap_graph(M, r);
  decomposing_to_connected_components(G);
  /* graph_print(G); */

  unsigned k;
  unsigned num_c = G->num_of_components;
  partition** P = (partition**)calloc(num_c, sizeof(partition*));

  /* printf("#component = %d\n", num_c); */
  for (k = 0; k < num_c; ++k) {
    /* if (1 < G->com[k]->size) { */
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
    /* } */
  }
  /* vset_print(E); putchar('\n'); */
  vset_cell* q;
  for (k = 0, q = E->head; NULL != q; q = q->next, ++k) { order[k] = q->key; }

  partitions_clear(P, G->num_of_components);
  graph_clear(G);
  row_set_clear(r, M->m);
  vset_clear(E);
  
  return order;
}

bool check_c1p_matrix(matrix* M) {
  unsigned i;
  for (i = 0; i < M->m; ++i) {
    unsigned j;
    /* state = 0 means that M[i][0] .. M[i][j-1] are '0'
     * state = 1 means that for some k, M[i][k] .. M[i][j-1] are '1' */
    unsigned state = 0;
    for (j = 0; j < M->n; ++j) {
      if ('1' == M->b[i][j] && 0 == state) { ++state; }
      else if ('0' == M->b[i][j] && 1 == state) { ++state; }
      else if ('1' == M->b[i][j] && 2 == state) { return false; }
      /* printf("i = %d, j = %d, state = %d\n", i, j, state); */
    }
  }
  return true;
}

#endif
