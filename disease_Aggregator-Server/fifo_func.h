#ifndef FIFO_FUNC_H
#define FIFO_FUNC_H

#include "libs.h"

int makeFifos( int i, int *writefd, int *readfd);
int openFifos(int bufferSize, char **toSentArray, int i, int *writefd, int *readfd);
int unlinkFifos(int numWorkers);

#endif /* FUNC_H */
