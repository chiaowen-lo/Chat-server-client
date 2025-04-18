the program provide a simple muti-chat system using C sockets and threads. The server.c can handle multiple client connection, and the maximum is 10 client, and the clients.c can send and receive messages.

Program Idea
The program create a basic chatroom that multiple user can chat through the server. The server listens for connection, manages clients and broadcast messages. The server also puts all message in the share file . Clients allow connection to server, and use pthread to send and receive message simultaneously.

Written method
	Server: Use socket to listen for client connection. When a new client connect, it will buid a new thread to deal with message send and receive. At the same time, server holds a client list, ensure message can broadcast to every client.
	Client: Connect to server and send message. Also using a thread to listen for message to ensure the smoothnes of instant messaging.
	Threading: Both client and server use pthread to deal with mutiple connection, ensure immediacy and stability.

Makefile:
	compile server and client

Usage:
	Start server :  ./server <PORT>
	Start client  : ./client <SERVER_IP> <PORT> <USERNAME>
