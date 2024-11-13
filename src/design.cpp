#include "bus.h"
#include "initiator.h"
#include "target.h"
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

class toplevel : sc_module {
public:
  sc_clock clock;
  bus *bus1;
  mem<1024> *target1;
  mem<1024> *target2;
  generic_initiator *init1;
  generic_initiator *init2;

  toplevel(const sc_module_name &name)
      : sc_module(name), clock("clock", 2, SC_NS) {
    // Create modules
    bus1 = new bus("bus", 2); // Now handling 2 initiators
    target1 = new mem<1024>("target1");
    target2 = new mem<1024>("target2");

    // Create initiators
    init1 = new generic_initiator("init1", 0);
    init2 = new generic_initiator("init2", 1);

    // Bind clock
    init1->clock(clock);
    init2->clock(clock);

    // Bind initiators to bus using TLM sockets
    init1->initiator_socket.bind(bus1->target_socket);
    init2->initiator_socket.bind(bus1->target_socket);

    // Bind bus to targets using TLM sockets
    bus1->initiator_socket[0].bind(target1->target_socket);
    bus1->initiator_socket[1].bind(target2->target_socket);

    // Register address ranges with bus
    bus1->register_target(0x100, 0x400);
    bus1->register_target(0x600, 0x400);
  }

  ~toplevel() {
    delete bus1;
    delete target1;
    delete target2;
    delete init1;
    delete init2;
  }
};
