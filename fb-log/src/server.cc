#include "server.h"

#include <iostream>


SessionServer::SessionServer(L4Re::Util::Registry_server<> &server, TextBox *tb) 
  : _registry_server(server), _tb(tb)
{} 

SessionServer::~SessionServer()
{
  for (auto s: _servers)
    delete s;
}

int Server::op_log(IServer::Rights, L4::Ipc::String<> s)
{
  std::string tmp = _tag + ": " + s.data;
  std::cout << tmp << "\n";
  
  if (_tb)
    _tb->print(tmp.c_str());

  return 0;
}



int SessionServer::op_create(L4::Factory::Rights, L4::Ipc::Cap<void> &res, 
  l4_mword_t type, L4::Ipc::Varg_list_ref args_ref)
{
  L4::Ipc::Varg_list<> args(args_ref);

  if (type != 0) 
    return -L4_ENODEV;

  L4::Ipc::Varg tag = args.pop_front();
  if (!tag.is_of<char const*>()) 
    return -L4_EINVAL;

  auto new_server = new Server(tag.value<char const*>(), _tb);
  _servers.push_back(new_server);

  if (!_registry_server.registry()->register_obj(new_server).is_valid())
  {
    std::cerr << "Could not register service!\n";
    return -L4_EINVAL;
  }

  res = L4::Ipc::make_cap_rw(new_server->obj_cap());

  return L4_EOK;
}



int FbProxy::init(const char *service)
{
  _goos = L4Re::Env::env()->get_cap<L4Re::Video::Goos>(service);
  if (!_goos.is_valid())
  {
    std::cerr << "FbProxy: could not get \"" << service << "\" service\n";
    return 1;
  }
  std::cout << "FbProxy: got \"" << service << "\" service\n";

  _fb.setup(_goos);

  _base = _fb.attach_buffer();
  int res = _fb.view_info(&_info);
  if (res)
  {
    std::cerr << "FbProxy: could not view_info\n";
    return 1;
  }
  std::cout << "FbProxy: got view info\n";

  _info_ptr = reinterpret_cast<l4re_video_view_info_t*>(&_info);

  if (gfxbitmap_font_init())
  {
    std::cerr << "FbProxy: could not init gfxbitmap_font\n";
    return 1;
  }
  std::cout << "FbProxy: init gfxbitmap: ok\n";

  _active = true;
  
  // Test print
  //rgb_print(0, 0, "Test print!", 0xff0000, 0x0);

  return 0;
}



void FbProxy::print(int xpos, int ypos, const char *text,
  gfxbitmap_color_pix_t color, gfxbitmap_color_pix_t background)
{
  if (_active)
  {
    gfxbitmap_font_text(_base, _info_ptr, GFXBITMAP_DEFAULT_FONT,
      text, GFXBITMAP_USE_STRLEN, xpos, ypos, color, background);
  }
  else
  {
    std::cerr << "FbProxy: not printing, not active!\n";
    return;
  }
}



void FbProxy::rgb_print(int xpos, int ypos, const char *text,
  gfxbitmap_color_t rgb_color, gfxbitmap_color_t rgb_background)
{
  if (_active)
  {
    gfxbitmap_font_text(_base, _info_ptr, GFXBITMAP_DEFAULT_FONT,
      text, GFXBITMAP_USE_STRLEN, xpos, ypos, 
      gfxbitmap_convert_color(_info_ptr, rgb_color),
      gfxbitmap_convert_color(_info_ptr, rgb_background));
  }
  else 
  {
    std::cerr << "FbProxy: not printing, not active!\n";
    return;
  }
}



int TextBox::init(unsigned int xLeft, unsigned int yTop,
    unsigned int xRight, unsigned int yBottom,
    const char* service)
{
  auto res = _proxy.init(service);
  if (res)
    return res;

  _xLeft = xLeft;
  _yTop =yTop;
  _xRight = xRight;
  _yBottom = yBottom;
  _yCurrent = yTop;

  // TODO: font colors
  _fontHeight = gfxbitmap_font_height(_font);
  _fontWidth = gfxbitmap_font_width(_font);
  
  _active = true;
  return 0;
}


void TextBox::print(const char* text)
{
  // TODO: colors
  if (_active)
  {
    auto len = strlen(text);
    if (len * _fontWidth < _xRight - _xLeft)
    {
      _proxy.rgb_print(_xLeft, _yCurrent, text, 0xffffff, 0x0);
      _yCurrent += _fontHeight;
    }
    else
    {
      auto fitting = (_xRight - _xLeft) / _fontWidth;
      auto done = false;
      unsigned int pos = 0;
      while (!done)
      {
        if (len - pos <= fitting)
        {
          _proxy.rgb_print(_xLeft, _yCurrent, &text[pos], 0xff0000, 0x0);
          done = true;
        }
        else
        {
          std::string tmp{&text[pos], fitting};
          pos += fitting;
          _proxy.rgb_print(_xLeft, _yCurrent, tmp.c_str(), 0xff0000, 0x0);
        }
        _yCurrent += _fontHeight;
      }
    }
    
  }
  else
    std::cerr << "TextBox: not active!\n";
}
