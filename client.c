#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER 1024

int cli_socket;

void *get_mes(void *arg){
	char buf[BUFFER];
	while(1){
		memset(buf,0,BUFFER); //clear buf
		int receive = recv(cli_socket, buf, BUFFER, 0); //receive mes from server
		if(receive<=0) break;
		printf("%s", buf);
	}
	return NULL;
}

int main(int argc, char *argv[]){
	if(argc != 4){
		//argc must equal 3, program need program name, server IP and username
		printf("Usage: %s <server_ip> <port> <username>\n",argv[0]);
		return 1;
	}
	
	struct sockaddr_in ser_addr;
	pthread_t thread;
	
	int port = atoi(argv[2]);
	
	//create client socket
	cli_socket = socket(AF_INET, SOCK_STREAM, 0);
	ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    inet_pton(AF_INET, argv[1], &ser_addr.sin_addr);
    
    if(connect(cli_socket, (struct sockaddr *)&ser_addr, sizeof(ser_addr)) == -1) {
        perror("Connection failed");
        return 1;
    }

    send(cli_socket, argv[3], strlen(argv[3]), 0);

    pthread_create(&thread, NULL, get_mes, NULL);
    pthread_detach(thread);

	
	//enter and send message
	char buf[BUFFER];
	while(1){
		fgets(buf, BUFFER, stdin);
		if(strncmp(buf, "bye", 3)==0){
			break;
		}
		send(cli_socket, buf, strlen(buf), 0);
	}
	
	close(cli_socket);
	return 0;
	
}




















