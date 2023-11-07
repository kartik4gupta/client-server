#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define MAX_CLIENTS 5
#define PORT 8081


// Function to compute the factorial of a number (capped at 20 if n > 20)
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

void handle_client(int client_socket) {
    char buffer[1024];
    int bytes_received;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        // Extract the number from the received payload and compute the factorial
        unsigned long long n;
        sscanf(buffer, "%llu", &n);
        unsigned long long result = fact(n);

        // Send the factorial result back to the client
        char response[1024];
        sprintf(response, "Factorial of %llu is %llu\n", n, result);
        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
    exit(0);
}

int main() {
    int server_socket, client_socket, pid;
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
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr) == -1)) {
        perror("Bind error");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listen error");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept client connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Accept error");
            continue;
        }

        // Fork a new process to handle the client
        pid = fork();
        if (pid == 0) {
            // Child process: Handle the client
            close(server_socket); // Close the server socket in the child process
            handle_client(client_socket);
        } else if (pid > 0) {
            // Parent process: Continue accepting new clients
            close(client_socket); // Close the client socket in the parent process
        } else {
            perror("Fork error");
        }
    }

    close(server_socket);

    return 0;
}