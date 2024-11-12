#ifndef TARGET_H
#define TARGET_H
#include "logger.h"
#include <array>
#include <systemc>

using namespace sc_core;
using namespace sc_dt;
class target_if : virtual public sc_interface {
public:
  virtual void target_write(const sc_uint<12> address,
                            const sc_uint<12> data) = 0;
  virtual void target_read(const sc_uint<12> address, sc_uint<12> data) = 0;
};

template <unsigned int MEM_SIZE = 64>
class mem : public sc_module, public target_if {
public:
  mem(const sc_core::sc_module_name &name, unsigned int start);
  void target_write(const sc_uint<12> address, const sc_uint<12> data) override;
  void target_read(const sc_uint<12> address, sc_uint<12> data) override;

private:
  const unsigned int start, size;
  std::array<sc_uint<12>, MEM_SIZE> memory;
};

// definitions
template <unsigned int MEM_SIZE>
mem<MEM_SIZE>::mem(const sc_core::sc_module_name &name, unsigned int start)
    : sc_module(name), start(start), size(MEM_SIZE) {
  std::stringstream ss;
  ss << "Initialized with start address 0x" << std::hex << start
     << " and size 0x" << MEM_SIZE;
  Logger::log(LogLevel::INFO, "Target", ss.str());
}

template <unsigned int MEM_SIZE>
void mem<MEM_SIZE>::target_write(const sc_uint<12> address,
                                 const sc_uint<12> data) {
  unsigned int effective_addr = address - start;
  std::stringstream ss;
  ss << "Writing to effective address 0x" << std::hex << effective_addr;
  Logger::logTransaction("Target", "Write", address, data);

  try {
    memory.at(effective_addr) = data;
    Logger::log(LogLevel::DEBUG, "Target", "Write completed successfully");
  } catch (const std::out_of_range &e) {
    Logger::logError("Target", "Memory access out of bounds");
    throw;
  }
}

template <unsigned int MEM_SIZE>
void mem<MEM_SIZE>::target_read(const sc_uint<12> address, sc_uint<12> data) {
  unsigned int effective_addr = address - start;
  std::stringstream ss;
  ss << "Reading from effective address 0x" << std::hex << effective_addr;
  Logger::log(LogLevel::DEBUG, "Target", ss.str());

  try {
    data = memory.at(effective_addr);
    Logger::logTransaction("Target", "Read Response", address, data);
  } catch (const std::out_of_range &e) {
    Logger::logError("Target", "Memory access out of bounds");
    throw;
  }
}

#endif
