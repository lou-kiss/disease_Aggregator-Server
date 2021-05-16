#include "Heap.h"

HeapQueue* QueueCreate(int size) { 
    HeapQueue* queue = malloc(sizeof(HeapQueue)); 
    queue->front = queue->rear = -1; 
    queue->size = size; 
    queue->array = malloc(queue->size *sizeof(HeapNode*)); 
    int i; 
    for (i = 0; i < size; ++i) 
        queue->array[i] = NULL; 
    return queue; 
} 

HeapNode* UpdateNode(HeapInfo heapInfo) { 
    HeapNode* node = malloc(sizeof(HeapNode)); 
    node->info = heapInfo; 
    node->left =  NULL;
    node->right = NULL; 
    return node; 
} 
  
int IsEmpty(HeapQueue* queue) { 
    return queue->front == -1;
} 
  
int IsFull(HeapQueue* queue) {  
    return queue->rear == queue->size - 1;
} 
  
int HasOnlyOneItem(HeapQueue* queue) {  
    return queue->front == queue->rear; 
} 
  
void Enqueue(HeapNode *root, HeapQueue* queue) { 
    if (IsFull(queue)) 
        return; 
    queue->array[++queue->rear] = root; 
    if (IsEmpty(queue)) 
        ++queue->front; 
} 
  
HeapNode* Dequeue(HeapQueue* queue) { 
    if (IsEmpty(queue)) 
        return NULL;
    HeapNode* temp = queue->array[queue->front]; 
    if (HasOnlyOneItem(queue)) 
        queue->front = queue->rear = -1; 
    else
        ++queue->front; 
    
    return temp; 
} 
  
HeapNode* GetFront(HeapQueue* queue) {  
    return queue->array[queue->front]; 
} 
  

int HasBothChild(HeapNode* temp) { 
    return temp && temp->left && temp->right; 
} 

void HeapInsert(HeapNode **root, HeapInfo info, HeapQueue* queue){ 
    HeapNode *node = UpdateNode(info);
    if (*root == NULL) {
        *root = node; 
    }
    else{ 
        HeapNode* front = GetFront(queue);
        if (front->left == NULL) {
            front->left = node; 
        }
        else if (front->right == NULL) {
            front->right = node; 
        }
        if (HasBothChild(front)) {
            Dequeue(queue); 
        }
    } 
    Enqueue(node, queue); 
} 

int IsHeap(HeapNode *root){
    /*checks if a subtree is a heap*/
    if (root->left == NULL){
        if(root->right == NULL){
            return 1;
        }
        else{
            if (root->info.count >= root->right->info.count){
                return 1;
            }
            else{
                SwapNodesInfo(&(root->info.count), &(root->right->info.count), &(root->info.key), &(root->right->info.key));
            }
        }
    }
    else{
        if (root->right == NULL){
            if(root->info.count >= root->left->info.count){
                return 1;
            }
            else{
                SwapNodesInfo(&(root->info.count), &(root->left->info.count), &(root->info.key), &(root->left->info.key));
            }
        }
        else{
            if(root->info.count >= root->left->info.count){
                if(root->info.count >= root->right->info.count){
                    return 1;
                }
                else{
                    SwapNodesInfo(&(root->info.count), &(root->right->info.count), &(root->info.key), &(root->right->info.key));
                }
            }
            else{
                if(root->left->info.count >= root->right->info.count){
                    SwapNodesInfo(&(root->info.count), &(root->left->info.count), &(root->info.key), &(root->left->info.key));
                }
                else{
                    SwapNodesInfo(&(root->info.count), &(root->right->info.count), &(root->info.key), &(root->right->info.key));
                }
            }       
        }
    }
    return 0;
}

void SwapNodesInfo(int *count1, int *count2, char **key1, char **key2){
    int temp = *count1;
    *count1 = *count2;
    *count2 = temp;
    char *tempKey = *key1;
    *key1 = *key2;
    *key2 = tempKey;
}

void MaxHeapify(HeapNode *root){
    if (root == NULL){
        return ;
    }
    MaxHeapify(root->left);
    MaxHeapify(root->right);
    IsHeap(root);
}

HeapNode* FindDeepestRightLeaf(HeapNode* root) { 
    HeapQueue* queue = QueueCreate(20); 
    Enqueue(root, queue); 
    HeapNode* result = NULL; 
    
    while (!IsEmpty(queue)) { 
        HeapNode* temp = Dequeue(queue); 
        //printf("{%s %d } \n",temp->info.key,  temp->info.count); 
        if (temp->left) 
            Enqueue(temp->left, queue); 
        if (temp->right) {
            Enqueue(temp->right, queue); 
            if(!temp->right->left && !temp->right->right) 
                result = temp->right; 
        } 
    }
    return result;
} 

void DestroyHeap(HeapNode* node) { 
    if (node == NULL) return; 
    /* first delete both subtrees */
    DestroyHeap(node->left); 
    DestroyHeap(node->right); 
    /* then delete the node */
    //printf("\nDeleting node: %s %d \n", node->info.key, node->info.count); 
    free(node); 
}  