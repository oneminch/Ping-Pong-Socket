#include "pingpong.h"
#include <stdio.h>

#include <unistd.h>

int main()
{
  int port = 10000;
  int retries = 100;

  while (retries--)
  {
    if (!run_server(port))
    {
      while (1)
      {
        sleep(10);
      }
    }
    else
    {
      port++;
    }
  }
  printf("Failed to start server\n");

  return 1;
}
