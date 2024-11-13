#ifndef TARGET_H
#define TARGET_H

#include "logger.h"
#include <array>
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>

using namespace sc_core;
using namespace sc_dt;

template <unsigned int MEM_SIZE = 64> class mem : public sc_module {
public:
  tlm_utils::simple_target_socket<mem> target_socket;

  SC_HAS_PROCESS(mem);
  mem(const sc_module_name &name);

protected:
  virtual void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay);

private:
  const unsigned int size;
  std::array<sc_uint<12>, MEM_SIZE> memory;
};

// Definitions
template <unsigned int MEM_SIZE>
mem<MEM_SIZE>::mem(const sc_module_name &name)
    : sc_module(name), target_socket("target_socket"), size(MEM_SIZE) {

  target_socket.register_b_transport(this, &mem::b_transport);

  std::stringstream ss;
  ss << "Initialized with size 0x" << MEM_SIZE;
  Logger::log(LogLevel::INFO, "Target", ss.str());
}

template <unsigned int MEM_SIZE>
void mem<MEM_SIZE>::b_transport(tlm::tlm_generic_payload &trans,
                                sc_time &delay) {
  tlm::tlm_command cmd = trans.get_command();
  sc_dt::uint64 addr = trans.get_address();
  unsigned char *ptr = trans.get_data_ptr();
  unsigned int len = trans.get_data_length();

  // Check address bounds
  if (addr >= size) {
    Logger::logError("Target", "Memory access out of bounds");
    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    return;
  }

  if (cmd == tlm::TLM_WRITE_COMMAND) {
    std::stringstream ss;
    ss << "Writing to local address 0x" << std::hex << addr;
    Logger::log(LogLevel::DEBUG, "Target", ss.str());

    sc_uint<12> data;
    memcpy(&data, ptr, sizeof(sc_uint<12>));
    memory[addr] = data;

    Logger::logTransaction("Target", "Write", addr, data);
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
  } else if (cmd == tlm::TLM_READ_COMMAND) {
    std::stringstream ss;
    ss << "Reading from local address 0x" << std::hex << addr;
    Logger::log(LogLevel::DEBUG, "Target", ss.str());

    sc_uint<12> data = memory[addr];
    memcpy(ptr, &data, sizeof(sc_uint<12>));

    Logger::logTransaction("Target", "Read Response", addr, data);
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
  } else {
    Logger::logError("Target", "Unknown command");
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
  }

  // Add memory access delay
  delay += sc_time(10, SC_NS);
}

#endif
