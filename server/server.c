#include <stdio.h>
#include <string.h>	
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <unistd.h>	
#include <stdlib.h>
#define MAX_LINE 4096
#define BUFFSIZE 4096

ssize_t total=0;
void handle_child(int);
void writefile(int sockFd, FILE *file){
    ssize_t n;
    char buff[MAX_LINE] = {0};
    while ((n = recv(sockFd, buff, MAX_LINE, 0)) > 0) {
	    total+=n;
        if (n == -1){
            perror("Error: File not received");
            exit(1);
        }
        if (fwrite(buff, sizeof(char), n, file) != n){
            perror("Error: Write File error");
            exit(1);
        }
        memset(buff, 0, MAX_LINE);
	return;
    }
}

int main(int argc , char *argv[]){
	int socketDesc , clientSock , c , readSize;
	struct sockaddr_in server , client;
	char clientMessage[2000];
	char *message;
	
	//Create socket
	socketDesc = socket(AF_INET , SOCK_STREAM , 0);
	if (socketDesc == -1){
		printf("Socket not created");
	}
	puts("Socket created");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 6060 );
	
	//Bind
	if( bind(socketDesc,(struct sockaddr *)&server , sizeof(server)) < 0){
		//print the error message
		perror("Error: Bind Failed");
		return 1;
	}
	puts("Bind Successful");
	
	//Listen
	listen(socketDesc , 10);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while (1){	
		//accept connection from an incoming client
		clientSock = accept(socketDesc, (struct sockaddr *)&client, (socklen_t*)&c);
		if (clientSock < 0){
			perror("accept failed");
			return 1;
		}
		puts("Connection accepted");
		if(!fork())
			handle_child(clientSock);
		
		close(clientSock);
	}	
	return 0;
}

void fileTransfer(FILE *file, int sockFd){
    int n;
    char sendline[MAX_LINE] = {0};
    while ((n = fread(sendline, sizeof(char), MAX_LINE, file)) > 0){
	    total+=n;
        if (n != MAX_LINE && ferror(file)){
            perror("Read File Error");
            exit(1);
        }

        if (write(sockFd, sendline, n) == -1){
            perror("Can't send file");
            exit(1);
        }
        memset(sendline, 0, MAX_LINE);
    }
}

void handle_child(int clientSock) {
	char filename[BUFFSIZE] = {0};
       char pattern[BUFFSIZE] = {0};
		if (recv(clientSock, pattern, BUFFSIZE, 0) == -1) {
			perror("Can't receive pattern");
			exit(1);
		}

		if (recv(clientSock, filename, BUFFSIZE, 0) == -1) {
			perror("Can't receive filename");
			exit(1);
		}

		
		FILE *file = fopen(filename, "wb");
		if (file == NULL) {
			perror("Can't open file");
			exit(1);
		}
		writefile(clientSock, file);
		fclose(file);
		char buf[32];
		sprintf(buf,"grep -w %s %s",pattern, filename);
		FILE *cmd=popen(buf, "r");
		char result[BUFFSIZE]={0x0};
		remove("output.txt");
		FILE *opFile=fopen("output.txt", "a");
		if (opFile == NULL) {
			printf("Error running grep on server");
			return;
		}
		while (fgets(result, sizeof(result), cmd) !=NULL){
			fwrite(result, sizeof(char), strlen(result), opFile);
		}
		fclose(opFile);
		opFile=fopen("output.txt", "r");
		fileTransfer(opFile, clientSock);
		fclose(opFile);
		pclose(cmd);
}

