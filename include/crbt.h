#ifndef __CRBT_CRBT_H__
#define __CRBT_CRBT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifndef __CRBT_HEADER_H__
#include "header.h"
#endif

#ifndef __CRBT_RULE_H__
#include "rule.h"
#endif

#ifndef __CRBT_PARTITION_H__
#include "partition.h"
#endif

struct SRBT {
  int var;
  struct SRBT* left;
  struct SRBT* right;
  unsigned tn;
  int rule;
  unsigned candidate_rule;
};
typedef struct SRBT srbt;

#define crbt srbt***

unsigned srbt_search(srbt* ptr, const unsigned n, const header h) {
  srbt* t = ptr;
  unsigned rn = n;
  /* printf("var = %d, tn = %u, rule = %d, candidate_rule = %u\n", t->var, t->tn, t->rule, t->candidate_rule); */
  do {
    if (-1 != t->rule && t->rule < (int)rn)
      rn = (unsigned)t->rule;
    if ('0' == h.string[t->var])
      t = t->left;
    else
      t = t->right;
    /* if (NULL != t) */
    /*   printf("var = %d, tn = %u, rule = %d, candidate_rule = %u\n", t->var, t->tn, t->rule, t->candidate_rule); */
  } while (NULL != t);
  return rn;
}

void srbt_print(srbt* ptr, const unsigned tn) {
  if (NULL == ptr || ptr->tn != tn)
    return;
  srbt_print(ptr->left, ptr->tn);
  srbt_print(ptr->right, ptr->tn);
  printf("var = %d, tn = %u, rule = %d, candidate_rule = %u\n", ptr->var, ptr->tn, ptr->rule, ptr->candidate_rule);
}

void srbts_print(srbt** S, const unsigned w) {
 unsigned i;
 for (i = 0; i < w; ++i) {
   if (NULL == S[i]->left && NULL == S[i]->right)
     continue;
   printf("========== s[%u] ==========\n", i);
   srbt_print(S[i], i);
   putchar('\n');
 }
}

unsigned crbt_search(crbt C, const unsigned size, const unsigned n, const header h) {
  unsigned i, m, candidate = n+1;
  for (i = 0; i < size; ++i) {
    m = srbt_search(C[i][0], candidate, h);
    if (m < candidate)
      candidate = m;
  }
  return candidate;
}

void do_crbt_search(crbt C, const unsigned size, const unsigned n, const header* H) {
  printf("===================== CRBT Search =====================\n");
  unsigned i;
  /* for (i = 0; i < num_of_headers; ++i) */
  /*   printf("h[%u] = %s ---> %u\n", H[i].num, H[i].string, crbt_search(C, size, n, H[i])); */
  for (i = 0; i < num_of_headers; ++i)
    crbt_search(C, size, n, H[i]);
}

unsigned* do_crbt_search_with_results(crbt C, const unsigned size, const unsigned n, const header* H) {
  printf("===================== CRBT Search =====================\n");
  unsigned* results = (unsigned*)calloc(num_of_headers, sizeof(unsigned));
  unsigned i;
  for (i = 0; i < num_of_headers; ++i)
    results[i] = crbt_search(C, size, n, H[i]);
  return results;
}

srbt* mk_srbt_node(int var, unsigned tn, unsigned candidate_rule) {
  srbt* new = (srbt*)calloc(1, sizeof(srbt));
  new->var = var;
  new->tn = tn;
  new->rule = -1;
  new->candidate_rule = candidate_rule;
  new->left = new->right = NULL;
  return new;
}

void set_roots(srbt** rbt, const unsigned w, const unsigned n, const unsigned *si) {
  unsigned i;
  for (i = 0; i < w; ++i)
    rbt[i] = mk_srbt_node(si[i], i, n+1);
}

void modify_tj(srbt** rbt, unsigned j, mrule* r, const unsigned* sigma, unsigned w) {
  srbt* t = rbt[j];
  int trie_num = j;

  /* if there is no rule on the T[j]
     then the candidate rule number of the root on T[j] is the rule number of r */
  if (r->num < t->candidate_rule)
    t->candidate_rule = r->num;
  for ( ; '*' != r->cond[sigma[j]] && '\0' != r->cond[sigma[j]] && j < w; ++j) {
    /* if the node pointed by t has a rule, then the rule r is redundant */
    if (-1 != t->rule)
      return;
    if ('0' == r->cond[sigma[j]]) {
      if (NULL == t->left)
	t->left = mk_srbt_node(sigma[j+1], trie_num, r->num);
      t = t->left;
    }
    else {
      if (NULL == t->right)
	t->right = mk_srbt_node(sigma[j+1], trie_num, r->num);
      t = t->right;
    }
  }
  /* add the rule to the node pointed by the pointer t */
  if (-1 == t->rule)
    t->rule = r->num;
}

unsigned get_start_point(const char* cond, const unsigned w, const unsigned* sigma) {
  unsigned i;
  for (i = 0; i < w; ++i)
    if ('*' != cond[sigma[i]])
      break;
  return i;
}

srbt** mk_backbone_rbt(list_mrule* R, const unsigned n) {
  const unsigned w = strlen(R->head->key->cond);
  srbt** rbt = (srbt**)calloc(w, sizeof(srbt*));
  set_roots(rbt, w, n, R->sigma);

  list_mrule_cell* p;
  for (p = R->head; NULL != p; p = p->next) {
    unsigned j = get_start_point(p->key->cond, w, R->sigma);
    modify_tj(rbt, j, p->key, R->sigma, w);
  }
  return rbt;
}

void sub_mk_srbt(srbt* y_srbt, srbt* o_srbt) {
  if (-1 != o_srbt->rule) {
    if (-1 == y_srbt->rule) {
      y_srbt->rule = o_srbt->rule;
      if (o_srbt->rule < (int)y_srbt->candidate_rule)
	y_srbt->candidate_rule = (unsigned)o_srbt->rule;
    }
    else if (o_srbt->rule < y_srbt->rule) {
      y_srbt->rule = o_srbt->rule;
      if (o_srbt->rule < (int)y_srbt->candidate_rule)
	y_srbt->candidate_rule = (unsigned)o_srbt->rule;
    }
  }
  if (NULL != o_srbt->left) {
    if (NULL == y_srbt->left) {
      if (-1 == y_srbt->rule || (int)o_srbt->left->candidate_rule < y_srbt->rule) { /* There may be a highest priority rule on nodes after pointed o_srbt. */
	y_srbt->left = o_srbt->left;
	if (o_srbt->left->candidate_rule < y_srbt->candidate_rule)
	  y_srbt->candidate_rule = o_srbt->left->candidate_rule;
      }
    }
    else {
      sub_mk_srbt(y_srbt->left, o_srbt->left);
      if (y_srbt->left->candidate_rule < y_srbt->candidate_rule)
	y_srbt->candidate_rule = y_srbt->left->candidate_rule;
    }
  }
  if (NULL != o_srbt->right) {
    if (NULL == y_srbt->right) {
      if (-1 == y_srbt->rule || (int)o_srbt->right->candidate_rule < y_srbt->rule) {
	y_srbt->right = o_srbt->right;
	if (o_srbt->right->candidate_rule < y_srbt->candidate_rule)
	  y_srbt->candidate_rule = o_srbt->right->candidate_rule;
      }
    }
    else {
      sub_mk_srbt(y_srbt->right, o_srbt->right);
      if (y_srbt->right->candidate_rule < y_srbt->candidate_rule)
	y_srbt->candidate_rule = y_srbt->right->candidate_rule;
    }
  }
}

srbt** mk_srbt(list_mrule* R, const unsigned n) {
  srbt** srbt = mk_backbone_rbt(R, n);
  const unsigned w = strlen(R->head->key->cond);
  /* printf("w = %d\n", w); */
  /* list_mrule_print(R); putchar('\n'); */

  int i;
  for (i = (int)w-1; 1 <= i; --i) {
    if (NULL == srbt[i-1]->left) {
      if (n+1 != srbt[i]->candidate_rule) {
	srbt[i-1]->left = srbt[i];
	if (srbt[i]->candidate_rule < srbt[i-1]->candidate_rule)
	  srbt[i-1]->candidate_rule = srbt[i]->candidate_rule;
      }
    }
    else {
      sub_mk_srbt(srbt[i-1]->left, srbt[i]);
      if (srbt[i-1]->left->candidate_rule < srbt[i-1]->candidate_rule)
	srbt[i-1]->candidate_rule = srbt[i-1]->left->candidate_rule;
    }
    if (NULL == srbt[i-1]->right) {
      if (n+1 != srbt[i]->candidate_rule) {
	srbt[i-1]->right = srbt[i];
	if (srbt[i]->candidate_rule < srbt[i-1]->candidate_rule)
	  srbt[i-1]->candidate_rule = srbt[i]->candidate_rule;
      }
    }
    else {
      sub_mk_srbt(srbt[i-1]->right, srbt[i]);
      if (srbt[i-1]->right->candidate_rule < srbt[i-1]->candidate_rule)
	srbt[i-1]->candidate_rule = srbt[i-1]->right->candidate_rule;
    }
  }
  return srbt;
}

crbt mk_crbt(list_mrulelist* RR, const unsigned n) {
  list_mrulelist_cell* p;
  crbt C = (crbt)calloc(RR->size, sizeof(srbt**));
  unsigned i;
  for (i = 0, p = RR->head; NULL != p; p = p->next, ++i)
    C[i] = mk_srbt(p->key, n);
  return C;
}

/***** based on circular ones property *****/

unsigned get_start_point_circ1p(const char* cond, const unsigned w, const unsigned* sigma) {
  /* printf("w = %u\n", w); */
  int i;
  bool state = false;
  for (i = (int)w-1; 0 <= i; --i) {
    if (!state && '*' != cond[sigma[i]])
      state = true;
    if (state && '*' == cond[sigma[i]])
      break;
  }
  return i+1;
}

void set_roots_circ1p(srbt** rbt, const unsigned w, const unsigned n, const unsigned *si) {
  unsigned i;
  for (i = 0; i < w; ++i) {
    rbt[i] = mk_srbt_node(si[i], i, n+1);
    /* rbt[w+i] = mk_srbt_node(si[i], w+i, n+1); */
  }
}

void modify_tj_circ1p(srbt** rbt, const unsigned i, mrule* r, const unsigned* sigma, unsigned w) {
  unsigned j = i;
  srbt* t = rbt[j];
  int trie_num = j;

  /* if there is no rule on the T[j]
     then the candidate rule number of the root on T[j] is the rule number of r */
  if (r->num < t->candidate_rule)
    t->candidate_rule = r->num;
  for ( ; '*' != r->cond[sigma[j]] && '\0' != r->cond[sigma[j]] && j < w; ++j) {
    /* if the node pointed by t has a rule, then the rule r is redundant */
    if (-1 != t->rule)
      return;
    if ('0' == r->cond[sigma[j]]) {
      if (NULL == t->left)
	t->left = mk_srbt_node(sigma[(j+1)%w], trie_num, r->num);
      t = t->left;
    }
    else {
      if (NULL == t->right)
	t->right = mk_srbt_node(sigma[(j+1)%w], trie_num, r->num);
      t = t->right;
    }
  }
  if (0 != i) { /* circular condition 1 */
    for (j = 0; '*' != r->cond[sigma[j]]; ++j) { /* circular condition 2 */
      if (-1 != t->rule)
      	return;
      if ('0' == r->cond[sigma[j]]) {
	if (NULL == t->left)
	  t->left = mk_srbt_node(sigma[j+1], trie_num, r->num);
	t = t->left;
      }
      else {
	if (NULL == t->right)
	  t->right = mk_srbt_node(sigma[j+1], trie_num, r->num);
	t = t->right;
      }
    }
  }
  /* add the rule to the node pointed by the pointer t */
  if (-1 == t->rule)
    t->rule = r->num;
}

srbt** mk_backbone_rbt_circ1p(list_mrule* R, const unsigned n) {
  const unsigned w = strlen(R->head->key->cond);
  /* srbt** rbt = (srbt**)calloc(w*2, sizeof(srbt*)); */
  srbt** rbt = (srbt**)calloc(w, sizeof(srbt*));
  set_roots_circ1p(rbt, w, n, R->sigma);

  list_mrule_cell* p;
  for (p = R->head; NULL != p; p = p->next) {
    unsigned j = get_start_point_circ1p(p->key->cond, w, R->sigma);
    /* printf("r[%u] starst at %u\n", p->key->num, j); */
    modify_tj_circ1p(rbt, j, p->key, R->sigma, w);
  }
  return rbt;
}

srbt** mk_srbt_circ1p(list_mrule* R, const unsigned n) {
  srbt** srbt = mk_backbone_rbt_circ1p(R, n);
  const unsigned w = strlen(R->head->key->cond);
  /* order_print(R->sigma, w); */
  /* list_mrule_print_with_order(R); putchar('\n'); */
  /* srbts_print(srbt, w); */

  int i;
  for (i = (int)w-1; 1 <= i; --i) {
    if (NULL == srbt[i-1]->left) {
      if (n+1 != srbt[i]->candidate_rule) {
  	srbt[i-1]->left = srbt[i];
  	if (srbt[i]->candidate_rule < srbt[i-1]->candidate_rule)
  	  srbt[i-1]->candidate_rule = srbt[i]->candidate_rule;
      }
    }
    else {
      sub_mk_srbt(srbt[i-1]->left, srbt[i]);
      if (srbt[i-1]->left->candidate_rule < srbt[i-1]->candidate_rule)
  	srbt[i-1]->candidate_rule = srbt[i-1]->left->candidate_rule;
    }
    if (NULL == srbt[i-1]->right) {
      if (n+1 != srbt[i]->candidate_rule) {
  	srbt[i-1]->right = srbt[i];
  	if (srbt[i]->candidate_rule < srbt[i-1]->candidate_rule)
  	  srbt[i-1]->candidate_rule = srbt[i]->candidate_rule;
      }
    }
    else {
      sub_mk_srbt(srbt[i-1]->right, srbt[i]);
      if (srbt[i-1]->right->candidate_rule < srbt[i-1]->candidate_rule)
  	srbt[i-1]->candidate_rule = srbt[i-1]->right->candidate_rule;
    }
  }

  return srbt;
}

crbt mk_crbt_circ1p(list_mrulelist* RR, const unsigned n) {
  list_mrulelist_cell* p;
  crbt C = (crbt)calloc(RR->size, sizeof(srbt**));
  unsigned i;
  for (i = 0, p = RR->head; NULL != p; p = p->next, ++i)
    C[i] = mk_srbt_circ1p(p->key, n);
  return C;
}

#endif
