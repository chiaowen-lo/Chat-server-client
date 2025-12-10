# Multi-threaded Network Chat Server

A multi-threaded chat room system implemented in **C** on **Linux**, utilizing **TCP sockets** and **POSIX Threads (pthreads)**. The system supports multiple clients communicating simultaneously with broadcasting and private messaging capabilities.

## Features

* **Multi-Client Support:** Handles up to 10 concurrent clients using a threaded architecture.
* **Real-time Broadcasting:** Messages sent by any user are instantly broadcasted to all other connected users.
* **Private Messaging:** Supports sending private messages to a specific user using a dedicated command.
* **Chat History Logging:** Automatically logs all chat activity (including timestamps) to a server-side file named `board.txt`.
* **Thread Synchronization:** Uses `pthread_mutex` to ensure thread safety when writing to the log file and broadcasting messages.
* **Client Management:** Automatically handles client disconnection and updates the active client list.

## Requirements
* Linux Environment (Ubuntu/CentOS)
* GCC Compiler
* Make

## How to Build

Use the included `Makefile` to compile both the server and client programs:

```bash
make
```
## Usage
1. Start the Server
Run the server on a specific port.

```bash
./server <PORT>
# Example: ./server 8888
```

2. Start the Client
Connect to the server from a client terminal. You need to specify the server IP, port, and a unique username.

```bash
./client <SERVER_IP> <PORT> <USERNAME>
# Example: ./client 127.0.0.1 8888 Alice
```
## Chat Commands

Once connected to the server, you can use the following commands to interact:

### Broadcast Message
Simply type any text and press **Enter**. The message will be visible to everyone currently connected to the server.

### Private Message
Send a message to a specific user only (whisper).

* **Format:** `chat <target_name> "<message>"`
* **Example:** `chat Bob "Hello Bob"`

> **Note:** The message content **must** be enclosed in double quotes (`""`).

### Exit
Type `bye` to disconnect from the server and close the client application.

```text
bye
```
## Clean Build
To remove compiled executables (server/client) and the log file for a fresh start:
```bash
make clean
```
