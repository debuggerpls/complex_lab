#pragma once

#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/sys/cxx/ipc_string>
#include <l4/sys/irq>

enum {Kb_proto = 0x53};

struct KeyboardInterface : public L4::Kobject_t<KeyboardInterface, L4::Kobject, Kb_proto, L4::Type_info::Demand_t<1>>
{
  L4_INLINE_RPC(int, register_irq, (L4::Ipc::Cap<L4::Irq> irq));
  L4_INLINE_RPC(int, is_pressed, (char key, bool *value));
  L4_INLINE_RPC(int, last_action, (char *key, bool *value));
  typedef L4::Typeid::Rpcs<register_irq_t, is_pressed_t, last_action_t> Rpcs;
};