#ifndef MIPC_COMMON_H__
#define MIPC_COMMON_H__

#include <stdint.h>


#define SHM_NAME "/mipc-shm"
#define SHM_SIZE 4096

#define MQ_NAME "/mipc-mq"
#define BUF_SIZE 8192

#define SEM_NAME "/mipc-sem"

#define CMD_ADD_KEY 0
#define CMD_REMOVE_KEY 1
#define CMD_CLEAR_KEY 2
#define CMD_SLEEP_KEY 3
#define CMD_PRINT_KEY 4
#define CMD_EXIT_KEY 5

struct mipc_msg_s {
    int key;
    int value;
} __attribute__((packed));

typedef struct mipc_msg_s mipc_msg_t;

struct mipc_node_s {
    int value;
    int16_t left;
    int16_t right;
} __attribute__((packed));

typedef struct mipc_node_s mipc_node_t;

struct mipc_bstree_s {
    int count;
    int16_t root;
    int16_t dummy;
} __attribute__((packed));

typedef struct mipc_bstree_s mipc_bstree_t;

#define MAX_NODES (SHM_SIZE / sizeof(mipc_node_t))

#define NODE_NONE -1

#endif


