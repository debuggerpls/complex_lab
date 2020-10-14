#include <l4/my_pkg/i_server.h>

#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/sys/cxx/ipc_string>

#include <stdio.h>

int error_msg(const char* str);

int main()
{
  printf("Hello world!\n"); 

  L4::Cap<IServer> server = L4Re::Env::env()->get_cap<IServer>("server");
  if (!server.is_valid())
    return error_msg("Could not get server cap!");

  if (server->log(L4::Ipc::String<>("Hello world!")))
    return error_msg("Could not send to server!");
  
  printf("Sent successful!\n");

  return 0;
}

int error_msg(const char* str)
{
  printf("Error: %s\n", str); 
  return 1;
}