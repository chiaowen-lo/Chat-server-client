#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/file.h>
#include <time.h>

#define PORT 1234
#define MAXCLI 10
#define BUFFER 1024
#define BOARD "board.txt"

typedef struct{
	int socket;
	char user[50];
	char ip[INET_ADDRSTRLEN]; //32bitIPv4
	int port;
}Client;

Client clients[MAXCLI];
int cli_count=0; //count of client
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //initialize mutex

void write_mes(const char *mes){
	pthread_mutex_lock(&mutex); //get exclusive lock
	sleep(10);
	
	//get current time
	time_t cur_time = time(NULL);
	struct tm *time_info = localtime(&cur_time);
	char timestr[100];
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", time_info);
	
	FILE *file = fopen(BOARD,"a");//cannot modify file, only append at end
	if(file){
		fprintf(file, "[%s] %s",timestr,mes);
		fflush(file);
		
		fclose(file);
	}
	pthread_mutex_unlock(&mutex); //release exclusive lock
}

void broadcast(const char *mes){
	//send mes to client
	write_mes(mes);
	pthread_mutex_lock(&mutex);
	for(int i=0;i<cli_count;i++){
		send(clients[i].socket, mes, strlen(mes), 0); //sent data by Socket
	}
	pthread_mutex_unlock(&mutex);
}

int find_name(const char *name){
	for(int i=0;i<cli_count; i++){
		if(strcmp(clients[i].user, name)==0){
			return i; //find user
		}
	}
	return -1; //no such user
}

void *cli_process(void *arg){
/*
	1. receive mes
	2. check client state
	3. sent chat mes
	4. notify everyone when client offline
*/
	int cli_socket = *(int *)arg; //get client socket descriptor
	struct sockaddr_in cli_addr; //IPv4 network structure to store client IP address and port
	socklen_t addr_len = sizeof(cli_addr); //save socket len
	char buf[BUFFER], uname[50];
	
	getpeername(cli_socket, (struct sockaddr *)&cli_addr, &addr_len); //get client IP and port
	inet_ntop(AF_INET, &cli_addr.sin_addr, buf,sizeof(buf));//trandfer IP format,deal with IPv4
	int port = ntohs(cli_addr.sin_port);//transfer port to native format
	
	//get client username
	recv(cli_socket,uname,sizeof(uname),0); //get username store in uname[]
	uname[strcspn(uname,"\n")]=0; //remove \n
	
	pthread_mutex_lock(&mutex);//lock to ensure thread synchronization
	//store client's socket, username, ip, port
	clients[cli_count].socket = cli_socket;
	strncpy(clients[cli_count].user, uname, sizeof(clients[cli_count].user)-1);
	strncpy(clients[cli_count].ip, buf, sizeof(clients[cli_count].ip)-1);
	clients[cli_count].port = port;
	cli_count++;
	pthread_mutex_unlock(&mutex);//unlock mutex allow other thread proceed
	
	char mes[BUFFER];
	snprintf(mes, BUFFER-1, "<User %.50s is on-line, socket address: %.15s/%d.>\n", uname, buf, port);
	//put the string in mes, max length BUFFER-1
	broadcast(mes);
	
	while(1){
		memset(buf,0,BUFFER); //clear buf
		int receive = recv(cli_socket, buf,BUFFER, 0); //received message from Client
		if(receive<=0) break; //>0: revceive; =0:Client off-line; <0:error
		
		if(strncmp(buf, "chat ", 5)==0){
		//starting with chat means a chat command
			char target[50], chat_mes[BUFFER];
			if(sscanf(buf+5, "%49s \"%900[^\"]\"",target, chat_mes) == 2){
				/*
				buf+5: ignore "chat"
				%49s : read target name
				\"%900[^\"]\": read chat mes inside double quote
				==2: ensure name and mes are successfully extracted
			
			*/
				int index = find_name(target);
				if(index !=-1){
					snprintf(mes, BUFFER-1, "<From %.50s><To %.50s> %.900s\n",uname,target, chat_mes);
					send(clients[index].socket,mes, strlen(mes),0);
					write_mes(mes);
				}
				else{
					snprintf(mes, BUFFER-1, "<User %s does not exist.>\n",target);
					send(cli_socket,mes, strlen(mes),0);
				}
			
			}
		
		}
	}
	
	//When client offline
	
	close(cli_socket);
	
	pthread_mutex_lock(&mutex); //avoid race condition since clients is a share array
	for(int i=0;i<cli_count;i++){
		if(clients[i].socket == cli_socket) //find offline client
		{
			clients[i] = clients[cli_count-1]; //use the last client to cover offline client
			cli_count--;
			break; //after find offline user, break for-loop
		}
	}
	pthread_mutex_unlock(&mutex);
	
	snprintf(mes, BUFFER-1, "<User %.50s is off-line.>\n",uname);
	broadcast(mes);	
	
	return NULL;

}
	
int main(int argc, char *argv[]){
	if(argc!=2){
		printf("Usage: %s <port>\n", argv[0]);
		return 1;
	}
	
	int port = atoi(argv[1]);
	int ser_socket, cli_socket;
	struct sockaddr_in ser_addr, cli_addr;
	socklen_t addr_size;
	pthread_t thread;
	
	FILE *file = fopen(BOARD, "w");
	if(file) fclose(file);
	
	//build server socket
	ser_socket = socket(AF_INET, SOCK_STREAM, 0);//buid TCP connect socket using IPv4
	ser_addr.sin_family = AF_INET; //use IPv4 address
	ser_addr.sin_addr.s_addr = INADDR_ANY; //Listen all available network interface
	ser_addr.sin_port = htons(port);  //set port number, convert network byte order
	
	//bind IP and port
	bind(ser_socket, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
	
	//server into listen mode
	listen(ser_socket,MAXCLI); //MAX 10 client
	printf("Server start, port: %d\n",port);
	
	//wait for client connect
	while(1){
		addr_size = sizeof(cli_addr);
		cli_socket = accept(ser_socket, (struct sockaddr *)&cli_addr, &addr_size); 
		//wait for client connect
		
		//create new thread for every client
		pthread_create(&thread, NULL, cli_process, &cli_socket);
		/*
			create new thread for client
			process cli_process to deal with client data
		*/
		pthread_detach(thread); //thread release resorece autommatic
		
	}
	
	close(ser_socket);
	return 0;
}























