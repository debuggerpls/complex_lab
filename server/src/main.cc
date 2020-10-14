#include <l4/my_pkg/i_server.h>

#include <l4/sys/cxx/ipc_epiface>
#include <l4/re/util/object_registry>
#include <l4/re/util/br_manager>
#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/sys/cxx/ipc_string>

#include <iostream>


class Server : public L4::Epiface_t<Server, IServer>
{
  public:
  int op_log(IServer::Rights, L4::Ipc::String<> s)
  {
    std::cout << "Received: " << s.data << "\n";

    return 0;
  }
};


int main(int argc, char **argv)
{
  std::cout << "Hello world!\n";

  L4Re::Util::Registry_server<L4Re::Util::Br_manager_hooks> server;
  Server server_obj;

  if (!server.registry()->register_obj(&server_obj, "server").is_valid())
  {
    std::cerr << "Could not register service!\n";
    return 1;
  }

  std::cout << "Service registered!\n";

  server.loop();

  return 0;
}
