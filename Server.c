// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024
#define PASSWORD "admin123"

void *handle_client(void *arg) {
    int new_socket = *((int *)arg);
    free(arg);

    char buffer[BUFFER_SIZE] = {0};
    char cmd_with_err[BUFFER_SIZE + 10];

    // Authentication
    recv(new_socket, buffer, BUFFER_SIZE, 0);
    if (strncmp(buffer, PASSWORD, strlen(PASSWORD)) != 0) {
        send(new_socket, "Authentication failed.\n", 24, 0);
        close(new_socket);
        pthread_exit(NULL);
    }

    send(new_socket, "Authenticated. You may now send commands.\n", 43, 0);

    while (1) {
        printf("[DEBUG] Waiting to receive command...\n");  // To see loop is active

        memset(buffer, 0, BUFFER_SIZE);
        int valread = recv(new_socket, buffer, BUFFER_SIZE, 0);
        if (valread <= 0) break;

        buffer[strcspn(buffer, "\n")] = 0;  // Remove newline

        printf("Command received: %s\n", buffer);  // <-- ✅ Your required print

        if (strncmp(buffer, "exit", 4) == 0) {
            send(new_socket, "Session closed.\n", 16, 0);
            break;
        }

        snprintf(cmd_with_err, sizeof(cmd_with_err), "%s 2>&1", buffer);
        FILE *fp = popen(cmd_with_err, "r");
        if (!fp) {
            send(new_socket, "Failed to execute command.\n", 28, 0);
            continue;
        }

        char output[BUFFER_SIZE];
        while (fgets(output, sizeof(output), fp) != NULL) {
            send(new_socket, output, strlen(output), 0);
        }

        pclose(fp);
        send(new_socket, "\n>> ", 4, 0);
    }

    close(new_socket);
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket, *new_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, MAX_CLIENTS);

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        new_sock = malloc(sizeof(int));
        *new_sock = new_socket;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, new_sock);
        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}
