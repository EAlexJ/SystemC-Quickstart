#include "bus.h"
#include "logger.h"
#include <ios>
#include <sysc/kernel/sc_time.h>
#include <systemc>
#include <utility>

bus::bus(const sc_core::sc_module_name &name, int n_initiators)
    : sc_module(name), num_initiators(n_initiators),
      request("request_signals", n_initiators),
      proceed("proceed_events", n_initiators) {
  SC_HAS_PROCESS(bus);
  SC_THREAD(control_bus);
  Logger::log(LogLevel::INFO, "Bus",
              "Initialized with " + std::to_string(n_initiators) +
                  " initiators");
}

void bus::register_target(sc_uint<12> start, sc_uint<12> size) {
  std::stringstream ss;
  ss << "Registering target at 0x" << std::hex << start << " with size 0x"
     << size;
  Logger::log(LogLevel::INFO, "Bus", ss.str());
  address_map.push_back(std::make_pair(start, size));
}

inline void bus::arbitrate(int id) {
  control_bus_e.notify(SC_ZERO_TIME);
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
    sc_uint<12> eff_addr = address - address_map[port_idx].first;
    target_ports[port_idx]->target_write(eff_addr, data);
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
    sc_uint<12> eff_addr = address - address_map[port_idx].first;
    target_ports[port_idx]->target_read(eff_addr, data);
    Logger::logTransaction("Bus", "Read Response", address, data, id);
  } else {
    Logger::logError("Bus", "Failed to find target port for read operation");
    SC_REPORT_ERROR("MyBus", "Failed to find target port for read operation");
  }
}

void bus::control_bus() {
  int highest;
  while (1) {
    wait(control_bus_e);
    highest = -1;
    for (int i = 0; i < num_initiators; i++) {
      if (request[i]) {
        if (highest != -1)
          // re-raise event in case more than one has requested
          control_bus_e.notify(SC_ZERO_TIME);
        highest = i;
      }
    }
    if (highest > -1) {
      proceed[highest].notify();
    }
  }
}

int bus::find_port(sc_uint<12> address) {
  int idx = 0;
  for (auto it = address_map.begin(); it != address_map.end(); ++it, ++idx) {
    sc_uint<12> start = it->first;
    sc_uint<12> size = it->second;
    if (address >= start && address < (start + size)) {
      return idx;
    }
  }

  Logger::logError("Bus", "Address 0x" + std::to_string(address.to_uint()) +
                              " out of range");
  SC_REPORT_ERROR("MyBus", "Address out of range");
  return -1;
}

void bus::end_of_elaboration() {
  // Initialize signals
  for (int i = 0; i < num_initiators; i++) {
    request[i].write(false);
  }

  // Check for overlapping memory ranges
  for (size_t i = 0; i < address_map.size(); i++) {
    for (size_t j = i + 1; j < address_map.size(); j++) {
      uint64_t start1 = address_map[i].first;
      uint64_t end1 = start1 + address_map[i].second - 1;
      uint64_t start2 = address_map[j].first;
      uint64_t end2 = start2 + address_map[j].second - 1;

      // Check for overlap
      if (start1 <= end2 && start2 <= end1) {
        std::stringstream ss;
        ss << "Memory range overlap detected between targets " << i << " and "
           << j << ":\n"
           << "Target " << i << ": 0x" << std::hex << start1 << " - 0x" << end1
           << "\n"
           << "Target " << j << ": 0x" << std::hex << start2 << " - 0x" << end2;
        Logger::logError("Bus", ss.str());
        SC_REPORT_ERROR("Bus", "Overlapping memory ranges detected");
      }
    }
  }
}
