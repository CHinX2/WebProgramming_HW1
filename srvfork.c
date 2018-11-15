// ===
// Web Programming HW#1
// Web server using fork()
// 405410034 | Chinxy Yang
// 20181113
// ===
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>

int listenfd, connfd;
char *ROOT;
void error(char *);
void respond(void);

int main(int argc, char* argv[])
{
	struct addrinfo hints, *res, *p;
	struct sockaddr_in cliaddr;
	socklen_t clilen;
	char c;    
	
	//Default Values PATH = ~/ and PORT=10000
	char PORT[6];
	ROOT = getenv("PWD");
	strcpy(PORT,"10001");

	int slot=0;
	int a;
	
	// Setting connfd to -1: there is no client connected
	connfd = -1;

	// getaddrinfo for host
	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo( NULL, PORT, &hints, &res);
	
	// socket and bind
	for (p = res; p!=NULL; p=p->ai_next)
	{
		listenfd = socket (p->ai_family, p->ai_socktype, 0);
		if (listenfd == -1) continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
	}
	
	freeaddrinfo(res);

	// listen for incoming connections
	listen (listenfd, 1000000);

	// ACCEPT connections
	for ( ; ; )
	{
		wait(&a);
		clilen = sizeof(cliaddr);
		if((connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen)) < 0)
		{
			if (connfd < 0)
				error("accept error");
			else continue;
		}

		if ( fork() == 0 )
		{
			close(listenfd);
			respond();
			exit(0);
		}
		close(connfd);
	}

	return 0;
}

//client connection
void respond()
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

//error - wrapper for perror
void error(char *msg) {
  perror(msg);
  exit(1);
}
