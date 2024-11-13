#ifndef BUS_H
#define BUS_H

#include <systemc>
#include <tlm>
#include <tlm_utils/multi_passthrough_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <vector>

using namespace sc_core;
using namespace sc_dt;

class bus : public sc_module {
public:
  // TLM-2 socket, multi-passthrough for multiple initiators
  tlm_utils::multi_passthrough_target_socket<bus> target_socket;

  // TLM-2 socket array, two sockets for two targets
  tlm_utils::simple_initiator_socket<bus> initiator_socket[2];

  SC_HAS_PROCESS(bus);
  bus(const sc_core::sc_module_name &name, int n_initiators);

  void register_target(sc_uint<12> start, sc_uint<12> size);

protected:
  // TLM-2 blocking transport method
  virtual void b_transport(int id, tlm::tlm_generic_payload &trans,
                           sc_time &delay);

private:
  int num_initiators;
  sc_vector<sc_signal<bool>> request;
  sc_vector<sc_event> proceed;
  sc_event control_bus_e;
  std::vector<std::pair<sc_uint<12>, sc_uint<12>>>
      address_map; // start_addr -> size

  void control_bus();
  int find_port(const uint64_t addr);
  void arbitrate(int id);
  void end_of_elaboration() override;
};

#endif
