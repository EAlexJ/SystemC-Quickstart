#ifndef INITIATOR_H
#define INITIATOR_H

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>

using namespace sc_core;
using namespace sc_dt;

class generic_initiator : public sc_module {
public:
  tlm_utils::simple_initiator_socket<generic_initiator> initiator_socket;
  sc_in<bool> clock;

  SC_HAS_PROCESS(generic_initiator);
  generic_initiator(const sc_module_name &name, int id);

  void write(const sc_uint<12> address, const sc_uint<12> data);
  void read(const sc_uint<12> address, sc_uint<12> &data);

private:
  const int m_id;
  sc_time timing_annotation;
};

#endif
