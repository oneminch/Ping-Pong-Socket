#include "pingpong.h"
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <string.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

using namespace std;

/*

   Function: sendMessage
   - This function will send the message to the server. 
   - 'msg' is a null terminated string to be sent to the server and it's length may be arbitrarily long and is NOT limited to a few kilobytes. 
   - 'con' contains the information required to send to the server. 
      - Note that this does mean that if there are multiple ConnectionInfo data structure this may not be the same server between calls.
   - If the send was successful return 0. 
   - If it was unsuccessful for any reason then a non 0 value is returned. 'msg' or 'con' being null is a sufficient condition for failure.

*/
int sendMessage(ConnectionInfo *con, char *msg) // Fix: msg length may be arbitrarily long and is NOT limited to a few kilobytes
{
   int msgSize;
   if (msg == NULL || con == NULL)
   {
      return 1;
   }
   else
   {
      if ((msgSize = send(con->socket_fd, msg, strlen(msg), 0)) < 0)
      {
         cout << "Send error." << endl;
         return 1;
      }
      con->msg_size = strlen(msg);
      return 0;
   }
}

/*

   Function: recieveMessage
   - 'con' contains the information required to receive from the server. 
      - Note that this does mean that if there are multiple ConnectionInfo data structures this may not be the same server between calls.
   - This return value is null if an error has occurred.  
      - Otherwise the returned value is a null terminated string that contains the entire message and thus must be able to hold 
      a large amount of data (more than a few kilobytes) and is most easily accomplished by using the heap to allocate the memory. 
      - This memory should not be modified by any further actions in the client library before dealocate_message is called on the returned value.

*/
char *recieveMessage(ConnectionInfo *con)
{
   int msgSize;
   char *message;

   if ((msgSize = recv(con->socket_fd, message, con->msg_size, 0)) < 0)
   {
      cout << "Receive error." << endl;
      return NULL;
   }

   return message;
}

/*
   
   Function: dealocate_message
   This function will deallocate the memory returned by recieveMessage. 
   This may be as simple as a wrapper to c's free function or c++'s delete.
   If the value passed in 'mem' did not come from recieveMessage then the results are undefined.

*/
void dealocate_message(char *mem) // typo corrected
{
   delete mem; // or delete[] mem
}

/*
   
   Function: connect_to_server
   This function will connect to a server and initialize the 'con' data structure so that it can be used in future calls to sendMessage. 
   'who' is the name of the host as a null terminated ASCII string. 
   'port' is the port the server is running on.
   'con' will contain after this call, if it is successful, the required information to be used in the sendMessage and recieveMessage functions.
   If the connection attempt was successful 0 is returned. 
   If the connection attempt was unsuccessful then a non 0 value is returned. 'who' or 'con' being null is a sufficient condition for  failure.
   In the event of a failure the state of the system and the arguments is such that another attempt can be made, for instance with a new port number.

*/
int connect_to_server(char *who, int port, ConnectionInfo *con)
{
   int sockfd;

   struct sockaddr_in server_addr;
   struct hostent *hent = gethostbyname(who);

   if (who == NULL || con == NULL)
   {
      return 1; // unsuccessful
   }
   else
   {
      // Clear the server address structure
      memset((void *)&server_addr, 0, sizeof(server_addr));

      // Set up the server address structure.
      server_addr.sin_family = AF_INET;
      server_addr.sin_addr = *((struct in_addr *)hent->h_addr);
      server_addr.sin_port = htons(port);

      // Create the client socket.
      if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      {
         cout << "Socket error." << endl;
         return 1;
      }

      // Set ConnectionInfo values
      con->socket_fd = sockfd;

      if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
      {
         cout << "Connect error." << endl;
         return 1;
      }
      else
      {
         return 0; // successful
      }
   }
}

/*
   
   Function: run_server
   'port' is a port number to bind to.
   If the bind was successful then the server is now running on a new thread and 0 is returned. 
   - If it was unsuccessful a nonzero value is returned. 
   - After unsuccessful it must be possible to reattempt the bind and have it succeed if the OS will allow the program to bind to the requested port number.

*/
int run_server(int port)
{
   int serv_sock, cli_sock, msg_len;
   char *buffer;
   unsigned int cli_len;

   // Create server and client address structures
   struct sockaddr_in serv_addr, cli_addr;

   // Create the server socket.
   if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      cout << "Socket error." << endl;
      return 1;
   }

   // Clear the server address structure
   memset((void *)&serv_addr, 0, sizeof(serv_addr));

   // Set up the server address structure.
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   serv_addr.sin_port = htons(port);

   // Bind the socket to the server address and port
   if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
   {
      cout << "Error binding" << endl;
      return 1;
   }

   // Listen on the socket, queue 5 incoming connections.
   listen(serv_sock, 10);

   // Loop forever, handling connections.
   while (1)
   {
      cli_len = sizeof(cli_addr);
      cli_sock = accept(serv_sock, (struct sockaddr *)&cli_addr, &cli_len);
      if (cli_sock < 0)
      {
         cout << "Error accepting" << endl;
         return 1;
      }

      // Clear the buffer
      memset(buffer, '\0', strlen(buffer));

      if ((msg_len = recv(cli_sock, buffer, strlen(buffer), 0)) < 0)
      {
         cout << "Error receiving message." << endl;
      }

      cout << "Message received from client: " << buffer << endl;

      // Set response to "PONG" if received message is "PING"
      // Else set it to the same value as received message
      char *response;
      if (strcmp(buffer, "PING"))
      {
         // response = "PONG";
         sprintf(response, "%s", "PONG");
      }
      else
      {
         sprintf(response, "Server: I received the following message:  %s", buffer);
      }

      // Send message back to client
      if ((msg_len = send(cli_sock, response, strlen(response), 0)) < 0)
      {
         cout << "Error sending message." << endl;
      }

      close(cli_sock);
   }
   return 0;
}
