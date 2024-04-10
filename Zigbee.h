#ifndef ZIGBEE_H
#define ZIGBEE_H

#include "mbed.h"

class Zigbee {
public:
    Zigbee(PinName tx, PinName rx);
    ~Zigbee();

    bool sendMessage(const char* message);
    bool receiveMessage(char* receivedMessage);

private:
    BufferedSerial serial;

    void sendFrame(const char* message, int length);
    char calculateChecksum(const char* message, int length);
    bool readFrame(char* receivedFrame, int maxLength);
};

#endif /* ZIGBEE_H */