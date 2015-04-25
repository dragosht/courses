#ifndef QUEUE_H__
#define QUEUE_H__

struct gm_queue_node_s {
    int value;
    struct gm_queue_node_s *next;
    struct gm_queue_node_s *prev;
};

typedef struct gm_queue_node_s gm_queue_node_t;

struct gm_queue_s {
    gm_queue_node_t *front;
    gm_queue_node_t *back;
    int size;
};

//That was me thinking that I would actually need a queue ...

typedef struct gm_queue_s gm_queue_t;

void queue_init(gm_queue_t* q);

void queue_push(gm_queue_t* q, int value);

void queue_pop(gm_queue_t* q);

int queue_front(gm_queue_t* q);

void queue_destroy(gm_queue_t* q);

int queue_empty(gm_queue_t* q);

#endif
