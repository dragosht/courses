#ifndef HASHTABLE_H__
#define HASHTABLE_H__

#include <stdio.h>

struct hashnode_s {
    char* str;
    struct hashnode_s* next;
};

typedef struct hashnode_s hashnode_t;

struct hashtable_s {
    hashnode_t** buckets;
    int size;
};

typedef struct hashtable_s hashtable_t;

void hashtable_init(hashtable_t** table, int size);

void hashtable_insert(hashtable_t* table, const char* str);

void hashtable_remove(hashtable_t* table, const char* str);

void hashtable_resize(hashtable_t** table, int size);

hashnode_t* hashtable_find(hashtable_t* table, const char* str);

void hashtable_clear(hashtable_t* table);

void hashtable_destroy(hashtable_t** table);

#endif //HASHTABLE_H__

