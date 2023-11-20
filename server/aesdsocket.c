#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT "9000"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold
#define FILENAME "/var/tmp/aesdsocketdata"

volatile bool bProgramClosed = false;

void sigchld_handler(int s)
{
    if(s == SIGINT || s == SIGTERM)
        bProgramClosed = true;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, new_fd;
    struct addrinfo hints, *servinf;
    struct sockaddr_storage their_addr; 
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    bool bDaemonMode = false;

    if(argc == 2 && argv[1][0] == '-' && argv[1][1] == 'd')
        bDaemonMode = true;

    openlog("aesdsocket", LOG_CONS | LOG_PID, LOG_USER);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinf)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    if ((sockfd = socket(servinf->ai_family, servinf->ai_socktype, servinf->ai_protocol)) == -1) {
        perror("server: socket");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        return -1;
    }

    if (bind(sockfd, servinf->ai_addr, servinf->ai_addrlen) == -1) {
        close(sockfd);
        perror("server: bind");
        return -1;
    }

    freeaddrinfo(servinf);

    if(bDaemonMode)
    {
        if (fork()) { // this is the parent process
            close(sockfd); // parent doesn't need the listener
        
            return 0;
        }
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        return -1;
    }

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        return -1;
    }

    int fileHandle = open(FILENAME, O_CREAT | O_RDWR | O_TRUNC, 0664);

    if(fileHandle == -1)
    {
        //Missing closing handles ******
        return -1;
    }

    char acReadBuffer[128];
    char acWriteBuffer[128];

    while(!bProgramClosed)
    {
        if (listen(sockfd, BACKLOG) == -1) {
            perror("listen");
            return -1;
        }

        printf("server: waiting for connections...\n");

        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            continue;
        }

        inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr),
        s, sizeof s);

        syslog(LOG_DEBUG, "Accepted connection from  %s", s);

        socklen_t addr_len = sizeof their_addr;
        bool bReceivedNewLine = false;

        while(!bReceivedNewLine)
        {
            int rxbytes = 0;
            if ((rxbytes = recvfrom(new_fd, acReadBuffer, 128, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) 
            {
                //Cleanup
                return -1;
            }

            int writecount = write(fileHandle,acReadBuffer,rxbytes);

            if(writecount != rxbytes)
            {
                syslog(LOG_ERR, "Failed to write to file");
                return 1;
            }

            if(acReadBuffer[rxbytes-1] == '\n')
                bReceivedNewLine = true;
        }

        //Read the file out and send to client
        
        if (lseek(fileHandle, 0, SEEK_SET) == -1) {
            
            //close(fileDescriptor);
            return -1;
        }

        int bytesRead;
        while ((bytesRead = read(fileHandle, acWriteBuffer, 128)) > 0) 
        {
            size_t dataToSend = (size_t)bytesRead;
            if(send(new_fd,acWriteBuffer,dataToSend,0) == -1)
            {
                return -1;
            }
        }

        if (bytesRead == -1) {
            //close(fileDescriptor);
            return -1;
        }

        close(new_fd);
        syslog(LOG_DEBUG, "Closed connection from  %s", s);
    }

    printf("caugh signal exiting");
    syslog(LOG_DEBUG, "Caught signal, exiting");
    
    close(sockfd);
    close(fileHandle);
    if(unlink(FILENAME) == -1)
    {
        printf("Failed to delete %s", FILENAME);
    }

    return 0;
}
