#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> 

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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <commandFile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    clock_t start = clock();

    char *commandFile = argv[1];
    FILE *fp = fopen(commandFile, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    printf("Command file: %s\n", commandFile);

    // Process commands
    char line[100], w1[100] = "", w2[100] = "", w3[100] = "";
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] == '\n') continue; 

        line[strcspn(line, "\n")] = '\0';

        char *p = strtok(line, " ");
        if (p != NULL) {
            strcpy(w1, p);
        }

        if (strcmp(w1, "WAIT") == 0) {
            p = strtok(NULL, " ");
            if (p != NULL) {
                int waitTime = atoi(p);
                printf("Waiting for %d seconds\n", waitTime);
                sleep(waitTime);
            } else {
                printf("Error: 'WAIT' command requires a number.\n");
            }
        } else {
            p = strtok(NULL, " ");
            if (p != NULL) {
                strcpy(w2, p);
            }
            p = strtok(NULL, " ");
            if (p != NULL) {
                strcpy(w3, p);
            }

            printf("Command: %s %s %s\n", w1, w2, w3);
            if (strcmp(w2, "PRIMES") == 0) {
                printf("PRIMES(%d) = %d\n", atoi(w3), PRIMES(atoi(w3)));
            } else if (strcmp(w2, "PRIMEDIVISORS") == 0) {
                printf("PRIMEDIVISORS(%d) = %d\n", atoi(w3), PRIMEDIVISORS(atoi(w3)));
            } else if (strcmp(w2, "ANAGRAMS") == 0) {
               char *anagrams = generateAnagrams(w3);
                //printf("ANAGRAMS(%s) = %s\n", w3, anagrams);
            }
            else{
                printf("Error: Invalid command\n");
            }
        }
    }

    fclose(fp);

    clock_t end = clock();
    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Execution time: %.2f seconds\n", elapsed_time);

    return 0;
}