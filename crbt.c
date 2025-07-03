#include <crbt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

long getrusageMem() {
  struct rusage r;
  getrusage(RUSAGE_SELF, &r);
  return r.ru_maxrss;
}

bool policy_violation(const unsigned* result1, const unsigned* result2, const header* H) {
  unsigned i;
  for (i = 0; i < num_of_headers; ++i)
    if (result1[i] != result2[i]) {
      printf("h[%u] = %s ---> %u\n", H[i].num, H[i].string, result1[i]);
      printf("h[%u] = %s ---> %u\n", H[i].num, H[i].string, result2[i]);
      return true;
    }
  return false;
}

int main(int argc, char** argv) {
  if (3 != argc && 4 != argc) {
    fprintf(stderr, "ERROR!!\nUsage: $ ./crbt [-c] <rule list> <header list>\n");
    exit(1);
  }

  header* H;
  list_mrule* R;
  int DEFAULT_RULE_NUMBER;
  
  if (!strcmp(argv[1], "-c")) {
    /* IMPLEMENT */
    /* printf("Input Rule List and Header List Forms are Class Bench\n"); */
    H = read_classbench_header_list(argv[3]);
    /* print_header_list(H); */
    
    R = read_classbench_rule_list(argv[2]);
    /* list_mrule_print(R); */
  }
  else {
    H = read_header_list(argv[2]);
    /* print_header_list(H); */

    R = read_mrule_list(argv[1]);
    /* list_mrule_print(R); */
  }
  DEFAULT_RULE_NUMBER = R->last->key->num;
    
  struct timespec s, e;
  clock_gettime(CLOCK_REALTIME, &s);
  /* do_linear_search(R, H); */
  unsigned* results_of_linear_search = do_linear_search_with_results(R, H);
  clock_gettime(CLOCK_REALTIME, &e);
  if (e.tv_nsec < s.tv_nsec) {
    printf("Linear Search Time = %10ld.%09ld\n", e.tv_sec - s.tv_sec - 1, e.tv_nsec + 1000000000 - s.tv_nsec);
  } else {
    printf("Linear Search Time = %10ld.%09ld\n", e.tv_sec - s.tv_sec, e.tv_nsec - s.tv_nsec);
  }    
    
  clock_gettime(CLOCK_REALTIME, &s);
  /* list_mrulelist* RR = partition_list_mrule(R); */
  list_mrulelist* RR = partition_list_mrule_circ1p(R);
  clock_gettime(CLOCK_REALTIME, &e);
  /* list_mrulelist_print_with_order(RR); */
  if (e.tv_nsec < s.tv_nsec) {
    printf("Partition Time     = %10ld.%09ld\n", e.tv_sec - s.tv_sec - 1, e.tv_nsec + 1000000000 - s.tv_nsec);
  } else {
    printf("Partition Time     = %10ld.%09ld\n", e.tv_sec - s.tv_sec, e.tv_nsec - s.tv_nsec);
  }    

  printf("The number of rule lists = %u\n", RR->size);

  /* crbt C = mk_crbt(RR, R->size); */
  /* crbt C = mk_crbt_circ1p(RR, R->size); */
  crbt C = mk_crbt_circ1p(RR, DEFAULT_RULE_NUMBER);
  printf("Memory Usage = %ld\n", getrusageMem());

  clock_gettime(CLOCK_REALTIME, &s);
  /* do_crbt_search(C, RR->size, R->size, H); */
  /* unsigned* results_of_crbt_search = do_crbt_search_with_results(C, RR->size, R->size, H); */
  unsigned* results_of_crbt_search = do_crbt_search_with_results(C, RR->size, DEFAULT_RULE_NUMBER, H);
  clock_gettime(CLOCK_REALTIME, &e);
  if (e.tv_nsec < s.tv_nsec) {
    printf("CRBT Search Time   = %10ld.%09ld\n", e.tv_sec - s.tv_sec - 1, e.tv_nsec + 1000000000 - s.tv_nsec);
  } else {
    printf("CRBT Search Time   = %10ld.%09ld\n", e.tv_sec - s.tv_sec, e.tv_nsec - s.tv_nsec);
  }
  
  if (policy_violation(results_of_linear_search, results_of_crbt_search, H))
    printf("ERROR: plicy violation occurs!!\n");
  free(results_of_linear_search);
  free(results_of_crbt_search);

  list_mrulelist_clear(RR); RR = NULL;
  list_mrule_clear(R); R = NULL;
  free_header_list(H); H = NULL;
  
  return 0;
}
