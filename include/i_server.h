#pragma once

#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/sys/cxx/ipc_string>

struct IServer : public L4::Kobject_t<IServer, L4::Kobject>
{
  L4_INLINE_RPC(int, log, (L4::Ipc::String<> s));
  typedef L4::Typeid::Rpcs<log_t> Rpcs;
};