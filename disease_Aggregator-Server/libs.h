#ifndef LIBS_H
#define LIBS_H

#include "patientRecord.h"
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "HashTable.h"
#include "HashBuck.h"
#include "func.h"
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <signal.h>
extern int errno;
#include <sys/select.h>
#include <stdbool.h>
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <netdb.h>	         /* gethostbyaddr */
#include <unistd.h>	         /* fork */
#include <stdlib.h>	         /* exit */
#include <ctype.h>	         /* toupper */
#include <err.h>

#endif
