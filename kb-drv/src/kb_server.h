#pragma once

#include <l4/my_pkg/i_kb_server.h>

#include <l4/sys/cxx/ipc_epiface>
#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/re/util/object_registry>
#include <l4/re/util/br_manager>

#include <vector>
#include <set>
#include <mutex>
#include <map>

class KbServer : public L4::Epiface_t<KbServer, KeyboardInterface>
{
  public:
  int op_register_irq(KeyboardInterface::Rights, const L4::Ipc::Snd_fpage &irq);
  int op_is_pressed(KeyboardInterface::Rights, char key, bool &value);
  int op_last_action(KeyboardInterface::Rights, char &key, bool &value);

  void release_key(char key);
  void press_key(char key);

  private:
  void trigger_irqs();

  char _last_key;
  bool _last_value;
  std::mutex _mutex;
  std::vector<L4::Cap<L4::Irq>> _irqs;
  std::set<char> _pressed_keys;
};


struct KeyboardArgs
{
  L4::Cap<L4::Irq> *irq;
  KbServer *kb;
  const std::map<l4_uint8_t, char> *keycodes;
  l4_cap_idx_t *pfc;
};


void *irq_receive(void *args);