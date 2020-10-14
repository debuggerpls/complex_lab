#include <l4/my_pkg/i_server.h>

#include <l4/sys/cxx/ipc_epiface>
#include <l4/re/util/object_registry>
#include <l4/re/util/br_manager>
#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/sys/cxx/ipc_string>

#include <stdio.h>
#include <string>
#include <iostream>


class Server : public L4::Epiface_t<Server, IServer>
{
  public:

  int op_log(IServer::Rights, L4::Ipc::String<> s)
  {
    std::cout << _tag << ": " << s.data << "\n";
    //printf("From %s: %s\n", _tag, s.data);

    return 0;
  }

  Server(const std::string &tag) : _tag(tag) {}

  private:
  std::string _tag;
};

class SessionServer : public L4::Epiface_t<SessionServer, L4::Factory>
{
  public:
  SessionServer(L4Re::Util::Registry_server<> &server) : _registry_server(server) {}

  int op_create(L4::Factory::Rights, L4::Ipc::Cap<void> &res, 
                l4_mword_t type, L4::Ipc::Varg_list<> &&args)
  {
    if (type != 0) 
      return -L4_ENODEV;

    L4::Ipc::Varg tag = args.pop_front();
    if (!tag.is_of<char const*>()) 
      return -L4_EINVAL;

    std::string tag_str(tag.value<char const*>());
    auto new_server = new Server(tag_str);

    if (!_registry_server.registry()->register_obj(new_server).is_valid())
    {
      printf("Could not register service!\n");
      return -L4_EINVAL;
    }

    res = L4::Ipc::make_cap_rw(new_server->obj_cap());

    return L4_EOK;
  }

  private:
  L4Re::Util::Registry_server<> &_registry_server;
};


int main()
{
  printf("Hello world!\n");

  /*
  Server server_obj;

  if (!server.registry()->register_obj(&server_obj, "server").is_valid())
  {
    std::cerr << "Could not register service!\n";
    return 1;
  }

  std::cout << "Service registered!\n";
  */

  L4Re::Util::Registry_server<> server;

  SessionServer sessionServer(server);

  if (!server.registry()->register_obj(&sessionServer, "server").is_valid())
  {
    printf("Could not register service!\n");
    return 1;
  }

  printf("Service registered!\n");

  server.loop();

  return 0;
}
