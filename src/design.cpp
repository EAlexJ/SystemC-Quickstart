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
  mem<256> *target1;
  mem<256> *target2;
  generic_initiator *init1;
  generic_initiator *init2;

  toplevel(const sc_module_name &name)
      : sc_module(name), clock("clock", 2, SC_NS) {
    // Create modules
    bus1 = new bus("bus", 2); // Now handling 2 initiators
    target1 = new mem<256>("target1");
    target2 = new mem<256>("target2");

    // Create initiators
    init1 = new generic_initiator("init1", 0);
    init2 = new generic_initiator("init2", 1);

    // Bind clock
    init1->clock(clock);
    init2->clock(clock);
    bus1->clock(clock);

    // Bind initiators to bus
    init1->initiator_port(*bus1);
    init2->initiator_port(*bus1);

    // Bind bus to targets
    bus1->target_ports(*target1);
    bus1->target_ports(*target2);

    // Register address ranges with bus
    bus1->register_target(0x100, 0x100);
    bus1->register_target(0x400, 0x100);
  }

  ~toplevel() {
    delete bus1;
    delete target1;
    delete target2;
    delete init1;
    delete init2;
  }
};
