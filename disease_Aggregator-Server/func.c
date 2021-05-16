/* koines synarthseis */
#include "func.h"
#include "Heap.h"

/* Polynomial hash code from the lectures (Data Structures). */
int HashFunction(char* key, int size){
    int hash = 0, a = 33;
    for (; *key != '\0'; key++)
	hash = (a*hash + * key) % size;
    return hash % size;
}

int DateCmp(char* entryDate, char* exitDate){
    int enDay = 0, enMonth = 0, enYear = 0;
    DateConversion(entryDate ,&enDay, &enMonth, &enYear);
    int  exDay = 0, exMonth = 0, exYear = 0;
    DateConversion(exitDate,&exDay, &exMonth, &exYear);

    if (exYear == 0 && exMonth == 0 && exDay == 0) return 0;
    if(enYear > exYear)
	return -1;
    else if(enYear == exYear){
        if (enMonth > exMonth)
            return -1;
        else if(enMonth == exMonth){
            if(enDay > exDay)
                return -1;
        }
    }
    return 0;
}

void DateConversion(char*  date, int * day, int *month, int *year){
    char* token;
    char*Date;
    Date = strdup(date);
    token = strtok(Date, "-");
    int Count = 0;
    while(token != NULL){
        if (Count == 0){
            *day = atoi(token);
        }
        else if(Count == 1){
            *month = atoi(token);
        }
        else if (Count == 2){
            *year = atoi(token);
        }
        token = strtok(NULL, "-");
        Count++;
    }
}

char* trimDate(char* date) {
    int i, n;
    char* new;
    for (i = 0; date[i] != '\0'; i++);
    // length of the new string
    n = i - 4 + 1;
    if (n < 1)
        return NULL;
    new = (char*) malloc(n * sizeof (char));
    for (i = 0; i < n - 1; i++)
        new[i] = date[i];
    new[i] = '\0';
    return new;
}

char** ChronOrder(char** array, int N){
    int i ;
    char* temp  = calloc(256, sizeof(char));
    for (i = 0; i < N; i++){
        for(int j = 0; j < N; j++)
            if(DateCmp(array[j], array[i]) == -1){ //array[j] > array[i]
                strcpy(temp, array[i]);
                strcpy(array[i], array[j]);
                strcpy(array[j], temp);
            }
    }
    return array;
}

void numPatientDischarges(HashBuck **disease,char* diseaseID, char* date1, char* date2, char* country, char** country_array, int country_count,char** answer){
    int answer_count = 0;
    int hashID = HashFunction(diseaseID, (*disease)->size);
    int entriesPerBucket = entriesPerBucketCalc(disease);
    HashBuck *node;
    node = disease[hashID];
    int i,count;char countc[10];int flag = 0;
    if (strcmp(country, "") == 0){
        //without country argument
        while(node != NULL){
            for(i = 0; i < entriesPerBucket; i++){
                if(node->bucket[i].key != NULL && strcmp(node->bucket[i].key, diseaseID) == 0){
                    for(int j = 0; j < country_count; j++){
                        count = 0;
                        BTSearchDatesExit(node->bucket[i].tree->ptr, date1, date2, &count, country_array[j]);
                        //printf("%s %d \n", country_array[j],count);
			            flag++;
			            if (answer_count == 0){
                            sprintf(*answer, "^%s %d\n",country_array[j], count);
                            answer_count++;
                        }else{
                            strcat(*answer, country_array[j]);
                            strcat(*answer," ");
                            sprintf(countc, " %d\n", count);
                            strcat(*answer, countc);
                        }
                    }
                }
            }
            node = node->next;
        }
    }
    else{
        count = 0;
        //with country argument
        while(node != NULL){
            for(i = 0; i < entriesPerBucket; i++){
                if(node->bucket[i].key != NULL && strcmp(node->bucket[i].key, diseaseID) == 0){
                    BTSearchDatesExit(node->bucket[i].tree->ptr, date1, date2, &count, country);
                    //printf("%s %d \n", country,count);
		            flag++;
                    sprintf(*answer, "^%s %d", country,count);
                }
             }
             node = node->next;
        }
    }
}

void numPatientAdmissions(HashBuck **disease, char* diseaseID,char* date1, char* date2, char* country, char** country_array, int country_count, char** answer){
    int answer_count = 0;
    int hashID = HashFunction(diseaseID, (*disease)->size);
    int entriesPerBucket = entriesPerBucketCalc(disease);
    HashBuck *node;
    node = disease[hashID];
    int i,count;
    int flag = 0;
    char countc[10];
    if (strcmp(country, "") == 0){
        //without country argument
        while(node != NULL){
            for(i = 0; i < entriesPerBucket; i++){
                if(node->bucket[i].key != NULL && strcmp(node->bucket[i].key, diseaseID) == 0){
                    for(int j = 0; j < country_count; j++){
                        count = 0;
                        BTSearchDatesKey(node->bucket[i].tree->ptr, date1, date2, &count, country_array[j]);
                        //printf("%s %d \n", country_array[j],count);
                        flag++;
                        if (answer_count == 0){
                            sprintf(*answer, "^%s %d\n",country_array[j], count);
                            answer_count++;
                        }else{
                            strcat(*answer, country_array[j]);
                            strcat(*answer," ");
                            sprintf(countc, " %d\n", count);
                            strcat(*answer, countc);
                        }
                    }
                }
            }
            node = node->next;
        }
    }
    else{
        count = 0;
        //with country argument
         while(node != NULL){
            for(i = 0; i < entriesPerBucket; i++){
                if(node->bucket[i].key != NULL && strcmp(node->bucket[i].key, diseaseID) == 0){
                    BTSearchDatesKey(node->bucket[i].tree->ptr, date1, date2, &count, country);
                    sprintf(*answer, "^%s %d",country, count);
                    //printf("%s\n", *answer);
                    flag++;
                }
            }
            node = node->next;
        }
    }
}

void topkAge(HashBuck** table, char *k, char*country, char*disease, char*date1, char*date2, char**answer){
    int K = atoi(k);
    int entriesPerBucket = entriesPerBucketCalc(table);
    HeapInfo heapInfo;
    int i, j, total_count=0;
    int age_count[4];
    for(i = 0; i < 4; i++){
        age_count[i] = 0;
    }
    HeapNode *rootHeap = NULL;
    HeapQueue *queue = QueueCreate((*table)->size);
    for(i = 0; i < (*table)->size; i++){
        HashBuck *node = table[i];
        while (node != NULL){
            for(j = 0; j < entriesPerBucket; j++){
                if (node->bucket[j].key != NULL && strcmp(node->bucket[j].key,disease) == 0){
                    //found the disease
                    BTSearchDatesAge(node->bucket[j].tree->ptr, date1, date2, age_count, &total_count, country);
                }
            }
            node = node->next;
        }

    }
    if (total_count != 0){
	    heapInfo.key = strdup("0-20");
	    heapInfo.count = age_count[0]*100/total_count;
	    HeapInsert(&rootHeap, heapInfo, queue);
	    heapInfo.key =  strdup("21-40");
	    heapInfo.count = age_count[1]*100/total_count;
	    HeapInsert(&rootHeap, heapInfo, queue);
	    heapInfo.key = strdup( "41-60");
	    heapInfo.count = age_count[2]*100/total_count;
	    HeapInsert(&rootHeap, heapInfo, queue);
	    heapInfo.key=strdup("60+");
	    heapInfo.count = age_count[3]*100/total_count;
	    HeapInsert(&rootHeap, heapInfo, queue);
	    FindTopk(rootHeap, K, answer);
    }
    else{
	    sprintf(*answer, "^No patients found with that disease: %s in %s\n",disease, country);
    }
}

void FindTopk(HeapNode *root, int K, char** answer){
    char countc[10];
    int answer_count=0;
    for(int i = 0; i < K; i++){
        MaxHeapify(root);
        if (root->info.count == -1 || root->info.count == 0){
            return;
        }else{
            //printf("%s %d % \n", root->info.key, root->info.count);
    	    if (answer_count == 0){
                sprintf(*answer, "^%s: %d %% \n",root->info.key, root->info.count);
                answer_count++;
    	    }else{
    		   strcat(*answer, root->info.key);
               strcat(*answer,": ");
               sprintf(countc, " %d%% \n", root->info.count);
               strcat(*answer, countc);
            }
            HeapNode *rightest = malloc(sizeof(HeapNode));
            rightest = FindDeepestRightLeaf(root);
            SwapNodesInfo(&(root->info.count), &(rightest->info.count), &(root->info.key), &(rightest->info.key));
            //root = rightestLeaf
            rightest->info.count = -1;
            rightest->info.key = "404";
        }
    }
}

void searchPatientRecord(HTMain **table, char* recordID, char **answer){
    HTMain*  curr = NULL;
    int i;
    int flag = 0;
    for( i = 0; i < (*table)->size; i++){
        curr = table[i];
        while(curr->next != NULL){
            if (strcmp(curr->record.recordID, recordID)==0){
                //PrintPatient(curr->record);
		        sprintf(*answer, "^%s %s %s %s %s %s %s %s %s", curr->record.recordID, curr->record.state, curr->record.patientFirstName, curr->record.patientLastName, curr->record.country, curr->record.diseaseID, curr->record.entryDate, curr->record.exitDate, curr->record.age);
                flag++;
            }
            curr = curr->next;
        }
    }
    if (flag == 0){
	   sprintf(*answer, "^Not found patient : %s\n", recordID);
   }
}

void diseaseFrequency(HashBuck** disease, char* diseaseID, char* date1, char* date2, char* country, char** answer){
    int entriesPerBucket = entriesPerBucketCalc(disease);
    int i,count = 0, flag = 0;
    int hashID = HashFunction(diseaseID, (*disease)->size);
    HashBuck *node = disease[hashID];
    if (strcmp(country, "") == 0){
        strcpy(country, "-");
        while(node != NULL){
            for(i = 0; i < entriesPerBucket; i++){
                if(node->bucket[i].key != NULL && strcmp(node->bucket[i].key , diseaseID) == 0){
                    BTSearchDatesKey(node->bucket[i].tree->ptr, date1, date2, &count, country);
                    //printf("%s %d\n",node->bucket[i].key, count);
		            sprintf(*answer, "^%d", count);
                    flag++;
                }
            }
            node = node->next;
        }
    }
    else{
        while(node != NULL){
            for(i = 0; i < entriesPerBucket; i++){
                if (node->bucket[i].key != NULL && strcmp(node->bucket[i].key , diseaseID) == 0){
                    BTSearchDatesKey(node->bucket[i].tree->ptr, date1, date2, &count, country);
                    //printf("%s %d\n",node->bucket[i].key,count);
		            sprintf(*answer, "^%d", count);
                    flag++;
                }
            }
            node = node->next;
        }
    }
}

void SummaryStatistics(char* country, FILE* fptr, char* date, char **summary, int recordsPerFile){
    sprintf(*summary, "%s#", date);
    int dis_counter = 0;
    char disease[256]; char ageS[5] = { 0 }; int age = 0; int temp; char state[6];
    sumInfo* stats = malloc(recordsPerFile*sizeof(sumInfo));
    int i,j;
    for(j = 0; j < 10; j++){
        for (i = 0; i < 4; i++){
            stats[j].array[i] = 0;
        }
    }
    char* line = NULL; char* token ; size_t len = 0;  ssize_t read; int recordCount = 0;
    while ((read = getline(&line, &len, fptr)) != -1) {
        token = strtok(line, " ");
        recordCount = 0;
        while(token != NULL){
            if(recordCount == 4)
                strcpy(disease,token);
            else if (recordCount == 5)
                strcpy(ageS, token);
            else if (recordCount == 1)
                strcpy(state, token);
            recordCount++;
            token = strtok(NULL, " ");
            age = atoi(ageS);
        }
        if (strcmp(state, "ENTER") == 0){
            temp = dis_counter;
            for(i = 0; i < dis_counter; i++){
                if (strcmp(stats[i].disease, disease) == 0){
                    dis_counter = i;
                    upgradeAges(stats[i].array, age);
                }
            }
            if (dis_counter != temp)
                dis_counter = temp; //found it
            else{
                stats[dis_counter].disease = strdup(disease);
                upgradeAges(stats[dis_counter].array, age);
                dis_counter++;
            }
        }
    }
    strcat(*summary,country);
    strcat(*summary, "#");
    for(i = 0; i < dis_counter; i++){
        strcat(*summary,stats[i].disease);
		strcat(*summary,"#");
        for(j = 0; j <= 3; j++){
            sprintf(ageS, "%d#", stats[i].array[j]);
            strcat(*summary, ageS);
        }
    }
    free(stats);
}

void upgradeAges(int ageArray[4], int age){
    if (age <= 20)
            ageArray[0]++;
    else if (age <= 40)
            ageArray[1]++;
    else if(age <= 60)
           ageArray[2]++;
    else
            ageArray[3]++;
}
