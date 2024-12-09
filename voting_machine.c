/***************************************************************************************
*  Filename: voting_machine.c
*  Description: A multithreaded server that accepts votes from multiple clients, 
*               tracks voting results in real-time,
*               and allows the server operator to view results using the SHOW command.
*  Authors: John Williams and Connor Albrecht
*  Date: 12/8/2024
*  Note: gcc -o voting_machine voting_machine.c 
***************************************************************************************/ 


#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_VOTERS 100
#define MAX_USERNAME_LENGTH 50
#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    char username[MAX_USERNAME_LENGTH];
    int vote; // Example: 1 for Candidate A, 2 for Candidate B, etc.
} Voter;

Voter voters[MAX_VOTERS];
int voterCount = 0;
pthread_mutex_t voterLock;

void addVote(char *username, int vote) {
    pthread_mutex_lock(&voterLock);

    // Check if the voter already exists
    for (int i = 0; i < voterCount; i++) {
        if (strcmp(voters[i].username, username) == 0) {
            printf("Error: User %s has already voted.\n", username);
            pthread_mutex_unlock(&voterLock);
            return;
        }
    }

    // Add a new voter
    if (voterCount < MAX_VOTERS) {
        strcpy(voters[voterCount].username, username);
        voters[voterCount].vote = vote;
        voterCount++;
        printf("Vote recorded successfully for username: %s\n", username);
    } else {
        printf("Error: Maximum voter limit reached.\n");
    }

    pthread_mutex_unlock(&voterLock);
}

void displayResults() {
    int candidate1Votes = 0;
    int candidate2Votes = 0;

    pthread_mutex_lock(&voterLock);
    for (int i = 0; i < voterCount; i++) {
        if (voters[i].vote == 1) {
            candidate1Votes++;
        } else if (voters[i].vote == 2) {
            candidate2Votes++;
        }
    }
    pthread_mutex_unlock(&voterLock);

    printf("\nVoting Results:\n");
    printf("Candidate A: %d votes\n", candidate1Votes);
    printf("Candidate B: %d votes\n", candidate2Votes);
}

void *handleClient(void *clientSocket) {
    int sock = *(int *)clientSocket;
    free(clientSocket);

    char buffer[BUFFER_SIZE];
    char username[MAX_USERNAME_LENGTH];
    int vote;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int readSize = recv(sock, buffer, BUFFER_SIZE, 0);
        if (readSize <= 0) {
            printf("Client disconnected.\n");
            close(sock);
            return NULL;
        }

        if (strncmp(buffer, "RESULTS", 7) == 0) {
            char results[BUFFER_SIZE];
            int candidate1Votes = 0;
            int candidate2Votes = 0;

            pthread_mutex_lock(&voterLock);
            for (int i = 0; i < voterCount; i++) {
                if (voters[i].vote == 1) {
                    candidate1Votes++;
                } else if (voters[i].vote == 2) {
                    candidate2Votes++;
                }
            }
            pthread_mutex_unlock(&voterLock);

            snprintf(results, BUFFER_SIZE, "Candidate A: %d votes\nCandidate B: %d votes\n", candidate1Votes, candidate2Votes);
            send(sock, results, strlen(results), 0);
        } else {
            sscanf(buffer, "%s %d", username, &vote);
            if (vote != 1 && vote != 2) {
                send(sock, "Invalid vote. Please vote for Candidate A (1) or Candidate B (2).\n", 64, 0);
            } else {
                addVote(username, vote);
                send(sock, "Vote recorded successfully.\n", 32, 0);
            }
        }
    }

    return NULL;
}

void *showResultsCommand(void *arg) {
    while (1) {
        char command[10];
        printf("\nType 'SHOW' to display results: ");
        scanf("%s", command);
        if (strcmp(command, "SHOW") == 0) {
            displayResults();
        }
    }
    return NULL;
}

int main() {
    int serverSocket, clientSocket, *newSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    pthread_mutex_init(&voterLock, NULL);

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(serverSocket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Voting server is running on port %d.\n", PORT);

    // Create a thread for showing results via console command
    pthread_t showResultsThread;
    if (pthread_create(&showResultsThread, NULL, showResultsCommand, NULL) != 0) {
        perror("Show results thread creation failed");
        exit(EXIT_FAILURE);
    }

    while ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen)) >= 0) {
        printf("New connection accepted.\n");

        newSocket = malloc(sizeof(int));
        *newSocket = clientSocket;

        pthread_t clientThread;
        if (pthread_create(&clientThread, NULL, handleClient, (void *)newSocket) != 0) {
            perror("Thread creation failed");
        }

        pthread_detach(clientThread);
    }

    close(serverSocket);
    pthread_mutex_destroy(&voterLock);
    return 0;
}
