#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <omp.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#define MASTER 0
#define MAX_WORKERS 100
#define MAX_TASKS 100
#define MAX_LINE_LENGTH 100
#define THRESHOLD 5
#define MAX_MATRIX_SIZE 100

/*
TAG:
0 - WORKER EXIT
1 - ANAGRAMS, PRIMES, PRIMEDIVISORS
100-199 - MATRIX ADD  103 ----> 3 CLI file number
300-399 - MATRIX MULT
10000 - 19999 - MATRIX ADD N WORKERS
10431 - MATRIX ADD 3 workers, currently at worker 1, CLI4

200-299 - MATRIX ADD MORE WORKERS   210 ----> 10 CLI file number
300-399 - MATRIX MULT
400-499 - MATRIX MULT MORE WORKERS


10000 - 19999 - MATRIX ADD N WORKERS
10431 - MATRIX ADD 3 workers, currently at worker 1, CLI4

FIX: 16 MATRIX ADD NOT WORKING
DO MATRIX ADD FOR N WORKERS
CLEAN CODE
*/

int* getNFreeWorkers(int workerStatus[MAX_WORKERS], int WORKERS_NUMBER_AND_MASTER, int workersNumber) {
    int* freeWorkersArray = malloc(workersNumber * sizeof(int));
    if (freeWorkersArray == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    while (1) {
        int found = 0;
        for (int i = 1; i < WORKERS_NUMBER_AND_MASTER; i++) {
            #pragma omp critical
            {
                if (workerStatus[i] == 0) {
                    found++;
                }
            }
        }
        if (found >= workersNumber)   // I found enoguh workers, I can assign them to the array
            break;
    }

    int count = 0, found2 = 0;
    for (int i = 1; i < WORKERS_NUMBER_AND_MASTER; i++) {
        #pragma omp critical
        {
            if (workerStatus[i] == 0) {
                freeWorkersArray[count] = i;
                count++;
                if (count == workersNumber) 
                    found2 = 1;
            }
        }
        if (found2 == 1)
            break;
        
    }
    return freeWorkersArray;
}


int getFreeWorker (int workerStatus[MAX_WORKERS], int WORKERS_NUMBER_AND_MASTER){
    while (1) {
        int found = 0;
        for (int i = 1; i < WORKERS_NUMBER_AND_MASTER; i++) {
            #pragma omp critical
            {
                if (workerStatus[i] == 0){
                    found = 1;
                }
            }
            if (found == 1)
                return i;
        }
    }
}

int GetCliNumber(char *line) {
    if (strncmp(line, "CLI", 3) != 0) {
        fprintf(stderr, "Error: Line does not start with 'CLI'.\n");
        exit(1);
    }
    int cliNumber;
    if (sscanf(line + 3, "%d", &cliNumber) != 1) {
        fprintf(stderr, "Error: Failed to extract CLI number.\n");
        exit(1);
    }    
    return cliNumber;
}

void printMatrix (int x, int y, int matrix[x][y], FILE *file){
    fprintf (file, "Matrix:\n");
    for (int i = 0; i < x; i++){
        for (int j = 0; j < y; j++){
            fprintf (file, "%d ", matrix[i][j]);
        }
        fprintf (file, "\n");
    }
}

void getNMatrix (char *filename1, char *filename2, int size, int workersNumber, int matrix[workersNumber][2 * THRESHOLD][size]){
    //TODO
    FILE *file1 = fopen(filename1, "r");
    if (file1 == NULL){
        printf("Error: File not found\n");
        exit(1);
    }
    FILE *file2 = fopen(filename2, "r");
    if (file2 == NULL){
        printf("Error: File not found\n");
        exit(1);
    }
    for (int currentWorker = 0; currentWorker < workersNumber; currentWorker++){ //read THRESHOLD lines from every file
        for (int i = 0; i < THRESHOLD; i++){
            for (int j = 0; j < size; j++){
                fscanf(file1, "%d", &matrix[currentWorker][i][j]);
                fscanf(file2, "%d", &matrix[currentWorker][i + THRESHOLD][j]);
            }
        }
    }
}

void getMatrix (char *filename1, char *filename2, int size, int matrix[2 * size][size], int skip_lines){
    FILE *file1 = fopen(filename1, "r");
    if (file1 == NULL){
        printf("Error: File not found\n");
        exit(1);
    }
    FILE *file2 = fopen(filename2, "r");
    if (file2 == NULL){
        printf("Error: File not found\n");
        exit(1);
    }
    for (int i = 0; i < skip_lines; i++){
        char buffer[1000];
        fgets(buffer, 1000, file1);
        fgets(buffer, 1000, file2);
    }
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            fscanf(file1, "%d", &matrix[i][j]);
        }
    }
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            fscanf(file2, "%d", &matrix[i + size][j]);
        }
    }
    fclose(file1);
    fclose(file2);
}

void getMatrixCol (char *filename1, char *filename2, int size, int matrix[2 * size][size], int skip_lines){
    FILE *file1 = fopen(filename1, "r");
    if (file1 == NULL){
        printf("Error: File not found\n");
        exit(1);
    }
    FILE *file2 = fopen(filename2, "r");
    if (file2 == NULL){
        printf("Error: File not found\n");
        exit(1);
    }
    for (int i = 0; i < skip_lines; i++){
        char buffer[1000];
        fgets(buffer, 1000, file1);
        fgets(buffer, 1000, file2);
    }
    int matrix_backup[size][size];
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            fscanf(file1, "%d", &matrix[i][j]);
            fscanf(file2, "%d", &matrix_backup[i][j]);
        }
    }
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            matrix[i + size][j] = matrix_backup[j][i];
        }
    }
    fclose(file1);
    fclose(file2);
}

void getMatrix2 (char *filename1, char *filename2, int size, int matrix[size][size], int skip_lines){
    FILE *file1 = fopen(filename1, "r");
    if (file1 == NULL){
        printf("Error: File not found\n");
        exit(1);
    }
    FILE *file2 = fopen(filename2, "r");
    if (file2 == NULL){
        printf("Error: File not found\n");
        exit(1);
    }
    printf("\n");
    for (int i = 0; i < skip_lines; i++){
        char buffer[1000];
        fgets(buffer, 1000, file1);
        printf ("buffer 1 :%s\n", buffer);
        fgets(buffer, 1000, file2);
        printf ("buffer 2 :%s\n", buffer);
    }
    for (int i = 0; i < size/2; i++){
        for (int j = 0; j < size; j++){
            fscanf(file1, "%d", &matrix[i][j]);
            printf ("%d ", matrix[i+skip_lines][j]);
            fscanf(file2, "%d", &matrix[i+size/2][j]);
            printf ("%d ", matrix[i+skip_lines][j]);
        }
    }
    /*for (int i = 0 ; i < size; i++){
        for (int j = 0; j < size; j++){
            printf ("%d ", matrix[i][j]);
        }
        printf ("\n");
    }*/
    fclose(file1);
    fclose(file2);
}

char* GetCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t now = tv.tv_sec;              
    struct tm *local = localtime(&now);

    char *timeString = malloc(30);
    if (timeString == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory for time string.\n");
        exit(EXIT_FAILURE);
    }

    snprintf(timeString, 30, "%02d:%02d:%02d.%03ld", 
             local->tm_hour, 
             local->tm_min, 
             local->tm_sec, 
             tv.tv_usec / 1000);

    return timeString;
}

bool isPrime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

int PRIMES(int N) {
    int count = 0;
    for (int i = 2; i <= N; i++) {
        if (isPrime(i)) count++;
    }
    return count;
}

int PRIMEDIVISORS(int N) {
    int count = 0;
    for (int i = 2; i <= N; i++) {
        if (N % i == 0 && isPrime(i)) count++;
    }
    return count;
}

void swap(char *x, char *y) {
    char temp = *x;
    *x = *y;
    *y = temp;
}

void generateAnagramsHelper(char *s, int l, int r, char **result, size_t *bufferSize) {
    if (l == r) {
        size_t currentLength = strlen(*result);
        size_t newLength = currentLength + strlen(s) + 2;

        if (newLength > *bufferSize) {
            *bufferSize *= 2;
            *result = realloc(*result, *bufferSize);
            if (*result == NULL) {
                fprintf(stderr, "Error reallocating memory.\n");
                exit(EXIT_FAILURE);
            }
        }

        strcat(*result, s);
        strcat(*result, "\n");
    } else {
        for (int i = l; i <= r; i++) {
            swap(&s[l], &s[i]);                 
            generateAnagramsHelper(s, l + 1, r, result, bufferSize); 
            swap(&s[l], &s[i]);          
        }
    }
}

char* generateAnagrams(char *s) {
    size_t initialBufferSize = 1024; 
    char *result = malloc(initialBufferSize);
    if (result == NULL) {
        fprintf(stderr, "Error allocating memory.\n");
        exit(EXIT_FAILURE);
    }
    result[0] = '\0';

    generateAnagramsHelper(s, 0, strlen(s) - 1, &result, &initialBufferSize);

    return result;
}

//implement log for master: time moment for each command when has been received, when it has been dispatched to a worker server, and when it has been finished

int main(int argc, char *argv[]) {
    printf("Starting...\n");
    int WORKERS_NUMBER_AND_MASTER, taskid, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &WORKERS_NUMBER_AND_MASTER);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Get_processor_name(hostname, &len);

    if (taskid == MASTER) {  // ************ MASTER CODE ************
        FILE *debug = fopen("debug.txt", "w");
        if (debug == NULL) {
            printf("Error: Unable to open debug file\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fprintf(debug, "Debug file opened\n");
        printf("MASTER: Number of MPI tasks is: %d\n", WORKERS_NUMBER_AND_MASTER);

        if (WORKERS_NUMBER_AND_MASTER < 2) {
            printf("Error: At least one worker is required.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Open the command file
        char* commandFile = argv[1];
        FILE *fp = fopen(commandFile, "r");
        if (fp == NULL) {
            printf("Error: File '%s' not found\n", commandFile);
            perror("Error: ");
            MPI_Abort(MPI_COMM_WORLD, 1); // Terminate if file is not found
        }

        printf("Command file: %s\n", commandFile);

        char tasks[MAX_TASKS][100]; // To store lines from the file
        int taskCount = 0;

        FILE *fileHandles[MAX_TASKS] = {NULL}; // Array of file handles

        char line[MAX_LINE_LENGTH];

        while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) { //Read the command file and put each line in tasks array
            line[strcspn(line, "\n")] = '\0'; // Remove newline
            if (strlen(line) == 0) continue; // Skip empty lines

            // Check if it's a CLI command
            if (strncmp(line, "CLI", 3) == 0) {  
                char cliNumber[10];
                sscanf(line, "CLI%s", cliNumber);// Extract the CLI number

                // Create a filename for this CLI command
                char filename[100];
                snprintf(filename, sizeof(filename), "CLI%s_output.txt", cliNumber);

                // Open the file if it hasn't been opened yet
                if (fileHandles[taskCount] == NULL) {
                    fileHandles[taskCount] = fopen(filename, "w");
                    if (fileHandles[taskCount] == NULL) {
                        perror("Error opening file for writing");
                        return 1;
                    }
                }
            }
            taskCount++;
        }
        fclose(fp);

        fp = fopen(commandFile, "r"); //reopen the file to read the commands
        if (fp == NULL) {
            printf("Error: File '%s' not found\n", commandFile);
            perror("Error: ");
            MPI_Abort(MPI_COMM_WORLD, 1); // Terminate if file is not found
        }

    
        int tasksRemaining = taskCount; // Total tasks remaining
        int workerStatus[MAX_WORKERS] = {0}; // Track worker availability (1 = busy, 0 = free)
        double start_time = omp_get_wtime();
        FILE *logFile = fopen("log.txt", "w");

        #pragma omp parallel num_threads(2)
        {
            // Thread 0: Send tasks to workers
            if (omp_get_thread_num() == 0) {
                while (tasksRemaining > 0) {
                    if (fgets (line, MAX_LINE_LENGTH, fp) == NULL){ 
                        printf("MASTER: No more tasks to process\n");
                        break;
                    }    
                    line[strcspn(line, "\n")] = '\0'; // Remove newline
                    if (strlen(line) == 0) continue; 
                    printf ("MASTER: Processing task '%s'\n", line);
                    char *currentTime = GetCurrentTime();
                    fprintf(logFile, "Task '%s' received at %s\n", line, currentTime);
                    free(currentTime); 

                         if (strstr (line, "WAIT") != NULL) { // WAIT COMMAND
                            fprintf(debug, "WAIT command found\n");
                            int waitTime = atoi(line + 5); 
                            printf("MASTER: Waiting for %d seconds\n", waitTime);
                            sleep(waitTime);
                            #pragma omp critical
                            {
                                tasksRemaining--;
                            }
                        }
                        else if (strstr (line, "MATRIXADD") != NULL) {     // *********** MATRIX ADD ***********
                            int CliNumber= GetCliNumber(line);
                            fprintf(debug, "MATRIXADD command found\n");  
                            printf("MASTER: MATRIXADD command found\n");
                            printf ("%s", line);
                            int size;
                            sscanf(line, "%*s %*s %d", &size);
                            char file1[100], file2[100];
                            sscanf(line, "%*s %*s %*s %s %s", file1, file2);
                            //FILE *Matrix1 = fopen(file1, "r");
                            //FILE *Matrix2 = fopen(file2, "r");
                            if (size < THRESHOLD){ //    *********** SEND TO 1 WORKER ***********
                                fprintf(debug, "Size: %d\n", size);
                                int tag = 100 + CliNumber;
                                int m[2* size][size];
                                getMatrix(file1, file2, size, m, 0);
                                printMatrix(2*size, size, m, debug);
                                //check for a free worker and send the matrix task
                                int found1 = 0;
                                while (found1 == 0) {
                                    //printf ("MASTER: waiting for a free worker...\n");
                                    for (int i = 1; i < WORKERS_NUMBER_AND_MASTER; i++) {
                                        #pragma omp critical
                                        {
                                            if (workerStatus[i] == 0){
                                                //printf ("MASTER: Free worker found\n");
                                                found1 = 1;
                                            }
                                            //printf ("MASTER: checking worker %d\n", i);
                                        }
                                        if (found1 == 1)
                                            break;      
                                    }
                                }
                                int found2 = 0;
                                for (int i = 1; i < WORKERS_NUMBER_AND_MASTER; i++) { //loop through workers to find a free worker to send the task
                                    
                                    #pragma omp critical
                                    {
                                        if (workerStatus[i] == 0)
                                        found2 =1;
                                    }
                                    if (found2 == 1) { 
                                        printf("MASTER: Sending task '%s' to worker %d\n", line, i);
                                        char *currentTime = GetCurrentTime();
                                        fprintf(logFile, "Task '%s' dispatched to worker %d at %s\n", line, i, currentTime);
                                        //printMatrix(2*size, size, m, debug);
                                        free(currentTime); // Don't forget to free the allocated memory
                                        MPI_Send(m, size * 2 * size, MPI_INT, i, tag, MPI_COMM_WORLD);
                                        #pragma omp critical
                                        {
                                            workerStatus[i] = 1;
                                        }
                                        break;
                                    }
                                }
                            }
                            else{  // SEND TO N WORKERS
                                int workersNumber = size / THRESHOLD;  // size % THRESHOLD == 0 MUST BE TRUE and workersNumber < WORKERS_NUMBER_AND_MASTER EX: THRESHOLD = 5, size = 15 => workersNumber = 3
                                //create the matrixes that need to be sent
                                //wait for workerNumber workers to be free
                                //send the matrixes to the workers
                                //specific tags for each worker
                                fprintf(debug, "Size: %d\n", size);
                                int tag = 10000 + CliNumber*100 + workersNumber*10; // I add the cli number and the number of workers to the tag
                                int m[workersNumber][2 * THRESHOLD][size];

                                getNMatrix(file1, file2, size, workersNumber, m);
                                
                                int* freeWorkerArray = getNFreeWorkers (workerStatus, WORKERS_NUMBER_AND_MASTER, workersNumber);

                                for (int i = 0; i < workersNumber; i++){
                                    printf("MASTER: Sending task '%s' to worker %d\n", line, freeWorkerArray[i]);
                                    char *currentTime = GetCurrentTime();
                                    fprintf(logFile, "Task '%s' dispatched to worker %d at %s\n", line, freeWorkerArray[i], currentTime);
                                    //printMatrix(2*size, size, m[i], debug);
                                    free(currentTime); // Don't forget to free the allocated memory
                                    int sendArray[2 * THRESHOLD][size];
                                    for (int j = 0; j < 2 * THRESHOLD; j++){
                                        for (int k = 0; k < size; k++){
                                            sendArray[j][k] = m[i][j][k];
                                        }
                                    }
                                    MPI_Send(sendArray, size * 2 * THRESHOLD, MPI_INT, freeWorkerArray[i], tag + freeWorkerArray[i], MPI_COMM_WORLD);  //tag + freeWorkerArray[i] to have different tags for each worker
                                    if (i>0){
                                        #pragma omp critical
                                        {
                                            tasksRemaining++;
                                        }
                                    }
                                    #pragma omp critical
                                    {
                                        workerStatus[freeWorkerArray[i]] = 1;
                                    }
                                }
                                free(freeWorkerArray);
                            }
                        }
                        else if (strstr (line, "MATRIXMULT") != NULL){             // ********** MATRIX MULT **********
                            int CliNumber= GetCliNumber(line);
                            fprintf(debug, "MATRIXADD command found\n");  
                            printf("MASTER: MATRIXADD command found\n");
                            printf ("%s", line);
                            int size;
                            sscanf(line, "%*s %*s %d", &size);
                            char file1[100], file2[100];
                            sscanf(line, "%*s %*s %*s %s %s", file1, file2);
                            //FILE *Matrix1 = fopen(file1, "r");
                            //FILE *Matrix2 = fopen(file2, "r");
                            int worker_number = getFreeWorker (workerStatus, WORKERS_NUMBER_AND_MASTER);
                            fprintf(debug, "Size: %d\n", size);
                            int tag = 300 + CliNumber;
                            int m[2* size][size];
                            getMatrixCol(file1, file2, size, m, 0);
                            printf("MASTER: Sending task '%s' to worker %d\n", line, worker_number);
                            char *currentTime = GetCurrentTime();
                            fprintf(logFile, "Task '%s' dispatched to worker %d at %s\n", line, worker_number, currentTime);
                            printMatrix(2*size, size, m, debug);
                            free(currentTime); // Don't forget to free the allocated memory
                            MPI_Send(m, size * 2 * size, MPI_INT, worker_number, tag, MPI_COMM_WORLD);
                            #pragma omp critical
                            {
                                workerStatus[worker_number] = 1;
                            }
                        }
                        else{ //send for PRIMES, PRIMEDIVISORS, ANAGRAMS
                            fprintf(debug, "PRIMES, PRIMEDIVISORS, ANAGRAMS command found\n");
                            int found1 = 0;
                            while (found1 == 0) {
                                //printf ("MASTER: waiting for a free worker...\n");
                                for (int i = 1; i < WORKERS_NUMBER_AND_MASTER; i++) {
                                    #pragma omp critical
                                    {
                                        if (workerStatus[i] == 0){
                                            //printf ("MASTER: Free worker found\n");
                                            found1 = 1;
                                        }
                                        //printf ("MASTER: checking worker %d\n", i);
                                    }
                                    if (found1 == 1)
                                        break;      
                                }
                            }
                            int found2 = 0;
                            for (int i = 1; i < WORKERS_NUMBER_AND_MASTER; i++) { //loop through workers to find a free worker to send the task
                                
                                #pragma omp critical
                                {
                                    if (workerStatus[i] == 0)
                                    found2 =1;
                                }
                                if (found2 == 1) { 
                                    printf("MASTER: Sending task '%s' to worker %d\n", line, i);
                                    char *currentTime = GetCurrentTime();
                                    fprintf(logFile, "Task '%s' dispatched to worker %d at %s\n", line, i, currentTime);
                                    free(currentTime); // Don't forget to free the allocated memory
                                    MPI_Send(line, strlen(line) + 1, MPI_CHAR, i, 1, MPI_COMM_WORLD);
                                    #pragma omp critical
                                    {
                                        workerStatus[i] = 1;
                                    }
                                    break;
                                }
                            }
                        }
                }
            }

            // Thread 1: Receive availability messages from workers
            else if (omp_get_thread_num() == 1) {
                while (tasksRemaining > 0) {
                    MPI_Status status;

                    // Probe to determine the size of the incoming message
                    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                    if (status.MPI_TAG == 1){  //the tag is 1 , so ANAGRAMS, PRIMES, PRIMEDIVISORS
                        // Get the size of the incoming message
                        int responseSize;
                        MPI_Get_count(&status, MPI_CHAR, &responseSize);

                        // Allocate memory for the response
                        char *response = malloc(responseSize + 1); // +1 for null terminator
                        if (response == NULL) {
                            fprintf(stderr, "Error: Unable to allocate memory for response.\n");
                            MPI_Abort(MPI_COMM_WORLD, 1);
                        } 

                        // Receive the message into the allocated buffer
                        MPI_Recv(response, responseSize, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        response[responseSize] = '\0'; // Ensure null-termination

                        int workerId = status.MPI_SOURCE;
                        printf("MASTER: RECEIVED COMPLETION FROM WORKER %d: %s\n", workerId, response);

                        char* currentTime = GetCurrentTime();
                        fprintf(logFile, "Worker %d completed the task at %s\n",  workerId, currentTime);
                        free(currentTime); // Don't forget to free the allocated memory

                        // Process the response
                        char responseCopy[responseSize + 1];
                        strcpy(responseCopy, response);

                        // Extract the first token from the copy
                        char *token = strtok(responseCopy, " ");
                        char filename[100];
                        snprintf(filename, sizeof(filename), "%s_output.txt", token);

                        // Open the file using the generated filename
                        FILE *fp = fopen(filename, "a");
                        if (fp == NULL) {
                            printf("Error: Could not open file '%s'\n", filename);
                            perror("Error");
                            free(response); // Free the dynamically allocated response
                            MPI_Abort(MPI_COMM_WORLD, 1);
                        }

                        // Write the full original response to the file
                        fprintf(fp, "%s\n", response);
                        fclose(fp); // Ensure the file is closed properly
                        
                        #pragma omp critical
                        {
                            tasksRemaining--;
                            workerStatus[workerId] = 0; // Mark worker as free
                        }

                        // Free the dynamically allocated memory
                        free(response);
                    }
                    else if (status.MPI_TAG >= 100 && status.MPI_TAG < 500){ // between 100 and 500, so MATRIX ADD and MATRIX MULT for 1 worker
                        
                        int recieve_size;
                        MPI_Get_count(&status, MPI_INT, &recieve_size);
                        int *recieveData = NULL;
                        recieveData = (int *) malloc (recieve_size * sizeof(int));
                        MPI_Recv(recieveData, recieve_size, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        /*
                        printf ("AAAAAAAAAAAAAAAAAA MASTER RECIEVED THIS: \n");
                        for (int i = 0 ; i < recieve_size; i++){
                            printf ("%d ", recieveData[i]);
                        }
                        */
                        int workerID = status.MPI_SOURCE;
                        printf("MASTER: RECEIVED COMPLETION FROM WORKER %d\n", workerID);
                        char *currentTime = GetCurrentTime();
                        fprintf(logFile, "Worker %d completed the task at %s\n",  workerID, currentTime);
                        free(currentTime); // Don't forget to free the allocated memory
                        int CliNumberOutput = status.MPI_TAG % 100;
                        char filename[100];
                        sprintf(filename, "CLI%d_output.txt", CliNumberOutput);
                        FILE *fp = fopen(filename, "a");
                        if (status.MPI_TAG <300){
                            fprintf(fp, "Matrix Addition with recieve size %d\n", recieve_size);
                        }
                        if (status.MPI_TAG >=300){
                            fprintf(fp, "Matrix Multiplication with recieve size %d\n", recieve_size);
                        }
                        
                        if (fp == NULL) {
                            printf("Error: Could not open file %s\n", filename);
                            perror("Error");
                            free(recieveData); // Free the dynamically allocated response
                            MPI_Abort(MPI_COMM_WORLD, 1);
                        }
                        if (status.MPI_TAG <200){
                            for (int i = 0 ; i < recieve_size; i++){
                                fprintf(fp, "%d ", recieveData[i]);
                                if ((i + 1) % (int)sqrt(recieve_size) == 0){
                                    fprintf(fp, "\n");
                                }
                            }
                        }
                        if (status.MPI_TAG >=300 && status.MPI_TAG < 400){
                            for (int i = 0 ; i < recieve_size; i++){
                                fprintf(fp, "%d ", recieveData[i]);
                                if ((i + 1) % (int)sqrt(recieve_size) == 0){
                                    fprintf(fp, "\n");
                                }
                            }
                        }

                        fclose(fp); // Ensure the file is closed properly
                        #pragma omp critical
                        {
                            tasksRemaining--;
                            workerStatus[workerID] = 0; // Mark worker as free
                        }
                        free(recieveData);
                    }
                    else if (status.MPI_TAG >= 10000 && status.MPI_TAG < 20000){  // PRINT MATRIX ADD FOR N WORKERS
                        int recieve_size;
                        MPI_Get_count(&status, MPI_INT, &recieve_size);
                        int *recieveData = NULL;
                        recieveData = (int *) malloc (recieve_size * sizeof(int));
                        MPI_Recv(recieveData, recieve_size, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        int workerID = status.MPI_SOURCE;

                        /*printf ("AAAAAAAAAAAAAAAAAA MASTER RECIEVED THIS: \n");
                        for (int i = 0 ; i < recieve_size; i++){
                            printf ("%d ", recieveData[i]);
                        }*/
                        
                        printf("MASTER: RECEIVED COMPLETION FROM WORKER %d\n", workerID);
                        char *currentTime = GetCurrentTime();
                        fprintf(logFile, "*******Worker %d completed part %d of the task at******** %s\n",  workerID, status.MPI_TAG%10,currentTime);
                        free(currentTime); // Don't forget to free the allocated memory
                        int CliNumberOutput = (status.MPI_TAG/100) % 100;
                        char filename[100];
                        sprintf(filename, "CLI%d_output.txt", CliNumberOutput);
                        FILE *fp = fopen(filename, "a");
                        if (status.MPI_TAG % 10 == 1){
                            fprintf(fp, "Matrix Addition with recieve size %d\n", recieve_size);
                        }
                        fprintf (fp, "Part number %d of matrix addtion\n", status.MPI_TAG % 10);
                        if (fp == NULL) {
                            printf("Error: Could not open file %s\n", filename);
                            perror("Error");
                            free(recieveData); // Free the dynamically allocated response
                            MPI_Abort(MPI_COMM_WORLD, 1);
                        }
                        for (int i = 0 ; i < recieve_size; i++){
                            fprintf(fp, "%d ", recieveData[i]);
                            if ((i + 1) % (recieve_size/THRESHOLD) == 0){
                                fprintf(fp, "\n");
                            }
                        }
                        fclose(fp); // Ensure the file is closed properly
                        #pragma omp critical
                        {
                            tasksRemaining--;
                            workerStatus[workerID] = 0; // Mark worker as free
                        }
                        free(recieveData);
                    }
                }
            }

        }

        // Notify workers that there are no more commands (send empty strings)
        for (int i = 1; i < WORKERS_NUMBER_AND_MASTER; i++) {
            MPI_Send("", 1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
        for (int i = 0 ; i < taskCount; i++) {
            if (fileHandles[i] != NULL) {
                //printf("Closing file %d\n", i);
                fclose(fileHandles[i]);
            }
        }
        double end_time = omp_get_wtime();
        double time_spent = end_time - start_time;

        fprintf(logFile, "Time spent: %f seconds\n", time_spent);
        printf("Time spent: %f seconds\n\n", time_spent);
        fclose(debug);
    }else { 
    // ************ WORKER CODE ************
    char task[100];

    while (1) {
        MPI_Status status;

        // Probe to check if there is a message
        MPI_Probe(MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == 1){  //the tag is o , so ANAGRAMS, PRIMES, PRIMEDIVISORS
             MPI_Recv(task, 100, MPI_CHAR, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (strlen(task) == 0) break; // Empty string indicates termination

            printf("Worker %d on %s received task: %s\n", taskid, hostname, task);

            // Allocate initial memory for response dynamically
            size_t responseBufferSize = 1024; // Start with 1 KB
            char *response = malloc(responseBufferSize);
            if (response == NULL) {
                fprintf(stderr, "Error: Unable to allocate memory for response.\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            response[0] = '\0'; // Initialize as an empty string

            char *p = strtok(task, " ");
            strcat(response, p);
            strcat(response, " "); // Append CLI_ to response
            p = strtok(NULL, " ");

            if (strcmp(p, "PRIMES") == 0) {
                p = strtok(NULL, " ");
                int N = atoi(p);
                int count = PRIMES(N);
                char countStr[100];
                sprintf(countStr, "%d", count);

                if (strlen(response) + strlen(countStr) + 1 > responseBufferSize) {
                    responseBufferSize *= 2; // Double the buffer size
                    response = realloc(response, responseBufferSize);
                    if (response == NULL) {
                        fprintf(stderr, "Error: Unable to reallocate memory for response.\n");
                        MPI_Abort(MPI_COMM_WORLD, 1);
                    }
                }

                strcat(response, countStr);
            } 
            else if (strcmp(p, "PRIMEDIVISORS") == 0) {
                p = strtok(NULL, " ");
                int N = atoi(p);
                int count = PRIMEDIVISORS(N);
                char countStr[100];
                sprintf(countStr, "%d", count);

                if (strlen(response) + strlen(countStr) + 1 > responseBufferSize) {
                    responseBufferSize *= 2; // Double the buffer size
                    response = realloc(response, responseBufferSize);
                    if (response == NULL) {
                        fprintf(stderr, "Error: Unable to reallocate memory for response.\n");
                        MPI_Abort(MPI_COMM_WORLD, 1);
                    }
                }

                strcat(response, countStr);
            } 
            else if (strcmp(p, "ANAGRAMS") == 0) {
                p = strtok(NULL, " "); // Extract the word to generate anagrams for
                char *anagrams = generateAnagrams(p);

                size_t anagramsLength = strlen(anagrams);

                // Resize the response buffer if necessary
                if (strlen(response) + anagramsLength + 1 > responseBufferSize) {
                    while (strlen(response) + anagramsLength + 1 > responseBufferSize) {
                        responseBufferSize *= 2; // Double the buffer size until it fits
                    }
                    response = realloc(response, responseBufferSize);
                    if (response == NULL) {
                        fprintf(stderr, "Error: Unable to reallocate memory for response.\n");
                        free(anagrams); // Free the anagrams buffer before exiting
                        MPI_Abort(MPI_COMM_WORLD, 1);
                    }
                }

                strcat(response, "\n");
                strcat(response, anagrams); // Append the anagrams to the response
                free(anagrams); // Free the dynamically allocated memory for anagrams
            }

            // Send the dynamically allocated response to master
            MPI_Send(response, strlen(response) + 1, MPI_CHAR, MASTER, 1, MPI_COMM_WORLD);

            free(response); // Free the dynamically allocated response memory
        }
        else if (status.MPI_TAG >= 100 && status.MPI_TAG <500){ //MATRIX ADD  and MULT FOR 1 WORKER
            int recieve_size;
            MPI_Get_count(&status, MPI_INT, &recieve_size);
            int size = recieve_size / 2;
            int matrix [recieve_size/2];
            int *recieveData = NULL;
            recieveData = (int *) malloc (recieve_size * sizeof(int));
            MPI_Recv(recieveData, recieve_size, MPI_INT, MASTER, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);//first half of recieveData is the first matrix and the second half is the second matrix
            printf("Worker %d on %s received matrix addition task\n", taskid, hostname);
            if (status.MPI_TAG < 300){  // ADD
                for (int i = 0 ; i < recieve_size ; i++){
                //printf ("%d ", recieveData[i]);
                matrix[i] = recieveData[i] + recieveData[i + recieve_size / 2];
                }
            }
            if (status.MPI_TAG >= 300){ // MULT
                int n = sqrt(size);
                int cont = 0;
                int x = 0;
                //printf ("%d ", recieveData[i]);
                for (int i = 0 ; i < recieve_size/2; i = i + n){
                    for (int j = recieve_size/2; j < recieve_size; j = j + n){
                        x = 0;
                        for (int k = 0; k < n; k++){
                            x = x + recieveData[i + k] * recieveData[j + k];
                        }
                        matrix[cont++] = x;
                    }
                }
            }

            free(recieveData);
            MPI_Send(matrix, recieve_size/2, MPI_INT, MASTER, status.MPI_TAG, MPI_COMM_WORLD); //send the result to the master
        }
        else if (status.MPI_TAG >=10000 && status.MPI_TAG < 20000){ // 10000 - 19999 MATRIX ADD FOR N WORKERS
            int recieve_size;
            MPI_Get_count(&status, MPI_INT, &recieve_size);
            int size = recieve_size / 2;
            int matrix [recieve_size/2];
            int *recieveData = NULL;
            recieveData = (int *) malloc (recieve_size * sizeof(int));
            MPI_Recv(recieveData, recieve_size, MPI_INT, MASTER, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);//first half of recieveData is the first matrix and the second half is the second matrix
            printf("Worker %d on %s received matrix addition task\n", taskid, hostname);
            for (int i = 0 ; i < recieve_size ; i++){
                //printf ("%d ", recieveData[i]);
                matrix[i] = recieveData[i] + recieveData[i + recieve_size / 2];
            }
            MPI_Send (matrix, size, MPI_INT, MASTER, status.MPI_TAG, MPI_COMM_WORLD);//send the result to the master

        }
        else if (status.MPI_TAG == 0){  //  WORKER EXIT
            break;
        }
    }

    printf("Worker %d on %s: No more tasks. Exiting.\n", taskid, hostname);
}
    MPI_Finalize();
    return 0;
}
