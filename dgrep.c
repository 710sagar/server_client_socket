#include <stdio.h>	//printf
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>
#include <stdlib.h>
#define BUFFSIZE 4096
#define MAX_LINE 4096
#define SIZE 4096

void sendfile(FILE *fp, int sockfd);
ssize_t total=0;

int main(int argc , char *argv[])
{
	int sock;
	struct sockaddr_in server;
	char message[1000] , server_reply[2000];
	
	// check if all arguments passed
	if (argc != 4) {
		printf("run: dgrep <pattern> <file1> <file2>\n");
		exit(0);
	}
	
	// check if both the files exist on the client
	if((access(argv[2], F_OK) && access(argv[3], F_OK))) {
		printf("Please check if files exist on the client\n");
		exit(0);
	}
	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");
	
	// socket binding
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( 8888 );

	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}
	puts("Connected\n");
	
	// connect and send message to server
	char buff[BUFFSIZE] = {0};
	strncpy(buff, argv[3], strlen(argv[3]));
	
	//Send file name
	if( send(sock , buff , BUFFSIZE , 0) < 0)
	{
		puts("Send failed");
		return 1;
	}
	
	//Send file data
	FILE *fp = fopen(argv[3], "rb");
	if (fp == NULL) {
			perror("Can't open file");
			exit(1);
	}
	sendfile(fp, sock);
	printf("File sent successfully\n");
	//Receive a reply from the server
	/*
	if( recv(sock , server_reply , 2000 , 0) < 0)
	{
		puts("recv failed");
		//break;
	}
	char buf[32];
	sprintf(buf,"grep -w %s %s",argv[1], argv[2]);
	printf("%s\n", buf);
	system(buf);
	*/
	
	close(sock);
	return 0;
}

     
void sendfile(FILE *fp, int sockfd) 
{
    int n; 
    char sendline[MAX_LINE] = {0}; 
    while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0) 
    {
	    total+=n;
        if (n != MAX_LINE && ferror(fp))
        {
            perror("Read File Error");
            exit(1);
        }
        
        if (send(sockfd, sendline, n, 0) == -1)
        {
            perror("Can't send file");
            exit(1);
        }
        memset(sendline, 0, MAX_LINE);
    }
}