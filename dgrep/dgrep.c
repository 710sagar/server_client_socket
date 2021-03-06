#include <stdio.h>	
#include <string.h>	
#include <sys/socket.h>	
#include <arpa/inet.h>
#include <unistd.h>
#include<stdlib.h>
#define BUFFSIZE 4096
#define MAX_LINE 4096
#define SIZE 4096
#define RED   "\x1B[31m"
#define RESET "\x1B[0m"

void fileTransfer(FILE *fp, int sockFd);
ssize_t total=0;

void writefile(int sockFd){
	remove("serverOutput.txt");
	FILE *fp=fopen("serverOutput.txt", "a");
    ssize_t n;
    char buff[MAX_LINE] = {0};
    while ((n = recv(sockFd, buff, MAX_LINE, 0)) > 0){
	    total+=n;
        if (n == -1){
            perror("Receive File Error");
            exit(1);
        }
       //printf("%s\n", buff);
        if (fwrite(buff, sizeof(char), n, fp) != n){
            perror("Write File Error");
            exit(1);
        }
        memset(buff, 0, MAX_LINE);
	fclose(fp);
    }
}

int main(int argc , char *argv[]){
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
	if (sock == -1){
		printf("Could not create socket");
	}
	puts("Socket created");
	
	// socket binding
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( 6060 );

	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0){
		perror("connect failed. Error");
		return 1;
	}
	puts("Connected\n");
	
	// connect and send message to server
	char buff[BUFFSIZE] = {0};

	strncpy(buff, argv[1], strlen(argv[1]));
	
	//Send pattern
	if( send(sock , buff , BUFFSIZE , 0) < 0){
		puts("Send failed");
		return 1;
	}

	strncpy(buff, argv[3], strlen(argv[3]));
	//Send file name
	if( send(sock , buff , BUFFSIZE , 0) < 0){
		puts("Send failed");
		return 1;
	}
	
	//Send file data
	FILE *fp = fopen(argv[3], "rb");
	if (fp == NULL) {
			perror("Can't open file");
			exit(1);
	}
	fileTransfer(fp, sock);

	// Receive data from server
	writefile(sock);

	// run grep on client
	char buf[32];
		sprintf(buf,"grep -w %s %s",argv[1], argv[2]);
		FILE *cmd=popen(buf, "r");
		FILE *checkCmd = popen(buf, "r");
		char result[BUFFSIZE]={0x0};
		remove("clientOutput.txt");
		FILE *opFile=fopen("clientOutput.txt", "a");
		if (opFile == NULL) {
			printf("Error: Command not executed");
			return 0;
		}
		if(fgets(result, sizeof(result), checkCmd)==NULL){
			printf("Pattern not found in %s\n",argv[2]);
		}
		else{
			while (fgets(result, sizeof(result), cmd) !=NULL){
					char del[] = " ";
				char *pa = argv[1];
				char *pp = strtok(result, del);
				printf("%s: ",argv[2]);
				while(pp != NULL) {
					if (*pa == *pp){
						printf(RED " %s" RESET, pp);
					} else
						printf(" %s", pp);
					pp = strtok(NULL, del);
				}
				fwrite(result, sizeof(char), strlen(result), opFile);
			}
		}
		fclose(opFile);
		opFile=fopen("serverOutput.txt", "r");
		FILE *checkFile=fopen("serverOutput.txt", "r");
		char *pat=argv[1];
			
		if(fgets(result, sizeof(result), checkFile)==NULL){
			printf("Pattern not found in %s\n", argv[3]);
		}
		else{
			while(fgets(result, sizeof(result), opFile)!=NULL) {
				char delim[] = " ";
				char *ptr = strtok(result, delim);
				printf("%s: ", argv[3]);
				while(ptr != NULL){
					if (*ptr == *pat) {
						printf(RED " %s" RESET, ptr);
					}
					else
						printf(" %s", ptr);
					ptr = strtok(NULL, delim);
				}
			}
		}
		
		fclose(opFile);

	
	
	close(sock);
	return 0;
}

     
void fileTransfer(FILE *fp, int sockFd) {
    int n; 
    char sendline[MAX_LINE] = {0}; 
    while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0) {
	    total+=n;
        if (n != MAX_LINE && ferror(fp)){
            perror("Read File Error");
            exit(1);
        }
        
        if (send(sockFd, sendline, n, 0) == -1){
            perror("Can't send file");
            exit(1);
        }
        memset(sendline, 0, MAX_LINE);
    }
}
