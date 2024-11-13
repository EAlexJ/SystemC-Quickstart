#include "bus.h"
#include "logger.h"
#include <sstream>

bus::bus(const sc_module_name &name, int n_initiators)
    : sc_module(name), target_socket("target_socket"),
      num_initiators(n_initiators), request("request", n_initiators),
      proceed("proceed", n_initiators) {

  // Register callback for multi-passthrough socket
  target_socket.register_b_transport(this, &bus::b_transport);

  // Initialize signals
  for (int i = 0; i < n_initiators; i++) {
    request[i].write(false);
  }

  SC_THREAD(control_bus);
}

void bus::b_transport(int id, tlm::tlm_generic_payload &trans, sc_time &delay) {
  sc_dt::uint64 addr = trans.get_address();
  unsigned char *data_ptr = trans.get_data_ptr();
  unsigned int len = trans.get_data_length();

  // Request bus access using the initiator ID from the socket
  request[id].write(true);
  // Notify control bus of new request
  control_bus_e.notify();
  wait(proceed[id]);

  // Find target port based on address
  int port = find_port(addr);
  if (port < 0) {
    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    request[id].write(false);
    return;
  }

  // Adjust address for target
  sc_dt::uint64 local_addr = addr - address_map[port].first;
  trans.set_address(local_addr);

  // Forward transaction to appropriate target
  initiator_socket[port]->b_transport(trans, delay);

  // Restore original address
  trans.set_address(addr);

  // Release bus and notify control bus to check for other pending requests
  request[id].write(false);
  control_bus_e.notify();
}

void bus::control_bus() {
  while (true) {
    bool found = false;

    // Check all initiators for pending requests
    for (int i = 0; i < num_initiators && !found; i++) {
      if (request[i].read()) {
        arbitrate(i);
        found = true;
      }
    }

    // If no requests found, wait for notification
    if (!found) {
      wait(control_bus_e);
    }
    // Add a small delay to allow other initiators to make requests
    else {
      wait(SC_ZERO_TIME);
    }
  }
}

void bus::arbitrate(int id) {
  // Grant bus access
  proceed[id].notify();

  // Wait for request to complete
  do {
    wait(SC_ZERO_TIME);
  } while (request[id].read());
}

int bus::find_port(const uint64_t addr) {
  for (size_t i = 0; i < address_map.size(); i++) {
    if (addr >= address_map[i].first &&
        addr < (address_map[i].first + address_map[i].second)) {
      return i;
    }
  }
  return -1;
}

void bus::register_target(sc_uint<12> start, sc_uint<12> size) {
  address_map.push_back(std::make_pair(start, size));
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
