#ifndef HASHTABLE_H
#define HASHTABLE_H
#include "patientRecord.h"
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include "HashBuck.h"
#include "stdbool.h"

typedef struct HashNode {
    int size;
    patientRecord record;
    struct HashNode *next;
}HTMain;

void HTCreate(HTMain**, int);
int HTInsert(HTMain **, patientRecord);
void HTInsertToHashBuck(HTMain **, HashBuck **);
void HTDestroy(HTMain **);
void HTPrint(HTMain **, int);
void HTUpdate(HTMain *, patientRecord );
int PrintPatient(patientRecord );

#endif /* HASHTABLE_H */

