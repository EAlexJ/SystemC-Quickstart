#ifndef BUS_H
#define BUS_H
#include "initiator.h"
#include "target.h"
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

class bus : public sc_module, public initiator_if {
public:
  sc_port<target_if, 2> target_ports;
  void write(const sc_uint<12> address, const sc_uint<12> data,
             int id) override;
  void read(const sc_uint<12> address, sc_uint<12> &data, int id) override;
  void control_bus();
  int find_port(const sc_uint<12> addr);
  void register_target(sc_uint<12> start, sc_uint<12> size);
  bus(const sc_core::sc_module_name &name, int n_initiators);

private:
  int num_initiators;
  sc_vector<sc_signal<bool>> request;
  sc_vector<sc_event> proceed;
  sc_event control_bus_e;
  std::vector<std::pair<sc_uint<12>, sc_uint<12>>>
      address_map; // start_addr -> size
  void end_of_elaboration() override;
  void arbitrate(int id);
};
#endif
