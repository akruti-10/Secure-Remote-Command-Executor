client code
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }
// Password authentication
    char password[BUFFER_SIZE];
    printf("Enter password: ");
    fgets(password, BUFFER_SIZE, stdin);
    password[strcspn(password, "\n")] = '\0';
    send(sock, password, strlen(password), 0);

    recv(sock, buffer, BUFFER_SIZE, 0);
    printf("%s", buffer);
    if (strstr(buffer, "failed")) {
        close(sock);
        return 0;
    }

    while (1) {
        printf("Enter command (or 'exit'): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        send(sock, buffer, strlen(buffer), 0);
        if (strncmp(buffer, "exit", 4) == 0)
            break;

        memset(buffer, 0, BUFFER_SIZE);
        while (1) {
            int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
 buffer[bytes] = '\0';
            printf("%s", buffer);
            if (strstr(buffer, "\n>> ") || bytes == 0)
                break;
            memset(buffer, 0, BUFFER_SIZE);
        }
    }

    close(sock);
    return 0;
}
