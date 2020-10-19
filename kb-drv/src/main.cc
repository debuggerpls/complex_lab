#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/c/namespace.h>
#include <l4/sys/utcb.h>
#include <l4/sys/irq.h>
#include <l4/sys/factory.h>
#include <l4/sys/icu.h>

#include <contrib/libio-io/l4/io/io.h>
 
#include <stdio.h>
#include <unistd.h>

#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <iostream>
#include <thread>
#include <functional>
#include <l4/re/error_helper>
#include <pthread-l4.h>
#include <l4/sys/thread>


void polling(L4::Cap<L4::Irq> &irq);

void polling(L4::Cap<L4::Irq> &irq)
{
  try
  {
    for (;;)
    {
      /*
      auto err = irq->receive();

      if (err.has_error())
      {
        printf("Error on IRQ receive: %d", err.has_error());
      }
      else
      {
        printf("Got IRQ with label 0x%1X\n", err.label());
      }
      */
      L4Re::chksys(irq->receive(), "Error on IRQ receive");
      std::cout << "Received IRQ\n";
    }
  }
  catch(L4::Runtime_error const &e)
  {
    std::cerr << "Keyboard polling exited with exception: " << e.str() << '\n';
  }
  
}


int main(void)
{
  //int irqno = 1;
  //l4_cap_idx_t irqcap, icucap;
  //l4_msgtag_t tag;
  //int err;

  if(l4io_request_ioport(0x60, 1) != 0)
  {
    printf("Failed requesting ioport\n");
    return 1;
  }

  //L4::Cap<L4::Icu> icu{ l4re_env_get_cap("icu") };
  auto icu = L4Re::chkcap(L4Re::Env::env()->get_cap<L4::Icu>("icu"),
                    "Failed to obtain ICU capability");
  if (!icu.is_valid())
  {
    std::cerr << "Could not get icu cap\n";
    return 1;
  }

  auto irq = L4Re::Util::cap_alloc.alloc<L4::Irq>();
  if (!irq.is_valid())
  {
    std::cerr << "Could not allocalte cap irq!\n";
    return 1;
  }

  
  auto err = L4Re::Env::env()->factory()->create_irq(irq);
  if (err.has_error())
  {
    std::cerr << "Could not create_irq!\n";
    return 1;
  }
  
  //L4Re::chksys(L4Re::Env::env()->factory()->create(irq), "Failed to create IRQ kernel obj");

  /*
  err = icu->bind(1, irq);
  if (err.has_error())
  {
    std::cerr << "Could not bind irq to icu\n";
    return 1;
  }
  */


  
  err = irq->bind_thread(L4Re::Env::env()->main_thread(), 0xaffe);
  if (err.has_error())
  {
    std::cerr << "Could not bind irq to main thread\n";
    return 1;
  }

  err = icu->bind(1, irq);
  if (err.has_error())
  {
    std::cerr << "Could not bind irq to icu\n";
    return 1;
  }
  

  //std::thread polling_thread(std::ref(irq));



  printf("Attached to key IRQ %d\nPress keys now, Shift-Q to exit\n", 1);
 
  //L4::Cap<L4::Thread> polling_cap(pthread_l4_cap(polling_thread.native_handle()));

  //L4Re::chksys(irq->bind_thread(polling_cap, 0xaffe), "Failed t attach to irq\n");

  //polling_thread.join();


  /* IRQ receive loop */
  
  while (1)
  {
    unsigned long label = 0;

    err = irq->receive();

    if (err.has_error())
    {
      printf("Error on IRQ receive: %d", err.has_error());
    }
    else
    {
      printf("Got IRQ with label 0x%1X\n", err.label());
      //irq->trigger();
    }

  }
  
  /* We're done, detach from the interrupt. */
  //tag = l4_irq_detach(irqcap);
  //if ((err = l4_error(tag)))
  //  printf("Error detach from IRQ: %d\n", err);
  
  std::cout << "Out of loop\n";

  return 0;
}