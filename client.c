#include "pingpong.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    printf("give port as command line argument\n");
    return 1;
  }

  int port = atoi(argv[1]);

  char *dest = "localhost";

  ConnectionInfo info;
  info.msg_size = 0;

  if (connect_to_server(dest, port, &info))
  {
    printf("Failed to connect to server\n");
    return 1;
  }

  while (1)
  {
    char buff[4096];
    memset(buff, '\0', strlen(buff));

    // Read a string
    printf("\nEnter message to send: ");
    fgets(buff, 4096, stdin);

    int len = strlen(buff);
    buff[len - 1] = 0; // Remove trailing \n

    // Send the string
    sendMessage(&info, buff);

    // Recieve the response
    char *msg = recieveMessage(&info);
    if (msg == NULL)
    {
      printf("Failed to recieve message\n");
      return 1;
    }

    // Display message received on Server
    printf("\nResponse received from Server: %s\n", msg);
    dealocate_message(msg);
  }

  return 2; //shouldn't ever run
}
