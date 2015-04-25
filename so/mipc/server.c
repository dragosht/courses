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

void mipc_tree_dump(char* mem)
{
    mipc_node_t* nodes =  NULL;
    mipc_bstree_t* tree = NULL;
    int i;

    assert(mem != NULL);
    tree = (mipc_bstree_t*) mem;
    nodes = (mipc_node_t*)(mem + sizeof(mipc_bstree_t));

    printf("Dumping tree:\n");
    printf("Node count: %d\n", tree->count);
    printf("Root node: %d\n", tree->root);

    for (i = 0; i < tree->count; i++) {
        printf("Node: %d: value: %d\n", i, nodes[i].value);
        printf("          left: %d\n", nodes[i].left);
        printf("          right: %d\n", nodes[i].right);
    }

}

void mipc_tree_init(char* mem)
{
    mipc_bstree_t* tree = NULL;

    assert(mem != NULL);
    memset(mem, 0, SHM_SIZE);
    tree = (mipc_bstree_t*) mem;
    tree->count = 0;
    tree->root = 0;
}


void mipc_tree_add(char* mem, int value)
{
    mipc_node_t* nodes =  NULL;
    mipc_node_t* parent = NULL;
    mipc_bstree_t* tree = NULL;
    int count;
    int root;

    printf("server: adding %d\n", value);

    assert(mem != NULL);
    tree = (mipc_bstree_t*) mem;
    nodes = (mipc_node_t*)(mem + sizeof(mipc_bstree_t));

    count = tree->count;
    root = tree->root;

    /* Make sure we don't get out of the mapped shared memory space */
    assert(count < MAX_NODES);

    nodes[count].value = value;
    nodes[count].left = NODE_NONE;
    nodes[count].right = NODE_NONE;
    tree->count++;

    if (count == 0) {
        return;
    }

    /* Now see where this should get linked */
    parent = &nodes[root];

    while (1) {
        if (value > parent->value) {
            if (parent->right != NODE_NONE) {
                parent = &nodes[parent->right];
            } else {
                parent->right = count;
                break;
            }
        } else {
            if (parent->left != NODE_NONE) {
                parent = &nodes[parent->left];
            } else {
                parent->left = count;
                break;
            }
        }
    }
}


int16_t mipc_tree_find(char* mem, int value, int16_t* parent)
{
    mipc_bstree_t* tree = NULL;
    mipc_node_t* nodes = NULL;
    int16_t idx = 0;
    int16_t pidx = 0;

    assert(mem != NULL);
    tree = (mipc_bstree_t*) mem;
    nodes = (mipc_node_t*)(mem + sizeof(mipc_bstree_t));

    if (tree->count == 0) {
        return NODE_NONE;
    }

    pidx = NODE_NONE;
    idx = tree->root;

    while (idx != NODE_NONE) {
        if (value == nodes[idx].value) {
            break;
        }

        pidx = idx;
        if (value > nodes[idx].value) {
            idx = nodes[idx].right;
        } else {
            idx = nodes[idx].left;
        }
    }

    if (parent != NULL) {
        *parent = pidx;
    }

    return idx;
}


int16_t mipc_tree_successor(char* mem, int16_t idx, int16_t* parent)
{
    mipc_bstree_t* tree = NULL;
    mipc_node_t* nodes = NULL;
    int16_t sidx = NODE_NONE;
    int16_t psidx = NODE_NONE;

    assert(mem != NULL);
    tree = (mipc_bstree_t*) mem;
    nodes = (mipc_node_t*)(mem + sizeof(mipc_bstree_t));

    assert(tree->count > idx && idx >= 0);

    if (nodes[idx].left != NODE_NONE) {
        sidx = nodes[idx].left;
        psidx = idx;

        while (nodes[sidx].right != NODE_NONE) {
            psidx = sidx;
            sidx = nodes[sidx].right;
        }
    } else if (nodes[idx].right != NODE_NONE) {
        sidx = nodes[idx].right;
        psidx = idx;
    }

    if (parent != NULL) {
        *parent = psidx;
    }

    return sidx;
}

void mipc_tree_remove(char* mem, int value)
{
    mipc_bstree_t* tree = NULL;
    mipc_node_t* nodes = NULL;
    int16_t idx = 0;
    int16_t pidx = 0;
    int16_t sidx = 0;
    int16_t psidx = 0;
    int i = 0;
    int last = 0;

    assert(mem != NULL);
    tree = (mipc_bstree_t*) mem;
    nodes = (mipc_node_t*)(mem + sizeof(mipc_bstree_t));

    if (tree->count == 0) {
        return;
    }

    idx = mipc_tree_find(mem, value, &pidx);
    if (idx == NODE_NONE) {
        return;
    }

    mipc_tree_dump(mem);

    sidx = mipc_tree_successor(mem, idx, &psidx);

    if (pidx != NODE_NONE) {
        if (nodes[pidx].left == idx) {
            nodes[pidx].left = sidx;
        } else if (nodes[pidx].right == idx) {
            nodes[pidx].right = sidx;
        }
    } else {
        tree->root = sidx;
    }

    if (psidx != NODE_NONE && psidx != idx) {
        if (nodes[psidx].right == sidx) {
            nodes[psidx].right = nodes[sidx].left;
        }
    }

    if (sidx != NODE_NONE) {
        if (nodes[idx].left != sidx) {
            nodes[sidx].left = nodes[idx].left;
        }
        if (nodes[idx].right != sidx) {
            nodes[sidx].right = nodes[idx].right;
        }
    }

    //Defragment the array ...
    last = tree->count - 1;
    if (idx != last) {
        nodes[idx] = nodes[last];
        for (i = 0; i < last; i++) {
            if (nodes[i].left == last) {
                nodes[i].left = idx;
            }
            if (nodes[i].right == last) {
                nodes[i].right = idx;
            }
        }
        if (tree->root == last) {
            tree->root = idx;
        }
    }

    tree->count--;
}

int main(int argc, char *argv[])
{
    unsigned int prio = 10;
    int shmfd;
    void *mem;
    mqd_t mqueue;
    mipc_msg_t msg;
    sem_t* sem;
    pid_t pid;
    pid_t sid;
    int value;

    /*
    pid = fork();
    if (pid < 0) {
        perror("fork()");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    sid = setsid();
    if (sid < 0) {
        perror("setsid()");
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    */

    shmfd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0644);
    ftruncate(shmfd, SHM_SIZE);
    mem = mmap(0, SHM_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shmfd, 0);
    sem = sem_open(SEM_NAME, O_CREAT, 0644, 1);
    mqueue = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0646, NULL);

    //sem_getvalue(sem, &value);
    //printf("sem value: %d\n", value);
    //if (value == 0) {
    //    sem_post(sem);
    //}

    mipc_tree_init(mem);

    while (1) {
        mq_receive(mqueue, (char*)&msg, BUF_SIZE, &prio);
        //printf("server: received: %d %d\n", msg.key, msg.value);

        switch(msg.key) {
            case CMD_ADD_KEY:
                sem_wait(sem);
                mipc_tree_add(mem, msg.value);
                sem_post(sem);
                break;
            case CMD_REMOVE_KEY:
                sem_wait(sem);
                mipc_tree_remove(mem, msg.value);
                sem_post(sem);
                break;
            case CMD_CLEAR_KEY:
                sem_wait(sem);
                mipc_tree_init(mem);
                sem_post(sem);
                break;
            case CMD_EXIT_KEY:
                goto exit_label;
                break;
            default:
                break;
        }
    }

exit_label:
    sem_close(sem);
    sem_unlink(SEM_NAME);
    mq_close(mqueue);
    mq_unlink(MQ_NAME);
    munmap(mem, SHM_SIZE);
    close(shmfd);
    shm_unlink(SHM_NAME);

    return 0;
}

