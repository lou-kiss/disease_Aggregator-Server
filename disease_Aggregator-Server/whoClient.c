#include "libs.h"
#define SOCKET_SIZE 1024

int serverPort;
char * serverIP;
int numThreads = 0; //argument from user
pthread_mutex_t client_mtx;
pthread_cond_t begin_cond;

typedef struct TI{
    int id;
    char* query;
    pthread_t ptid;
}threadInfo;

void usage(int argc, char** argv, char** queryFile){
    int numT = 0;
    char* qf = calloc(256, sizeof(char));
    int sp;
    serverIP = calloc(256, sizeof(char));
    int port = 0;
    if (argc != 9) {
        fprintf(stderr, "Give the right arguments : –q queryFile -w numThreads –sp serverPort –sip serverIP !\n");
        exit(1);
    } else {
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], "-w") == 0) {
                numT = atoi(argv[i + 1]);
                if (numT < 1) {
                    fprintf(stderr, "Give numThreads > 1 \n");
                    exit(1);
                }
            } else if ((strcmp(argv[i], "-q") == 0)) {
                strcpy(qf,argv[i + 1]);
            } else if ((strcmp(argv[i], "-sp") == 0)) {
                sp = atoi(argv[i+1]);
            }else if ((strcmp(argv[i], "-sip") == 0)) {
                strcpy(serverIP,argv[i + 1]);
            }
        }
    }
    *queryFile = qf;
    numThreads = numT;
    serverPort = sp;
}

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int thread_counter = 0;

void handleQuery(threadInfo *thread){
    pthread_mutex_lock(&client_mtx);
    while (thread_counter < numThreads){
        printf("Thread (%d) : %s is waiting\n", thread->id, thread->query);
        fflush(stdout);
        pthread_cond_wait(&begin_cond, &client_mtx);
    }
    printf("_______________________________________________________\n");
    //printf("Connect To Server with IP : %s and Port : %d\n", serverIP, serverPort);
    int sock;
    char resultBuff[1024];
    int nw,nr;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *hp; struct in_addr IP;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror_exit("whoClient-whoServer socket");
    }
    /* Find server address */
    if (!inet_aton(serverIP, &IP)){
        errx(1, "can't parse IP address %s", serverIP);
    }
    if ((hp = gethostbyaddr((const void *)&IP, sizeof IP, AF_INET)) == NULL){
        errx(1, "no name associated with %s", serverIP);
    }
    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
    server.sin_port = htons(serverPort);         /* Server port */
    /* Initiate connection */
    if (connect(sock, serverptr, sizeof(server)) < 0){
       perror_exit("connect");
    }
    printf("Q : %s\n", thread->query);
    if ((nw = write(sock, thread->query, SOCKET_SIZE)) < 0){
        perror_exit("whoClient: cant write to socket");
    }
    //print the result
    printf("R : ");
    if((nr = read(sock, resultBuff, SOCKET_SIZE)) < 0){
        perror_exit("whoClient : can't read from socket");
    }
    else{
        printf("%s \n", resultBuff);
    }
    pthread_mutex_unlock(&client_mtx);
}

void* queryThread(void* v){
    threadInfo *thread = (threadInfo *)v;
    handleQuery(thread);
    int ok = 1;
    pthread_exit(&ok);
    printf("Thread (%d) is exiting \n",thread->id);
}

int main(int argc, char*argv[]){
    pthread_mutex_init(&client_mtx, 0);
    pthread_cond_init(&begin_cond, 0);
    char * queryFile;
    printf("------------------------Arguments----------------------\n");
    usage(argc, argv, &queryFile);
    printf("numThreads : %d \nqueryFile : %s \nserverPort :%d \nserverIP : %s\n", numThreads,queryFile,  serverPort, serverIP);
    printf("----------------------File-&-Threads-------------------\n");
    FILE* fptr1, *fptr2;
    fptr1 = fopen(queryFile, "r");
    char*line = NULL;
    int lines_num = 0, i = 0;
    size_t len = 0;
    ssize_t reads;
    while ((reads = getline(&line, &len, fptr1)) != -1) {
        lines_num++;
    }
    //whatever the case (numThreads = lines_num) the code below serves the purpose of letting the user know *also check readme.txt
    if(numThreads > lines_num){
        printf("We don't need %d threads because the total lines number is : %d \n",(numThreads-lines_num), lines_num);
        numThreads = lines_num;
    }else if (numThreads < lines_num){
        printf("We don't have enough threads to send all the lines of the text file : %d \n", lines_num);
        numThreads = lines_num;
    }
    threadInfo *threadsInfo = malloc(numThreads*sizeof(threadInfo));
    void *status;
    fptr2 = fopen(queryFile, "r");
    while ((reads = getline(&line, &len, fptr2)) != -1) {
        line[strlen(line)-1] = 0;
        threadsInfo[i].query = calloc(1024, sizeof(char));
        threadsInfo[i].id = i;
        strcpy(threadsInfo[i].query, line);
        pthread_create(&(threadsInfo[i].ptid), 0, queryThread, &threadsInfo[i]);
        i++;
    }
    sleep(2);
    if(i == numThreads){
        thread_counter = i;
        pthread_cond_broadcast(&begin_cond);
        printf("All Threads Begin Now ! \n");
        fflush(stdout);
    }
    //cleanup
    for(i = 0; i < numThreads; i++){
        pthread_join(threadsInfo[i].ptid, &status);
    }
    pthread_mutex_destroy(&client_mtx);
    pthread_cond_destroy(&begin_cond);
    for(i = 0; i < numThreads; i++){
        free(threadsInfo[i].query);
    }
    free(threadsInfo);
    free(queryFile);
    free(serverIP);
    free(line);
    fclose(fptr1);
    fclose(fptr2);
    sleep(1);
    printf("-----------------------WhoClientEnd---------------------\n");
    printf("Main Thread (%ld) is exiting \n",pthread_self());
    return 0;
}
