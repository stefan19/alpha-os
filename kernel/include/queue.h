#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stddef.h>

struct queue_item;

struct queue_item
{
    struct queue_item* next;
    uint32_t v;
};
typedef struct queue_item queue_item_t;

typedef struct
{
    queue_item_t* front;
    queue_item_t* last;
    size_t size;
} queue_t;

void queueInit(queue_t* q);
void queueAdd(queue_t* q, uint32_t v);
uint32_t queueRemove(queue_t* q);
uint32_t queueFront(queue_t* q);

#endif