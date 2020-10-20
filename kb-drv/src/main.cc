#include <l4/sys/irq>
#include <l4/sys/factory>
#include <l4/sys/icu.h>

#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/re/error_helper>

#include <l4/util/port_io.h>
#include <l4/io/io.h>

#include <pthread-l4.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <map>

#include "kb_server.h"


int main(void)
{
  if(l4io_request_ioport(0x60, 1) != 0)
  {
    printf("Failed requesting ioport\n");
    return 1;
  }

  auto icu = L4Re::Env::env()->get_cap<L4::Icu>("icu");
  if (!icu.is_valid())
  {
    std::cerr << "Could not get ICU capacity\n";
    return 1;
  }

  auto irq = L4Re::Util::cap_alloc.alloc<L4::Irq>();
  if (!irq.is_valid())
  {
    std::cerr << "Could not allocalte IRQ capacity!\n";
    return 1;
  }
  
  L4Re::chksys(L4Re::Env::env()->factory()->create<L4::Irq>(irq), "Failed to create IRQ");
  //auto err = L4Re::Env::env()->factory()->create<L4::Irq>(irq);
  /*
  if (err.has_error())
  {
    std::cerr << "Could not create IRQ!\n";
    return 1;
  }
  */

  auto err = icu->bind(1, irq);
  if (err.has_error())
  {
    std::cerr << "Could not bind IRQ to ICU\n";
    return 1;
  }

  const std::map<l4_uint8_t, char> Keycodes
  {
    {2, '1'}, {3, '2'}, {4, '3'}, {5, '4'}, {6, '5'}, {7, '6'}, {8, '7'}, {9, '8'}, {10, '9'}, {11, '0'},
    {16, 'q'}, {17, 'w'}, {18, 'e'}, {19, 'r'}, {20, 't'}, {21, 'z'}, {22, 'u'}, {23, 'i'}, {24, 'o'}, {25, 'p'},
    {30, 'a'}, {31, 's'}, {32, 'd'}, {33, 'f'}, {34, 'g'}, {35, 'h'}, {36, 'j'}, {37, 'k'}, {38, 'l'}, //{39, 'ö'},
    {44, 'y'}, {45, 'x'}, {46, 'c'}, {47, 'v'}, {48, 'b'}, {49, 'n'}, {50, 'm'}, {51, ','}, {52, '.'}, {53, '-'},
  };

  // TODO: find out why L4Re::Util::Br_manager_hooks is needed
  L4Re::Util::Registry_server<L4Re::Util::Br_manager_hooks> server;
  KbServer kb_server;

  if (!server.registry()->register_obj(&kb_server, "keyboard").is_valid())
  {
    std::cerr << "Could not register \"keyboard\" service!\n";
    return 1;
  }
  std::cout << "Service \"keyboard\" registered!\n";

  auto pfc = icu.cap();

  KeyboardArgs args{ &irq, &kb_server, &Keycodes, &pfc };

  pthread_t irq_thread;
  pthread_create(&irq_thread, NULL, irq_receive, (void *)&args);

  L4::Cap<L4::Thread> irq_thread_cap(pthread_l4_cap(irq_thread));

  err = irq->bind_thread(irq_thread_cap, 0xaffe);
  if (err.has_error())
  {
    std::cerr << "Could not bind IRQ to receiving thread\n";
    return 1;
  }

  std::cout << "IRQ was bound to receiving thread\n";

  // TODO: clean up resources etc  
  //tag = l4_irq_detach(irqcap);
  //if ((err = l4_error(tag)))
  //  printf("Error detach from IRQ: %d\n", err);

  std::cout << "Starting server loop..\n";

  server.loop();

  return 0;
}