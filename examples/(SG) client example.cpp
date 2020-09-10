// This an simple exmple of client. The client
//  takes the server name and port number as an command line arguments.
// The client calls  DNS function to resolve the server name,
//  it then request a connection to server.  Once connection is established
//  the client ask user to enter a message and send the message to server.
// After sending the message the client wait for reply from the server.
// When client receives a message from the server it print the message on
//  standard out, closes the connection and exits.

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef __WIN32__
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

using namespace std;

int main(int argc, char *argv[])
{
	int sockfd;
	int msgSize, port;

	struct sockaddr_in server_addr;

	char message[1024];
	char output[1024];			 // Output message from server.
	memset(output, '\0', 1024); // Clear the buffer.

	struct hostent *hent;

	// Error handling: Check for correct command line input.
	if (argc < 3)
	{
		cerr << "Usage: ./client [server name] [port]" << endl;
		exit(1);
	}

	port = atoi(argv[2]); // Get port number from command line

	// Error handling: check the port number.
	if (port <= 10000)
	{
		cerr << "Port > 10000 required." << endl;
		exit(1);
	}

	// Error handling: check the server name: argv[1]
	if ((hent = gethostbyname(argv[1])) == NULL)
	{
		cerr << "Invalid host name." << endl;
		exit(1);
	}

	// Create the client socket.
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cerr << "Socket error." << endl;
		exit(1);
	}

	memset((void *)&server_addr, 0, sizeof(server_addr)); // Clear the server address structure.

	// Set up the server address structure.
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr = *((struct in_addr *)hent->h_addr);
	server_addr.sin_port = htons(port);

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		cerr << "Connect error." << endl;
		exit(1);
	}

	// Prompt message to send
	cout << "What message to send: ";
	cin.getline(message, 1024);

	if ((msgSize = send(sockfd, message, strlen(message), 0)) < 0)
	{
		cerr << "Send error." << endl;
	}

	// Wait to receive response.
	if ((msgSize = recv(sockfd, output, 1023, 0)) < 0)
	{
		cerr << "Receive error." << endl;
	}

	cout << output << endl;
	close(sockfd);

	return 0;
}
