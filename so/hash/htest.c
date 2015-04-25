#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hashtable.h"
#include "hash.h"

int main(int argc, char* argv[])
{
    hashtable_t* table = NULL;
    hashnode_t* node = NULL;
    int i;
    int size;

    table = (hashtable_t*)malloc(sizeof(hashtable_t));
    assert(table != NULL);

    size = 50;
    hashtable_init(table, size);
    for (i = 0; i < size; i++) {
        assert(table->buckets[i] == NULL);
    }
    assert(table->size == 50);

    hashtable_insert(table, "abcd");
    node = hashtable_find(table, "abcd");
    assert(node != NULL);


    hashtable_insert(table, "bcde");
    node = hashtable_find(table, "bcde");
    assert(node != NULL);

    size = 80;
    hashtable_resize(&table, size);
    assert(table->size == size);

    node = hashtable_find(table, "abcd");
    assert(node != NULL);
    node = hashtable_find(table, "bcde");
    assert(node != NULL);

    hashtable_clear(table);
    free(table);

    return 0;
}

