#ifndef HEAP_H
#define HEAP_H
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include "stdbool.h"

typedef struct Info{
    char* key;
    int count;
}HeapInfo;

typedef struct Heap{
    HeapInfo info;
    struct Heap *left;
    struct Heap *right;
}HeapNode;

typedef struct Queue{
    int front, rear;
    int size;
    HeapNode **array;
}HeapQueue;

HeapQueue* QueueCreate(int size);
HeapNode* UpdateNode(HeapInfo heapInfo) ;
int IsEmpty(HeapQueue* queue);
int IsFull(HeapQueue* queue);
int HasOnlyOneItem(HeapQueue* queue);
void Enqueue(HeapNode *root, HeapQueue* queue);
HeapNode* GetFront(HeapQueue* queue) ;
HeapNode* Dequeue(HeapQueue* queue);
int HasBothChild(HeapNode* temp);
void HeapInsert(HeapNode **root, HeapInfo info, HeapQueue* queue);
void SwapNodesInfo(int *, int *, char **, char **);
void MaxHeapify(HeapNode *);
void printPostOrder(HeapNode* node);
HeapNode* FindDeepestRightLeaf(HeapNode*);
void DestroyHeap(HeapNode* node) ;

#endif /* HEAP_H */

