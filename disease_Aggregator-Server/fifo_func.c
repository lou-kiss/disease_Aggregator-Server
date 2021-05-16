#include "fifo_func.h"

#define PERMS 0666

int makeFifos( int i, int *writefd, int *readfd) {
    //concatenate the name of every fifo
    char *fifo = malloc(sizeof (char*));
    strcpy(fifo, "fifo");
    char *temp = malloc(sizeof (char*));
    strcpy(temp, "fifo");
    strcat(fifo, "r"); //fiforXXXX
    strcat(temp, "w"); //fifowXXXX
    char num[12];
    sprintf(num, "%d", i);
    strcat(temp, num);
    strcat(fifo, num);

    //make the fifos-pipes
    if (mkfifo(fifo, PERMS) < 0 && errno != EEXIST) {
        perror("can't create fifo");
    }
    if (mkfifo(temp, PERMS) < 0 && errno != EEXIST) {
        unlink(fifo);
        perror("can't create fifo");
    }
	free(temp);
    free(fifo);
    return 0;
}

int openFifos(int bufferSize, char** toSentArray, int i, int *writefd, int *readfd) {
    char *fifo = malloc(sizeof (char*));
    strcpy(fifo, "fifo");
    char *temp = malloc(sizeof (char*));
    strcpy(temp, "fifo");
    strcat(fifo, "r"); //fiforXXXX
    strcat(temp, "w"); //fifowXXXX
    char num[12];
    sprintf(num, "%d", i);
    strcat(temp, num);
    strcat(fifo, num);

    if (((*readfd) = open(fifo, O_RDONLY)) < 0) {
        perror("main: cant open read fifo");
    }
    if (((*writefd) = open(temp, O_WRONLY)) < 0) {
        perror("main: can't open write fifo");
    }

    if (write(*writefd, toSentArray[i], bufferSize) < 0) {
        perror("write in openFifos");
        exit(1);
    }

	free(temp); free(fifo);
    return 0;
}

int unlinkFifos(int numWorkers) {
    int i;
    for (i = 0; i < numWorkers; i++) {
        char * fifo = malloc(40 * sizeof (char));
        char * temp = malloc(40 * sizeof (char));
        strcpy(fifo, "fifo");
        strcpy(temp, "fifo");

        strcat(fifo, "r"); //fiforXXXX
        strcat(temp, "w"); //fifowXXXX
        char num[12];
        sprintf(num, "%d", i);
        strcat(fifo, num);
        strcat(temp, num);
        if (unlink(fifo) < 0)
            //perror("Aggregator: can't unlink \n");
        if (unlink(temp) < 0)
            //perror("Aggregator: can't unlink \n");

        free(fifo);
        free(temp);
    }
    return 0;
}
