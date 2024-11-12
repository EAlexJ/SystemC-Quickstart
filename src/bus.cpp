#include "bus.h"
#include "logger.h"
#include <ios>
#include <systemc>

bus::bus(const sc_core::sc_module_name &name, int n_initiators)
    : sc_module(name), num_initiators(n_initiators),
      request("request_signals", n_initiators),
      proceed("proceed_events", n_initiators) {
  SC_HAS_PROCESS(bus);
  SC_THREAD(control_bus);
  sensitive << clock.neg();
  Logger::log(LogLevel::INFO, "Bus",
              "Initialized with " + std::to_string(n_initiators) +
                  " initiators");
}

void bus::register_target(sc_uint<12> start, sc_uint<12> size) {
  std::stringstream ss;
  ss << "Registering target at 0x" << std::hex << start << " with size 0x"
     << size;
  Logger::log(LogLevel::INFO, "Bus", ss.str());
  address_map[start] = size;
}

inline void bus::arbitrate(int id) {
  Logger::logBusArbitration(id, false);
  request[id] = true;
  wait(proceed[id]);
  request[id] = false;
  Logger::logBusArbitration(id, true);
}

void bus::write(sc_uint<12> address, sc_uint<12> data, int id) {
  arbitrate(id);
  int port_idx = find_port(address);
  if (port_idx >= 0) {
    Logger::logTransaction("Bus", "Write Forward", address, data, id);
    target_ports[port_idx]->target_write(address, data);
  } else {
    Logger::logError("Bus", "Failed to find target port for write operation");
    SC_REPORT_ERROR("MyBus", "Failed to find target port for write operation");
  }
}

void bus::read(sc_uint<12> address, sc_uint<12> &data, int id) {
  arbitrate(id);
  int port_idx = find_port(address);
  if (port_idx >= 0) {
    Logger::logTransaction("Bus", "Read Forward", address, data, id);
    target_ports[port_idx]->target_read(address, data);
    Logger::logTransaction("Bus", "Read Response", address, data, id);
  } else {
    Logger::logError("Bus", "Failed to find target port for read operation");
    SC_REPORT_ERROR("MyBus", "Failed to find target port for read operation");
  }
}

void bus::control_bus() {
  int highest;
  for (;;) {
    wait();
    highest = -1;
    for (int i = 0; i < num_initiators; i++) {
      if (request[i])
        highest = i;
    }
    if (highest > -1) {
      Logger::log(LogLevel::DEBUG, "Bus",
                  "Granting access to initiator " + std::to_string(highest));
      proceed[highest].notify();
    }
  }
}

int bus::find_port(sc_uint<12> address) {
  std::stringstream ss;
  ss << "Searching port for address 0x" << std::hex << address;
  // Logger::log(LogLevel::DEBUG, "Bus", ss.str());

  int idx = 0;
  for (auto it = address_map.begin(); it != address_map.end(); ++it, ++idx) {
    sc_uint<12> start = it->first;
    sc_uint<12> size = it->second;

    std::stringstream entry_ss;
    entry_ss << "Checking range 0x" << std::hex << start << " - 0x"
             << (start + size);
    // Logger::log(LogLevel::DEBUG, "Bus", entry_ss.str());
    //
    if (address >= start && address < (start + size)) {
      // Logger::log(LogLevel::DEBUG, "Bus",
      //             "Found matching port: " + std::to_string(idx));
      return idx;
    }
  }

  Logger::logError("Bus", "Address 0x" + std::to_string(address.to_uint()) +
                              " out of range");
  SC_REPORT_ERROR("MyBus", "Address out of range");
  return -1;
}

void bus::end_of_elaboration() {
  Logger::log(LogLevel::INFO, "Bus", "End of elaboration reached");
  return;
}
