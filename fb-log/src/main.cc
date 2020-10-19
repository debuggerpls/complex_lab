#include <l4/my_pkg/i_server.h>
#include "server.h"

#include <l4/re/util/object_registry>
#include <l4/re/util/br_manager>
#include <l4/re/env>

#include <stdio.h>
#include <iostream>


int main(int argc, char** argv)
{
  std::cout << "Started..\n";

  L4Re::Util::Registry_server<> server;

  FbProxy fb;

  std::string opt = "-s";
  bool fb_found = false;
  for (int i = 1; i < argc; ++i)
  {
    if (opt.compare(argv[i]) == 0 && i+1 < argc)
    {
      std::cout << "Found -s option: " << argv[i+1] << '\n';
      
      if (fb.init(argv[i+1]))
        return 1;
      fb_found = true;
      
      break;
    }
  }

  SessionServer sessionServer(server, fb_found ? &fb : nullptr);  
  if (!server.registry()->register_obj(&sessionServer, "server").is_valid())
  {
    std::cerr << "Could not register \"server\" service!\n";
    return 1;
  }
  std::cout << "Service \"server\" registered!\n";

  server.loop();

  return 0;
}
