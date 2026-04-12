/*
 * Created by Ed Fillingham on 24/07/2025.
 *
 * The message queue class is used to store messages in a fixed size queue circular queue.
 * All messages are stamped with a timestamp
*/

#ifndef MESSAGEQUEUE_MESSAGEQUEUE_H
#define MESSAGEQUEUE_MESSAGEQUEUE_H


#include <string>
#include <vector>

using namespace std;

class MessageQueue {
public:
    MessageQueue(int size = 30);

    void pushMessage(const string &message);

    vector<string> getMessages() const; // returns the messages in the queue, (oldest to newest)
    vector<int> getTimestamps() const; // returns how long ago in seconds each message was sent (oldest to newest)

    void clearQueue(); // clears the queue

    // static method to format the message queue for display
    static std::vector<std::string> formatMessages(const std::vector<std::string>& messages,
                                                   const std::vector<int>& timestamps,
                                                   int maxMessages = 10,
                                                   bool reverse = true);

    std::vector<std::string> getFormattedMessages(int maxMessages = 10,
                                                  bool reverse = true) const {
      return formatMessages(getMessages(), getTimestamps(), maxMessages, reverse);
    }
private:
    int capacity;                       // max number of elements
    int count;                          // current number of elements
    int head;                           // index of the next insertion point
    std::vector<std::string> buffer;    // circular buffer of messages
    std::vector<uint32_t> timeBuffer;   // corresponding timestamps (millis at push)

    int physicalIndex(int logicalIndex) const; // helper: map 0..count-1 (oldest->newest) to buffer index
};


#endif //MESSAGEQUEUE_MESSAGEQUEUE_H
