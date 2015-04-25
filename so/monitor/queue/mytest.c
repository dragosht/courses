#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "queue.h"

int main(int argc, char* argv[])
{
    gm_queue_t q;
    int i;

    queue_init(&q);
    assert(q.size == 0);
    assert(q.front == NULL);
    assert(q.back == NULL);
    assert(queue_empty(&q));

    queue_push(&q, 5);
    assert(q.front != NULL);
    assert(q.back != NULL);
    assert(q.front == q.back);
    assert(q.front->value == 5);
    assert(!queue_empty(&q));

    queue_push(&q, 7);
    assert(q.front->value == 5);
    assert(q.back->value == 7);
    assert(!queue_empty(&q));
    assert(queue_front(&q) == 5);

    queue_push(&q, 8);
    assert(q.front->value = 5);
    assert(q.front->next->value = 7);
    assert(q.back->value = 8);
    assert(q.back->prev->value == 7);
    assert(!queue_empty(&q));
    assert(queue_front(&q) == 5);

    queue_pop(&q);
    assert(!queue_empty(&q));
    assert(queue_front(&q) == 7);

    /* these are only for valgrind to crunch */
    for (i = 0; i < 100; i++) {
        queue_push(&q, i);
    }

    for (i = 0; i < 50; i++) {
        queue_pop(&q);
    }

    queue_destroy(&q);
    assert(q.size == 0);
    assert(q.front == NULL);
    assert(q.back == NULL);
    assert(queue_empty(&q));

    return 0;
}
