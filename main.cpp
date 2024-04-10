#include "mbed.h"
#include <chrono>
#include <cstdint>
#include <cstring>
#include "Zigbee.h"

#define BUFFSIZE        64

BufferedSerial pc(USBTX,USBRX, 115200);
Zigbee xbee(PB_9,PB_8); // Zigbee module configured with 115200 baud rate - Change in Zigbee.cpp if required.

char buffer[BUFFSIZE]   = {0};
char msgBuff[BUFFSIZE]  = {0};
char rcvBuff[BUFFSIZE]  = {0};
int counter             = 0;
int len                 = 0;

Thread readThread;
Mutex serialMutex;

void reader()
{
    while(1)
    {
        if(xbee.receiveMessage(rcvBuff))
        {
            if(serialMutex.trylock_for(chrono::milliseconds(500)))
            {
                len = snprintf(msgBuff, BUFFSIZE, "\r\n%s", rcvBuff);
                pc.write(msgBuff, len);
                serialMutex.unlock();
            }
        }
        ThisThread::sleep_for(chrono::milliseconds(10));
    }
}

// main() runs in its own thread in the OS
int main()
{
    pc.set_format(
        /* bits */ 8,
        /* parity */ BufferedSerial::None,
        /* stop bit */ 1
    );
    readThread.start(reader);
    
    if(serialMutex.trylock_for(chrono::milliseconds(500)))
    {
        len = snprintf(buffer, BUFFSIZE, "\r\nHello, I have started :)\r\n");
        pc.write(buffer, len);
        serialMutex.unlock();
    }

    while (true) {
        len = snprintf(msgBuff, BUFFSIZE, "counter %d", counter);
        xbee.sendMessage(msgBuff);
        counter++;
        ThisThread::sleep_for(chrono::seconds(10));
    }
}