/* koines synarthseis */
#ifndef FUNC_H
#define FUNC_H

#include "patientRecord.h"
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include "stdbool.h"
#include "HashTable.h"
#include "Heap.h"
#include "HashBuck.h"

typedef struct sum{
    char* date;
    char* country;
    char* disease;
    int array[4];
}sumInfo;

char** ChronOrder(char** array, int N);
char* trimDate(char* date);
int DateCmp(char*, char*);
void DateConversion(char*, int*, int*, int*);
int HashFunction(char *, int);
void giveCommand(HTMain **, HashBuck**,char**  ,int);
void upgradeAges(int[4], int );
void SummaryStatistics(char* , FILE* , char* , char**, int );
void diseaseFrequency(HashBuck** , char* , char* , char* , char* , char**);
void topk(HashBuck **, int  ,char*, char* , char* , char* );
void FindTopk(HeapNode *, int, char**);
void searchPatientRecord(HTMain **table, char* recordID, char**);
void numPatientDischarges(HashBuck **, char* , char* , char* , char*, char**,int,char** );
void topkAge(HashBuck** table, char *k, char*country, char*disease, char*date1, char*date2, char**);
void numPatientAdmissions(HashBuck **, char* , char* , char* , char* , char**,int, char**);

#endif /* FUNC_H */
