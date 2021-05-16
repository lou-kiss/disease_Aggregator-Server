#include "libs.h"
#include "func.h"
#define SOCKET_SIZE 1024
#define new_max(x,y) ((x) >= (y)) ? (x) : (y)

typedef struct workerInfo {
    int workerCounter;
    int workerPort;
    char **countryArray;
    int countriesNum;
} WorkerInfo;

typedef struct CB{
    int *fd;
    int end ;
    int start;
    int count;
    int bufferSize;
} circleBuffer;

WorkerInfo **workersInfo;
char* ip;

void initWI(){
    /*mem alocation for workersInfo*/
    workersInfo = malloc(50*sizeof(WorkerInfo*));
    int i,j;
    for (i = 0; i < 50; i++){
        workersInfo[i] = malloc(sizeof(WorkerInfo));
        workersInfo[i]->countryArray = calloc(195, sizeof(char*));
        for(j = 0; j < 195; j++){
            workersInfo[i]->countryArray[j] = calloc(SOCKET_SIZE, sizeof(char));
        }
    }
    ip = calloc(1024, sizeof(char));
}

int numWorkers = 0;
int numOfThreads = 0;

pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void usage(int argc, char** argv, int *numThreads, int* bufferSize, int* queryPort, int* statsPort){
    int numT;
    int bufferS;
    int qp;
    int sp;
    if (argc != 9) {
        fprintf(stderr, "Give the right arguments : –q queryPortNum -s statisticsPortNum –w numThreads –b bufferSize!\n");
        exit(1);
    } else {
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], "-w") == 0) {
                numT = atoi(argv[i + 1]);
                if (numT < 1) {
                    fprintf(stderr, "Give numThreads >= 1 \n");
                    exit(1);
                }
            } else if ((strcmp(argv[i], "-b") == 0)) {
                bufferS = atoi(argv[i + 1]);
                if (bufferS < 1) {
                    fprintf(stderr, "Give bufferSize >= 1 \n");
                    exit(1);
                }
            } else if ((strcmp(argv[i], "-q") == 0)) {
                qp = atoi(argv[i+1]);
            }else if ((strcmp(argv[i], "-s") == 0)) {
                sp = atoi(argv[i+1]);
            }
        }
    }
    *bufferSize = bufferS;
    *numThreads = numT;
    numOfThreads = numT;
    *statsPort = sp;
    *queryPort = qp;
}

void printStatistics(char* stats){
    int break_flag = 0;
    printf("----------------------------------------\n");
        char* token, *rest = NULL;
        int age_counter = -1;
        int words = 0;
        token = strtok_r(stats, "#", &rest);
        printf("%ld \n", pthread_self());
        while (token != NULL) {
            if (strcmp(token, "EOF") == 0) {
                break_flag = 1; //double break while and for loop
                break;
            }
            if (words == 0) {
                printf("%s \n", token);
                words++;
            } else if (isdigit(token[0])) {
                if (age_counter == 0) {
                    printf("Age range 0-20 years: %s cases \n", token);
                    age_counter++;
                } else if (age_counter == 1) {
                    printf("Age range 21-40 years: %s cases \n", token);
                    age_counter++;
                } else if (age_counter == 2) {
                    printf("Age range 41-60 years: %s cases \n", token);
                    age_counter++;
                } else if (age_counter == 3) {
                    printf("Age range 60+ years: %s cases \n", token);
                    age_counter++;
                }

            } else {
                printf("%s \n", token);
                age_counter = 0;
            }
            token = strtok_r(NULL, "#",&rest);
        }
}

void sendQueryToWorkers(char *query, int i, int *workerSockFd){
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *hp; struct in_addr IP;
    if ((*workerSockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror_exit("socket");
    }
    /* Find worker address */
    if (!inet_aton(ip, &IP)){
        errx(1, "can't parse IP address %s", ip);
    }
    if ((hp = gethostbyaddr((const void *)&IP, sizeof IP, AF_INET)) == NULL){
        errx(1, "no name associated with %s", ip);
    }
    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
    server.sin_port = htons(workersInfo[i]->workerPort);         /* worker port */
    /* Initiate connection */
    if (connect(*workerSockFd, serverptr, sizeof(server)) < 0){
       perror_exit("connect");
    }
    write(*workerSockFd, query, SOCKET_SIZE);
}

void handle_query(char* buff, int clientFd){
    printf("_______________________________________________________\n");
    printf("Q: %s \n", buff); fflush(stdout);
    int i, j, disease_flag = 0;
    char** arguments = malloc(8*sizeof (char*)); /*allocate memory for the char array for each command*/
    for (i = 0; i < 8; i++){
        arguments[i] = calloc(256, sizeof (char));
    }
    char *toSendCommand = calloc(SOCKET_SIZE, sizeof (char));
    char *rest = NULL;
    char* token = strtok_r(buff, " ", &rest);
    int count = 0; int workerSockFd[numWorkers];int oneWorkerSockFd;
    bool wrongFlag = false;
    bool querySent = false;
    while (token != NULL) {
        strcpy(arguments[count], token);
        token = strtok_r(NULL, " ", &rest);
        count++;
    }
    bool moreThanOneResult = false;
    if (strcmp(arguments[0], "/searchPatientRecord") == 0) {
        strcpy(toSendCommand, "search ");
        strcat(toSendCommand, arguments[1]);
        moreThanOneResult = true;
        for (i = 0; i < numWorkers; i++){
            //printf("->Imma send %s to all the workers !\n", toSendCommand);
            sendQueryToWorkers(toSendCommand, i, &workerSockFd[i]);
            querySent = true;
        }
    }else if (strcmp(arguments[0], "/topk-AgeRanges") == 0) {
        strcpy(toSendCommand, "topk");
        moreThanOneResult = false;
        for (i = 1; i <= 5; i++) {
            strcat(toSendCommand, " ");
            strcat(toSendCommand, arguments[i]);
        }
        for (i = 0; i < numWorkers; i++) {
            for (j = 0; j < workersInfo[i]->countriesNum; j++) {
                if (strcmp(workersInfo[i]->countryArray[j], arguments[2]) == 0) {
                    //printf("Imma send %s to worker %d \n",toSendCommand, i );
                    sendQueryToWorkers(toSendCommand, i, &oneWorkerSockFd);
                    querySent = true;
                }
            }
        }
    }else if (strcmp(arguments[0], "/numPatientAdmissions") == 0) {
        strcpy(toSendCommand, "Admissions");
        for (i = 1; i <= 4; i++) {
            strcat(toSendCommand, " ");
            strcat(toSendCommand, arguments[i]);
        }
        if (strcmp(arguments[4], "") != 0) {
            //country arg
            moreThanOneResult = false;
            for (i = 0; i < numWorkers; i++) {
                for (j = 0; j < workersInfo[i]->countriesNum; j++) {
                    if (strcmp(workersInfo[i]->countryArray[j], arguments[4]) == 0) {
                        //printf("Imma send %s to worker %d \n",toSendCommand, i );
                        sendQueryToWorkers(toSendCommand, i, &oneWorkerSockFd);
                        querySent = true;
                    }
                }
            }
        } else {
            //no country arg
            moreThanOneResult = true;
            for (i = 0; i < numWorkers; i++){
                //printf("Imma send %s to all the workers !\n", toSendCommand);
                sendQueryToWorkers(toSendCommand, i, &workerSockFd[i]);
                querySent = true;
            }
        }
    } else if (strcmp(arguments[0], "/numPatientDischarges") == 0) {
        strcpy(toSendCommand, "Discharges");
        for (i = 1; i <= 4; i++) {
            strcat(toSendCommand, " ");
            strcat(toSendCommand, arguments[i]);
        }
        if (strcmp(arguments[4], "") != 0) {
            //country arg
            moreThanOneResult = false;
            for (i = 0; i < numWorkers; i++) {
                for (j = 0; j < workersInfo[i]->countriesNum; j++) {
                    if (strcmp(workersInfo[i]->countryArray[j], arguments[4]) == 0) {
                        //printf("Imma send %s to worker %d \n",toSendCommand, i );
                        sendQueryToWorkers(toSendCommand, i, &oneWorkerSockFd);
                        querySent = true;
                    }
                }
            }
        } else {
            //no country arg
            moreThanOneResult = true;
            for (i = 0; i < numWorkers; i++){
                //printf("Imma send %s to all the workers !\n", toSendCommand);
                sendQueryToWorkers(toSendCommand, i, &workerSockFd[i]);
                querySent = true;
            }
        }
    }
    else if (strcmp(arguments[0], "/diseaseFrequency") == 0) {
        strcpy(toSendCommand, "diseaseFrequency");
        for (i = 1; i <= 4; i++) {
            strcat(toSendCommand, " ");
            strcat(toSendCommand, arguments[i]);
        }
        if (strcmp(arguments[4], "") != 0) {
            //country arg
            moreThanOneResult = false;
            for (i = 0; i < numWorkers; i++) {
                for (j = 0; j < workersInfo[i]->countriesNum; j++) {
                    if (strcmp(workersInfo[i]->countryArray[j], arguments[4]) == 0) {
                        //printf("Imma send %s to worker %d \n",toSendCommand, i );
                        sendQueryToWorkers(toSendCommand, i, &oneWorkerSockFd);
                        querySent = true;
                    }
                }
            }
        } else {
            //no country arg
            disease_flag = 1;
            moreThanOneResult = true;
            for (i = 0; i < numWorkers; i++){
                sendQueryToWorkers(toSendCommand, i, &workerSockFd[i]);
                querySent = true;
            }
        }
    }else{
        //no known queryFile
        printf("Error. Wrong command \n");
        //won't send it to the client
        wrongFlag = true;
        querySent = true; //
    }
    if (querySent == false){
        printf("Error. Wrong country \n");
        wrongFlag = true;
    }

    char res[SOCKET_SIZE];
    char* resNew;
    char* resultToSentToClient = calloc(1024, sizeof(char));

    if (wrongFlag == false){
        //first print the result of the query
        int disease_sum = 0;
        printf("R: ");
        if (moreThanOneResult == true){
            for(i = 0; i < numWorkers; i++){
                if (read(workerSockFd[i], res, 1024) > 0){
                    resNew = res+1;
                    if (disease_flag == 1 && resNew[0] != 'N'){
                        disease_sum = disease_sum + atoi(resNew);
                    }else if (disease_flag != 1){
                        if(i == 0){
                            sprintf(resultToSentToClient, "%s", resNew);
                        }else{
                            strcat(resultToSentToClient, resNew);
                        }
                    }
                }
            }
            if (disease_flag == 1){
                printf("%d \n", disease_sum);
                fflush(stdout);
            }else{
                printf("%s \n", resultToSentToClient); /*this is the result of the query */
                fflush(stdout);
            }
        }else{
            if (read(oneWorkerSockFd, res, SOCKET_SIZE) > 0){
                char* resNew = res+1;
                sprintf(resultToSentToClient, "%s", resNew);
                printf("%s \n", resultToSentToClient); /*this is the result of the query */
                fflush(stdout);
            }
        }
        //now lets sent it to the client
        char *disease_sum_str = calloc(SOCKET_SIZE, sizeof(char));
        if (disease_flag != 1){
            if(write(clientFd, resultToSentToClient, SOCKET_SIZE) < 0){
                perror_exit("whoServer-whoClient write");
            }
        }else{
            sprintf(disease_sum_str, "%d", disease_sum);
            if(write(clientFd, disease_sum_str, SOCKET_SIZE) < 0){
                perror_exit("whoServer-whoClient write");
            }
        }
    }else{
        if (querySent == false){
            strcpy(resultToSentToClient, "Error. Wrong country \n");
            if(write(clientFd, resultToSentToClient, SOCKET_SIZE) < 0){
                perror_exit("whoServer-whoClient write");
            }
        }else{
            strcpy(resultToSentToClient, "Error. Wrong command \n");
            if(write(clientFd, resultToSentToClient, SOCKET_SIZE) < 0){
                perror_exit("whoServer-whoClient write");
            }
        }
    }
}

void handle_connection(int fd){
    int i,j;
    int counter = 0; int workerStatsDone = 0; bool doNotPrint = false;
    char buff[SOCKET_SIZE]; int wc = 0;
    while(workerStatsDone == 0) {
        if(read(fd, buff, SOCKET_SIZE) > 0){
            if(buff[0] == 'W'){
                int cc = 0; //country count
                counter = 0;
                //WorkerInfo
                char* rest = NULL;
                char*token = strtok_r(buff, "#", &rest);
                while(token!=NULL){
                    if (counter == 0){
                        counter++;
                    }else if(counter == 1){
                        counter++;
                        numWorkers = atoi(token);
                    }else if(counter == 2){
                        counter++;
                        wc = atoi(token);
                        workersInfo[wc]->workerCounter = wc;
                    }else if(counter == 3){
                        counter++;
                        workersInfo[wc]->workerPort = atoi(token);
                    }else if(counter == 4){
                        counter++;
                        workersInfo[wc]->countriesNum = atoi(token);
                    }else{
                        strcpy(workersInfo[wc]->countryArray[cc], token);
                        cc++;
                    }
                    token = strtok_r(NULL, "#", &rest);
                }
            }else if (strcmp(buff, "FIN") == 0){
                //end of info+stats from worker !
                workerStatsDone++;
            }else if (buff[0] == '/'){
                //we have a query from the client
                handle_query(buff, fd);
                doNotPrint = true;
                workerStatsDone++;
            }
            else {
                //stats
                strcat(buff, "EOF#");
                //printStatistics(buff);
                fflush(stdout);
            }
        }else{
            perror_exit("whoServer read");
        }
    }
    if(doNotPrint == false){
        printf("-------------------------%ld------------------------- \n", pthread_self());
        printf("numWorkers : %d \n", numWorkers);
        printf("pid : %d , port : %d and countries %d : ", workersInfo[wc]->workerCounter, workersInfo[wc]->workerPort, workersInfo[wc]->countriesNum);
        for(j = 0; j <  workersInfo[wc]->countriesNum; j++){
            printf("%s ", workersInfo[wc]->countryArray[j]);
        }
        printf("\n");
    }
    fflush(stdout);
}

void initCB(circleBuffer *cb, int bufferSize){
    cb->fd = malloc(bufferSize * sizeof(int));
    cb->end = 0;
    cb->start = 0;
    cb->count = 0;
    cb->bufferSize = bufferSize;
    //mutex init
    pthread_mutex_init(&mtx, 0);
    pthread_cond_init(&cond_nonempty, 0);
    pthread_cond_init(&cond_nonfull, 0);
    printf("Created a circle buffer with size %d and initialized the mutexes \n", bufferSize);
}

void writeToBuffer(circleBuffer *cb, int fdToWrite){
    pthread_mutex_lock(&mtx);
    while (cb->count >= cb->bufferSize){
        //printf("Buffer is full, cannot write ! %ld\n", pthread_self());
        pthread_cond_wait(&cond_nonfull, &mtx);
    }
    cb->fd[cb->start] = fdToWrite;
    cb->start = (cb->start +1) % cb->bufferSize;
    cb->count++;
    pthread_mutex_unlock(&mtx);
}

void readFromBuffer(circleBuffer *cb, int *fdToRead){
    pthread_mutex_lock(&mtx);
    while (cb->count <= 0){
        //printf("Buffer is empty, cannot read ! %ld \n", pthread_self());
        pthread_cond_wait(&cond_nonempty, &mtx);
    }
    *fdToRead = cb->fd[cb->end];
    cb->end = (cb->end + 1) % cb->bufferSize;
    cb->count--;
    pthread_mutex_unlock(&mtx);
}

void* thread_func(void* arg){
    printf("Hello server I'm thread : %ld \n", pthread_self());
    fflush(stdout);
    circleBuffer *cb = (circleBuffer*)arg;
    int socketfd;
    while(1){
        if (cb->count > 0) {
            readFromBuffer(cb, &socketfd);
            //printf("I'm thread %ld and I'm going to read from socket desciptor %d  \n", pthread_self(), socketfd);
            fflush(stdout);
            handle_connection(socketfd);
            close(socketfd);
            pthread_cond_signal(&cond_nonfull);
        }
    }
}

void produce(circleBuffer *cb,  int socketfd){
    writeToBuffer(cb, socketfd);
    //printf("Insert socket desciptor %d into the circle buffer [cb->count = %d]\n", socketfd, cb->count );
    pthread_cond_signal(&cond_nonempty);
}

void* destroyMtx(){
    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cond_nonempty);
    pthread_cond_destroy(&cond_nonfull);
}

int main(int argc, char*argv[]){
    int numThreads = 0;
    int bufferSize = 0;
    int queryPort;
    initWI();
    int statsPort;
    printf("---------------Arguments---------------\n");
    usage(argc, argv, &numThreads, &bufferSize, &queryPort, &statsPort);
    printf("numThreads : %d \nbufferSize : %d \nqueryPortNum :%d \nstatisticsPortNum : %d\n", numThreads, bufferSize, queryPort, statsPort);
    printf("----------------Threads----------------\n");
    int i,j;
    pthread_t ptid[numThreads] ;
    circleBuffer *cb = malloc(sizeof(circleBuffer));
    initCB(cb, bufferSize);
    for(i = 0; i < numThreads; i++){
        pthread_create(&(ptid[i]), 0, thread_func, cb);
    }
    sleep(2);
    printf("----------------Sockets----------------\n");
    struct sockaddr_in server1, server2, worker, client;
    socklen_t server1len, server2len;
    struct sockaddr *server1ptr=(struct sockaddr *)&server1;
    struct sockaddr *clientptr=(struct sockaddr *)&client;
    struct sockaddr *server2ptr=(struct sockaddr *)&server2;
    struct sockaddr *workerptr=(struct sockaddr *)&worker;
    struct hostent *rem1, *rem2;
    // Create sockets
    int workerSock, clientSock;
    if ((workerSock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror_exit("whoServer workerSocket");
    }
    if ((clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror_exit("whoServer clientSocket");
    }
    client.sin_family = AF_INET;       // Internet domain
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    client.sin_port = htons(queryPort);      // The given port

    worker.sin_family = AF_INET;       // Internet domain
    worker.sin_addr.s_addr = htonl(INADDR_ANY);
    worker.sin_port = htons(statsPort);      // The given port
    // Bind socket to address
    if (bind(workerSock, workerptr, sizeof(worker)) < 0){
        perror_exit("whoServer bind");
    }
    // Bind socket to address
    if (bind(clientSock, clientptr, sizeof(client)) < 0){
        perror_exit("whoServer bind");
    }
    // Listen for connections
    if (listen(workerSock, 100) < 0) {
        perror_exit("whoServer listen");
    }
    printf("Listening for worker connections at port : %d\n", statsPort);
    // Listen for connections
    if (listen(clientSock, 100) < 0) {
        perror_exit("whoServer listen");
    }
    printf("Listening for whoClient connections at port : %d\n", queryPort);
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    fd_set readSocks;
    while(1){
        int readers, newWorkerSock, newClientSock, retval;
        int smax = new_max(workerSock, clientSock);
        FD_ZERO(&readSocks);
        FD_SET(clientSock, &readSocks);
        FD_SET(workerSock, &readSocks);

        retval = select(smax+1, &readSocks, (fd_set *) 0, (fd_set *) 0, &timeout);
        if (retval > 0){
            if(FD_ISSET(clientSock, &readSocks)){
                //client connection
                server1len = sizeof(server1);
                if ((newClientSock = accept(clientSock, server1ptr, &server1len)) < 0) {
                    perror_exit("whoServer accept");
                }
                /* Find client's address */
            	if ((rem1 = gethostbyaddr((char *) &server1.sin_addr.s_addr, sizeof(server1.sin_addr.s_addr), server1.sin_family)) == NULL) {
           	        herror("whoServer gethostbyaddr");
                    exit(1);
                }
                //insert newsock at the circleBuffer
                produce(cb, newClientSock);
            }
            if (FD_ISSET(workerSock, &readSocks)){
                //worker connection
                server2len = sizeof(server2);
                if ((newWorkerSock = accept(workerSock, server2ptr, &server2len)) < 0) {
                    perror_exit("whoServer accept");
                }
                strcpy(ip, inet_ntoa(server2.sin_addr));
                /* Find client's address */
            	if ((rem2 = gethostbyaddr((char *) &server2.sin_addr.s_addr, sizeof(server2.sin_addr.s_addr), server2.sin_family)) == NULL) {
           	        herror("whoServer gethostbyaddr");
                    exit(1);
                }
                //insert newsock at the circleBuffer
                produce(cb, newWorkerSock);
            }

        }else if (retval < 0){
            perror_exit("whoServer select");
            break;
        }
    }
    printf("Closing connection.\n");
    close(workerSock); close(clientSock);
    return 0;
}
