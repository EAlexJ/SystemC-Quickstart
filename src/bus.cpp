#include <systemc>
#include "bus.h"

bus::bus(const sc_core::sc_module_name& name, int n_initiators) : 
	sc_module(name),
	num_initiators(n_initiators),
	request("request_signals", n_initiators),
	proceed("proceed_events", n_initiators)
	{
    
    SC_HAS_PROCESS(bus);
    SC_THREAD(control_bus);
    sensitive << clock.neg();
}
void bus::register_target(sc_core::sc_port<target_if>& port, sc_uint<8> start, sc_uint<8> size) {
    address_map[start] = std::make_pair(&port, size);
}
inline void bus::arbitrate(int id){
  request[id] = true;
  wait(proceed[id]);
  request[id] = false;
}
void bus::write(sc_uint<8> address, sc_uint<12> data, int id) {
    arbitrate(id);
    auto* target_port = find_port(address);
    if (target_port) {
        (*target_port)->target_write(address, data);
    } else {
        SC_REPORT_ERROR("MyBus", "Failed to find target port for write operation");
    }
}

void bus::read(sc_uint<8> address, sc_uint<12>& data, int id) {
    arbitrate(id);
    auto* target_port = find_port(address);
    if (target_port) {
        (*target_port)->target_read(address, data);
    } else {
        SC_REPORT_ERROR("MyBus", "Failed to find target port for read operation");
    }
}

void bus::control_bus(){
  int highest;
    for(;;){
      wait();
      highest = -1;
      for(int i= 0; i<num_initiators; i++){
          if(request[i])
             highest = i;
      }
      if(highest > -1)
        proceed[highest].notify();
    }
}
sc_port<target_if>* bus::find_port(sc_uint<8> address) {
    for (auto it = address_map.begin(); it != address_map.end(); ++it) {
        sc_uint<8> start = it->first;
        sc_core::sc_port<target_if>* port = it->second.first;
        sc_uint<8> size = it->second.second;
        if (address >= start && address < (start + size)) {
            return port;
        }
    }
    SC_REPORT_ERROR("MyBus", "Address out of range");
  	return nullptr;
}
void bus::end_of_elaboration(){
	return;
}