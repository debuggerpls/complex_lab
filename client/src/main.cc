#include <l4/my_pkg/i_server.h>

#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/sys/cxx/ipc_string>

#include <l4/re/util/video/goos_fb>
#include <l4/re/video/goos>

#include <iostream>
#include <unistd.h>

int error_msg(const char* str);

int main()
{
  std::cout << "Hello world!\n"; 

  L4::Cap<IServer> server = L4Re::Env::env()->get_cap<IServer>("server");
  if (!server.is_valid())
    return error_msg("Could not get server cap!");


  if (server->log(L4::Ipc::String<>("Hello world!")))
    return error_msg("Could not send to server!");

  std::cout << "Sent successful!\n";

  std::cout << "Drawing\n";

  /*
  L4::Cap<L4Re::Video::Goos> goos = L4Re::Env::env()->get_cap<L4Re::Video::Goos>("fb");
  if (!server.is_valid())
    return error_msg("Could not get fb cap!");
  */

  sleep(5);

  L4Re::Util::Video::Goos_fb fb("fb");
  //fb.setup(goos);
  
  auto base = fb.attach_buffer();
  L4Re::Video::View::Info info;
  int r = fb.view_info(&info);
  if (r != 0)
  {
    std::cerr << "Could not view_info\n";
    return 1;
  }

  for (int y = 10; y < 50; ++y)
  {
    for (int x = 10; x < 50; ++x)
    {
      auto addr = base + y * (info.pixel_info.bytes_per_pixel() * info.width)
                  + x * info.pixel_info.bytes_per_pixel();

      L4Re::Video::Pixel_info pixel(info.pixel_info.bits_per_pixel(), 255, 0, 255, 0, 255, 0, 255, 0);
      *static_cast<unsigned*>(addr) = 255;
    }
  }


  return 0;
}

int error_msg(const char* str)
{
  printf("Error: %s\n", str); 
  return 1;
}