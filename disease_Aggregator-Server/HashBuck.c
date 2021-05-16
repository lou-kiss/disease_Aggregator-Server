#include "HashBuck.h"
#include "func.h"

static int entriesPerBucket;

void HashCreate(HashBuck **hash, int size, int bucketSize){
    int i;
    (*hash)->size = size;
    (*hash)->bucketSize = bucketSize;
    (*hash)->next = NULL;

    for (i=0;i<size;i++) {
        hash[i]->next = NULL;
    }
    entriesPerBucket = entriesPerBucketCalc(hash);
    if (entriesPerBucket == -1) exit(-1);
    //printf("Creating a HashTable with size [%d] and epb [%d] \n", (*hash)->size, entriesPerBucket);
}

 int entriesPerBucketCalc(HashBuck **hashtable){
    /*calculate how many bucketEntries a bucket can fit*/
    int entrySpace = ((*hashtable)->bucketSize)- (sizeof((*hashtable)->bucketSize)+sizeof((*hashtable)->next)+sizeof((*hashtable)->size));
    double entriesPerBucket = (double)entrySpace / (double)(sizeof(bucketEntry));
    /*if entriesPerBucket < 1 return -1 exit the app*/
    if (entriesPerBucket < 1){
        printf("For the given bucketSize [  %d  ], no bucketEntry can fit in. Exiting the app. Give a proper bucketSize !\n", (*hashtable)->bucketSize);
        return -1;
    }
    int epb = (int)entriesPerBucket; /*int part of entriesPerBucket*/
    return epb;
}

 bucketEntry* HashBucketMemUpdate(bucketEntry *bucket){
    bucket = malloc(entriesPerBucket*sizeof(bucketEntry));
    for (int j = 0; j < entriesPerBucket; j++){
        bucket[j].key = NULL;
        bucket[j].tree = NULL;
    }
    return bucket;
 }

int HashInsert(HashBuck **hashtable, patientRecord *record, char key[20]){
    int ok = 0;
    char keyID[20];
    if (strcmp(key, "country") == 0){
        strcpy(keyID, record->country);
    }else if (strcmp(key, "disease") == 0)    {
        strcpy(keyID, record->diseaseID);
    }
    else {
        printf("Give the right keyword to make the hash table ! \n");
        return -1;
    }
    //printf("keyID : %s\n",keyID);
    int hSize = (*hashtable)->size;
    int hashID = HashFunction(keyID, hSize);

    HashBuck *node ;
    node = hashtable[hashID];
    if (HashSearch(node, keyID, record) == 1){
        /*there was already a record in hashtable[hashID], so I updated it's binary tree*/
        /*nothing else to do here*/
        return 0;
    }
    int i = 0, full = 0;

    while(node->next != NULL){
        node = node->next;
    }

    while(i < entriesPerBucket){
        if (node->bucket[i].key != NULL  && ok == 0){
            full++;
        }
        if (full == entriesPerBucket ){
            node->next = malloc(sizeof(HashBuck));
            node->next->bucket = HashBucketMemUpdate(node->next->bucket);
            node = node->next;
            ok = 1;
            full = 0;
        }
        i++;
    }
    ok = 0;
    for(i = 0; i < entriesPerBucket; i++){
        if (node->bucket[i].key == NULL && ok == 0){
            /*insert*/
            //printf("-->Hashtable[%d] : Lets insert %s at bucket[%d] \n",hashID, keyID, i);
            node->bucket[i].key = strdup(keyID);
            /*at the first time that a record arrives at the hash table we are making a tree*/
            node->bucket[i].tree = BTCreate();
            node->bucket[i].tree->ptr = BTInsert(node->bucket[i].tree, node->bucket[i].tree->ptr, record);
            ok = 1;
        }
    }
    return 0;
}


int HashSearch(HashBuck *node, char *keyID, patientRecord *record){
    while(node != NULL){
        for(int i = 0; i < entriesPerBucket; i++){
            if (node->bucket[i].key != NULL && strcmp(node->bucket[i].key, keyID) == 0){
                /*key (country or disease) exist */
                if (node->bucket[i].tree == NULL){
                    printf("Tree is null\n");
                }
                else if (node->bucket[i].tree->ptr == NULL){
                    printf("tree ptr is NULL \n");
                }
                //printf("Updated the tree at bucket[%d] with key [%s] \n", i, keyID);
                node->bucket[i].tree->ptr = BTInsert(node->bucket[i].tree, node->bucket[i].tree->ptr, record);
                return 1;
            }
        }
        node = node->next;
    }

    return 0;
}

void PrintHash(HashBuck **hash){
    for (int i = 0; i < (*hash)->size; i++){
        printf("\n----------HashTable[%d]---------- \n", i);
        PrintBucket(hash[i]);
    }

}

void PrintBucket(HashBuck *node){
    while(node != NULL){
        for (int i = 0; i < entriesPerBucket; i++){
            if (node->bucket[i].key != NULL){
               printf("I. bucket[%d] key is : %s \n", i,node->bucket[i].key);
               printf("II. bucket[%d] tree in LevelOrder is : ",i);
               BTLevelOrder(node->bucket[i].tree);
               printf("\n");
            }
        }
        node = node->next;
    }
}

void HashDestroy(HashBuck **hashtable){
    int i ,j;
    int entriesPerBucket = entriesPerBucketCalc(hashtable);
    HashBuck *curr = NULL;
    HashBuck *next = NULL;
    int size = (*hashtable)->size;
    for(i = 0; i < size; i++){
	curr = hashtable[i];
	while (curr != NULL){
            next = curr->next;
            for( j = 0; j < entriesPerBucket; j++){
                if(curr->bucket[j].key != NULL){
                    free(curr->bucket[j].key);
                    curr->bucket[j].key = NULL;
                    BTDestroy(curr->bucket[j].tree);
                    free(curr->bucket[j].tree);
                    curr->bucket[j].tree = NULL;
                }
            }
            free(curr->bucket);
            curr->bucket = NULL;
            free(curr);
            curr = next;
	}
    }
    free(hashtable);
    //printf("A hash table was just destroyed ! \n");
}
