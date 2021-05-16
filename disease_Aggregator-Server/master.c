#include "libs.h"
#include "fifo_func.h"
#include "worker.h"

#define PERMS 0666

typedef struct pidInfo {
    pid_t pid;
    int files;
    int pid_counter;
    int numCountries;
    char **countryArray;
} PidInfo;

void waitChildProcess() {
    int status;
    wait(&status);
}

void checkArgs(int argc, char**argv, int *numWorkers, int *bufferSize, char**input_dir, int *s_port, char**s_ip) {
    int numW = 0;
    int bufferS = 0;
    char* dir = calloc(256, sizeof(char));
	char* ip = calloc(256, sizeof(char));
	int port = 0;
    if (argc != 11) {
        fprintf(stderr, "Give the right arguments : –w numWorkers -b bufferSize –s serverIP –p serverPort -i input_dir !\n");
        exit(1);
    } else {
        printf("----------------------Arguments---------------------\n");
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], "-w") == 0) {
                numW = atoi(argv[i + 1]);
                if (numW < 1) {
                    fprintf(stderr, "Give numWorkers >= 1 \n");
                    exit(1);
                }
            } else if ((strcmp(argv[i], "-b") == 0)) {
                bufferS = atoi(argv[i + 1]);
                if (bufferS < 1) {
                    fprintf(stderr, "Give bufferSize >= 1 \n");
                }
            } else if ((strcmp(argv[i], "-i") == 0)) {
                strcpy(dir,argv[i + 1]);
            }else if ((strcmp(argv[i], "-p") == 0)) {
				port = atoi(argv[i + 1]);
			}else if ((strcmp(argv[i], "-s") == 0)) {
				strcpy(ip,argv[i + 1]);
            }
        }
    }
    *input_dir = dir;
    *bufferSize = bufferS;
    *numWorkers = numW;
	*s_ip = ip;
	*s_port = port;
}

bool new_child = false;

void sig_handler(int signum) { //to handle SIGINT and/or SIGQUIT
	if (signum == SIGCHLD) {
        new_child = true;
        printf("PARENT %d : I HAVE RECEIVED A SIGCHILD \n", getpid());
    }
	printf("Press [ENTER] to continue\n");
}

void newChild(PidInfo pidInfo[],int bufferSize,  int numWorkers, char* input_dir_name, int writefd[], int readfd[]) {
    int status;
    pid_t pid;
    pid = wait(&status);
    printf("Im gonna make a new worker beacuse worker%d died \n", pid);
    for (int i = 0; i < numWorkers; i++) {
        if (pidInfo[i].pid == pid) {
            //this worker stopped
            char *toSentArray = calloc(bufferSize, sizeof(char));
            for (int j = 0; j < pidInfo[i].numCountries; j++) {
                strcat(toSentArray, pidInfo[i].countryArray[j]);
                strcat(toSentArray, "#");
            }
			char pid_c[4]; //in case we have 999 pids lol
			sprintf(pid_c, "%d", pidInfo[i].pid_counter);
            char numWorkersStr[5];
            sprintf(numWorkersStr, "%d", numWorkers);
			//make fifo for the new worker
			char *fifo = malloc(sizeof (char*));
    		strcpy(fifo, "fifo");
    		char *temp = malloc(sizeof (char*));
    		strcpy(temp, "fifo");
    		strcat(fifo, "r"); //fiforXXXX
    		strcat(temp, "w"); //fifowXXXX
    		strcat(temp, pid_c);
    		strcat(fifo, pid_c);
			//make the fifos-pipes
			if (mkfifo(fifo, PERMS) < 0 && errno != EEXIST) {
				perror("can't create fifo");
			}
			if (mkfifo(temp, PERMS) < 0 && errno != EEXIST) {
				unlink(fifo);
				perror("can't create fifo");
			}
			//fix the execlp args
            char country_sum [4];
            char file_sum[4];
            char new_flag[2];
            char buffS[5];
            sprintf(file_sum, "%d", pidInfo[i].files);
            sprintf(country_sum, "%d", pidInfo[i].numCountries);
            sprintf(buffS, "%d", bufferSize);
            sprintf(new_flag, "%d", new_child);
            pid_t childpid = fork();
            pidInfo[i].pid = childpid;
            if (childpid == -1) {
                perror("Failed to fork");
                exit(0);
            } else if (childpid == 0) {
                int argc = 7;
                char* argv[] = {"Worker", pid_c, input_dir_name, buffS, country_sum, file_sum,numWorkersStr, NULL};
                workerFunc(argc, argv);
                exit(0);
            }
            if ((readfd[pidInfo[i].pid_counter] = open(fifo, O_RDONLY)) < 0) {
        		perror("new child: cant open read fifo");
    		}
    		if ((writefd[pidInfo[i].pid_counter] = open(temp, O_WRONLY)) < 0) {
        		perror("new child: can't open write fifo");
    		}

    		if (write(writefd[pidInfo[i].pid_counter], toSentArray, bufferSize) < 0) {
        		perror("master write in new child");
        		exit(1);
    		}
			free(toSentArray);
        }
    }
}

int main(int argc, char** argv){
	int serverPort = 0, i	;
	int bufferSize = 0;
	char* input_dir_name;
	char* ip, serverPortString[50];
	int numWorkers = 0;
	checkArgs(argc, argv, &numWorkers, &bufferSize, &input_dir_name, &serverPort, &ip);
	printf("numWorkers : %d \n", numWorkers);
	printf("bufferSize : %d \n", bufferSize);
	printf("input_dir : %s \n", input_dir_name);
	printf("serverPort : %d \n", serverPort);
	printf("serverIP : %s \n", ip);
    sprintf(serverPortString, "%d", serverPort);

	printf("---------------------Directory----------------------\n");
    DIR *input_dir, *input_dir_country;
    int country_count = 0;
    input_dir_country = opendir(input_dir_name); //we will use this variable to count the countries
	input_dir = opendir(input_dir_name);
    if (input_dir == NULL) {
        puts("Unable to read directory");
        return 1;
    } else {
        puts("Directory is opened!");
    }
    struct dirent *sub_entry_country, *sub_entry, *file_entry;
    int total_files = 0; //total files
	char *new_dir_name = calloc(256, sizeof(char));
	int total_countries = 0; //number of countries in input_dir directory
	//calculate how many countries the input_dir has !
	while ((sub_entry_country = readdir(input_dir_country)) != NULL) {
        if (strcmp(sub_entry_country->d_name, ".") != 0 && strcmp(sub_entry_country->d_name, "..") != 0)
			total_countries++;
	}
	closedir(input_dir_country);

	printf("Countries : %d \n", total_countries);
	char **country_array = malloc(total_countries * sizeof (char*)); //array to store the name of each country inside input_dir

	for (i = 0; i < total_countries; i++) {
        country_array[i] = (char*) malloc(50 * sizeof (char));
    }

	while ((sub_entry = readdir(input_dir)) != NULL) {
        if (strcmp(sub_entry->d_name, ".") != 0 && strcmp(sub_entry->d_name, "..") != 0) {
            strcpy(country_array[country_count], sub_entry->d_name);
			country_count++;
            strcpy(new_dir_name, input_dir_name);
            strcat(new_dir_name, "/");
            strcat(new_dir_name, sub_entry->d_name);
            DIR* sub_dir = opendir(new_dir_name);
            while ((file_entry = readdir(sub_dir)) != NULL) {
                if (strcmp(file_entry->d_name, ".") != 0 && strcmp(file_entry->d_name, "..") != 0){
                    total_files++; //total files at input_dir (files_per_country * countries)
				}
            }
			closedir(sub_dir);
        }
    }

	printf("Total files : %d \n", total_files);
    int files_per_country = (int) total_files / country_count ;
    printf("Files per country : %d \n", files_per_country);
    if (country_count < numWorkers) {
        printf("We dont need %d of the %d workers \n", (numWorkers - country_count), numWorkers);
        numWorkers = country_count;
    }
	double countriesPerWorker = (double) country_count / numWorkers;

	printf("-------------------Aggregator work------------------\n");
    int pid_counter = -1;
    PidInfo pidInfo[numWorkers];
    //char arr[numWorkers][BUFFER_SIZE];

    char** toSentArray = malloc(numWorkers* sizeof(char*));
    for(i = 0; i < numWorkers; i++){
        toSentArray[i]= calloc(bufferSize, sizeof(char));
    }
    for (i = 0; i < numWorkers; i++) {
        pidInfo[i].numCountries = (int) countriesPerWorker;
        pidInfo[i].countryArray = malloc(country_count * sizeof (char*));
        strcpy(toSentArray[i], "#");
        strcat(toSentArray[i], ip);
        strcat(toSentArray[i], "#");
        strcat(toSentArray[i], serverPortString);
        strcat(toSentArray[i], "#");
    }
    int rest = (int) country_count - ((int) countriesPerWorker * numWorkers);
    for (i = 0; i < rest; i++) {
        pidInfo[i + 1].numCountries++;
    }
    if (numWorkers > country_count) {
        printf("We don't need %d workers \n", numWorkers - country_count);
        numWorkers = country_count;
    }
    int j, cc = 0;
    int readfd[numWorkers], writefd[numWorkers];

    for (i = 0; i < numWorkers; i++) {
        printf("Number of countries for worker %d is %d\n", i, pidInfo[i].numCountries);
        for (j = 0; j < pidInfo[i].numCountries; j++) {
            pidInfo[i].countryArray[j] = calloc(50, sizeof(char));
            strcpy(pidInfo[i].countryArray[j],country_array[cc]);
            printf("%s\n", pidInfo[i].countryArray[j]);
            strcat(toSentArray[i], country_array[cc]);
            strcat(toSentArray[i], "#");
            cc++;
        }
        makeFifos(i, &writefd[i], &readfd[i]);
    }

    printf("----------------------Workers-----------------------\n");
    for (i = 0; i < numWorkers; i++) /* loop will run n times (n=5) */ {
        pid_counter++;
        char pid_c[4]; //in case we have 999 pids lol
        char country_sum [4];
        char numWorkersStr[5];
        char file_sum[4];
        char new_flag[2];
        char buffS[6];
        sprintf(numWorkersStr, "%d", numWorkers);
        sprintf(file_sum, "%d", files_per_country);
        sprintf(buffS, "%d", bufferSize);
        sprintf(country_sum, "%d", (int) countriesPerWorker + 1);
        sprintf(pid_c, "%d", pid_counter);
        pid_t childpid = fork();
        pidInfo[i].pid = childpid;
        pidInfo[i].pid_counter = pid_counter;
        pidInfo[i].files = (int) files_per_country;
        if (childpid == -1) {
            perror("master fork");
            return 1;
        } else if (childpid == 0) {
            int argc = 7;
            char* argv[] = {"Worker", pid_c, input_dir_name,buffS, country_sum, file_sum, numWorkersStr, NULL};
            workerFunc(argc, argv);
            exit(0);
        }
    }
	//aggregator work continues
    for (i = 0; i < numWorkers; i++) {
        openFifos(bufferSize, toSentArray, i, &writefd[i], &readfd[i]);
    }
	//handler work
    /*
    static struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigaction(SIGCHLD, &sa, 0);
    while(1){
		if (new_child == true){
			newChild(pidInfo, bufferSize, numWorkers, input_dir_name, writefd, readfd);
            new_child = false;
			continue;
		}
	}*/

    //Wait child processes to exit
    for (i = 0; i < numWorkers; i++) {
        waitChildProcess();
        close(writefd[i]);
        close(readfd[i]);
    }
    //unlink the fifos
    unlinkFifos(numWorkers);
    return 0;
}
