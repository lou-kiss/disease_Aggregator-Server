#ifndef BT_H
#define BT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "patientRecord.h"


typedef struct BTNodeStruct {
    struct BTNodeStruct * left;
    struct BTNodeStruct *right;
    char Date[20];
    patientRecord *pRecord;
} BTNodeStruct;

typedef struct BTNodeStruct * BTNode;

typedef struct BTTree {
    BTNode ptr;
    int nodes;
}BTTree;

BTTree *BTCreate( );
int BTHeight(BTTree * );
int maxDepth(BTNode );
int BTSize(BTTree* );
BTNode BTInsert(BTTree*, BTNode, patientRecord*);
char* BTGetDate(BTTree *, BTNode);
void printGivenLevel(BTTree* , BTNode , int  );
void BTLevelOrder(BTTree *  );
void DestroyNode( BTTree *, BTNode);
void BTDestroy(BTTree *);
void printNode(BTTree *, BTNode) ;
void BTSearchDatesKey(BTNode , char* , char*, int*, char*);
void BTSearchDatesExit(BTNode node, char* date1, char*date2, int*count, char* key);        
void BTSearchDatesAge(BTNode , char* , char*, int*,int*, char*);
void KeyCountNode(BTTree * , BTNode , char*, int );
void KeyCountGivenLevel(BTTree* , BTNode , int , char* , int );
int BTKeyCount(BTTree *, char* , int );

#endif /* BT_H */
