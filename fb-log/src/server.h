#pragma once

#include <l4/my_pkg/i_server.h>

#include <l4/sys/cxx/ipc_epiface>
#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/sys/cxx/ipc_string>
#include <l4/re/util/object_registry>
#include <l4/re/util/br_manager>
#include <l4/re/util/video/goos_fb>
#include <l4/re/video/goos>
#include <l4/libgfxbitmap/bitmap.h>
#include <l4/libgfxbitmap/font.h>

#include <stdio.h>
#include <vector>
#include <string>
#include <string.h>

class FbProxy
{
  public:
  int init(const char *service);
  void print(int xpos, int ypos, const char* text,
    gfxbitmap_color_pix_t color, gfxbitmap_color_pix_t background);
  void rgb_print(int xpos, int ypos, const char* text,
    gfxbitmap_color_t rgb_color, gfxbitmap_color_t rgb_background);

  private:
  bool _active{false};
  l4re_video_view_info_t* _info_ptr{nullptr};
  L4Re::Video::View::Info _info;
  void *_base;
  L4::Cap<L4Re::Video::Goos> _goos;
  L4Re::Util::Video::Goos_fb _fb;
};


class TextBox
{
  public:
  int init(unsigned int xLeft, unsigned int yTop,
    unsigned int xRight, unsigned int yBottom,
     const char* service);
  void print(const char* text);
  // TODO: colors, font size etc

  private:
  bool _active{false};
  unsigned int _xLeft;
  unsigned int _xRight;
  unsigned int _yTop;
  unsigned int _yBottom;
  unsigned int _yCurrent;
  gfxbitmap_font_t _font{GFXBITMAP_DEFAULT_FONT};
  unsigned int _fontHeight;
  unsigned int _fontWidth;
  FbProxy _proxy;
};


class Server : public L4::Epiface_t<Server, IServer>
{
  public:
  Server(const char *tag, TextBox *tb = nullptr) : _tag(tag), _tb(tb) {}
  int op_log(IServer::Rights, L4::Ipc::String<> s);

  private:
  std::string _tag;
  TextBox *_tb;
};


class SessionServer : public L4::Epiface_t<SessionServer, L4::Factory>
{
  public:
  SessionServer(L4Re::Util::Registry_server<> &server, TextBox *tb = nullptr); 
  ~SessionServer();

  int op_create(
    L4::Factory::Rights, L4::Ipc::Cap<void> &res, 
    l4_mword_t type, L4::Ipc::Varg_list_ref args_ref);

  private:
  L4Re::Util::Registry_server<> &_registry_server;
  std::vector<Server*> _servers;
  TextBox* _tb;
};

