#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "queue.h"

void queue_init(gm_queue_t* q)
{
    assert(q != NULL);
    q->size = 0;
    q->front = NULL;
    q->back = NULL;
}


void queue_push(gm_queue_t* q, int value)
{
    gm_queue_node_t* node = NULL;
    assert(q != NULL);

    node = malloc(sizeof(gm_queue_node_t));
    node->value = value;
    node->next = NULL;
    node->prev = NULL;

    if (q->size == 0) {
        q->front = q->back = node;
    } else {
        node->prev = q->back;
        q->back->next = node;
        q->back = node;
    }

    q->size++;
}


void queue_pop(gm_queue_t* q)
{
    gm_queue_node_t* node = NULL;
    assert(q != NULL);

    if (q->size == 0) {
        return;
    }

    node = q->front;
    if (q->size == 1) {
        q->front = q->back = NULL;
    } else {
        q->front = node->next;
        q->front->prev = NULL;
    }

    free(node);
}


int queue_front(gm_queue_t* q)
{
    assert(q != NULL);
    assert(q->size > 0);
    return q->front->value;
}


void queue_destroy(gm_queue_t* q)
{
    gm_queue_node_t* node = NULL;
    assert(q != NULL);
    if (q->size == 0) {
        return;
    }

    node = q->front;

    while (node != NULL) {
        q->front = node->next;
        free(node);
        node = q->front;
    }

    q->back = NULL;
    q->size = 0;
}


int queue_empty(gm_queue_t* q)
{
    assert(q != NULL);
    return (q->size == 0);
}


