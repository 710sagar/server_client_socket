#include <stdio.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write
#include <stdlib.h>
#define MAX_LINE 4096
#define BUFFSIZE 4096

ssize_t total=0;
void child(int );
void writefile(int sockfd, FILE *fp){
    ssize_t n;
    char buff[MAX_LINE] = {0};
    while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0) 
    {
	    total+=n;
        if (n == -1)
        {
            perror("Receive File Error");
            exit(1);
        }
       printf("%s\n", buff); 
        if (fwrite(buff, sizeof(char), n, fp) != n)
        {
            perror("Write File Error");
            exit(1);
        }
        memset(buff, 0, MAX_LINE);
    }
}

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[2000];
	char *message;
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");
	
	//Listen
	listen(socket_desc , 3);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while (1) {	
		//accept connection from an incoming client
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if(!fork()){
			child(client_sock);
		}
	}
	return 0;
}
void child(int client_sock){
	if (client_sock < 0)
		{
			perror("accept failed");
			return 1;
		}
		puts("Connection accepted");
		
		char filename[BUFFSIZE] = {0}; 
		if (recv(client_sock, filename, BUFFSIZE, 0) == -1) {
			perror("Can't receive filename");
			exit(1);
		}	

		printf("%s", filename);
		
		FILE *fp = fopen(filename, "wb");
		if (fp == NULL) {
			perror("Can't open file");
			exit(1);
		}
		writefile(client_sock, fp);
		fclose(fp);

	
}
