#include "../src/sgc.h"

int main() {
  sgc_init();

  void *test = sgc_realloc(NULL, 123);

  test = sgc_realloc(test, 124);
  
  void *test2 = sgc_malloc(2000);
  test = sgc_realloc(test, 3000);

  sgc_exit();
}
