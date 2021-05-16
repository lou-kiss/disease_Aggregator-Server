#include "worker.h"
#define DISEASE_ENTRIES 10
#define BUCKET_SIZE 1000
#define SOCKET_SIZE 1024

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int workerFunc(int argc, char* argv[]) {
    //args
    int pid_counter = atoi(argv[1]);
    char*dir_name = argv[2];
    int bufferSize = atoi(argv[3]);
    int country_sum = atoi(argv[4]);
    int file_sum = 2*atoi(argv[5]);
    char*numWorkersStr = argv[6];
    char **country_array = malloc(country_sum * sizeof (char*)); //array of countries each worker has
    char *summary = calloc(SOCKET_SIZE, sizeof (char)); //summary statistics for each file -200 shouldnt be hardcoded
    int i, readfd, writefd, j;
    int serverPort ;
    int workerPort = 1030+pid_counter;
    char workerPortStr[10];
    sprintf(workerPortStr, "%d", workerPort);
    char*ip = calloc(50, sizeof(char));
    for (i = 0; i < country_sum; i++){
        country_array[i] = (char*) malloc(256*sizeof (char));
    }
    char buff[bufferSize];
    int records_per_country = 0;
    int nw, nr;
    char *fifo = malloc(sizeof (char*));
    strcpy(fifo, "fifo");
    char *temp = malloc(sizeof (char*));
    strcpy(temp, "fifo");
    strcat(fifo, "r"); //fiforXXXX
    strcat(temp, "w"); //fifowXXXX
    char pid[12];
    sprintf(pid, "%d", pid_counter);
    strcat(temp, pid);
    strcat(fifo, pid);

    if ((writefd = open(fifo, O_WRONLY)) < 0) {
        perror("worker:cant open write fifo \n");
    }
    if ((readfd = open(temp, O_RDONLY)) < 0) {
        perror("worker: can't open read fifo \n");
    }
    char*token;
    int c, counter = 0, country_count = 0;
    if ((nr = read(readfd, buff, bufferSize)) > 0) {
        printf("***I am worker %d %d, I opened pipe : %s and my countries are : ", pid_counter, getpid(), fifo);
        token = strtok(buff, "#");
        c = 0;
        counter = 0;
        while (token != NULL) {
            if (counter == 0){
                strcpy(ip, token);
                counter++;
                token = strtok(NULL, "#");
            }else if(counter == 1){
                serverPort = atoi(token);
                counter++;
                token = strtok(NULL, "#");
            }else{
                strcpy(country_array[c], token);
                printf("%s ", country_array[c]);
                token = strtok(NULL, "#");
                c++;
            }
        }
    } else {
        perror("Worker could not get countries ");
        exit(1);
    }
    int countries = c;
    printf("and my workerPort is :  %d ***\n", workerPort);
    //SOCKETS
    printf("----------------------------------------------------\n");
    int sock;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *hp; struct in_addr IP;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
    	perror_exit("socket");
    }
    /* Find server address */
    if (!inet_aton(ip, &IP)){
        errx(1, "can't parse IP address %s", ip);
    }
    if ((hp = gethostbyaddr((const void *)&IP, sizeof IP, AF_INET)) == NULL){
        errx(1, "no name associated with %s", ip);
    }
    //printf("name associated with %s is %s\n", ip, hp->h_name);
    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
    server.sin_port = htons(serverPort);         /* Server port */
    /* Initiate connection */
    if (connect(sock, serverptr, sizeof(server)) < 0){
	   perror_exit("connect");
   }
    //printf("Worker %d connecting to %s port %d\n",getpid(),ip , serverPort);

    /*compose the message that gives info about the worker*/
    char*msg = calloc(SOCKET_SIZE, sizeof(char));
    strcpy(msg, "W");
    strcat(msg, "#");
    strcat(msg, numWorkersStr);
    strcat(msg, "#");
    strcat(msg, pid);
    strcat(msg, "#");
    strcat(msg, workerPortStr);
    strcat(msg, "#");
    char countriesStr[20] ;
    sprintf(countriesStr, "%d", countries);
    strcat(msg, countriesStr);
    for(i = 0; i < countries; i++){
        strcat(msg, "#");
        strcat(msg, country_array[i]);
    }
    if ((nw = write(sock, msg, SOCKET_SIZE)) < 0){
            perror("Worker: cant write to socket");
    }
    free(msg);

    /*creation of Hash Table to store the lines of file*/
    HTMain **mainHashTable;
    int size = 10;
    mainHashTable = malloc(size * sizeof (HTMain*));
    for (i = 0; i < size; i++) {
        mainHashTable[i] = calloc(1, sizeof (HTMain));
    }
    HTCreate(mainHashTable, size);
    /*creation of disease hash tables to store the patients*/
    HashBuck **diseaseHashTable;
    diseaseHashTable = malloc(DISEASE_ENTRIES * sizeof (HashBuck*));
    for (i = 0; i < DISEASE_ENTRIES; i++)
        diseaseHashTable[i] = calloc(1, sizeof (HashBuck));
    HashCreate(diseaseHashTable, DISEASE_ENTRIES, BUCKET_SIZE);
    for (i = 0; i < DISEASE_ENTRIES; i++)
        diseaseHashTable[i]->bucket = HashBucketMemUpdate(diseaseHashTable[i]->bucket);
    /*end of HashTable creations*/

    int files = 0; DIR * sub_dir;
    for (i = 0; i < countries; i++) {
        char new_dir_name[300];
    	sprintf(new_dir_name, "%s/%s", dir_name, country_array[i]);
        country_count++;
        sub_dir = opendir(new_dir_name);
        struct dirent *file_entry;
    	char** dateFiles = malloc(file_sum*sizeof(char*));
        for(int j = 0; j < file_sum; j++){
        	dateFiles[j] = calloc(256, sizeof(char));
        }
        int dc = 0; int txt_flag = 0; //flag = 0 .txt flag = 1 no.txt

        while ((file_entry = readdir(sub_dir)) != NULL) {
            if (strcmp(file_entry->d_name, ".") != 0 && strcmp(file_entry->d_name, "..") != 0) {
                FILE *fptr_temp;
    			char file_name[300];
                sprintf(file_name, "%s/%s", new_dir_name, file_entry->d_name);
                fptr_temp = fopen(file_name, "r"); //same file 2 pointers
                if (fptr_temp == NULL) {
                    printf("Error null fptr\n");
                } else {
                    char *dateHead = calloc(256,sizeof(char));
    		    	int length = strlen(file_entry->d_name);
    		    	if (file_entry->d_name[length-1] == 't'){
                    	dateHead = trimDate(file_entry->d_name);
                    }else{
    					txt_flag = 1;
                    	strcpy(dateHead, file_entry->d_name);
                    }
    				strcpy(dateFiles[dc], dateHead);
    	 		}
                dc++;
    			fclose(fptr_temp);
    		}
    	}
    	dateFiles = ChronOrder(dateFiles, dc);
        for(int j = 0; j < dc; j++){
    		char *date = calloc(256,sizeof(char));
            strcpy(date, dateFiles[j]);
            char file_name[300];
            if (txt_flag == 0) {
                strcat(dateFiles[j], ".txt");
            }
            sprintf(file_name, "%s/%s", new_dir_name, dateFiles[j]);
            FILE *fptr, *fptr2; char* line = NULL; char* token ; size_t len = 0;
            ssize_t read; int line_count = 0; int recordCount = 0; patientRecord pR ;
            fptr = fopen(file_name,"r");
    		fptr2 = fopen(file_name, "r"); //one for summaryStatistics the other for the Hash Tables Insert
    	    while ((read = getline(&line, &len, fptr)) != -1) {
                token = strtok(line, " ");
                recordCount = 0;
                while (token != NULL) {
                	if (recordCount == 0) {
                    	strcpy(pR.recordID, token);
                    } else if (recordCount == 1) {
                        strcpy(pR.state, token);
                        if (strcmp(pR.state, "ENTER") == 0) {
                            strcpy(pR.entryDate, date);
                            strcpy(pR.exitDate, "-");
                         } else if (strcmp(pR.state, "EXIT") == 0) {
                            strcpy(pR.exitDate, date);
                            strcpy(pR.entryDate, "-");
                        } else {
                            printf("Error \n");
                            break;
                        }
                    } else if (recordCount == 2) {
                    	strcpy(pR.patientFirstName, token);
                    } else if (recordCount == 3) {
                         strcpy(pR.patientLastName, token);
                    } else if (recordCount == 4) {
                         strcpy(pR.diseaseID, token);
                    } else if (recordCount == 5) {
                         strcpy(pR.age, token);
                         //pR.age[strlen(pR.age) - 1] = '\0';
                    }
                    strcpy(pR.country, country_array[i]);
                    recordCount++;
                    token = strtok(NULL, " ");
                }
                line_count++;
    			HTInsert(mainHashTable, pR);
           	}
           	fclose(fptr);
           	SummaryStatistics(country_array[i], fptr2, date, &summary, line_count);
           	if ((nw = write(sock, summary, SOCKET_SIZE)) < 0){
                    perror("Worker stats: cant write to socket");
            }
           	strcpy(summary, "");
    		free(date);
			fclose(fptr2);
        }
    	for(int j = 0; j < dc; j++){
			free(dateFiles[j]);
        }
        free(dateFiles);
    }
    HTInsertToHashBuck(mainHashTable, diseaseHashTable);
    free(summary);
    if ((nw = write(sock, "FIN", SOCKET_SIZE)) < 0){
            perror("Worker: cant write to socket");
    }
    //some cleanup
    //close fds
    closedir(sub_dir);
    close(writefd);
    close(readfd);
    //unlink its fifos
	if (unlink(fifo) < 0){
        perror("worker: can't unlink \n");
     }
    if (unlink(temp) < 0){
        perror("worker: can't unlink \n");
    }
    sleep(2);
    //connect to server as a client
    close(sock);
    int workerSock, workerNewSock;
    struct sockaddr_in worker, client;
    socklen_t clientlen;
    struct sockaddr *workerptr=(struct sockaddr *)&worker;
    struct sockaddr *clientptr=(struct sockaddr *)&client;
    struct hostent *rem;
    /* Create socket */
    if ((workerSock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        perror_exit("worker socket");
    }
    worker.sin_family = AF_INET;       /* Internet domain */
    worker.sin_addr.s_addr = htonl(INADDR_ANY);
    worker.sin_port = htons(workerPort);      /* The given port */
    /* Bind socket to address */
    if (bind(workerSock, workerptr, sizeof(worker)) < 0)
        perror_exit("worker bind");
    /* Listen for connections */
    if (listen(workerSock, 100) < 0) {
        perror_exit("worker listen");
    }
    //printf("\nI'm worker (%d) %d and I'm listening for connections to port %d\n",pid_counter, getpid(), workerPort);
    char **arg = malloc(8 * sizeof (char*)); // arguments of query passed from the agg to each worker-child
    char *answer = calloc(SOCKET_SIZE, sizeof (char)); //answer of the query
    for (i = 0; i < 8; i++){
           arg[i] = (char*) calloc(256, sizeof (char));
    }
    if(pid_counter == 0){
        printf("____________________________________________________\n");
        printf("Workers Will Receive & Answer Queries From whoServer\n");
        printf("____________________________________________________\n");
    }
    while(1){
        clientlen = sizeof(client);
        /* accept connection */
    	if ((workerNewSock = accept(workerSock, clientptr, &clientlen)) < 0) {
            perror_exit("worker accept");
        }
        /* Find client's address */
    	if ((rem = gethostbyaddr((char *) &client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), client.sin_family)) == NULL) {
   	        herror("worker gethostbyaddr");
            exit(1);
        }
        char queryBuff[SOCKET_SIZE];
        char* token;
        if (read(workerNewSock, queryBuff, SOCKET_SIZE) > 0){
            token = strtok(queryBuff, " ");
            c = 0;
            while (token != NULL) {
                strcpy(arg[c], token);
                token = strtok(NULL, " ");
                c++;
            }
            if (strcmp(arg[0], "search") == 0) {
                searchPatientRecord(mainHashTable, arg[1], &answer);
                if(write(workerNewSock, answer, SOCKET_SIZE) < 0){
                    perror("write-worker:");
                }
            } else if (strcmp(arg[0], "topk") == 0) {
                topkAge(diseaseHashTable, arg[1], arg[2], arg[3], arg[4], arg[5], &answer);
                if(write(workerNewSock, answer, SOCKET_SIZE) < 0){
                    perror("write-worker:");
                }
            } else if (strcmp(arg[0], "diseaseFrequency") == 0) {
                diseaseFrequency(diseaseHashTable, arg[1], arg[2], arg[3], arg[4], &answer);
                if(write(workerNewSock, answer, SOCKET_SIZE) < 0){
                    perror("write-worker:");
                }
            } else if (strcmp(arg[0], "Admissions") == 0) {
                numPatientAdmissions(diseaseHashTable, arg[1], arg[2], arg[3], arg[4], country_array, country_count, &answer);
                if(write(workerNewSock, answer, SOCKET_SIZE) < 0){
                    perror("write-worker:");
                }
            } else if (strcmp(arg[0], "Discharges") == 0) {
                numPatientDischarges(diseaseHashTable, arg[1], arg[2], arg[3], arg[4], country_array, country_count, &answer);
                if(write(workerNewSock, answer, SOCKET_SIZE) < 0){
                    perror("write-worker:");
                }
            }

            printf("worker (%d) %d answered query : %s\n",pid_counter, getpid(),queryBuff);
        }
        fflush(stdout);
        for (i = 0; i <= 5; i++){
            strcpy(arg[i], "");
        }
        strcpy(answer, "");
        close(workerNewSock);
    }
	//Hashtable deletions
	HTDestroy(mainHashTable);
    HashDestroy(diseaseHashTable);
    printf("worker (%d) %d finished normally \n", pid_counter, getpid());
    return 0;
}
