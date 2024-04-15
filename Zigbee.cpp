#include "Zigbee.h"

Zigbee::Zigbee(PinName tx, PinName rx) : serial(tx, rx) {
    serial.set_baud(115200); // Set baud rate as per your Zigbee module configuration
}

Zigbee::~Zigbee() {
    // Destructor
}

bool Zigbee::sendMessage(const char* message) {
    int length = strlen(message);
    if (length > 0xFF) {
        return false; // Message too long
    }

    sendFrame(message, length);
    return true;
}

void Zigbee::sendFrame(const char* message, int length) {
    char ChksmPreamble[]    = {0x10, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x00};
    char txBuffer[256]; // Adjust the buffer size as per your requirement
    txBuffer[0] = 0x7E; // Start delimiter

    uint16_t frameLength    = length + 14; // Data length + frame type
    txBuffer[1]             = (frameLength >> 8) & 0xFF; // MSB of frame length
    txBuffer[2]             = frameLength & 0xFF; // LSB of frame length

    txBuffer[3]             = 0x10; // Frame type (Transmit Request)

    txBuffer[4]             = 0x01; // Frame ID (set to 0 for no response)

    txBuffer[5]             = 0xFF; // 64-bit Destination Address
    txBuffer[6]             = 0xFF;
    txBuffer[7]             = 0xFF;
    txBuffer[8]             = 0xFF;
    txBuffer[9]             = 0xFF;
    txBuffer[10]            = 0xFF;
    txBuffer[11]            = 0xFF;
    txBuffer[12]            = 0xFF;

    txBuffer[13]            = 0xFF; // 16-bit Destination Address
    txBuffer[14]            = 0xFE;

    txBuffer[15]            = 0x00; // Broadcast Radius

    txBuffer[16]            = 0x00; // Options

    memcpy(&txBuffer[17], message, length); // Copy message to buffer

    int finalLen            = 17+length+1;
    char chksum             = calculateChecksum(txBuffer, finalLen);

    txBuffer[finalLen-1]    = chksum;

    serial.write(txBuffer, finalLen); // Send the frame
}
bool Zigbee::receiveMessage(char* receivedMessage) {
    enum ReceiveState {
         WaitingForStart,
         ReadingFrame,
         ReadingPayload
    };

    static ReceiveState state   = WaitingForStart;
    static int idx              = 0;
    static int frameLength      = 0;
    static char rxBuffer[256];                                              // Adjust the buffer size as per your requirement

    while (serial.readable()) {
        char byte;
        serial.read(&byte, 1);

        switch (state) {
            case WaitingForStart:
                if (byte == 0x7E) {
                    idx = 0;
                    rxBuffer[idx++] = byte;
                    state = ReadingFrame;
                }
                break;

            case ReadingFrame:
                rxBuffer[idx++] = byte;
                if (idx == 3) {
                    frameLength = (rxBuffer[1] << 8) | rxBuffer[2];
                    state = ReadingPayload;
                }
                break;

            case ReadingPayload:
                rxBuffer[idx++] = byte;
                if (idx == frameLength + 4) {
                    if(rxBuffer[3] == 0x90)
                    {
                        int messageLength = frameLength - 12;
                        for (int i = 0; i < messageLength; ++i) {
                            receivedMessage[i] = rxBuffer[15 + i];
                        }
                        receivedMessage[messageLength] = '\0';              // Null-terminate the received message
                        state = WaitingForStart;
                        return true;
                    } else {
                        state = WaitingForStart;
                    }
                }
                break;
        }
    }
    return false;
}

char Zigbee::calculateChecksum(const char* message, int length) {
    int temp = 0;
    for(int i=3;i<length-1;i++)
    {
        temp += message[i];
    }
    return (0xFF - temp) & 0xFF;
}