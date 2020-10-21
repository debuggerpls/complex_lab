#include <l4/my_pkg/i_kb_server.h>

#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/re/error_helper>

#include <iostream>
#include <set>
#include <unistd.h>
#include <pthread.h>
#include <pthread-l4.h>


int error_msg(const char* str);

int main(int argc, char **argv)
{
  std::cout << "Starting..\n";

  std::set<char> chars;
  for (int i = 1; i < argc; ++i)
  {
    chars.insert(std::tolower(argv[i][0]));
  }

  if (chars.empty())
  {
    std::cerr << "Client was not provided with lowercase chars!\n";
    return 1;
  }

  std::cout << "Listening for keys: ";
  for (auto c: chars)
    std::cout << "\'" << c << "\', ";
  std::cout << '\n';


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
      for (auto c: chars)
      {
        bool value = false;
        if (keyboard->is_pressed(c, &value))
        {
          std::cerr << "Error while calling is_pressed()\n";
        }
        else
        {
          std::cout << "\'" << c << "\':[" << (value ? "x" : " ") << "], ";
        }
      }

      /*
      {
        // For testing only
        char c;
        bool v;
        keyboard->last_action(&c, &v);
        std::cout << "LK: \'" << c << "\': [" << v << "]";
      }
      */
      std::cout << '\n';
    }
    
  }

  return 0;
}

int error_msg(const char* str)
{
  printf("Error: %s\n", str); 
  return 1;
}