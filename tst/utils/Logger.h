/*
 * Created by Ed Fillingham on 24/07/2025.
 *
 * This class is a logger for the gun. It sends all logged messages to a serial port
 * and to an internal message queue.
*/

#ifndef LOGGER_LOGGER_H
#define LOGGER_LOGGER_H

#include "Arduino.h"
#include "string"
#include "MessageQueue.h"

class Logger {
public:
    Logger(); // default constructor for no serial port
    explicit Logger(int baudRate); // constructor for serial port

    void log(const std::string& message);

    void log(const char *message);

    void log(const String &message);

    MessageQueue& getMessageQueue() {
        return messageQueue; // Accessor for the internal message queue
    }

private:
    bool usingSerial = false;
    MessageQueue messageQueue = MessageQueue(40); // Internal message queue for storing log messages
};


#endif //LOGGER_LOGGER_H
