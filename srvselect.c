// ===
// Web Programming HW#1
// Web server using select()
// 405410034 | Chinxy Yang
// 20181113
// ===
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>

char *ROOT;
void error(char *);
void respond(int);

int main(int argc, char **argv)
{
    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[1000];
    ssize_t n;
    fd_set rset, allset;
    char buf[1024];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    //Default Values PATH = ~/ and PORT=10000
    int PORT = 10000;
    ROOT = getenv("PWD");

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(PORT);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, 10000000);

    maxfd = listenfd;           /* initialize */
    maxi = -1;                  /* index into client[] array */
    for (i = 0; i < 1000; i++)
        client[i] = -1;         /* -1 indicates available entry */
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for ( ; ; ) {
        rset = allset;      /* structure assignment */
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) {    /* new client connection */
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
#ifdef  NOTDEF
            printf("new client: %s, port %d\n",
                    Inet_ntop(AF_INET, &cliaddr.sin_addr, 4, NULL),
                    ntohs(cliaddr.sin_port));
#endif

            for (i = 0; i < 1000; i++)
                if (client[i] < 0) {
                    client[i] = connfd; /* save descriptor */
                    break;
                }
            if (i == 1000)
                error("too many clients");

            FD_SET(connfd, &allset);    /* add new descriptor to set */
            if (connfd > maxfd)
                maxfd = connfd;         /* for select */
            if (i > maxi)
                maxi = i;               /* max index in client[] array */

            if (--nready <= 0)
                continue;               /* no more readable descriptors */
        }

        for (i = 0; i <= maxi; i++) {   /* check all clients for data */
            if ( (sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset)) {
                /*if ( (n = read(sockfd, buf, 1024)) == 0) {
                        //4connection closed by client 
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else
                    write(sockfd, buf, n);
                */
                respond(sockfd);
                if (--nready <= 0)
                    break;              /* no more readable descriptors */
            }
        }
    }
}

void error(char *msg) {
  perror(msg);
  exit(1);
}

//client connection
void respond(int connfd)
{
    char mesg[99999], *reqline[3], data_to_send[1024], path[99999];
    int rcvd, fd, bytes_read;

    memset( (void*)mesg, (int)'\0', 99999 );

    rcvd=recv(connfd, mesg, 99999, 0);

    if (rcvd<0)    // receive error
        fprintf(stderr,("recv() error\n"));
    else if (rcvd==0)    // receive socket closed
        fprintf(stderr,"Client disconnected upexpectedly.\n");
    else    // message received
    {
        printf("%s", mesg);
        reqline[0] = strtok (mesg, " \t\n");
        if ( strncmp(reqline[0], "GET\0", 4)==0 )
        {
            reqline[1] = strtok (NULL, " \t");
            reqline[2] = strtok (NULL, " \t\n");
            if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
            {
                write(connfd, "HTTP/1.0 400 Bad Request\n", 25);
            }
            else
            {
                //if no file is specified, index.html will be opened by default
                if ( strncmp(reqline[1], "/\0", 2)==0 )
                    reqline[1] = "/index.html"; 

                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], reqline[1]);
                printf("#####\n\n");

                if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
                {
                    send(connfd, "HTTP/1.0 200 OK\n\n", 17, 0);
                    while ( (bytes_read=read(fd, data_to_send, 1024))>0 )
                        write (connfd, data_to_send, bytes_read);
                }
                else    write(connfd, "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
            }
        }
    }

    //Closing SOCKET
    close(connfd);
}