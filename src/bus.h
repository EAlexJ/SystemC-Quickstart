#ifndef BUS_H
#define BUS_H
#include <systemc>
#include "target.h"
#include "initiator.h"
#include <map>

using namespace sc_core;
using namespace sc_dt;

class bus : public sc_module, public initiator_if{
  public:
 	sc_in<bool> clock;
 	void write(const sc_uint<8> address, const sc_uint<12> data, int id);
	void read(const sc_uint<8> address, sc_uint<12>& data, int id);
	void control_bus();
  	sc_port<target_if>* find_port(const sc_uint<8> addr);
  	void register_target(sc_core::sc_port<target_if>& port, sc_uint<8> start, sc_uint<8> size);
  	bus(const sc_core::sc_module_name& name, int n_initiators);
  private:
  	int num_initiators;
    sc_vector<sc_signal<bool>> request;
    sc_vector<sc_event> proceed;
    std::map<sc_uint<8>, std::pair<sc_core::sc_port<target_if>*, sc_uint<8>>> address_map;
    void end_of_elaboration();
  	void arbitrate(int id);
  
};
#endif