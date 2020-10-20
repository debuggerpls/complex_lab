#include "kb_server.h"

#include <l4/util/port_io.h>
#include <iostream>

// REMOVE WHEN REBOOT IS NOT NEEDED
#include <l4/re/env.h>
#include <l4/sys/platform_control.h>


int KbServer::op_register_irq(KeyboardInterface::Rights, const L4::Ipc::Snd_fpage &irq)
{
  if (!irq.cap_received())
    return -L4_EMSGMISSARG;

  auto irq_cap = server_iface()->rcv_cap<L4::Irq>(0);
  if (!irq_cap.is_valid())
    return -L4_EINVAL;

  if (server_iface()->realloc_rcv_cap(0) < 0)
    return -L4_ENOMEM;

  _mutex.lock();
  _irqs.push_back(irq_cap);
  _mutex.unlock();

  return L4_EOK;
}


int KbServer::op_is_pressed(KeyboardInterface::Rights, char key, bool &value)
{
  _mutex.lock();
  value = _pressed_keys.find(key) != _pressed_keys.end();
  _mutex.unlock();

  return L4_EOK;
}


void KbServer::release_key(char key)
{
  _mutex.lock();
  auto res = _pressed_keys.erase(key);
  _mutex.unlock();
  
  if (res > 0)
    trigger_irqs();
}


void KbServer::press_key(char key)
{
  _mutex.lock();
  auto res = _pressed_keys.insert(key);  
  _mutex.unlock();

  if (res.second)
    trigger_irqs();
}


void KbServer::trigger_irqs()
{
  for (auto &irq: _irqs)
    irq->trigger();
}



void *irq_receive(void *args)
{
  KeyboardArgs *t_args = (KeyboardArgs*) args;

  for(;;)
  {
    auto err = (*(t_args->irq))->receive();
    if (err.has_error())
    {
      printf("Error on IRQ receive: %d", err.has_error());
    }
    else
    {
      auto val = l4util_in8(0x60);
      auto key_val = val & ~(0x80);
      auto key = t_args->keycodes->find(key_val);
      if (key != t_args->keycodes->end())
      {
        auto key_char = key->second;
        if (val > 128)
        {
          std::cout << "Released: " << key_char << '\n';
          //t_args->mutex->lock();
          //t_args->pressed->erase(val);
          //t_args->mutex->unlock();
          t_args->kb->release_key(key_char);
        }
        else
        {
          std::cout << "Pressed: " << key_char << '\n';
          //t_args->mutex->lock();
          //t_args->pressed->insert(val);
          //t_args->mutex->unlock();
          t_args->kb->press_key(key_char);
        }
      }
      else
      {
        std::cerr << "Unknown keycode: " << static_cast<unsigned int>(val) << '\n';

        // TODO: Remove when reboot is not needed
        if (val == 1)
        {
          if (t_args->pfc)
          {
            std::cout << "Rebooting..\n";
            l4_platform_ctl_system_shutdown(*(t_args->pfc), 1);
          }
          else
          {
            std::cerr << "pfc is nullptr\n";
          }
          
          
          /*
          l4_cap_idx_t pfc = l4re_env_get_cap("pfc");
          if (l4_is_valid_cap(pfc))
            l4_platform_ctl_system_shutdown(pfc, 1);
          else
            std::cerr << "Could not get pfc capability! Cannot reboot!\n";
          */         
        }

      }
    }
  }
}