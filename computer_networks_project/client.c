#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#define PORT 5000
#define MAXDATASIZE 1024

int receive_message(int sock_fd) {
    char buffer[MAXDATASIZE];
    uint32_t message_length, net_message_length;
    int bytes_received, total_received = 0;

    // while (total_received < sizeof(net_message_length)) {
    //     bytes_received = recv(sock_fd, (char*)&net_message_length + total_received, 
    //                         sizeof(net_message_length) - total_received, 0);
    //     if (bytes_received <= 0) {
    //         perror("Error receiving message length");
    //         return -1;
    //     }
    //     total_received += bytes_received;
    // }
    // if (bytes_received <= 0) {
    //     perror("Error receiving message length or server disconnected!\n");
    //     return -1;
    // }

    // message_length = ntohl(net_message_length);
    // printf("Incoming message length: %d\n", message_length);
    while ( (bytes_received = recv(sock_fd, buffer, MAXDATASIZE, 0)) > 0 ) {
        if (bytes_received >= 3 && !strncmp(buffer + bytes_received - 4, "EOF", 3)) {
            break;
        }
        // buffer[bytes_received] = '\0';
        printf("%s", buffer);
        // memset(buffer, 0, MAXDATASIZE);
        total_received += bytes_received;
    }
    if (bytes_received == -1) {
        perror("Error recieving data!\n");
        return -1;
    }
    return total_received;
}

int main(int argc, char *argv[]) {
    int sockfd, numbytes;
    struct hostent *he;
    struct sockaddr_in their_addr;

    if (argc != 2) {
        fprintf(stderr,"Invalid number of arguments. Usage: <exec> hostname\n");
        argv[1] = "localhost";
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
    their_addr.sin_port = htons(PORT); // questionable htons
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), 0, 8);

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "Error connecting to the server!\n");
        exit(1);
    }

    int yes = 1;
    if (setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    while(1) {
        int bytes_received = receive_message(sockfd);
        printf("Finished receiving\n");
        if (bytes_received < 0) {
            printf("Server disconnected.\n");
            break;
        }

        int choice;
        scanf("%d", &choice);
        uint32_t net_choice = htonl(choice);
        if (send(sockfd, &net_choice, sizeof(net_choice), 0) == -1) {
            fprintf(stderr, "Could not send message to server!\n");
            close(sockfd);
            break;
        }
        printf("message sent?\n");
    }

    return 0;
}
