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
  mem(const sc_core::sc_module_name &name);
  void target_write(const sc_uint<12> address, const sc_uint<12> data) override;
  void target_read(const sc_uint<12> address, sc_uint<12> data) override;

private:
  const unsigned int size;
  std::array<sc_uint<12>, MEM_SIZE> memory;
};

// definitions
template <unsigned int MEM_SIZE>
mem<MEM_SIZE>::mem(const sc_core::sc_module_name &name)
    : sc_module(name), size(MEM_SIZE) {
  std::stringstream ss;
  ss << "Initialized with size 0x" << MEM_SIZE;
  Logger::log(LogLevel::INFO, "Target", ss.str());
}

template <unsigned int MEM_SIZE>
void mem<MEM_SIZE>::target_write(const sc_uint<12> address,
                                 const sc_uint<12> data) {
  std::stringstream ss;
  ss << "Writing to local address 0x" << std::hex << address;
  Logger::logTransaction("Target", "Write", address, data);

  try {
    memory.at(address) = data;
    Logger::log(LogLevel::DEBUG, "Target", "Write completed successfully");
  } catch (const std::out_of_range &e) {
    Logger::logError("Target", "Memory access out of bounds");
    throw;
  }
}

template <unsigned int MEM_SIZE>
void mem<MEM_SIZE>::target_read(const sc_uint<12> address, sc_uint<12> data) {
  std::stringstream ss;
  ss << "Reading from local address 0x" << std::hex << address;
  Logger::log(LogLevel::DEBUG, "Target", ss.str());

  try {
    data = memory.at(address);
    Logger::logTransaction("Target", "Read Response", address, data);
  } catch (const std::out_of_range &e) {
    Logger::logError("Target", "Memory access out of bounds");
    throw;
  }
}

#endif
