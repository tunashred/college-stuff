#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#define MYPORT 5000
#define BACKLOG 10

#define MAXDATASIZE 1024

void send_file_contents(int client_fd, const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Error seeking file");
        fclose(file);
        return;
    }
    int file_size = ftell(file);
    if (file_size < 0) {
        perror("Error getting file size");
        fclose(file);
        return;
    }
    printf("file size: %d\n", file_size);
    rewind(file);

    // uint32_t net_file_size = htonl(file_size);
    // if (send(client_fd, &net_file_size, sizeof(net_file_size), 0) == -1) {
    //     perror("Error sending file size");
    //     fclose(file);
    //     return;
    // }

    char buffer[MAXDATASIZE];
    int total_sent = 0, bytes_sent, bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) { // need to add null terminator to each buffer sent
        total_sent = 0;
        bytes_sent = send(client_fd, buffer, sizeof(buffer), 0);
        if (bytes_sent <= 0) {
                perror("Error sending file data");
                fclose(file);
                return;
        }
        // while (total_sent < bytes_read) {
        //     bytes_sent = send(client_fd, buffer + total_sent, bytes_read - total_sent, 0);
        //     if (bytes_sent <= 0) {
        //         perror("Error sending file data");
        //         fclose(file);
        //         return;
        //     }
        //     total_sent += bytes_sent;
        // }
    }
    printf("File sent\n");

    fclose(file);
}

void send_command_output(int client_fd, const char *command) {
    FILE *pipe = popen(command, "r");
    if (pipe == NULL) {
        perror("Error executing command");
        return;
    }

    char buffer[MAXDATASIZE];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        send(client_fd, buffer, strlen(buffer), 0);
    }
    pclose(pipe);
}

char* create_menu(int* size) {
    const char *reset = "\033[0m";
    const char *red = "\033[1;31m";
    const char *green = "\033[1;32m";
    const char *yellow = "\033[1;33m";
    const char *blue = "\033[1;34m";
    const char *cyan = "\033[1;36m";

    int required_size = snprintf(NULL, 0,
                 "\n%s==================================%s\n"
                 "%sSystem Information Menu%s\n"
                 "%s==================================%s\n"
                 "%s1.%s Print /proc/cpuinfo\n"
                 "%s2.%s Print /proc/meminfo\n"
                 "%s3.%s Execute lspci\n"
                 "%s4.%s Execute df -h (Disk Usage)\n"
                 "%s5.%s Execute free -h (Memory Usage)\n"
                 "%s6.%s Execute lsscsi (SCSI Devices)\n"
                 "%s0.%s Exit\n"
                 "%s==================================%s\n"
                 "%sEnter your choice: %s",
                 cyan, reset,
                 yellow, reset,
                 cyan, reset,
                 green, reset,
                 green, reset,
                 green, reset,
                 green, reset,
                 green, reset,
                 green, reset,
                 red, reset,
                 cyan, reset,
                 blue, reset);

    char *buffer = (char*) malloc(required_size + 1);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    snprintf(buffer, required_size + 1,
                 "\n%s==================================%s\n"
                 "%sSystem Information Menu%s\n"
                 "%s==================================%s\n"
                 "%s1.%s Print /proc/cpuinfo\n"
                 "%s2.%s Print /proc/meminfo\n"
                 "%s3.%s Execute lspci\n"
                 "%s4.%s Execute df -h (Disk Usage)\n"
                 "%s5.%s Execute free -h (Memory Usage)\n"
                 "%s6.%s Execute lsscsi (SCSI Devices)\n"
                 "%s0.%s Exit\n"
                 "%s==================================%s\n"
                 "%sEnter your choice: %s",
                 cyan, reset,
                 yellow, reset,
                 cyan, reset,
                 green, reset,
                 green, reset,
                 green, reset,
                 green, reset,
                 green, reset,
                 green, reset,
                 red, reset,
                 cyan, reset,
                 blue, reset);

    *size = required_size;
    return buffer;
}

void* thread_job(void* args) {
    int sock_fd = *(int*) args;
    uint32_t net_choice, net_menu_size;
    int bytes = 0;
    char eof_msg[4] = "EOF";

    char* menu = create_menu(&net_menu_size);
    bytes = send(sock_fd, menu, MAXDATASIZE, 0);
    bytes = send(sock_fd, eof_msg, sizeof(eof_msg), 0);
    while (1) {
        printf("Menu sent\n");
        if (bytes == -1) {
            fprintf(stderr, "Could not send message to client!\n");
            return NULL; // what if i make it try send a couple of times before halting connection?
        }
        printf("Waiting for choice...\n");
        bytes = recv(sock_fd, &net_choice, sizeof(net_choice), 0);
        if (bytes) {
            net_choice = ntohl(net_choice);
            printf("Option received: %d\n", net_choice);
        }
        switch (net_choice) {
            case 1:
                send_file_contents(sock_fd, "/proc/cpuinfo");
                break;
            case 2:
                send_file_contents(sock_fd, "/proc/meminfo");
                break;
            case 3:
                send_command_output(sock_fd, "lspci");
                break;
            case 4:
                send_command_output(sock_fd, "df -h");
                break;
            case 5:
                send_command_output(sock_fd, "free -h");
                break;
            case 6:
                send_command_output(sock_fd, "lsscsi");
                break;
            case 0:
                close(sock_fd);
                break;
            default:
                bytes = send(sock_fd, "Invalid choice", 15, 0);
                if (bytes == -1) {
                    fprintf(stderr, "Could not send message to client!\n");
                    return NULL; // what if i make it send a couple of times before halting connection?
                }
                break;
        }
        bytes = send(sock_fd, menu, MAXDATASIZE, 0);
        bytes = send(sock_fd, eof_msg, sizeof(eof_msg), 0);
    }
    return NULL;
}

int main() {
    int sockfd, new_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    int sin_size;
    int yes=1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(MYPORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero), '\0', 8);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))
                                                                    == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sin_size = sizeof(struct sockaddr_in);
    while(1) {
        pthread_t thread;

        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        // would be nice to give time for that thread to be created before creating another one
        pthread_create(&thread, NULL, thread_job, (void*)&new_fd);
        printf("server: conexiune de la: %s\n", inet_ntoa(their_addr.sin_addr));

        pthread_detach(thread);
    }
    close(new_fd);
    close(sockfd);

    return 0;
}
