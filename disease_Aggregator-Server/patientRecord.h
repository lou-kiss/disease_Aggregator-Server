#ifndef PATIENTRECORD_H
#define PATIENTRECORD_H

typedef struct patient{
    char recordID[20];
    char patientFirstName[20];
    char patientLastName[20];
    char diseaseID[20];
    char country[20];
    char entryDate[20];
    char exitDate[20];
    char age[4];
    char state[20];
} patientRecord;

#endif /* PATIENTRECORD_H */

