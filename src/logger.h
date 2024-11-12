#ifndef LOGGER_H
#define LOGGER_H

#include <iomanip>
#include <iostream>
#include <sstream>
#include <systemc>

enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

class Logger {
private:
  static LogLevel currentLevel;

public:
  static void setLogLevel(LogLevel level) { currentLevel = level; }

  static LogLevel getLogLevel() { return currentLevel; }

  static void log(LogLevel level, const std::string &module,
                  const std::string &message) {
    if (level < currentLevel)
      return;

    std::stringstream ss;

    // Add timestamp
    ss << "[" << std::fixed << std::setprecision(2)
       << sc_core::sc_time_stamp().to_seconds() * 1e9 << "ns] ";

    // Add log level
    switch (level) {
    case LogLevel::DEBUG:
      ss << "[DEBUG]   ";
      break;
    case LogLevel::INFO:
      ss << "[INFO]    ";
      break;
    case LogLevel::WARNING:
      ss << "[WARNING] ";
      break;
    case LogLevel::ERROR:
      ss << "[ERROR]   ";
      break;
    }

    // Add module name and message
    ss << "[" << std::setw(10) << std::left << module << "] " << message;

    std::cout << ss.str() << std::endl;
  }

  static void logTransaction(const std::string &module,
                             const std::string &operation,
                             const sc_dt::sc_uint<12> &address,
                             const sc_dt::sc_uint<12> &data, int id = -1) {
    if (LogLevel::INFO < currentLevel)
      return;

    std::stringstream ss;
    ss << operation << " - "
       << "Address: 0x" << std::hex << address << " Data: 0x" << data;
    if (id >= 0) {
      ss << " ID: " << std::dec << id;
    }
    log(LogLevel::INFO, module, ss.str());
  }

  static void logBusArbitration(int requesterId, bool granted) {
    if (LogLevel::DEBUG < currentLevel)
      return;

    std::stringstream ss;
    ss << "Arbitration " << (granted ? "granted" : "requested")
       << " for initiator " << requesterId;
    log(LogLevel::DEBUG, "Bus", ss.str());
  }

  static void logError(const std::string &module, const std::string &message) {
    log(LogLevel::ERROR, module, message);
  }
};

#endif
