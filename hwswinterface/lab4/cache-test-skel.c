/*
Coursera HW/SW Interface
Lab 4 - Mystery Caches

Mystery Cache Geometries (for you to keep notes):
mystery0:
    block size =
    cache size =
    associativity =
mystery1:
    block size =
    cache size =
    associativity =
mystery2:
    block size =
    cache size =
    associativity =
mystery3:
    block size =
    cache size =
    associativity =
*/

#include <stdlib.h>
#include <stdio.h>

#include "mystery-cache.h"

/*
 * NOTE: When using access_cache() you do not need to provide a "real"
 * memory addresses. You can use any convenient integer value as a
 * memory address, you should not be able to cause a segmentation
 * fault by providing a memory address out of your programs address
 * space as the argument to access_cache.
 */

/*
   Returns the size (in B) of each block in the cache.
*/
int get_block_size(void) {
  /* YOUR CODE GOES HERE */

  int addr = 0;

  flush_cache();

  access_cache(addr);
  while (access_cache(addr)) {
    addr++;
  }

  return addr;
}

/*
   Returns the size (in B) of the cache.
*/
int get_cache_size(int bsize) {
  /* YOUR CODE GOES HERE */

  int addr = 0;
  int res = 0;
  int i = 0;

  while (1) {
    flush_cache();
    for (i = 0; i <= addr; i += bsize) {
      access_cache(i);
    }

    res = access_cache(0);

    if (res == FALSE) {
      break;
    }

    addr += bsize;
  }

  return addr;
}

/*
   Returns the associativity of the cache.
*/
int get_cache_assoc(int csize) {
  /* YOUR CODE GOES HERE */

  int n = 1;
  int i = 0;
  int res = 0;

  while (1) {
    flush_cache();
    for (i = 0; i <= n; i++) {
      access_cache(i*csize);
    }

    res = access_cache(0);
    if (res == FALSE) {
      break;
    }

    n++;
  }

  return n;
}

//// DO NOT CHANGE ANYTHING BELOW THIS POINT
int main(void) {
  int size;
  int assoc;
  int block_size;

  /* The cache needs to be initialized, but the parameters will be
     ignored by the mystery caches, as they are hard coded.  You can
     test your geometry paramter discovery routines by calling
     cache_init() w/ your own size and block size values. */
  cache_init(0,0);

  block_size=get_block_size();
  size=get_cache_size(block_size);
  assoc=get_cache_assoc(size);

  printf("Cache block size: %d bytes\n", block_size);
  printf("Cache size: %d bytes\n", size);
  printf("Cache associativity: %d\n", assoc);

  return EXIT_SUCCESS;
}
