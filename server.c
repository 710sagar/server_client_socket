#include <stdio.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write
#include <stdlib.h>
#define MAX_LINE 4096
#define BUFFSIZE 4096

ssize_t total=0;
void child(int);
void writefile(int sockfd, FILE *fp){
    ssize_t n;
    char buff[MAX_LINE] = {0};
    while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0) {
	    printf("n=%d \n", n);
	    total+=n;
        if (n == -1){
            perror("Receive File Error");
            exit(1);
        }
       printf("%s\n", buff); 
        if (fwrite(buff, sizeof(char), n, fp) != n){
            perror("Write File Error");
            exit(1);
        }
        memset(buff, 0, MAX_LINE);
	return;
    }
}

int main(int argc , char *argv[]){
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[2000];
	char *message;
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1){
		printf("Could not create socket");
	}
	puts("Socket created");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
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
	while (1){	
		//accept connection from an incoming client
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if (client_sock < 0){
			perror("accept failed");
			return 1;
		}
		puts("Connection accepted");
		if(!fork())
			child(client_sock);
		
		close(client_sock);
	}	
	return 0;
}

void sendfile(FILE *fp, int sockfd){
	printf("here\n");
    int n;
    char sendline[MAX_LINE] = {0};
    while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0){
	    total+=n;
        if (n != MAX_LINE && ferror(fp)){
            perror("Read File Error");
            exit(1);
        }

        if (write(sockfd, sendline, n) == -1){
            perror("Can't send file");
            exit(1);
        }
        memset(sendline, 0, MAX_LINE);
    }
    printf("sent\n");
}

void child(int client_sock) {
	char filename[BUFFSIZE] = {0};
       char pattern[BUFFSIZE] = {0};
		if (recv(client_sock, pattern, BUFFSIZE, 0) == -1){
			perror("Can't receive pattern");
			exit(1);
		}

		printf("pattern is: %s\n", pattern);
		if (recv(client_sock, filename, BUFFSIZE, 0) == -1) {
			perror("Can't receive filename");
			exit(1);
		}

		printf("filename is: %s\n", filename);
		
		FILE *fp = fopen(filename, "wb");
		if (fp == NULL) {
			perror("Can't open file");
			exit(1);
		}
		writefile(client_sock, fp);
		printf("writefile ran\n");
		fclose(fp);
		printf("it is now here\n");
		char buf[32];
		sprintf(buf,"grep -w %s %s",pattern, filename);
		printf("buff: %s\n", buf);
		FILE *cmd=popen(buf, "r");
		char result[24]={0x0};
		remove("output.txt");
		FILE *opFile=fopen("output.txt", "a");
		if (opFile == NULL) {
			printf("Error running grep on server");
			return;
		}
		//fwrite(buffer, sizeof(buffer[0]), MAX_SIZE, fp);
		while (fgets(result, sizeof(result), cmd) !=NULL){
    			printf("result: %s\n", result);
			fwrite(result, sizeof(char), strlen(result), opFile);
		}
		printf("it is here\n");
		fclose(opFile);
		printf("close\n");
		opFile=fopen("output.txt", "r");
		printf("why\n");	
		sendfile(opFile, client_sock);
		fclose(opFile);
		pclose(cmd);
}

