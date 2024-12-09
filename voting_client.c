/*************************************
 *  Filename: voting_client.c
 *  Description: 
 *  Authors: Connor Albrecht and John Williams
 *  Date: 12/8/24
 *  Note: gcc -o voting_client voting_client.c
 *************************************/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    char username[50];
    int vote;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the voting server.\n");

    while (1) {
        printf("\nEnter your username: ");
        scanf("%s", username);
        printf("Enter your vote (1 for Candidate A, 2 for Candidate B): ");
        scanf("%d", &vote);

        // Prepare and send the vote
        snprintf(buffer, BUFFER_SIZE, "%s %d", username, vote);
        send(sock, buffer, strlen(buffer), 0);

        // Receive server response
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("Server response: %s\n", buffer);

        // Optional: Exit after voting
        printf("Do you want to vote again? (y/n): ");
        char choice;
        scanf(" %c", &choice);
        if (choice == 'n' || choice == 'N') {
            break;
        }
    }

    close(sock);
    return 0;
}