#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <mqueue.h>
#include <semaphore.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "common.h"

static const char* cmd_add = "a";
static const char* cmd_remove = "r";
static const char* cmd_clear = "c";
static const char* cmd_sleep = "s";
static const char* cmd_print = "p";
static const char* cmd_exit = "e";


void mipc_print_tree(char* mem) {
    mipc_bstree_t* tree = NULL;
    mipc_node_t* nodes = NULL;
    int ndxs[MAX_NODES];
    int front = 0;
    int back = 0;
    int newback = 0;
    int count = 0;
    int root = 0;
    int i;

    assert(mem != NULL);
    tree = (mipc_bstree_t*) mem;
    nodes = (mipc_node_t*)(mem + sizeof(mipc_bstree_t));
    count = tree->count;
    root = tree->root;

    if (count == 0) {
        printf("*\n");
        return;
    }

    ndxs[0] = root;
    front = 0;
    back = 1;

    while (front < back) {
        newback = back;
        for (i = front; i < back; i++) {
            if (ndxs[i] == NODE_NONE) {
                printf("* ");
            } else {
                printf("%d ", nodes[ndxs[i]].value);
                assert(newback + 2 < MAX_NODES);
                ndxs[newback++] = nodes[ndxs[i]].left;
                ndxs[newback++] = nodes[ndxs[i]].right;
            }
        }
        printf("\n");

        front = back;
        back = newback;
    }
}



int main(int argc, char *argv[])
{
    int i;
    int val;
    int shmfd;
    void *mem;
    mipc_msg_t msg;
    mqd_t mqueue;
    sem_t* sem;

    shmfd = shm_open(SHM_NAME, O_RDONLY, 0644);
    mem = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shmfd, 0);
    assert(mem != MAP_FAILED);
    mqueue = mq_open(MQ_NAME, O_RDWR, 0644, NULL);
    sem = sem_open(SEM_NAME, 0);

    for (i = 1; i < argc; ) {
        if (!strcmp(argv[i], cmd_add)) {
            assert(i + 1 < argc);
            msg.key = CMD_ADD_KEY;
            msg.value = atoi(argv[i+1]);
            mq_send(mqueue, (char*) &msg, sizeof(mipc_msg_t), 10);
            i += 2;
        }
        else if (!strcmp(argv[i], cmd_remove)) {
            assert(i + 1 < argc);
            msg.key = CMD_REMOVE_KEY;
            msg.value = atoi(argv[i+1]);
            mq_send(mqueue, (char*) &msg, sizeof(mipc_msg_t), 10);
            i += 2;
        }
        else if (!strcmp(argv[i], cmd_clear)) {
            msg.key = CMD_CLEAR_KEY;
            mq_send(mqueue, (char*) &msg, sizeof(mipc_msg_t), 10);
            i++;
        }
        else if (!strcmp(argv[i], cmd_sleep)) {
            assert(i + 1 < argc);
            val = atoi(argv[i+1]);
            usleep(val);
            i += 2;
        }
        else if (!strcmp(argv[i], cmd_print)) {
            sem_wait(sem);
            mipc_print_tree(mem);
            sem_post(sem);
            i++;
        }
        else if(!strcmp(argv[i], cmd_exit)) {
            msg.key = CMD_EXIT_KEY;
            mq_send(mqueue, (char*) &msg, sizeof(mipc_msg_t), 0);
            i++; /* ? */
        }
        else {
            /* ? */
        }
    }

    sem_close(sem);
    mq_close(mqueue);
    munmap(mem, SHM_SIZE);
    close(shmfd);

    return 0;
}

