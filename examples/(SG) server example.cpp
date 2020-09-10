// This an simple example of single threaded echo server.
// The server opens a socket with
//  a port(command line argument) and listen for connection. If a connection
//  request comes from the client it accpts the connection and wait for message
//  from the client. When it receives a message from client it prints the message
//  standard out. Also server prepend "Server: I received the following message: "
//  to the received message and send it back to client. The server than
//  close the connection.

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

using namespace std;

void handleConnection(int clisock)
{

	int msgSize;
	char buffer[1016];
	memset(buffer, '\0', 1016); // Clear the buffer.

	if ((msgSize = recv(clisock, buffer, 1015, 0)) < 0)
	{
		cerr << "Receive error." << endl;
	}

	cout << "Message received from client: " << buffer << endl;
	char response[1024];
	sprintf(response, "Server: I received the following message:  %s", buffer);

	if ((msgSize = send(clisock, response, strlen(response), 0)) < 0)
	{
		cerr << "Send error." << endl;
	}

	close(clisock);
}

int main(int argc, char *argv[])
{

	// Set up the socket.

	int sockfd, newsockfd;
	unsigned int clilen;
	int port = atoi(argv[1]);
	// Structures for client and server addresses.
	struct sockaddr_in server_addr, cli_addr;

	// Create the server socket.
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cerr << "Socket error." << endl;
		exit(1);
	}

	memset((void *)&server_addr, 0, sizeof(server_addr)); // Clear the server address structure.

	// Set up the server address structure.
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	// Bind the socket to the server address and port.
	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		cerr << "Bind error.";
		exit(1);
	}

	// Listen on the socket, queue 5 incoming connections.
	listen(sockfd, 10);

	// Loop forever, handling connections.
	while (1)
	{
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd < 0)
		{
			cerr << "Accept error." << endl;
			exit(1);
		}
		handleConnection(newsockfd);
	}

	return 0;
}
