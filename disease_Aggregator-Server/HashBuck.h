#ifndef HASHBUCK_H
#define HASHBUCK_H
#include "bt.h"

typedef struct Bucket{
    char* key;
    BTTree *tree;
}bucketEntry;
    
typedef struct Hash {
    int bucketSize;
    int size;
    bucketEntry *bucket;
    struct Hash *next;
}HashBuck;
    

int entriesPerBucketCalc(HashBuck **hashtable);
void HashCreate(HashBuck **hash, int size, int bucketSize);
void InsertAtBT(bucketEntry bucket ,char *dateIn);
int HashInsert(HashBuck **hash, patientRecord *record, char key[20]);
bucketEntry* HashBucketMemUpdate(bucketEntry *);
void PrintHash(HashBuck **hash);
void PrintBucket(HashBuck *node);
int HashSearch(HashBuck *node, char *keyID, patientRecord *record);
void HashDestroy(HashBuck**);


#endif /* HASHBUCK_H */

