#include "pingpong.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

/*

   Function: sendMessage
   - This function will send the message to the server. 
   - 'msg' is a null terminated string to be sent to the server and it's length may be arbitrarily long and is NOT limited to a few kilobytes. 
   - 'con' contains the information required to send to the server. 
      - Note that this does mean that if there are multiple ConnectionInfo data structure this may not be the same server between calls.
   - If the send was successful return 0. 
   - If it was unsuccessful for any reason then a non 0 value is returned. 'msg' or 'con' being null is a sufficient condition for failure.

*/
int sendMessage(ConnectionInfo *con, char *msg)
{
   // Error handling: Check if values are null
   if (msg == NULL || con == NULL)
   {
      return 1;
   }

   // Send message
   if ((send(con->socket_fd, msg, strlen(msg), 0)) < 0)
   {
      printf("Error sending message\n");
      return 1;
   }

   // Update ConnectionInfo value
   con->msg_size = strlen(msg) + 1;

   return 0;
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
   // Declare variables
   int msg_len = con->msg_size;                    // Length of string sent
   char *message = malloc(sizeof(char) * msg_len); // Message to be received
   memset(message, '\0', msg_len);                 // Clear message before recv()

   // Receive message on socket from passed ConnectionInfo structure
   if (recv(con->socket_fd, message, (msg_len - 1), 0) < 0)
   {
      printf("Error receiving message\n");
      return NULL;
   }

   // Update ConnectionInfo data
   con->msg_size = strlen(message);

   return message;
}

/*
   
   Function: dealocate_message
   This function will deallocate the memory returned by recieveMessage. 
   This may be as simple as a wrapper to c's free function or c++'s delete.
   If the value passed in 'mem' did not come from recieveMessage then the results are undefined.

*/
void dealocate_message(char *mem)
{
   free(mem);
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
   // Error handling: Check if values are null
   if (who == NULL || con == NULL)
   {
      return 1; // unsuccessful
   }

   int sockfd;                                           // Socket file descriptor
   struct sockaddr_in server_addr;                       // Server address
   memset((void *)&server_addr, 0, sizeof(server_addr)); // Clear the server address structure

   // Create the Client socket
   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      printf("Error creating socket.\n");
      return 1;
   }

   // Set up the server address structure.
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(port);

   // Set initial ConnectionInfo values
   con->socket_fd = sockfd;
   con->msg_size = 0;

   // Connect the socket to the Server
   if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
   {
      printf("Error connecting.\n");
      return 1;
   }

   // Confirm Client and Server are connected
   printf("Client and Server connected successfully!!!\n");

   return 0;
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
   printf("\nInitializing Server...\n");

   // Declare variables
   int serv_sock, cli_sock; // Client & Server sockets
   unsigned int len;        // Length of Server address

   struct sockaddr_in serv_addr;                     // Create Server address structure
   memset((void *)&serv_addr, 0, sizeof(serv_addr)); // Clear the server address structure

   // Create the Server socket
   if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      printf("Error creating socket.\n");
      return 1;
   }

   // Set up the Server address structure
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   serv_addr.sin_port = htons(port);

   len = sizeof(serv_addr); // Set len(gth of serv_addr)

   // Create loop to rebind if it fails <-------------------------------------------

   // Bind the socket to the Server address and port
   if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
   {
      printf("Error binding.\n");
      return 1;
   }

   // Listen on socket
   if (listen(serv_sock, 10) < 0)
   {
      printf("Error listening.\n");
      return 1;
   }

   // Accept connection
   if ((cli_sock = accept(serv_sock, (struct sockaddr *)&serv_addr, (socklen_t *)&len)) < 0)
   {
      printf("Error accepting\n");
      return 1;
   }

   // Confirm server is running on given port
   printf("\nServer running on port %i\n", port);

   /* ===== BELOW HERE LIES THE CODE RESPONSIBLE FOR EXCHANGING MESSAGES ===== */

   // Initialize variables
   char buffer[4096];   // Message to be received
   char response[4096]; // Message to be sent back

   int i = 0;
   while (i < 25)
   {
      // Clear message variables
      memset(buffer, '\0', strlen(buffer));
      memset(response, '\0', strlen(response));

      // Wait for message from Client
      if ((recv(cli_sock, buffer, 4095, 0)) < 0)
      {
         printf("Error receiving message.\n");
      }

      // Check received message (buffer)
      // If "PING" and set response to "PONG"
      // Else set response same as buffer
      if (strcmp(buffer, "PING") == 0)
      {
         sprintf(response, "%s\n", "PONG");
      }
      else
      {
         sprintf(response, "%s\n", buffer);
         response[strlen(response) - 1] = 0; // Remove trailing \n
      }

      // Display message received on Server
      printf("\nMessage received from Client: %s \n", buffer);

      // Send response back to Client
      if ((send(cli_sock, response, strlen(response), 0)) < 0)
      {
         printf("Error sending message.\n");
      }
      i++;
   }

   // Close socket
   close(cli_sock);

   return 0;
}
