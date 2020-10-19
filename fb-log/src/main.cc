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

  TextBox tb;

  std::string opt = "-s";
  bool tb_found = false;
  for (int i = 1; i < argc; ++i)
  {
    if (opt.compare(argv[i]) == 0 && i+1 < argc)
    {
      std::cout << "Found -s option: " << argv[i+1] << '\n';
      
      if (tb.init(0, 0, 0, 0, argv[i+1]))
        return 1;
      tb_found = true;
      
      break;
    }
  }

  SessionServer sessionServer(server, tb_found ? &tb : nullptr);  
  if (!server.registry()->register_obj(&sessionServer, "server").is_valid())
  {
    std::cerr << "Could not register \"server\" service!\n";
    return 1;
  }
  std::cout << "Service \"server\" registered!\n";

  tb.print("Long line: abcdefghjklmnoprqstuwz1234567890abcdefghjklmnoprqstuwz1234567890abcdefghjklmnoprqstuwz1234567890abcdefghjklmnoprqstuwz1234567890abcdefghjklmnoprqstuwz1234567890abcdefghjklmnoprqstuwz1234567890");
  tb.print("Hello line 2");
  tb.print("Hello line 3");
  tb.print("Hello line 4");
  tb.print("Long line: abcdefghjklmnoprqstuwz1234567890abcdefghjklmnoprqstuwz1234567890");


  server.loop();

  return 0;
}
