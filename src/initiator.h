#ifndef INITIATOR_H
#define INITIATOR_H
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

class initiator_if : virtual public sc_interface{
	public:
  		virtual void write(const sc_uint<8> address, const sc_uint<12> data, int id) = 0;
  		virtual void read(const sc_uint<8> address, sc_uint<12>& data, int id) = 0;

};
#endif