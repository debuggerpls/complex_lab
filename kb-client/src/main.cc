#include <l4/my_pkg/i_kb_server.h>

#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/re/error_helper>

#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <pthread-l4.h>


int error_msg(const char* str);

int main()
{
  std::cout << "Starting..\n"; 

  auto keyboard = L4Re::Env::env()->get_cap<KeyboardInterface>("keyboard");
  if (!keyboard.is_valid())
  {
    return error_msg("Could not get \"keyboard\" cap!");
  }

  std::cout << "Got \"keyboard\" cap!\n";

  auto irq = L4Re::Util::cap_alloc.alloc<L4::Irq>();
  if (!irq.is_valid())
  {
    return error_msg("Could not allocalte IRQ capacity!");
  }
  
  L4Re::chksys(L4Re::Env::env()->factory()->create<L4::Irq>(irq), "Failed to create IRQ");

  L4::Cap<L4::Thread> irq_cap(pthread_l4_cap(pthread_self()));

  auto err = irq->bind_thread(irq_cap, 0xdead);
  if (err.has_error())
  {
    return error_msg("Could not bind IRQ to main thread!");
  }

  if (keyboard->register_irq(irq))
  {
    return error_msg("Could not register IRQ to Keyboard server!");
  }

  std::cout << "Starting receive loop..\n";
  for (;;)
  {
    err = irq->receive();
    //std::cout << "Received IRQ, ";
    
    if (err.has_error())
    {
      std::cerr << "Error on IRQ receive: " << err.has_error() << '\n';
    }
    else
    {
      //std::cout << "no errors, calling is_pressed()\n";
      bool value = false;
      if (keyboard->is_pressed('a', &value))
      {
        std::cerr << "Error while calling is_pressed()\n";
      }
      else
      {
        std::cout << "A is pressed: [" << (value ? "x" : " ") << "]\n";
      }
    }
    
  }

  return 0;
}

int error_msg(const char* str)
{
  printf("Error: %s\n", str); 
  return 1;
}