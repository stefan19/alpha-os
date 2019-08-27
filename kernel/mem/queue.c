#include "queue.h"
#include "kheap.h"

void queueInit(queue_t* q)
{
    q->front = NULL;
    q->last = NULL;
    q->size = 0;
}

void queueAdd(queue_t* q, uint32_t v)
{
    q->size++;

    queue_item_t* new = (queue_item_t*)kheapAlloc(sizeof(queue_item_t));
    new->v = v;
    new->next = NULL;

    if(q->front == NULL)
    {
        q->last = new;
        q->front = q->last;
    }
    else
    {
        q->last->next = new;
        q->last = new;
    }
    
}

uint32_t queueRemove(queue_t* q)
{
    if(q->size == 0)
        return 0;
    
    q->size--;
    uint32_t v = q->front->v;

    queue_item_t* tmp = q->front;
    q->front = q->front->next;
    kheapFree(tmp);

    return v;
}

uint32_t queueFront(queue_t* q)
{
    if(q->size == 0)
        return 0;

    return q->front->v;
}