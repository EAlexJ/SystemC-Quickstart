#include "initiator.h"
#include "logger.h"
#include <iostream>
#include <sstream>

generic_initiator::generic_initiator(const sc_module_name &name, int id)
    : sc_module(name), initiator_socket("initiator_socket"), m_id(id),
      timing_annotation(sc_time(10, SC_NS)) {
  std::stringstream ss;
  ss << "Initialized with ID " << m_id;
  Logger::log(LogLevel::INFO, "Initiator" + std::to_string(m_id), ss.str());
}

void generic_initiator::write(const sc_uint<12> address,
                              const sc_uint<12> data) {
  tlm::tlm_generic_payload trans;
  sc_uint<12> data_buf = data;

  trans.set_command(tlm::TLM_WRITE_COMMAND);
  trans.set_address(address.to_uint());
  trans.set_data_ptr(reinterpret_cast<unsigned char *>(&data_buf));
  trans.set_data_length(sizeof(sc_uint<12>));
  trans.set_streaming_width(sizeof(sc_uint<12>));
  trans.set_byte_enable_ptr(0);
  trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  Logger::logTransaction("Initiator" + std::to_string(m_id), "Write Request",
                         address, data, m_id);

  initiator_socket->b_transport(trans, timing_annotation);

  if (trans.is_response_error()) {
    Logger::logError("Initiator" + std::to_string(m_id),
                     "Write transaction failed");
    SC_REPORT_ERROR("TLM-2", "Write transaction failed");
  }
}

void generic_initiator::read(const sc_uint<12> address, sc_uint<12> &data) {
  tlm::tlm_generic_payload trans;
  sc_uint<12> data_buf;

  trans.set_command(tlm::TLM_READ_COMMAND);
  trans.set_address(address.to_uint());
  trans.set_data_ptr(reinterpret_cast<unsigned char *>(&data_buf));
  trans.set_data_length(sizeof(sc_uint<12>));
  trans.set_streaming_width(sizeof(sc_uint<12>));
  trans.set_byte_enable_ptr(0);
  trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  Logger::logTransaction("Initiator" + std::to_string(m_id), "Read Request",
                         address, data, m_id);

  initiator_socket->b_transport(trans, timing_annotation);

  if (trans.is_response_error()) {
    Logger::logError("Initiator" + std::to_string(m_id),
                     "Read transaction failed");
    SC_REPORT_ERROR("TLM-2", "Read transaction failed");
  }

  data = data_buf;
  Logger::logTransaction("Initiator" + std::to_string(m_id), "Read Response",
                         address, data, m_id);
}
