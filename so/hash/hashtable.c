#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "hash.h"

static void hashnode_remove(hashnode_t* node);



/* Creates empty hashtable */
void hashtable_init(hashtable_t** table, int size)
{
    int i;
    assert(table != 0 && size > 0);
    *table = (hashtable_t*)malloc(sizeof(hashtable_t));
    (*table)->buckets = (hashnode_t**)malloc(size*sizeof(hashnode_t*));
    for (i = 0; i < size; i++) {
        (*table)->buckets[i] = NULL;
    }
    (*table)->size = size;
}


void hashtable_insert(hashtable_t* table, const char* str)
{
    unsigned int hval = hash(str, table->size);
    hashnode_t* node = NULL;
    hashnode_t* last = NULL;

    assert(table != NULL && str != NULL);

    node = table->buckets[hval];
    for (; node != NULL; node = node->next) {
        if (!strcmp(node->str, str)) {
            //already here - no need to insert it
            return;
        }
        last = node;
    }

    node = (hashnode_t*)malloc(sizeof(hashnode_t));
    assert(node != NULL);
    node->str = strdup(str);
    node->next = NULL;

    if (last != NULL) {
        last->next = node;
    } else {
        table->buckets[hval] = node;
    }
}


void hashtable_remove(hashtable_t* table, const char* str)
{
    unsigned int hval;

    assert(table != NULL && str != NULL);

    hval = hash(str, table->size);

    hashnode_t* first = table->buckets[hval];
    hashnode_t* prev = NULL;

    while (first != NULL) {
        if (!strcmp(first->str, str)) {
            //found it - wipe it out
            if (prev != NULL) {
                prev->next = first->next;
            } else {
                table->buckets[hval] = first->next;
            }
            hashnode_remove(first);
            first = NULL;
            return;
        }

        prev = first;
        first = first->next;
    }
}


void hashtable_resize(hashtable_t** table, int size)
{
    hashtable_t* newtable = NULL;
    int i;

    assert(table != NULL && size > 0);

    hashtable_init(&newtable, size);

    for (i = 0; i < (*table)->size; i++) {
        hashnode_t* crt = (*table)->buckets[i];
        for (; crt != NULL; crt = crt->next) {
            hashtable_insert(newtable, crt->str);
        }
    }

    hashtable_destroy(table);

    *table = newtable;
}


hashnode_t* hashtable_find(hashtable_t* table, const char* str)
{
    int i;
    assert(table != NULL && str != NULL);

    for (i = 0; i < table->size; i++) {
        hashnode_t* crt = table->buckets[i];

        for (; crt != NULL; crt = crt->next) {
            if (!strcmp(str, crt->str)) {
                return crt;
            }
        }
    }

    return NULL;
}


void hashtable_clear(hashtable_t* table)
{
    int i;

    if (table == NULL) {
        return;
    }

    for (i = 0; i < table->size; i++) {
        hashnode_t* crt = table->buckets[i];
        hashnode_t* prev = crt;

        while (crt != NULL) {
            prev = crt;
            crt = crt->next;
            hashnode_remove(prev);
        }
        table->buckets[i] = NULL;
    }
}


void hashtable_destroy(hashtable_t** table) {
    hashtable_clear(*table);
    free((*table)->buckets);
    free(*table);
    *table = NULL;
}

static void hashnode_remove(hashnode_t* node)
{
    if (node != NULL) {
        free(node->str);
        free(node);
    }
}



