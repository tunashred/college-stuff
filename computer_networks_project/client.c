#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 5000

#define MAXDATASIZE 1024

int receive_message(int sock_fd) {
    char buffer[MAXDATASIZE];
    uint32_t message_length, net_message_length;
    int bytes_received, total_received;

    bytes_received = recv(sock_fd, &net_message_length, sizeof(net_message_length), 0);
    if (bytes_received <= 0) {
        perror("Error receiving message length or server disconnected!\n");
        return -1;
    }

    message_length = ntohl(net_message_length);
    while (total_received < message_length) {
        bytes_received = recv(sock_fd, buffer, MAXDATASIZE - 1, 0);
        if (bytes_received == -1) {
            perror("Error recieving data!\n");
            return -1;
        }
        buffer[bytes_received] = '\0';
        printf("%s", buffer);

        total_received += bytes_received;
    }
    return total_received;
}

int main(int argc, char *argv[]) {
    int sockfd, numbytes;
    struct hostent *he;
    struct sockaddr_in their_addr;

    if (argc != 2) {
        fprintf(stderr,"Invalid number of arguments. Usage: <exec> hostname\n");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL) {
        fprintf(stderr, "Host name not provided.\n");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Could not create socket!\n");
        exit(1);
    }

    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(PORT);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "Error connecting to the server!\n");
        exit(1);
    }

    while(1) {
        int bytes_received = receive_message(sockfd);

        if (bytes_received == -1) {
            fprintf(stderr, "Error while trying to receive message from server!\n");
            exit(1);
        }

        int choice;
        scanf("%d", &choice);
        if (send(sockfd, &choice, sizeof(choice), 0) == -1) {
            fprintf(stderr, "Could not send message to server!\n");
            close(sockfd);
            break;
        }
        printf("message sent?\n");
    }

    return 0;
}
