#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#define PORT 8081
#define MAX_CLIENTS 4000
#define MAX_BUFFER_SIZE 1024

unsigned long long fact(unsigned long long n) {
    if (n > 20) {
        return 2432902008176640000; // Factorial of 20
    }

    unsigned long long result = 1;
    for (unsigned long long i = 1; i <= n; i++) {
        result *= i;
    }
    return result;
}

int main() {
    int server_socket, client_sockets[MAX_CLIENTS];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation error");
        exit(1);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind error");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listen error");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    fd_set readfds, masterfds;
    FD_ZERO(&masterfds);
    FD_SET(server_socket, &masterfds);

    int max_socket = server_socket;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    while (1) {
        readfds = masterfds;

        if (select(max_socket + 1, &readfds, NULL, NULL, NULL) == -1) {
            perror("Select error");
            exit(1);
        }

        if (FD_ISSET(server_socket, &readfds)) {
            // Accept a new client connection
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

            if (client_socket == -1) {
                perror("Accept error");
                continue;
            }

            // Add the new client socket to the array
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_socket;
                    if (client_socket > max_socket) {
                        max_socket = client_socket;
                    }
                    break;
                }
            }
        }

        // Handle data from clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int client_socket = client_sockets[i];
            if (FD_ISSET(client_socket, &readfds)) {
                char buffer[MAX_BUFFER_SIZE];
                int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

                if (bytes_received <= 0) {
                    // Client disconnected
                    close(client_socket);
                    FD_CLR(client_socket, &masterfds);
                    client_sockets[i] = 0;
                } else {
                    // Extract the number from the received payload and compute the factorial
                    unsigned long long n;
                    sscanf(buffer, "%llu", &n);
                    unsigned long long result = fact(n);

                    // Send the factorial result back to the client
                    char response[MAX_BUFFER_SIZE];
                    sprintf(response, "Factorial of %llu is %llu\n", n, result);
                    send(client_socket, response, strlen(response), 0);
                }
            }
        }
    }

    close(server_socket);

    return 0;
}
