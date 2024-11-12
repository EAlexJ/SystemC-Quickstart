#ifndef INITIATOR_H
#define INITIATOR_H
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

class initiator_if : virtual public sc_interface {
public:
  virtual void write(const sc_uint<12> address, const sc_uint<12> data,
                     int id) = 0;
  virtual void read(const sc_uint<12> address, sc_uint<12> &data, int id) = 0;
};

class generic_initiator : public sc_module {
public:
  sc_port<initiator_if> initiator_port;
  sc_in<bool> clock;

  SC_HAS_PROCESS(generic_initiator);
  generic_initiator(const sc_module_name &name, int id);

  void write(const sc_uint<12> address, const sc_uint<12> data);
  void read(const sc_uint<12> address, sc_uint<12> &data);

private:
  const int m_id;
};

#endif
