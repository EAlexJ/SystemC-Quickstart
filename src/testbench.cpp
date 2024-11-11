// Code your testbench here.
// Uncomment the next line for SystemC modules.
#include <systemc>
#include "design.cpp"
int sc_main(int, char*[]){
	toplevel my_top("my_top");
  	sc_clock clk("clk", 10, SC_NS);

  	my_top.bus1->clock(clk);
  	sc_start(50, SC_NS);
  	//my_top.bus1->write(0x1000, 10, 1);
}