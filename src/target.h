#ifndef TARGET_H
#define TARGET_H
#include <systemc>
#include <array>

using namespace sc_core;
using namespace sc_dt;
class target_if : virtual public sc_interface{
	public:
  		virtual void target_write(const sc_uint<8> address, const sc_uint<12> data) = 0;
  		virtual void target_read(const sc_uint<8> address, sc_uint<12> data) = 0;
};
template <unsigned int MEM_SIZE = 64>
class mem : sc_module, target_if{
  public:
	mem(const sc_core::sc_module_name& name, unsigned int start);
  	void target_write(const sc_uint<8> address, const sc_uint<12> data) override;
	void target_read(const sc_uint<8> address, sc_uint<12> data) override;
  private:
  	const unsigned int start, size;
  	std::array<sc_uint<12>, MEM_SIZE> memory;
};
//definitions
template <unsigned int MEM_SIZE>
  mem<MEM_SIZE>::mem(const sc_core::sc_module_name& name, unsigned int start) :sc_module(name), start(start), size(MEM_SIZE){
  }

template <unsigned int MEM_SIZE>
  void mem<MEM_SIZE>::target_write(const sc_uint<8> address, const sc_uint<12> data){
    std::cout << "Writing to address " << address << " data " << data << std::endl;
    memory.at(address-start) = data;
  
  }
template <unsigned int MEM_SIZE>
  void mem<MEM_SIZE>::target_read(const sc_uint<8> address, sc_uint<12> data){
    std::cout << "Reading from address " << address << " data " << data << std::endl;
    data = memory.at(address-start);
  }
#endif