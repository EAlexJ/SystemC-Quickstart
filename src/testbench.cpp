#include "design.cpp"
#include "logger.h"
#include <random>
#include <sysc/kernel/sc_time.h>
#include <systemc>

LogLevel Logger::currentLevel = LogLevel::WARNING;

SC_MODULE(test_driver) {
  sc_in<bool> clock;
  generic_initiator *init1;
  generic_initiator *init2;

  // for random generation
  std::random_device rd;
  std::mt19937 gen;
  std::uniform_int_distribution<> addr_dist;
  std::uniform_int_distribution<> data_dist;

  SC_HAS_PROCESS(test_driver);

  test_driver(sc_module_name name, generic_initiator * i1,
              generic_initiator * i2)
      : sc_module(name), init1(i1), init2(i2), gen(rd()), addr_dist(0, 0x400),
        data_dist(0, 0xFFF) {
    SC_THREAD(test_thread_1);
    sensitive << clock.pos();
    SC_THREAD(test_thread_2);
    sensitive << clock.pos();
  }

  void test_thread_1() {

    // Wait for a few cycles before starting
    wait(30, sc_core::SC_NS);

    // Generate transactions with 5ns delay
    for (int i = 0; i < 10; i++) {
      sc_uint<12> addr = 0x100 + addr_dist(gen);
      sc_uint<12> data = data_dist(gen);

      init1->write(addr, data);
      wait(5, SC_NS);

      sc_uint<12> read_data;
      init1->read(addr, read_data);
      wait(5, SC_NS);
    }
  }

  void test_thread_2() {

    // Wait for a few cycles before starting
    wait(40, sc_core::SC_NS);

    // Generate transactions with 7ns delay
    for (int i = 0; i < 8; i++) {
      sc_uint<12> addr = 0x600 + addr_dist(gen);
      sc_uint<12> data = data_dist(gen);

      init2->write(addr, data);
      wait(7, SC_NS);

      sc_uint<12> read_data;
      init2->read(addr, read_data);
      wait(7, SC_NS);
    }
  }
};

int sc_main(int argc, char *argv[]) {
  // Set log level to see detailed transaction and arbitration information
  Logger::setLogLevel(LogLevel::DEBUG);

  toplevel my_top("my_top");
  test_driver driver("test_driver", my_top.init1, my_top.init2);

  // Connect test driver to clock
  driver.clock(my_top.clock);

  // Run for longer to see multiple transactions and arbitration
  sc_start(200, SC_NS);

  return 0;
}
