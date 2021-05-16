#include "HashTable.h"
#include "func.h"

void HTCreate(HTMain **hash, int size) {
    int i;
    (*hash)->size = size;
    for (i=0;i<size;i++) {
        hash[i]->next = NULL;
    }
    //printf("Creating a Hash Table with size [%d] \n", (*hash)->size);
}

void HTPrint(HTMain **hashtable, int pid) {
    HTMain* curr = NULL;
    int i, count = 0;
    for (i = 0; i < (*hashtable)->size; i++) {
        printf("For Hash : [ %d ] \n ", i);
        curr = hashtable[i];
        while (curr->next != NULL) {
            if (PrintPatient(curr->record) != 1)
                count++;
            curr = curr->next;
        }
    }
    printf("I finally printed %d patient's records :) \n", count);
}

void HTInsertToHashBuck(HTMain **hashtable, HashBuck **disease) {
    HTMain* curr = NULL;
    int i, count = 0;
    for (i = 0; i < (*hashtable)->size; i++) {
        curr = hashtable[i];
        while (curr->next != NULL) {
            if (strcmp(curr->record.recordID, "-404") != 0) {
                HashInsert(disease, &(curr->record), "disease");
                count++;
            }
            curr = curr->next;
        }
    }
    //printf("I finally moved %d patient's records :) \n", count);
}

int PrintPatient(patientRecord pr) {
    if (strcmp(pr.recordID, "-404") == 0) {
        return 1;
    }
    printf("%s %s %s %s %s %s %s %s %s\n", pr.recordID, pr.state, pr.patientFirstName, pr.patientLastName, pr.country, pr.diseaseID, pr.entryDate, pr.exitDate, pr.age);

    return 0;
}

void HTUpdate(HTMain *node, patientRecord pr) {
    //counter++;
    strcpy(node->record.recordID, pr.recordID);
    strcpy(node->record.country, pr.country);
    strcpy(node->record.diseaseID, pr.diseaseID);
    strcpy(node->record.entryDate, pr.entryDate);
    strcpy(node->record.exitDate, pr.exitDate);
    strcpy(node->record.patientFirstName, pr.patientFirstName);
    strcpy(node->record.patientLastName, pr.patientLastName);
    strcpy(node->record.age, pr.age);
    strcpy(node->record.state, pr.state);
    // printf("will update this patient [  %s  ] records \n", node->record.recordID);
}

int HTInsert(HTMain **hashtable, patientRecord pr) {
    char rID[20];
    strcpy(rID, pr.recordID);
    int hSize = (*hashtable)->size;
    int HashID = HashFunction(rID, hSize);
    //printf("For patient with record ID = %s the hash is %d \n", rID, HashID);
    HTMain * node;
    node = hashtable[HashID];
    while (node->next != NULL) {
        if (strcmp(rID, node->record.recordID) == 0) {
            //duplicate
            if (strcmp(node->record.state, "ENTER") == 0 && strcmp(pr.state, "ENTER") == 0){
                //duplicate record id
                printf("ERROR %s \n", pr.recordID);
                return 1;
            }
            else if(strcmp(node->record.state, "ENTER/EXIT") == 0 && strcmp(pr.state, "EXIT") == 0){
                printf("ERROR %s \n", pr.recordID);
                return 1;
            }
            else if (strcmp(node->record.state, "ENTER") == 0 && strcmp(pr.state, "EXIT") == 0){
                strcpy(node->record.exitDate, pr.exitDate);
                strcpy(node->record.state, "ENTER/EXIT");
                return 0;
            }
        }
        node = node->next;
    }
    if (strcmp(pr.state, "EXIT") == 0) {
        printf("ERROR %s\n", pr.recordID);
        return 1;
    }
    HTUpdate(node, pr);
    node->next = malloc(sizeof (HTMain));
    node->next->next = NULL;
    return 0;
}

void HTDestroy(HTMain **hashtable) {
    int i;
    HTMain *curr = NULL;
    HTMain *next = NULL;
    int size = (*hashtable)->size;
    for (i = 0; i < size; i++) {
        curr = hashtable[i];
        while (curr->next != NULL) {
            next = curr->next;
            free(curr);
            curr = next;
        }
    }
    free(hashtable);
    //printf("A hash table was just destroyed ! \n");
}
