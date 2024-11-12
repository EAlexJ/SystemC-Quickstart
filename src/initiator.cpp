#include "initiator.h"
#include "logger.h"
#include <iostream>
#include <sstream>

generic_initiator::generic_initiator(const sc_module_name &name, int id)
    : sc_module(name), m_id(id) {
  std::stringstream ss;
  ss << "Initialized with ID " << m_id;
  Logger::log(LogLevel::INFO, "Initiator" + std::to_string(m_id), ss.str());
}

void generic_initiator::write(const sc_uint<12> address,
                              const sc_uint<12> data) {
  Logger::logTransaction("Initiator" + std::to_string(m_id), "Write Request",
                         address, data, m_id);
  initiator_port->write(address, data, m_id);
}

void generic_initiator::read(const sc_uint<12> address, sc_uint<12> &data) {
  Logger::logTransaction("Initiator" + std::to_string(m_id), "Read Request",
                         address, data, m_id);
  initiator_port->read(address, data, m_id);
  Logger::logTransaction("Initiator" + std::to_string(m_id), "Read Response",
                         address, data, m_id);
}
