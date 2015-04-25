#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "hashtable.h"

#define BUCKET_ALL (-1)
#define MAXLINE 15000

static char line[MAXLINE];

static const char *separators = " \n\t";

static const char *cmd_add = "add";
static const char *cmd_remove = "remove";
static const char *cmd_clear = "clear";
static const char *cmd_find = "find";
static const char *cmd_print_bucket = "print_bucket";
static const char *cmd_print = "print";
static const char *cmd_resize = "resize";

static const char* param_double = "double";
static const char* param_halve = "halve";

static void hashtable_print(hashtable_t* table, FILE* file, int bucket);

static void run_commands(hashtable_t** table);



//#define HTABLE_DEBUG

#ifdef HTABLE_DEBUG
    #define HTABLE_PRINT printf
#else
    #define HTABLE_PRINT
#endif


int main(int argc, char *argv[])
{
    hashtable_t* table = NULL;
    int hsize = 0;
    int i;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <size> <file1> <file2> ...\n", argv[0]);
        return -1;
    }

    hsize = atoi(argv[1]);

    hashtable_init(&table, hsize);

    if (argc > 2) {
        for (i = 2; i < argc; i++) {
            freopen(argv[i], "r", stdin);
            run_commands(&table);
        }
    } else {
        run_commands(&table);
    }

    hashtable_destroy(&table);

    return 0;
}


static void run_commands(hashtable_t** ptable)
{

    for (; fgets(line, MAXLINE, stdin); ) {
        char* token = strtok(line, separators);
        if (token == NULL) {
            continue;
        }

        if (!strcmp(token, cmd_add)) {
            token = strtok(NULL, separators);
            hashtable_insert(*ptable, token);
        }
        else if (!strcmp(token, cmd_remove)) {
            token = strtok(NULL, separators);
            hashtable_remove(*ptable, token);
        }
        else if (!strcmp(token, cmd_clear)) {
            hashtable_clear(*ptable);
        }
        else if (!strcmp(token, cmd_find)) {
            hashnode_t* node = NULL;
            FILE* file = stdout;

            token = strtok(NULL, separators);
            node = hashtable_find(*ptable, token);

            token = strtok(NULL, separators);
            if (token != NULL) {
                file = fopen(token, "a");
            }

            if (node != NULL) {
                fprintf(file, "True\n");
            } else {
                fprintf(file, "False\n");
            }

            if (file != stdout) {
                fclose(file);
            }
        }
        else if (!strcmp(token, cmd_print_bucket)) {
            int bucket = 0;
            FILE* file = stdout;

            token = strtok(NULL, separators);
            bucket = atoi(token);

            token = strtok(NULL, separators);
            if (token != NULL) {
                file = fopen(token, "a");
            }

            hashtable_print(*ptable, file, bucket);

            if (file != stdout) {
                fclose(file);
            }

        }
        else if (!strcmp(token, cmd_print)) {
            FILE* file = stdout;

            token = strtok(NULL, separators);
            if (token != NULL) {
                file = fopen(token, "a");
            }

            hashtable_print(*ptable, file, BUCKET_ALL);

            if (file != stdout) {
                fclose(file);
            }
        }
        else if (!strcmp(token, cmd_resize)) {
            token = strtok(NULL, separators);

            if (!strcmp(token, param_double)) {
                hashtable_resize(ptable, (*ptable)->size * 2);
            } else if (!strcmp(token, param_halve)) {
                hashtable_resize(ptable, (*ptable)->size / 2);
            } else {
                assert(0);
            }
         }
        else {
            continue;
        }

    }

}


void hashtable_print(hashtable_t* table, FILE* file, int bucket)
{
    int i;

    assert(table != NULL);

    if (bucket == BUCKET_ALL) {
        for (i = 0; i < table->size; i++) {
            hashnode_t* crt = table->buckets[i];
            for (; crt != NULL; crt = crt->next) {
                fprintf(file, "%s ", crt->str);
            }
            if (table->buckets[i] != NULL) {
                fprintf(file, "\n");
            }
        }
    } else {
        hashnode_t* node = table->buckets[bucket];
        int empty = (node == NULL);
        while (node != NULL) {
            fprintf(file, "%s ", node->str);
            node = node->next;
        }
        if (!empty) {
            fprintf(file, "\n");
        }
    }
}

