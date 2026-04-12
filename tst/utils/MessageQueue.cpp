/*
 * Created by Ed Fillingham on 24/07/2025.
 */

#include "MessageQueue.h"
#include "Arduino.h"

/**
 * Constructor for the MessageQueue class.
 *
 * @param size The maximum number of messages the queue can hold.
 */
MessageQueue::MessageQueue(int size)
        : capacity(size > 0 ? size : 30),
          count(0),
          head(0),
          buffer(capacity),
          timeBuffer(capacity) {}


/**
 * Insert a message into the queue.
 *
 * @param message The message to be added to the queue.
 */
void MessageQueue::pushMessage(const std::string &message) {
  // Insert at head
  buffer[head] = message;
  timeBuffer[head] = millis();  // stamp with current time (ms since boot)

  // Move head forward in circular fashion
  head = (head + 1) % capacity;

  // Update count (cap at capacity)
  if (count < capacity) {
    count++;
  }
}

/**
 * Get all messages in the queue.
 *
 * @return A vector of strings containing the messages in the queue,
 * ordered from oldest to newest.
 */
std::vector<std::string> MessageQueue::getMessages() const {
  std::vector<std::string> out;
  out.reserve(count);
  for (int i = 0; i < count; ++i) {
    out.push_back(buffer[physicalIndex(i)]);
  }
  return out;
}

/**
 * Get timestamps for each message in the queue.
 *
 * @return A vector of integers representing how long ago each message was sent, in seconds,
 * ordered from oldest to newest.
 */
std::vector<int> MessageQueue::getTimestamps() const {
  const uint32_t now = millis();
  std::vector<int> out;
  out.reserve(count);
  for (int i = 0; i < count; ++i) {
    uint32_t msgTime = timeBuffer[physicalIndex(i)];
    // Unsigned subtraction handles millis wrap correctly (wrap after ~49 days)
    uint32_t diffMs = now - msgTime;
    out.push_back(static_cast<int>(diffMs / 1000)); // convert ms to seconds
  }
  return out;
}

/**
 * Clear the message queue.
 */
void MessageQueue::clearQueue() {
  count = 0;
  head = 0;

  for (int i = 0; i < capacity; ++i) {
    buffer[i].clear();
    timeBuffer[i] = 0;
  }
}

/**
 * Helper function to map a logical index (0..count-1) to the physical index in the buffer.
 *
 * @param logicalIndex The logical index of the message in the queue (0 is oldest, count-1 is newest).
 * @return The physical index in the buffer corresponding to the logical index.
 */
int MessageQueue::physicalIndex(int logicalIndex) const {
  // logicalIndex 0 is oldest, count-1 is newest
  // oldest element index = (head - count + capacity) % capacity
  int oldest = head - count;
  if (oldest < 0) oldest += capacity;
  return (oldest + logicalIndex) % capacity;
}

/**
 * Format message age as a string
 *
 * @param secs The age in seconds to format.
 * @return formatted string representing the age in seconds, minutes, hours, or days.
 */
static std::string fmtAge(int secs) {
  if (secs < 60)         return std::to_string(secs) + "s";
  if (secs < 3600)       return std::to_string(secs / 60) + "m";
  if (secs < 86400)      return std::to_string(secs / 3600) + "h";
  return std::to_string(secs / 86400) + "d";
}

/**
 * Format a message queue nicely for display.
 *
 * @param messages The messages to format.
 * @param timestamps The timestamps corresponding to each message, in seconds since the message was sent.
 * @param maxMessages The maximum number of messages to format.
 * @param reverse Whether to format from newest to oldest (true) or oldest to newest (false). (default: true)
 * @return The formatted messages as a vector of strings, each prefixed with its age.
 */
std::vector<std::string> MessageQueue::formatMessages(const std::vector<std::string>& messages,
                                                      const std::vector<int>& timestamps,
                                                      int maxMessages,
                                                      bool reverse)
{
  std::vector<std::string> out;

  const size_t count = std::min(messages.size(), timestamps.size());
  if (count == 0) return out;

  const int num = std::min<int>(count, maxMessages);
  out.reserve(num);

  if (reverse) {
    // newest -> oldest
    for (int i = static_cast<int>(count) - 1; i >= static_cast<int>(count) - num; --i) {
      out.push_back("[" + fmtAge(timestamps[i]) + "] " + messages[i]);
    }
  } else {
    // oldest (within the last num) -> newest
    const size_t start = count - num;
    for (size_t i = 0; i < num; ++i) {
      out.push_back("[" + fmtAge(timestamps[i]) + "] " + messages[i]);
    }
  }

  return out;
}
