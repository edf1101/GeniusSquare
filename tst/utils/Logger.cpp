/*
 * Created by Ed Fillingham on 24/07/2025.
*/

#include "Logger.h"

/**
 * Default constructor for the Logger class.
 * This constructor initializes the logger without a serial port.
 */
Logger::Logger() {}

/**
 * Constructor for the Logger class that initializes the logger with a serial port.
 *
 * @param baudRate The baud rate for the serial communication.
 */
Logger::Logger(int baudRate) {
  Serial.begin(baudRate);
  usingSerial = true;


}

/**
 * Log a c++ string message
 *
 * @param message The message to log
 */
void Logger::log(const std::string &message) {
  if (usingSerial) {
    Serial.println(message.c_str());
  }
  messageQueue.pushMessage(message); // add message to the message queue
}

/**
 * Log a C-style string message
 *
 * @param message The message to log
 */
void Logger::log(const char *message) {

  if (usingSerial) {
    Serial.println(message);
  }
  // add message to the message queue
  messageQueue.pushMessage(std::string(message));
}


/**
 * Log an Arduino String message
 *
 * @param message The message to log
 */
void Logger::log(const String &message) {

  if (usingSerial) {
    Serial.println(message);
  }
  // add message to the message queue
  messageQueue.pushMessage(std::string(message.c_str()));
}
